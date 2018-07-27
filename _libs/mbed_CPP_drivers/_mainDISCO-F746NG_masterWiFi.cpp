#include "mbed.h"
#include "Serial.h"
#include "LCD_DISCO_F746NG.h"
#include "TS_DISCO_F746NG.h"
#include "SD_DISCO_F746NG.h"
#include "SpwfInterface.h"
#include "SDFileSystem.h"
#include "TCPSocket.h"
#include "Rtc_Ds1307.h"

//NUCLEO: D1->UART6_TX , D0->UART6_RX
SpwfSAInterface wifi(D1, D0, false);
TCPSocket socket, socket2;


LCD_DISCO_F746NG lcd;
TS_DISCO_F746NG ts;
SDFileSystem sdc(D7, D8, D13, D2, "sd"); // MOSI, MISO, SCK, CS
SPI spi(A2, A3, A4); // mosi, miso, sclk
DigitalOut chipSelect(D4);
Serial pc(USBTX, USBRX); // tx, rx
DigitalIn button(USER_BUTTON);
//timer
Rtc_Ds1307 rtc(PB_9, PB_8);

//uint8_t tabellachip[3];
uint8_t stop = 0;
uint16_t Xpos = 0;
uint16_t Ypos = 0;
char c;
char temp = '#';
bool sdck = 0;
uint8_t setpage = 0;
uint8_t channel = 0;
uint8_t logrun = 0;
char serverip[12];
char USER[15];
char PASSWORD[12];
int controllo = 0;

/////////Variabili Gestione File///////////
char filename1[100];
char filename2[100];
char filename3[100];

///////Variabili WIFI////////
int32_t connected;
char *ssid = "dlink";
char *seckey = "STMdemoPWD";
const char *mac;
const char *ip;
char str[4096];

///////Variabili Touch screen////////
uint16_t x, y;
uint8_t text[30];
uint8_t status_ts;
uint8_t idx = 0;
uint8_t cleared = 0;
uint8_t prev_nb_touches = 0;

////////Variabili Timer///////
int year;
int mon;
int date;
int hour;
int min;
int sec;

//FILE *fp = fopen("/sd/sdtest.txt", "w");
FILE *fp1;
FILE *fp2;
FILE *fp3;
//FILE *foto;

////////////////INTERFACCIA GRAFICA///////////////
//////////////////////////////////////////////////
/*  Elenco Pagine
    menu_page setpage=0
    logging_page setpage=1
*/

void set_gps_page() {
    lcd.Clear(LCD_COLOR_BLUE);
    lcd.SetTextColor(LCD_COLOR_YELLOW);
    lcd.FillRect(100, 200, 100, 50);  ///Disegna bottone
    lcd.FillRect(125, 175, 50, 100);  ///Disegna bottone
}

void menu_page() {
    lcd.Clear(LCD_COLOR_BLUE);
    lcd.SetTextColor(LCD_COLOR_GREEN);
    lcd.FillRect(10, 200, 50, 50);  ///Disegna bottone Verde
    lcd.SetTextColor(LCD_COLOR_RED);
    lcd.FillRect(100, 200, 50, 50);  ///Disegna bottone Rosso
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.SetBackColor(LCD_COLOR_BLUE);
    lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"Menu Principale", CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(7), (uint8_t *)"Press Green Button to start logging", LEFT_MODE);
    lcd.DisplayStringAt(0, LINE(8), (uint8_t *)"Press Red Button to stop logging", LEFT_MODE);
}

void logging_page() {
    lcd.Clear(LCD_COLOR_BLUE);
    lcd.SetTextColor(LCD_COLOR_RED);
    lcd.FillRect(100, 200, 50, 50);  ///Disegna bottone
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.SetBackColor(LCD_COLOR_BLUE);
    lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"Logging Process", CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(8), (uint8_t *)"Press Red Button to stop logging", LEFT_MODE);
    wait(1);
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.DisplayStringAt(0, LINE(10), (uint8_t *)"Logging...Don't remove SD Card", CENTER_MODE);
}

