/**
  ******************************************************************************
    @file
    @author  PavelB
    @version V1.0
    @date    03-April-2017
    @brief
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "lcd_log.h"

/* FatFs includes component */
#include "ff_gen_drv.h"
#include "sd_diskio.h"

/* Jpeg includes component */
#include "jpeglib.h"

/* Private define ------------------------------------------------------------*/
#define IMAGE_HEIGHT 240
#define IMAGE_WIDTH  320
#define IMAGE_QUALITY 90
#define LCD_FRAME_BUFFER 0xC0000000

#define USE_DMA2D
#define SWAP_RB

/* Private typedef -----------------------------------------------------------*/
typedef struct RGB {
    uint8_t B;
    uint8_t G;
    uint8_t R;
} RGB_typedef;

/* This struct contains the JPEG decompression parameters */
static struct jpeg_decompress_struct cinfoD;
/* This struct contains the JPEG compression parameters */
static struct jpeg_compress_struct cinfoC;
/* This struct represents a JPEG error handler */
static struct jpeg_error_mgr jerr;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
FATFS SDFatFs;  /* File system object for SD card logical drive */
FIL MyFile;     /* File object */
char SDPath[4]; /* SD card logical drive path */
RGB_typedef *RGB_matrix;
uint8_t   _aucLine[2048];
uint32_t  offset = 0;
uint32_t line_counter = 239;

/* Private function prototypes -----------------------------------------------*/
static void jpeg_decode(JFILE *file, uint32_t width, uint8_t *buff, uint8_t (*callback)(uint8_t *, uint32_t));
static void jpeg_encode(JFILE *file, JFILE *file1, uint32_t width, uint32_t height, uint32_t image_quality, uint8_t *buff);
static uint8_t Jpeg_CallbackFunction(uint8_t *Row, uint32_t DataLength);

/**
    @brief
    @param
    @retval None
*/
static void task(void const *pvParameters) {
    LCD_UsrLog("vStartJPEGTask()\n");

    /*##-2- Link the micro SD disk I/O driver ##################################*/
    if (FATFS_LinkDriver(&SD_Driver, SDPath) == 0) {
        /*##-3- Register the file system object to the FatFs module ##############*/
        if (f_mount(&SDFatFs, (TCHAR const *)SDPath, 0) == FR_OK) {
            /*##-4- Open the JPG image with read access ############################*/
            if (f_open(&MyFile, "image.jpg", FA_READ) == FR_OK) {
            }
        }
    }

    /*##-5- Decode the jpg image file ##########################################*/
    jpeg_decode(&MyFile, IMAGE_WIDTH, _aucLine, Jpeg_CallbackFunction);
    /*##-4- Close the JPG image ################################################*/
    f_close(&MyFile);

    /* Infinite loop */
    for (;;) {
        osDelay(10);
    }
}

/*----------------------------------------------------------------------------*/
void vStartJPEGTask() {
    /* Create that task */
    osThreadDef(JPEG,
                task,
                osPriorityNormal,
                0,
                0x100);
    osThreadCreate(osThread(JPEG), NULL);
}

/*
    The modules that use fread() and fwrite() always invoke them through
    these macros.  On some systems you may need to twiddle the argument casts.
    CAUTION: argument order is different from underlying functions!
*/
size_t read_file(JFILE  *file, uint8_t *buf, uint32_t sizeofbuf) {
    static size_t BytesReadfile ;
    f_read(file, buf , sizeofbuf, &BytesReadfile);
    return BytesReadfile;
}

size_t write_file(JFILE  *file, uint8_t *buf, uint32_t sizeofbuf) {
    static size_t BytesWritefile ;
    f_write(file, buf , sizeofbuf, &BytesWritefile);
    return BytesWritefile;
}

/* Private functions ---------------------------------------------------------*/
/**
    @brief  Jpeg Decode
    @param  callback: line decoding callback
    @param  file:     pointer to the jpg file
    @param  width:    image width
    @param  buff:     pointer to the image line
    @retval None
*/
void jpeg_decode(JFILE *file, uint32_t width, uint8_t *buff, uint8_t (*callback)(uint8_t *, uint32_t)) {
    /* Decode JPEG Image */
    JSAMPROW buffer[2] = {0}; /* Output row buffer */
    uint32_t row_stride = 0; /* physical row width in image buffer */
    buffer[0] = buff;
    /* Step 1: allocate and initialize JPEG decompression object */
    cinfoD.err = jpeg_std_error(&jerr);
    /* Initialize the JPEG decompression object */
    jpeg_create_decompress(&cinfoD);
    jpeg_stdio_src(&cinfoD, file);
    /* Step 3: read image parameters with jpeg_read_header() */
    jpeg_read_header(&cinfoD, TRUE);
    /* TBC */
    /* Step 4: set parameters for decompression */
    cinfoD.dct_method = JDCT_FLOAT;
    /* Step 5: start decompressor */
    jpeg_start_decompress(&cinfoD);
    row_stride = width * 3;

    while (cinfoD.output_scanline < cinfoD.output_height) {
        (void) jpeg_read_scanlines(&cinfoD, buffer, 1);

        /* TBC */
        if (callback(buffer[0], row_stride) != 0) {
            break;
        }
    }

    /* Step 6: Finish decompression */
    jpeg_finish_decompress(&cinfoD);
    /* Step 7: Release JPEG decompression object */
    jpeg_destroy_decompress(&cinfoD);
}

