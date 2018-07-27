/*******************************************************************************
    Description: This file implements the board support package for the USB host library
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usbh_core.h"

HCD_HandleTypeDef hhcd_USB_OTG_HS;
void Error_Handler(void);

/*******************************************************************************
                       LL Driver Callbacks (HCD -> USB Host Library)
*******************************************************************************/
/* MSP Init */

void HAL_HCD_MspInit(HCD_HandleTypeDef *hcdHandle) {
    GPIO_InitTypeDef GPIO_InitStruct;

    if (hcdHandle->Instance == USB_OTG_HS) {
        /** USB_OTG_HS GPIO Configuration
            PB5     ------> USB_OTG_HS_ULPI_D7
            PH4     ------> USB_OTG_HS_ULPI_NXT
            PB13     ------> USB_OTG_HS_ULPI_D6
            PB12     ------> USB_OTG_HS_ULPI_D5
            PC0     ------> USB_OTG_HS_ULPI_STP
            PC2     ------> USB_OTG_HS_ULPI_DIR
            PA5     ------> USB_OTG_HS_ULPI_CK
            PB10     ------> USB_OTG_HS_ULPI_D3
            PA3     ------> USB_OTG_HS_ULPI_D0
            PB1     ------> USB_OTG_HS_ULPI_D2
            PB0     ------> USB_OTG_HS_ULPI_D1
            PB11     ------> USB_OTG_HS_ULPI_D4
        */
        GPIO_InitStruct.Pin = ULPI_D7_Pin | ULPI_D6_Pin | ULPI_D5_Pin | ULPI_D3_Pin
                              | ULPI_D2_Pin | ULPI_D1_Pin | ULPI_D4_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = ULPI_NXT_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
        HAL_GPIO_Init(ULPI_NXT_GPIO_Port, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = ULPI_STP_Pin | ULPI_DIR_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = ULPI_CLK_Pin | ULPI_D0_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        /* Peripheral clock enable */
        __HAL_RCC_USB_OTG_HS_CLK_ENABLE();
        __HAL_RCC_USB_OTG_HS_ULPI_CLK_ENABLE();
        /* Peripheral interrupt init */
        HAL_NVIC_SetPriority(OTG_HS_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(OTG_HS_IRQn);
    }
}

void HAL_HCD_MspDeInit(HCD_HandleTypeDef *hcdHandle) {
    if (hcdHandle->Instance == USB_OTG_HS) {
        /* Disable Peripheral clock */
        __HAL_RCC_USB_OTG_HS_CLK_DISABLE();
        __HAL_RCC_USB_OTG_HS_ULPI_CLK_DISABLE();
        /** USB_OTG_HS GPIO Configuration
            PB5     ------> USB_OTG_HS_ULPI_D7
            PH4     ------> USB_OTG_HS_ULPI_NXT
            PB13     ------> USB_OTG_HS_ULPI_D6
            PB12     ------> USB_OTG_HS_ULPI_D5
            PC0     ------> USB_OTG_HS_ULPI_STP
            PC2     ------> USB_OTG_HS_ULPI_DIR
            PA5     ------> USB_OTG_HS_ULPI_CK
            PB10     ------> USB_OTG_HS_ULPI_D3
            PA3     ------> USB_OTG_HS_ULPI_D0
            PB1     ------> USB_OTG_HS_ULPI_D2
            PB0     ------> USB_OTG_HS_ULPI_D1
            PB11     ------> USB_OTG_HS_ULPI_D4
        */
        HAL_GPIO_DeInit(GPIOB, ULPI_D7_Pin | ULPI_D6_Pin | ULPI_D5_Pin | ULPI_D3_Pin
                        | ULPI_D2_Pin | ULPI_D1_Pin | ULPI_D4_Pin);
        HAL_GPIO_DeInit(ULPI_NXT_GPIO_Port, ULPI_NXT_Pin);
        HAL_GPIO_DeInit(GPIOC, ULPI_STP_Pin | ULPI_DIR_Pin);
        HAL_GPIO_DeInit(GPIOA, ULPI_CLK_Pin | ULPI_D0_Pin);
        /* Peripheral interrupt Deinit*/
        HAL_NVIC_DisableIRQ(OTG_HS_IRQn);
    }
}

/**
    @brief  SOF callback.
    @param  hhcd: HCD handle
    @retval None
*/
void HAL_HCD_SOF_Callback(HCD_HandleTypeDef *hhcd) {
    USBH_LL_IncTimer(hhcd->pData);
}

/**
    @brief  SOF callback.
    @param  hhcd: HCD handle
    @retval None
*/
void HAL_HCD_Connect_Callback(HCD_HandleTypeDef *hhcd) {
    USBH_LL_Connect(hhcd->pData);
}

/**
    @brief  SOF callback.
    @param  hhcd: HCD handle
    @retval None
*/
void HAL_HCD_Disconnect_Callback(HCD_HandleTypeDef *hhcd) {
    USBH_LL_Disconnect(hhcd->pData);
}

/**
    @brief  Notify URB state change callback.
    @param  hhcd: HCD handle
    @retval None
*/
void HAL_HCD_HC_NotifyURBChange_Callback(HCD_HandleTypeDef *hhcd, uint8_t chnum, HCD_URBStateTypeDef urb_state) {
    /* To be used with OS to sync URB state with the global state machine */
#if (USBH_USE_OS == 1)
    USBH_LL_NotifyURBChange(hhcd->pData);
#endif
}
/*******************************************************************************
                       LL Driver Interface (USB Host Library --> HCD)
*******************************************************************************/
/**
    @brief  USBH_LL_Init
            Initialize the Low Level portion of the Host driver.
    @param  phost: Host handle
    @retval USBH Status
*/
USBH_StatusTypeDef  USBH_LL_Init(USBH_HandleTypeDef *phost) {
    /* Init USB_IP */
    if (phost->id == HOST_HS) {
        /* Link The driver to the stack */
        hhcd_USB_OTG_HS.pData = phost;
        phost->pData = &hhcd_USB_OTG_HS;
        hhcd_USB_OTG_HS.Instance = USB_OTG_HS;
        hhcd_USB_OTG_HS.Init.Host_channels = 12;
        hhcd_USB_OTG_HS.Init.speed = HCD_SPEED_HIGH;
        hhcd_USB_OTG_HS.Init.dma_enable = DISABLE;
        hhcd_USB_OTG_HS.Init.phy_itface = USB_OTG_ULPI_PHY;
        hhcd_USB_OTG_HS.Init.Sof_enable = DISABLE;
        hhcd_USB_OTG_HS.Init.low_power_enable = DISABLE;
        hhcd_USB_OTG_HS.Init.vbus_sensing_enable = DISABLE;
        hhcd_USB_OTG_HS.Init.use_external_vbus = ENABLE;

        if (HAL_HCD_Init(&hhcd_USB_OTG_HS) != HAL_OK) {
            Error_Handler();
        }

        USBH_LL_SetTimer(phost, HAL_HCD_GetCurrentFrame(&hhcd_USB_OTG_HS));
    }

    return USBH_OK;
}

/**
    @brief  USBH_LL_DeInit
            De-Initialize the Low Level portion of the Host driver.
    @param  phost: Host handle
    @retval USBH Status
*/
USBH_StatusTypeDef  USBH_LL_DeInit(USBH_HandleTypeDef *phost) {
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBH_StatusTypeDef usb_status = USBH_OK;
    hal_status = HAL_HCD_DeInit(phost->pData);

    switch (hal_status) {
        case HAL_OK :
            usb_status = USBH_OK;
            break;

        case HAL_ERROR :
            usb_status = USBH_FAIL;
            break;

        case HAL_BUSY :
            usb_status = USBH_BUSY;
            break;

        case HAL_TIMEOUT :
            usb_status = USBH_FAIL;
            break;

        default :
            usb_status = USBH_FAIL;
            break;
    }

    return usb_status;
}

/**
    @brief  USBH_LL_Start
            Start the Low Level portion of the Host driver.
    @param  phost: Host handle
    @retval USBH Status
*/
USBH_StatusTypeDef  USBH_LL_Start(USBH_HandleTypeDef *phost) {
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBH_StatusTypeDef usb_status = USBH_OK;
    hal_status = HAL_HCD_Start(phost->pData);

    switch (hal_status) {
        case HAL_OK :
            usb_status = USBH_OK;
            break;

        case HAL_ERROR :
            usb_status = USBH_FAIL;
            break;

        case HAL_BUSY :
            usb_status = USBH_BUSY;
            break;

        case HAL_TIMEOUT :
            usb_status = USBH_FAIL;
            break;

        default :
            usb_status = USBH_FAIL;
            break;
    }

    return usb_status;
}

/**
    @brief  USBH_LL_Stop
            Stop the Low Level portion of the Host driver.
    @param  phost: Host handle
    @retval USBH Status
*/
USBH_StatusTypeDef  USBH_LL_Stop(USBH_HandleTypeDef *phost) {
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBH_StatusTypeDef usb_status = USBH_OK;
    hal_status = HAL_HCD_Stop(phost->pData);

    switch (hal_status) {
        case HAL_OK :
            usb_status = USBH_OK;
            break;

        case HAL_ERROR :
            usb_status = USBH_FAIL;
            break;

        case HAL_BUSY :
            usb_status = USBH_BUSY;
            break;

        case HAL_TIMEOUT :
            usb_status = USBH_FAIL;
            break;

        default :
            usb_status = USBH_FAIL;
            break;
    }

    return usb_status;
}

/**
    @brief  USBH_LL_GetSpeed
            Return the USB Host Speed from the Low Level Driver.
    @param  phost: Host handle
    @retval USBH Speeds
*/
USBH_SpeedTypeDef USBH_LL_GetSpeed(USBH_HandleTypeDef *phost) {
    USBH_SpeedTypeDef speed = USBH_SPEED_FULL;

    switch (HAL_HCD_GetCurrentSpeed(phost->pData)) {
        case 0 :
            speed = USBH_SPEED_HIGH;
            break;

        case 1 :
            speed = USBH_SPEED_FULL;
            break;

        case 2 :
            speed = USBH_SPEED_LOW;
            break;

        default:
            speed = USBH_SPEED_FULL;
            break;
    }

    return  speed;
}

/**
    @brief  USBH_LL_ResetPort
            Reset the Host Port of the Low Level Driver.
    @param  phost: Host handle
    @retval USBH Status
*/
USBH_StatusTypeDef USBH_LL_ResetPort(USBH_HandleTypeDef *phost) {
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBH_StatusTypeDef usb_status = USBH_OK;
    hal_status = HAL_HCD_ResetPort(phost->pData);

    switch (hal_status) {
        case HAL_OK :
            usb_status = USBH_OK;
            break;

        case HAL_ERROR :
            usb_status = USBH_FAIL;
            break;

        case HAL_BUSY :
            usb_status = USBH_BUSY;
            break;

        case HAL_TIMEOUT :
            usb_status = USBH_FAIL;
            break;

        default :
            usb_status = USBH_FAIL;
            break;
    }

    return usb_status;
}

/**
    @brief  USBH_LL_GetLastXferSize
            Return the last transfered packet size.
    @param  phost: Host handle
    @param  pipe: Pipe index
    @retval Packet Size
*/
uint32_t USBH_LL_GetLastXferSize(USBH_HandleTypeDef *phost, uint8_t pipe) {
    return HAL_HCD_HC_GetXferCount(phost->pData, pipe);
}

/**
    @brief  USBH_LL_OpenPipe
            Open a pipe of the Low Level Driver.
    @param  phost: Host handle
    @param  pipe_num: Pipe index
    @param  epnum: Endpoint Number
    @param  dev_address: Device USB address
    @param  speed: Device Speed
    @param  ep_type: Endpoint Type
    @param  mps: Endpoint Max Packet Size
    @retval USBH Status
*/
USBH_StatusTypeDef   USBH_LL_OpenPipe(USBH_HandleTypeDef *phost,
                                      uint8_t pipe_num,
                                      uint8_t epnum,
                                      uint8_t dev_address,
                                      uint8_t speed,
                                      uint8_t ep_type,
                                      uint16_t mps) {
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBH_StatusTypeDef usb_status = USBH_OK;
    hal_status = HAL_HCD_HC_Init(phost->pData,
                                 pipe_num,
                                 epnum,
                                 dev_address,
                                 speed,
                                 ep_type,
                                 mps);

    switch (hal_status) {
        case HAL_OK :
            usb_status = USBH_OK;
            break;

        case HAL_ERROR :
            usb_status = USBH_FAIL;
            break;

        case HAL_BUSY :
            usb_status = USBH_BUSY;
            break;

        case HAL_TIMEOUT :
            usb_status = USBH_FAIL;
            break;

        default :
            usb_status = USBH_FAIL;
            break;
    }

    return usb_status;
}

/**
    @brief  USBH_LL_ClosePipe
            Close a pipe of the Low Level Driver.
    @param  phost: Host handle
    @param  pipe_num: Pipe index
    @retval USBH Status
*/
USBH_StatusTypeDef   USBH_LL_ClosePipe(USBH_HandleTypeDef *phost, uint8_t pipe) {
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBH_StatusTypeDef usb_status = USBH_OK;
    hal_status = HAL_HCD_HC_Halt(phost->pData, pipe);

    switch (hal_status) {
        case HAL_OK :
            usb_status = USBH_OK;
            break;

        case HAL_ERROR :
            usb_status = USBH_FAIL;
            break;

        case HAL_BUSY :
            usb_status = USBH_BUSY;
            break;

        case HAL_TIMEOUT :
            usb_status = USBH_FAIL;
            break;

        default :
            usb_status = USBH_FAIL;
            break;
    }

    return usb_status;
}

/**
    @brief  USBH_LL_SubmitURB
            Submit a new URB to the low level driver.
    @param  phost: Host handle
    @param  pipe: Pipe index
            This parameter can be a value from 1 to 15
    @param  direction : Channel number
             This parameter can be one of the these values:
              0 : Output
              1 : Input
    @param  ep_type : Endpoint Type
             This parameter can be one of the these values:
               @arg EP_TYPE_CTRL: Control type
               @arg EP_TYPE_ISOC: Isochrounous type
               @arg EP_TYPE_BULK: Bulk type
               @arg EP_TYPE_INTR: Interrupt type
    @param  token : Endpoint Type
             This parameter can be one of the these values:
               @arg 0: PID_SETUP
               @arg 1: PID_DATA
    @param  pbuff : pointer to URB data
    @param  length : Length of URB data
    @param  do_ping : activate do ping protocol (for high speed only)
             This parameter can be one of the these values:
              0 : do ping inactive
              1 : do ping active
    @retval Status
*/

USBH_StatusTypeDef   USBH_LL_SubmitURB(USBH_HandleTypeDef *phost,
                                       uint8_t pipe,
                                       uint8_t direction ,
                                       uint8_t ep_type,
                                       uint8_t token,
                                       uint8_t *pbuff,
                                       uint16_t length,
                                       uint8_t do_ping) {
    HAL_StatusTypeDef hal_status = HAL_OK;
    USBH_StatusTypeDef usb_status = USBH_OK;
    hal_status = HAL_HCD_HC_SubmitRequest(phost->pData,
                                          pipe,
                                          direction ,
                                          ep_type,
                                          token,
                                          pbuff,
                                          length,
                                          do_ping);

    switch (hal_status) {
        case HAL_OK :
            usb_status = USBH_OK;
            break;

        case HAL_ERROR :
            usb_status = USBH_FAIL;
            break;

        case HAL_BUSY :
            usb_status = USBH_BUSY;
            break;

        case HAL_TIMEOUT :
            usb_status = USBH_FAIL;
            break;

        default :
            usb_status = USBH_FAIL;
            break;
    }

    return usb_status;
}

/**
    @brief  USBH_LL_GetURBState
            Get a URB state from the low level driver.
    @param  phost: Host handle
    @param  pipe: Pipe index
            This parameter can be a value from 1 to 15
    @retval URB state
             This parameter can be one of the these values:
               @arg URB_IDLE
               @arg URB_DONE
               @arg URB_NOTREADY
               @arg URB_NYET
               @arg URB_ERROR
               @arg URB_STALL
*/
USBH_URBStateTypeDef  USBH_LL_GetURBState(USBH_HandleTypeDef *phost, uint8_t pipe) {
    return (USBH_URBStateTypeDef)HAL_HCD_HC_GetURBState(phost->pData, pipe);
}

/**
    @brief  USBH_LL_DriverVBUS
            Drive VBUS.
    @param  phost: Host handle
    @param  state : VBUS state
             This parameter can be one of the these values:
              0 : VBUS Active
              1 : VBUS Inactive
    @retval Status
*/
USBH_StatusTypeDef  USBH_LL_DriverVBUS(USBH_HandleTypeDef *phost, uint8_t state) {
    if (phost->id == HOST_HS) {
        if (state == 0) {
            /* Drive high Charge pump */
            /* ToDo: Add IOE driver control */
        } else {
            /* Drive low Charge pump */
            /* ToDo: Add IOE driver control */
        }
    }

    HAL_Delay(200);
    return USBH_OK;
}

/**
    @brief  USBH_LL_SetToggle
            Set toggle for a pipe.
    @param  phost: Host handle
    @param  pipe: Pipe index
    @param  pipe_num: Pipe index
    @param  toggle: toggle (0/1)
    @retval Status
*/
USBH_StatusTypeDef   USBH_LL_SetToggle(USBH_HandleTypeDef *phost, uint8_t pipe, uint8_t toggle) {
    HCD_HandleTypeDef *pHandle;
    pHandle = phost->pData;

    if (pHandle->hc[pipe].ep_is_in) {
        pHandle->hc[pipe].toggle_in = toggle;
    } else {
        pHandle->hc[pipe].toggle_out = toggle;
    }

    return USBH_OK;
}

/**
    @brief  USBH_LL_GetToggle
            Return the current toggle of a pipe.
    @param  phost: Host handle
    @param  pipe: Pipe index
    @retval toggle (0/1)
*/
uint8_t  USBH_LL_GetToggle(USBH_HandleTypeDef *phost, uint8_t pipe) {
    uint8_t toggle = 0;
    HCD_HandleTypeDef *pHandle;
    pHandle = phost->pData;

    if (pHandle->hc[pipe].ep_is_in) {
        toggle = pHandle->hc[pipe].toggle_in;
    } else {
        toggle = pHandle->hc[pipe].toggle_out;
    }

    return toggle;
}

/**
    @brief  USBH_Delay
            Delay routine for the USB Host Library
    @param  Delay: Delay in ms
    @retval None
*/
void  USBH_Delay(uint32_t Delay) {
    HAL_Delay(Delay);
}