void server_connession_page() {
    lcd.Clear(LCD_COLOR_BLUE);
    //lcd.SetTextColor(LCD_COLOR_RED);
    //lcd.FillRect(100, 200, 50, 50);  ///Disegna bottone
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.SetBackColor(LCD_COLOR_BLUE);
    lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"Connessione al Server", CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(8), (uint8_t *)"attendere trasferimento file", LEFT_MODE);
    wait(1);
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.DisplayStringAt(0, LINE(10), (uint8_t *)"Uploading...Don't remove SD Card", CENTER_MODE);
}

void write_sd(char c) {
    if (logrun) {
        if (fp1 != NULL && fp2 != NULL && fp3 != NULL && sdck) {
            //it checks the communication channel and writes the character in the right file
            switch (channel) {
                case (1):
                    fprintf(fp1, "%c", c);
                    break;

                case (2):
                    fprintf(fp2, "%c", c);
                    break;

                case (3):
                    fprintf(fp3, "%c", c);
                    break;

                default:
                    fprintf(fp1, "%c", c);
                    break;
            }
        } else {
            controllo = 1;
            logrun = 0;
            lcd.Clear(LCD_COLOR_RED);
            lcd.SetTextColor(LCD_COLOR_WHITE);
            lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"Error SD! File not created", CENTER_MODE);
        }

        if (button || stop) {
            //fprintf(fp,"Canale = %d",channel);
            fclose(fp1);
            fclose(fp2);
            fclose(fp3);
            logrun = 0;
            sdc.unmount();
            lcd.Clear(LCD_COLOR_BLUE);
            lcd.SetTextColor(LCD_COLOR_WHITE);
            lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"Logging Stopped", CENTER_MODE);
            //lcd.DisplayStringAt(0, LINE(6), (uint8_t *)"Remove SD", CENTER_MODE);
        }
    }
}

//****************FIND IP FUNCTION**********************
//******************************************************
void read_IP() {
    FILE *fd = fopen("/sd/indirizzo_ip.txt", "r"); //open file where is saved the ip of server

    if (fd != NULL) {
        fgets(serverip, 12, fd);
        sdck = 1; //pc.printf("\r\nip:%s",indip);
    } else {
        pc.printf("\r\nerror reading server ip");
        sdck = 0;
    }

    fclose(fd);
}

int read_ftp_code(char *response) {
    //The FTP FileZilla Server answers with determinated codes
    //so this function finds the first three number that identify
    //the answer and return it
    char s_code[4];
    strncpy(s_code, response, 3);
    return atoi(s_code);
}

int get_data_port(char *response) {
    //It Finds the port for exchange data with server
    //The server send this port with cripted code
    //192,168,nnn,nnn,xxx,yyy and the solution is port=((xxx*256)+yyy)
    //the problem is find this code in the answer
    //so analyzing the string it search the 4th comma ',' and read xxx and yyy
    char data[4];
    char *ptrstart;
    char *ptrend;
    int i = 0;
    int leng;
    int port = -1;
    ptrstart = response; //pointer to the pointer of the string
    pc.printf("cerco la virgola");

    for (i = 0; i < 4; i++) { ptrstart = strchr(ptrstart + 1, ','); } //it finds 4th ','

    if (ptrstart == NULL) { return -1; }

    pc.printf("virgola trovata");
    ptrstart++;
    ptrend = strchr(ptrstart, ','); //it finds 5th ',' to calculate number of character

    if (ptrend == NULL) { return -1; }

    pc.printf("calcolo caratteri");
    leng = (ptrend - ptrstart);  //number of character
    strncpy(data, ptrstart, leng);
    port = atoi(data) * 256; //xxx*256
    ptrstart = ptrend + 1;
    ptrend = strchr(ptrstart, '\r');

    if (ptrend == NULL) { return -1; }

    leng = (ptrend - ptrstart);
    strncpy(data, ptrstart, leng);
    port = port + atoi(data); //(xxx*256)+yyy
    return port;
}