/**
    @brief  Jpeg Encode
    @param  file:          pointer to the bmp file
    @param  file1:         pointer to the jpg file
    @param  width:         image width
    @param  height:        image height
    @param  image_quality: image quality
    @param  buff:          pointer to the image line
    @retval None
*/
void jpeg_encode(JFILE *file, JFILE *file1, uint32_t width, uint32_t height, uint32_t image_quality, uint8_t *buff) {
    /* Encode BMP Image to JPEG */
    JSAMPROW row_pointer;    /* Pointer to a single row */
    uint32_t bytesread;
    /* Step 1: allocate and initialize JPEG compression object */
    /* Set up the error handler */
    cinfoC.err = jpeg_std_error(&jerr);
    /* Initialize the JPEG compression object */
    jpeg_create_compress(&cinfoC);
    /* Step 2: specify data destination */
    jpeg_stdio_dest(&cinfoC, file1);
    /* Step 3: set parameters for compression */
    cinfoC.image_width = width;
    cinfoC.image_height = height;
    cinfoC.input_components = 3;
    cinfoC.in_color_space = JCS_RGB;
    /* Set default compression parameters */
    jpeg_set_defaults(&cinfoC);
    cinfoC.dct_method  = JDCT_FLOAT;
    jpeg_set_quality(&cinfoC, image_quality, TRUE);
    /* Step 4: start compressor */
    jpeg_start_compress(&cinfoC, TRUE);
    f_read(file, buff, 54, (UINT *)&bytesread);

    while (cinfoC.next_scanline < cinfoC.image_height) {
        /* In this application, the input file is a BMP, which first encodes the bottom of the picture */
        /* JPEG encodes the highest part of the picture first. We need to read the lines upside down   */
        /* Move the read pointer to 'last line of the picture - next_scanline'    */
        f_lseek(file, ((cinfoC.image_height - 1 - cinfoC.next_scanline)*width * 3) + 54);

        if (f_read(file, buff, width * 3, (UINT *)&bytesread) == FR_OK) {
            row_pointer = (JSAMPROW)buff;
            jpeg_write_scanlines(&cinfoC, &row_pointer, 1);
        }
    }

    /* Step 5: finish compression */
    jpeg_finish_compress(&cinfoC);
    /* Step 6: release JPEG compression object */
    jpeg_destroy_compress(&cinfoC);
}

/**
    @brief  Copy decompressed data to display buffer.
    @param  Row: Output row buffer
    @param  DataLength: Row width in output buffer
    @retval None
*/
static uint8_t Jpeg_CallbackFunction(uint8_t *Row, uint32_t DataLength) {
#ifdef USE_DMA2D
    static DMA2D_HandleTypeDef hdma2d_eval;
    offset = (LCD_FRAME_BUFFER + (IMAGE_WIDTH * (IMAGE_HEIGHT - line_counter - 1) * 4));
    /* Configure the DMA2D Mode, Color Mode and output offset */
    hdma2d_eval.Init.Mode         = DMA2D_M2M_PFC;
    hdma2d_eval.Init.ColorMode    = DMA2D_OUTPUT_ARGB8888;
    hdma2d_eval.Init.OutputOffset = 0;
    /* Foreground Configuration */
    hdma2d_eval.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d_eval.LayerCfg[1].InputAlpha = 0xFF;
    hdma2d_eval.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
    hdma2d_eval.LayerCfg[1].InputOffset = 0;
    hdma2d_eval.Instance = DMA2D;

    /* DMA2D Initialization */
    if (HAL_DMA2D_Init(&hdma2d_eval) == HAL_OK) {
        if (HAL_DMA2D_ConfigLayer(&hdma2d_eval, 1) == HAL_OK) {
            if (HAL_DMA2D_Start(&hdma2d_eval, (uint32_t)Row, (uint32_t)offset, IMAGE_WIDTH, 1) == HAL_OK) {
                /* Polling For DMA transfer */
                HAL_DMA2D_PollForTransfer(&hdma2d_eval, 10);
            }
        }
    }

#else /* DMA2D not used */
    RGB_matrix = (RGB_typedef *)Row;
    uint32_t  ARGB32Buffer[IMAGE_WIDTH];
    uint32_t counter = 0;

    for (counter = 0; counter < IMAGE_WIDTH; counter++) {
        ARGB32Buffer[counter]  = (uint32_t)
                                 (
                                     ((RGB_matrix[counter].B << 16) |
                                      (RGB_matrix[counter].G << 8) |
                                      (RGB_matrix[counter].R) | 0xFF000000)
                                 );
        *(__IO uint32_t *)(LCD_FRAME_BUFFER + (counter * 4) + (IMAGE_WIDTH * (IMAGE_HEIGHT - line_counter - 1) * 4)) = ARGB32Buffer[counter];
    }

#endif
#ifdef SWAP_RB
    uint32_t pixel = 0, width_counter, result = 0, result1 = 0;

    for (width_counter = 0; width_counter < IMAGE_WIDTH; width_counter++) {
        pixel = *(__IO uint32_t *)(LCD_FRAME_BUFFER + (width_counter * 4) + (IMAGE_WIDTH * (IMAGE_HEIGHT - line_counter - 1) * 4));
        result1 = (((pixel & 0x00FF0000) >> 16) | ((pixel & 0x000000FF) << 16));
        pixel = pixel & 0xFF00FF00;
        result = (result1 | pixel);
        *(__IO uint32_t *)(LCD_FRAME_BUFFER + (width_counter * 4) + (IMAGE_WIDTH * (IMAGE_HEIGHT - line_counter - 1) * 4)) = result;
    }

#endif
    line_counter--;
    return 0;
}