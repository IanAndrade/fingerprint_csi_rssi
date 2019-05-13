#include "config.h"
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiMulti.h>


typedef enum {
    type_loratxt,
    type_lorattm,
    type_reset,
    type_updatefw,
    type_begincoleta,
    type_sensorstatus,
    type_requestdata,
    type_unknown
}msg_types_t;

struct FingerprintState_t
{
    String local_reference;
    uint8_t number_of_scans;
    String roda_distance;
    String id_coleta;
    bool send_data_again = false;
};
extern FingerprintState_t fingerprintState;



extern WebServer server_ota;
extern WiFiUDP udp;
extern WiFiClient client;
extern int len;
void generateWiFiAP(const char *ssid, const char *pwd, int channel, int ssid_hidden, int max_connection);
void connectToWiFiEvent(const char * ssid, const char * pwd);
bool connectToWiFi(const char *ssid, const char *pwd,unsigned long time_out_seconds);
void disconnectToWiFi();
void Start_tcp_server(uint16_t port);
void WiFiEvent(WiFiEvent_t event);
bool SendTCPMessage(IPAddress IPToSend, unsigned int PortToCommunicate, String MessageToSend, bool stop_connection_after);
void SendUDPMessage(IPAddress IPToSend, unsigned int PortToSend, String  MessageToSend);
bool ReceiveUDPMessage(unsigned int PortToCommunicate);
bool ReceiveTCPMessage(unsigned int PortToCommunicate);
bool update_fw();
String getHeaderValue(String header, String headerName);
bool execOTA();
msg_types_t getMsgType(char strbuf[MAX_BUF_SIZE], int lenstrbuf);
msg_types_t wait_for_client();
extern boolean connected;
extern char receivePacket[255];