int send_data_to_server(char *path) {
    int rx;
    int port;
    int size = 0;
    int res;
    int len = 0;
    static char rxbuffer[256];
    char type[20];
    char mode[20];
    char stru[20];
    char pasv[20];
    char store[256];
    strcpy(type, "TYPE A\r\n");
    strcpy(stru, "STRU F\r\n");
    strcpy(mode, "MODE S\r\n");
    strcpy(pasv, "PASV\r\n");
    strcpy(store, "STOR ");
    strncat(store, path, 100);
    strncat(store, "\r\n", 5);
    pc.printf("comando di store del file = %s", store);
    socket.send(type, strlen(type)); //exchange data in ascii type
    socket.recv(rxbuffer, sizeof(rxbuffer));
    pc.printf("rxbuffer=%s\r\n", rxbuffer);
    socket.send(stru, strlen(stru));
    socket.recv(rxbuffer, sizeof(rxbuffer));
    pc.printf("rxbuffer=%s\r\n", rxbuffer);
    socket.send(mode, strlen(mode));
    socket.recv(rxbuffer, sizeof(rxbuffer));
    pc.printf("rxbuffer=%s\r\n", rxbuffer);
    socket.send(pasv, strlen(pasv));
    socket.recv(rxbuffer, sizeof(rxbuffer));
    rx = read_ftp_code(rxbuffer);
    port = get_data_port(rxbuffer);
    pc.printf("Porta dati =%d\r\n", port);
    //if(port<0)port=65535;

    if (rx == 227) {
        socket2.open(&wifi); //open new socket for data transmission
        socket.recv(rxbuffer, sizeof(rxbuffer));
        pc.printf("rxbuffer=%s\r\n", rxbuffer);
        socket2.bind(ip, port); //bind the data port that is available on the server calculated by get_data_port function
        socket.recv(rxbuffer, sizeof(rxbuffer));
        pc.printf("rxbuffer=%s\r\n", rxbuffer);
        socket2.connect(serverip, port);
        socket.recv(rxbuffer, sizeof(rxbuffer));
        pc.printf("rxbuffer=%s\r\n", rxbuffer);
        socket.send(store, strlen(store)); //STOR reserve a memory area for a file
        socket.recv(rxbuffer, sizeof(rxbuffer));
        pc.printf("rxbuffer=%s\r\n", rxbuffer);
        FILE *fp = fopen(path, "r"); //open file
        pc.printf("file aperto\r\n");
        // find dimension of file
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        pc.printf("file size= %d\r\n", size);
        int n = 0;

        while (1) {
            n++;
            res = fread(&str, 1, 4094, fp);

            if (n * 4094 <= size) {
                pc.printf("Invio paccheto da 4096 byte\r\n");
                socket2.send(str, strlen(str));     //send multiple of 4094 byte at time
            } else {
                len = strlen(str) - (n * 4094 - size); //calculate the rest of byte to send
                char log[len];

                for (int i = 0; i < len; i++) {
                    log[i] = str[i];
                }

                log[len] = '\0'; //terminate the string with '\0'
                socket2.send(log, len);
                pc.printf("Invio paccheto da %d byte\r\n", len);
                break;
            }
        }

        fclose(fp);
    }

    socket2.close();
    return 1;
}

void connect_to_server() {
    int failed;
    static char rxbuffer[256];
    int rx;
    int sended;
    char user[1024];
    char pwd[1024];
    //char store2[50];
    //char mode[20];
    //char stru[20];
    //char pasv[20];
    //creation of string command to send to server to start the communication
    strcpy(user, "USER datalogger\r\n");
    strcpy(pwd, "PASS project\r\n");
    socket.set_timeout(3000);
    socket2.set_timeout(3000);
    pc.printf("prova di connessione a server ftp");
    int erroropen = socket.open(&wifi);
    socket.bind(ip, 22);

    if (!erroropen) {
        pc.printf("connessione in corso...\r\n");
        failed = socket.connect(serverip, 21);
        controllo = failed;
        socket.recv(rxbuffer, sizeof(rxbuffer));
        rx = read_ftp_code(rxbuffer);

        if (rx == 220) {
            pc.printf("%d connected on port 21\r\n", rx);
            pc.printf("rxbuffer=%s\r\n", rxbuffer);
            socket.send(user, strlen(user));
            socket.recv(rxbuffer, sizeof(rxbuffer));
            pc.printf("rxbuffer=%s\r\n", rxbuffer);
            socket.send(pwd, strlen(pwd));
            socket.recv(rxbuffer, sizeof(rxbuffer));
            pc.printf("rxbuffer=%s\r\n", rxbuffer);
            sended = send_data_to_server(filename1);

            if (sended) { send_data_to_server(filename2); }
        } else {
            controllo = -1;
            pc.printf("Errore connessione con il server\r\n");
        }
    }

    socket.close();
}

