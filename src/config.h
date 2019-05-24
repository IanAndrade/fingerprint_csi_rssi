#include <Arduino.h>


//############### CONFIGURAÇÕES GERAIS ####################
#define VERSAOFW "3.6"
#define NODENAME "TRAS"
#define SPARKFUN_MPU9250_LIB 1
//############### CONFIGURAÇÕES WIFI ####################
#define MAX_BUF_SIZE 255
#define FORMATO_DADOS 1     //0->JSON e 1->CSV
#define WIFINAME "wififinger"
#define WIFIPASS "wififinger"
#define KEEP_CONNECTED_WHILE_SCANNING 0
#define TIMEOUT_WIFI_CONNECT_S 10
#define UDPDEF 0
#define TCPDEF 1 //não está totalmente implementado
#define UDP_PORT 8000
#define TCP_PORT 8554                    //porta de recebimento TCP
//#define TX_TCP_PORT 8555                    //porta de envio TCP
#if 0                                       //Se 1: envia dados wifi para o IPTOSEND_DEF; Se 0: envia dados wifi para o gateway 
#define IPTOSEND_DEF {52,67,205,233}
#endif

//############### alteracao de firmware ota ################################
#define ENABLE_FW_OTA 1
#define AWS_AUTO_UPDATE 1
#define TIME_WAITING_FOR_CONNECT_FW_OTA_S 20       //tempo que o sensor fica esperando para uma conexão de mudança de FW
#define HOSTNAME NODENAME                           //Apelido do sensor
#define WIFIOTA "phyll"                           //Nome da rede WiFi que deverá ser gerada pelo computador ou celular
#define WIFIPASSOTA "phyllfwota"                    //Senha da rede WiFi que deverá ser gerada pelo computador ou celular
#define LOGIN_FW_OTA "phyll"                    //Login da pagina HTTP inicial
#define SENHA_FW_OTA "phyll"                  //Senha da pagina HTTP inicial

//############### MPU 9250 ####################
#define SDA_PIN 27
#define SCL_PIN 26

//############### CONFIG FINGERPRINT WIFI ####################
#define SCANS_RSSI_ENABLE 1
#define SCAN_RSSI_UNIQUE_CHANNEL 1
#define UNIQUE_CHANNEL_SCAN_RSSI 11
#define CSI_ENABLE 0
#define DEBUG_CSI 0
#define SEM_FILTRO 0  //0:ATIVA O FILTRO , 1:DESATIVA O FILTRO E COLETA TODAS REDES!
#if 1
    #define STATIC_CHANNEL 9
#endif
#define N_COLETAS 15
// #define DEFAULT_SSID "CONNECTED_2.4G"
// #define DEFAULT_PWD "nickyfofo"
#define DEFAULT_SSID "Prosaico01"
#define DEFAULT_PWD "testecsi"