void init_wifi() {
    mac = wifi.get_mac_address();
    //connected=wifi.connect(ssid, seckey, NSAPI_SECURITY_WPA2);
    //ip = wifi.get_ip_address();
}

void init_ts() {
    ////////Touch Screen////////
    //TS_StateTypeDef TS_State;
    status_ts = ts.Init(lcd.GetXSize(), lcd.GetYSize());
}

void init_time() {
    //INIT TIME
    Rtc_Ds1307::Time_rtc tm = {};
    year = tm.year = 2017;
    mon = tm.mon = 1;
    date = tm.date = 23;
    hour = tm.hour = 9;
    min = tm.min = 3;
    sec = tm.sec = 00;
    rtc.setTime(tm, true, false); //rtc initialization
    rtc.startClock();
    rtc.getTime(tm);
    pc.printf("%02d-%02d-%04d_%02d.%02d.%02d", tm.date, tm.mon, tm.year, tm.hour, tm.min, tm.sec);
}

void boot() {
    init_time();
    lcd.Clear(LCD_COLOR_WHITE);
    lcd.SetBackColor(LCD_COLOR_WHITE);
    lcd.SetTextColor(LCD_COLOR_BLUE);
    lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"STMicroelectronics", CENTER_MODE);
    lcd.SetFont(&Font12);
    lcd.DisplayStringAt(0, LINE(0), (uint8_t *)"Inizializzazione...", LEFT_MODE);
    wait(0.5);
    lcd.DisplayStringAt(0, LINE(1), (uint8_t *)"WI FI init...", LEFT_MODE);
    init_wifi();    //inizializzazione wi-fi
    lcd.DisplayStringAt(0, LINE(2), (uint8_t *)"MAC ADDRESS", LEFT_MODE);
    lcd.DisplayStringAt(30, LINE(2), (uint8_t *)mac, LEFT_MODE);
    /*  if(connected) {
        lcd.DisplayStringAt(0, LINE(2), (uint8_t *)"WI FI connected......", LEFT_MODE);
        } else {
        lcd.DisplayStringAt(0, LINE(2), (uint8_t *)"WI FI NOT connected......", LEFT_MODE);

        }

        lcd.DisplayStringAt(0, LINE(1), (uint8_t *)ip, LEFT_MODE);
    */
    init_ts();  //inizializzazione touch_screen

    if (status_ts != TS_OK) {
        lcd.SetTextColor(LCD_COLOR_RED);
        lcd.DisplayStringAt(0, LINE(4), (uint8_t *)"TOUCHSCREEN INIT FAIL", LEFT_MODE);
    } else {
        lcd.DisplayStringAt(0, LINE(4), (uint8_t *)"Touchscreen init success", LEFT_MODE);
    }

    ////////  SD  //////////////
    //sdck=sdc.card_present();
    read_IP(); //legge l'indirizzo ip dal file di configurazione per connessione a server ftp

    if (sdck) {
        lcd.DisplayStringAt(0, LINE(3), (uint8_t *)"SD PRESENTE", LEFT_MODE);
        wait(1);
    }

    if (!sdck) {
        lcd.SetTextColor(LCD_COLOR_RED);
        lcd.DisplayStringAt(0, LINE(3), (uint8_t *)"SD NON PRESENTE", LEFT_MODE);
        lcd.DisplayStringAt(0, LINE(4), (uint8_t *)"SI PREGA INSERIRE SD E RIAVVIARE", LEFT_MODE);
        wait(2);
    }

    //connect_to_server();
    //foto = fopen("/sd/bitmap.bmp", "w");
    //lcd.DrawBitmap(0,0,*foto);
    wait(1);
}




int main() {
    int stato = 0;
    int valueToSendToSlave = 99; // Starting value only
    pc.baud(115200);
    spi.format(16, 0);       // Setup:  bit data
    spi.frequency(24000000); //1MHz
    boot();
    mkdir("/sd/log", 0777);
    sprintf(filename1, "/sd/log/file_serial2_%02d-%02d-%04d_%02d.%02d.%02d_gps1.txt", date, mon, year, hour, min, sec);
    sprintf(filename2, "/sd/log/file_serial2_%02d-%02d-%04d_%02d.%02d.%02d_gps2.txt", date, mon, year, hour, min, sec);
    sprintf(filename3, "/sd/log/file_serial2_%02d-%02d-%04d_%02d.%02d.%02d_gps3.txt", date, mon, year, hour, min, sec);
    fp1 = fopen(filename1, "w");
    pc.printf("file 1 aperto\r\n");
    fp2 = fopen(filename2, "w");
    pc.printf("file 2 aperto\r\n");
    fp3 = fopen(filename3, "w");
    pc.printf("file 3 aperto\r\n");
    //connect_to_server();
    lcd.SetBackColor(LCD_COLOR_BLUE);
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.Clear(LCD_COLOR_WHITE);
    lcd.SetBackColor(LCD_COLOR_WHITE);
    lcd.SetTextColor(LCD_COLOR_BLUE);
    wait(0.3);
    lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"Welcome to", CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(6), (uint8_t *)"GNNS LOGGER V3.0", CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(10), (uint8_t *)"Touch The Screen to Start", CENTER_MODE);

    while (1) {
        ///////////////Touch Screen///////////////
        TS_StateTypeDef TS_State;
        ts.GetState(&TS_State);

        switch (stato) {
            case (0):

                //initial state
                //it draws the menu page anche check the touchscreen
                if (TS_State.touchDetected && setpage == 0) {
                    menu_page();

                    for (idx = 0; idx < TS_State.touchDetected; idx++) {
                        x = TS_State.touchX[idx];
                        y = TS_State.touchY[idx];
                    }

                    if ((x >= 10) && (x <= 60) && (y >= 200) && (y <= 250)) {
                        //if is pressed the green button, the logging starts
                        logrun = 1;
                        setpage = 1;
                        stato = 1;
                        logging_page();
                    }
                }

                break;

            case (1):

                //in this state starts the communication by SPI with slave
                //and starts to save data on sdcard
                if (logrun) {
                    valueToSendToSlave = 99;
                    chipSelect = 1;   // Deselect device
                    chipSelect = 0; // Select device
                    uint16_t dataFromSlave =  spi.write(valueToSendToSlave);
                    chipSelect = 1;   // Deselect device
                    c = dataFromSlave;
                    channel = dataFromSlave >> 8;

                    if (c != 0) { write_sd(c); }

                    for (idx = 0; idx < TS_State.touchDetected; idx++) {
                        x = TS_State.touchX[idx];
                        y = TS_State.touchY[idx];
                    }

                    if ((x >= 100) && (x <= 160) && (y >= 200) && (y <= 250)) {
                        lcd.Clear(LCD_COLOR_BLUE);
                        lcd.SetTextColor(LCD_COLOR_WHITE);
                        lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"Connecting to WiFi Network", CENTER_MODE);
                        stop = 1;
                        logrun = 0;
                        setpage = 0;
                        stato = 2;
                    }

                    if (controllo) {
                        pc.printf("errore file\r\n");
                        stato = 0;
                    }

                    break;

                case (2):

                    //Here start the connection to server by Wi-Fi module to send data to FileZilla Server
                    if (!controllo) {
                        connected = wifi.connect(ssid, seckey, NSAPI_SECURITY_WPA2);
                        ip = wifi.get_ip_address();

                        if (connected) {
                            lcd.Clear(LCD_COLOR_BLUE);
                            lcd.SetTextColor(LCD_COLOR_WHITE);
                            lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"WI FI connected......", LEFT_MODE);
                            connect_to_server();
                            server_connession_page();
                            stato = 0;
                            stop = 0;
                        } else {
                            lcd.DisplayStringAt(0, LINE(2), (uint8_t *)"WI FI NOT connected......", LEFT_MODE);
                        }
                    }

                    if (controllo) {
                        pc.printf("errore connessione server\r\n");
                        stato = 0;
                        stop = 0;
                    }

                    break;
                }
        }
    }
}