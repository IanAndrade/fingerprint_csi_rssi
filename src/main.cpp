#include "config.h"
//#include "WiFi_Functions.h"
#include "csi_functions.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "functions.h"
#if SPARKFUN_MPU9250_LIB == 1
  #include "MPU9250.h"
  float compass_heading = 0;
#else
  #include "MPU9250_asukiaaa.h"
  MPU9250 mySensor;
#endif


//10 networks found
//1: CONNECTED_2.4G ,11, (-46) MAC: 98:1E:19:AD:2E:6A
//2: wififinger ,11, (-50) MAC: 0E:01:7C:FE:22:21
//3: Stallone ,1, (-66) MAC: 4C:D0:8A:8E:5D:CA
//4: #NET-CLARO-WIFI ,1, (-66) MAC: 4E:D0:8A:8E:5D:CB
//5: VALFIERZ ,1, (-81) MAC: 84:A4:23:A0:3D:2E
//6: Gabriela ,6, (-89) MAC: 90:B1:34:C5:AA:BB
//7: FRIDA 53 ,7, (-92) MAC: 80:29:94:DE:DF:90
//8: Casinha ,1, (-93) MAC: E8:20:E2:56:4F:A4
//9: TUCA ,1, (-94) MAC: F4:F2:6D:97:66:B0
//10: jardimtijucapiscina ,1, (-94) MAC: 58:10:8C:51:10:02


bool wakeup_from_reset = false;
WebServer server_ota(80);
uint16_t TCP_SEND_PORT;

bool ready_to_send_data = false;
WiFiClient client;
const char * networkName = WIFINAME;
const char * networkPswd = WIFIPASS;
int tcpPort = TCP_PORT;
bool begin_coleta = false;
int len = 0;
//FingerprintState_t fingerprintState;
String dataMessage = "0";
int request_cont = 0;   //Cota numero de solicitações
char receivePacket[255];
bool set_static_channel = false;

void wifi_list(void);
void print_wakeup_reason();

void mpu9250_listUDP(){
#if SPARKFUN_MPU9250_LIB == 0
  //dataMessage = "{";
  mySensor.accelUpdate();
//Serial.println("print accel values");
//Serial.println("accelX: " + String(mySensor.accelX()));
//Serial.println("accelY: " + String(mySensor.accelY()));
//Serial.println("accelZ: " + String(mySensor.accelZ()));
//Serial.println("accelSqrt: " + String(mySensor.accelSqrt()));
 
mySensor.magUpdate();
//Serial.println("print mag values");
//Serial.println("magX: " + String(mySensor.magX()));
//Serial.println("maxY: " + String(mySensor.magY()));
//Serial.println("magZ: " + String(mySensor.magZ()));
//Serial.println("horizontal direction: " + String(mySensor.magHorizDirection()));

mySensor.gyroUpdate();
//Serial.println("print gyro values");
//Serial.println("gyroX: " + String(mySensor.gyroX()));
//Serial.println("gyroY: " + String(mySensor.gyroY()));
//Serial.println("gyroZ: " + String(mySensor.gyroZ()));
#if FORMATO_DADOS == 0
  dataMessage +=",";
  dataMessage += "{ \"acx\":\"" + String(mySensor.accelX()) +"\",\"acy\":\"" + String(mySensor.accelY())+ "\", \"acz\":\""+ String(mySensor.accelZ()) + "\", \"acsqrt\":"+ String(mySensor.accelSqrt())+"}";
  dataMessage +=",";
  dataMessage += "{ \"magx\":\"" + String(mySensor.magX()) +"\",\"magy\":\"" + String(mySensor.magY())+ "\", \"magz\":\""+ String(mySensor.magZ()) + "\", \"maghdir\":"+ String(mySensor.magHorizDirection())+"}";
  dataMessage +=",";
  dataMessage += "{ \"gyrox\":\"" + String(mySensor.gyroX()) +"\",\"gyroy\":\"" + String(mySensor.gyroY())+ "\", \"gyroz\":"+ String(mySensor.gyroZ()) +"}";
#else
  dataMessage += String(mySensor.accelX()) + ",";
  dataMessage += String(mySensor.accelY()) + ",";
  dataMessage += String(mySensor.accelZ())  + ",";
  dataMessage += String(mySensor.accelSqrt()) + ",";
  dataMessage += String(mySensor.magX()) + ",";
  dataMessage += String(mySensor.magY()) + ",";
  dataMessage += String(mySensor.magZ())  + ",";
  dataMessage += String(mySensor.magHorizDirection()) + ",";
  dataMessage += String(mySensor.gyroX()) + ",";
  dataMessage += String(mySensor.gyroY()) + ",";
  dataMessage += String(mySensor.gyroZ());
#endif
#else
  MPU_updateValues();
  printIMUData();
  #if FORMATO_DADOS == 0
  dataMessage +=",";
  dataMessage += "{ \"acx\":\"" + String(imu.ax) +"\",\"acy\":\"" + String(imu.ay)+ "\", \"acz\":\""+ String(imu.az) + "\", \"acsqrt\":"+ String(mySensor.accelSqrt())+"}";
  dataMessage +=",";
  dataMessage += "{ \"magx\":\"" + String(imu.mx) +"\",\"magy\":\"" + String(imu.my)+ "\", \"magz\":\""+ String(imu.mz) + "\", \"maghdir\":"+ String(mySensor.magHorizDirection())+"}";
  dataMessage +=",";
  dataMessage += "{ \"gyrox\":\"" + String(imu.gx) +"\",\"gyroy\":\"" + String(imu.gy)+ "\", \"gyroz\":"+ String(imu.gz) +"}";
#else

  dataMessage += String(imu.ax) + ",";
	dataMessage += String(imu.ay) + ",";
	dataMessage += String(imu.az)  + ",";
	dataMessage += String(imu.mx)  + ",";
	dataMessage += String(imu.my) + ",";
	dataMessage += String(imu.mz) + ",";
  dataMessage += String(compass_heading) + ",";
  dataMessage += String(imu.gx) + ",";
	dataMessage += String(imu.gy) + ",";
	dataMessage += String(imu.gz);
#endif

#endif
}

void wifi_list() {
#if SCANS_RSSI_ENABLE == 1
  request_cont++;

#if (UDP_DEF)
 int max = String(receivePacket).substring(0,2).toInt();
 String geo_reference = String(receivePacket).substring(2,(len-2));
#elif TCPDEF == 1
  int max = fingerprintState.number_of_scans;
  String geo_reference = fingerprintState.local_reference;
  String roda_distance = fingerprintState.roda_distance;
  String id_coleta = fingerprintState.id_coleta;
#endif
    dataMessage = "";
    //dataMessage += "device,n_scan,mac,reqID,ssdid,channel,rssi,acx,acy,acz,acsqrt,magx,magy,magz,maghdir,gyrox,gyroy,gyroz\n";
    // dataMessage += "device,n_scan,mac,reqID,channel,rssi,acx,acy,acz,acsqrt,magx,magy,magz,maghdir,gyrox,gyroy,gyroz\n";
  #if SPARKFUN_MPU9250_LIB == 0
    dataMessage += "device,n_scan,mac,ssid,reqID,roda_distance,id_coleta,channel,rssi,acx,acy,acz,acsqrt,magx,magy,magz,maghdir,gyrox,gyroy,gyroz\n";
  #else
    dataMessage += "device,n_scan,mac,ssid,reqID,roda_distance,id_coleta,channel,rssi,acx,acy,acz,magx,magy,magz,maghdir,gyrox,gyroy,gyroz\n";
  #endif

  for (int i = 0; i < max; i++) {
#if FORMATO_DADOS == 0
#if KEEP_CONNECTED_WHILE_SCANNING == 1
    dataMessage = "{";
#else
    dataMessage += "{";
#endif
#else
    //dataMessage += "device,n_scan,mac,reqID,ssdid,channel,rssi,acx,acy,acz,acsqrt,magx,magy,magz,maghdir,gyrox,gyroy,gyroz\n";
#endif
    // WiFi.scanNetworks will return the number of networks found
    // unsigned long tempo = millis();
    int n = WiFi.scanNetworks();
    // printf("Tempo de duracao scan:%d",(millis()-tempo));
    //UDP.println("scan done");
    if (n == 0){
#if FORMATO_DADOS == 0
      dataMessage += "\"reqID\":\"" + geo_reference + "\",""\"networks\":0}";
#else

#endif
    }else{
#if FORMATO_DADOS == 0
      dataMessage += "\"device\":\""NODENAME"\",";
      dataMessage += "\"n_scan\":\""+ String(i) +"\",";
      dataMessage += "\"scan\":[";
#else
      //dataMessage += NODENAME;
      //dataMessage += ",";
      //dataMessage += String(i) + ",";
#endif
#if FORMATO_DADOS == 1
      // dataMessage += NODENAME;
#endif
      for (int j = 0; j < n; ++j){
      if(WiFi.SSID(j) != WIFINAME){
        // Print SSID and RSSI for each network found
#if FORMATO_DADOS == 0
      dataMessage += "{ \"mac\":\"" + String(WiFi.BSSIDstr(j)) +"\",\"reqID\":\"" + geo_reference + "\", \"ssid\":\""+ String(WiFi.SSID(j))+ "\", \"channel\":\""+ String(WiFi.channel(j)) + "\", \"rssi\":"+ String(WiFi.RSSI(j))+"}";
#else    
      dataMessage += NODENAME;
      dataMessage += ",";
      dataMessage += String(i) + ",";
      String mac_sem_dois_pontos = String(WiFi.BSSIDstr(j)).substring(0,2) + String(WiFi.BSSIDstr(j)).substring(3,5) + String(WiFi.BSSIDstr(j)).substring(6,8) + String(WiFi.BSSIDstr(j)).substring(9,11) + String(WiFi.BSSIDstr(j)).substring(12,14) + String(WiFi.BSSIDstr(j)).substring(15,17);
      dataMessage += mac_sem_dois_pontos +",";
      dataMessage += String(WiFi.SSID(j)) +",";
      //dataMessage += String(WiFi.BSSIDstr(j)) +",";
      dataMessage += geo_reference  +",";
      dataMessage += roda_distance  +",";
      dataMessage += id_coleta  +",";
      dataMessage += String(WiFi.channel(j))  +",";
      dataMessage += String(WiFi.RSSI(j)) +",";
#endif      
      if(j<n-1){
#if FORMATO_DADOS == 0
          dataMessage +=",";
#else
          //dataMessage += ",,,,,,,,,,\n";
          dataMessage += "\n";
#endif
        }
      }
        
      }
      mpu9250_listUDP();

#if FORMATO_DADOS == 0
      dataMessage += "]}";
      dataMessage += "\n\r";
#else
    if(i<(max-1))
      dataMessage += "\n";
#endif      
    }
    
#if KEEP_CONNECTED_WHILE_SCANNING == 1
    int cnt = 0;
    while(WiFi.status() != WL_CONNECTED){
      Serial.print('.');
      if(cnt++>50){
        Serial.println();
        cnt=0;
      }
      
      delay(10);
    }/*

    if (client.connected()) {
      client.println(dataMessage);
    } else if (client.connect(WiFi.gatewayIP(), tcpPort)) {
      client.println(dataMessage);
    } else Serial.println("Can't Send TCP Msg!");*/

    //if ((client.connected())||(client.connect(WiFi.gatewayIP(), tcpPort))) {
    //  client.println(dataMessage);
    //  client.stop();
    //} else Serial.println("Can't Send TCP Msg!");
    Serial.println(("Transmissão:\t"+String(i+1)));
    Serial.println("Tamanho enviando:\t"+String(dataMessage.length()));
    Serial.println(dataMessage);
#ifdef IPTOSEND_DEF
    SendUDPMessage(IPTOSEND_DEF,UDP_PORT,dataMessage);
#else
    SendUDPMessage(WiFi.gatewayIP(),UDP_PORT,dataMessage);
#endif
#endif

    //UDP.beginPacket(WiFi.gatewayIP(), UDP_PORT);
    ////UDP.beginPacket(pcip, UDP_PORT);
    //UDP.println(dataMessage);
    //UDP.endPacket();
    
    delay(10);
  }
  
  //Serial.println("Scan das redes finalizado. Tamanho dataMessage:\t"+String(dataMessage.length()));
  //Serial.println(dataMessage);
#endif
  Serial.println("Scan das redes finalizado!");
  dataMessage += "\;\n";
  ready_to_send_data = true;
  begin_coleta = false;
  #if (CSI_ENABLE)
  
  #else
  while(WiFi.status() != WL_CONNECTED)
  connectToWiFi(networkName, networkPswd, TIMEOUT_WIFI_CONNECT_S);
  #endif
}

void csi_coleta(){
   while(!send_csi_message){
   for(int bandwith=0; bandwith<2;bandwith++){ // HT40- is not avaible for all channels
			//int ht_chan=0;
			switch (bandwith){
				case 0:
					ht_chan=WIFI_SECOND_CHAN_NONE;
					break;
				case 1:
					ht_chan=WIFI_SECOND_CHAN_ABOVE;
					break;
				case 2:
					ht_chan=WIFI_SECOND_CHAN_BELOW;
					break;
				default:
					ht_chan=WIFI_SECOND_CHAN_NONE;
			}
#ifdef STATIC_CHANNEL	
			// esp_wifi_set_channel(STATIC_CHANNEL, ht_chan);
      // printf("Switching channel to %d with bandwith [None/above/bellow]=%d\n", STATIC_CHANNEL, ht_chan);
      // esp_wifi_set_channel(STATIC_CHANNEL, WIFI_SECOND_CHAN_ABOVE);
      // printf("Switching channel to %d with bandwith [None/above/bellow]=%d\n", STATIC_CHANNEL, WIFI_SECOND_CHAN_ABOVE);
      esp_wifi_set_channel(STATIC_CHANNEL, WIFI_SECOND_CHAN_NONE);
      printf("Switching channel to %d with bandwith [None/above/bellow]=%d\n", STATIC_CHANNEL, WIFI_SECOND_CHAN_NONE);
      
      
      Serial.println("Disconnect!");
			ESP_ERROR_CHECK(esp_wifi_disconnect());
			usleep(300*1000);
			Serial.println("Connect!");
			ESP_ERROR_CHECK(esp_wifi_connect());
			usleep(1800*1000);
			//ESP_ERROR_CHECK(ret);
      if(millis()>time_out_csi){
        Serial.println("Estou sem receber pacotes CSI há um tempo!");
        //time_out_csi = return_end_time(15*1e3);
        //ESP_ERROR_CHECK(esp_wifi_disconnect());
        //setup_wifi_csi();
      }
#else
			for (int chan=1;chan<12;chan++){
				printf("Switching channel to %d with bandwith [None/above/bellow]=%d\n", chan, bandwith);
				//ret=esp_wifi_set_channel(chan, ht_chan);
        esp_wifi_set_channel(chan, ht_chan);
				//ESP_ERROR_CHECK(ret);
        usleep(1*1000*1000);
			}
#endif
      //usleep(100*1000);
  }
  }
  ESP_ERROR_CHECK(esp_wifi_disconnect());
  delay(1000);
}
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  //WiFi.mode(WIFI_STA);
  //WiFi.disconnect();
  //delay(1000);
  Serial.begin(115200);
  Serial.print("NODENAME:");Serial.println(NODENAME);
  Serial.print("VERSAOFW:");Serial.println(VERSAOFW);
  /* setup wifi */
  // wifi_country_t wifi_config_country;
  // auto esp_err = esp_wifi_get_country(&wifi_config_country);
  //  wifi_config_country.schan=1;
  //  wifi_config_country.nchan=11;
  //  strcpy(wifi_config_country.cc, "USA");
  // ESP_ERROR_CHECK( esp_wifi_set_country(&wifi_config_country) ); /* set country for channel range [1, 13] */
#if (CSI_ENABLE)
  ret = nvs_flash_init();
    //if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) { [CHANGED]: This was not letting compile
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
  //toaquii  ESP_ERROR_CHECK( ret );
  setup_wifi_csi();
  //toaquiidelay(1000);
  //toaquiiesp_wifi_set_promiscuous(false);
#endif

#if ENABLE_FW_OTA == 1
  print_wakeup_reason();
#endif
  //Configuração do MPU9250
#if SPARKFUN_MPU9250_LIB == 0
  Wire.begin(SDA_PIN, SCL_PIN); //sda, scl
  mySensor.setWire(&Wire);
  mySensor.beginAccel(ACC_FULL_SCALE_16_G);
  mySensor.beginMag();
  mySensor.beginGyro(GYRO_FULL_SCALE_2000_DPS);
  // you can set your own offset for mag values
  // mySensor.magXOffset = -50;
  // mySensor.magYOffset = -55;
  // mySensor.magZOffset = -10;
#else
  setup_MPU();
#endif
    if(NODENAME == "FRENTE") TCP_SEND_PORT = 8555;
    else if(NODENAME == "DIR") TCP_SEND_PORT = 8556;
    else if(NODENAME == "ESQ") TCP_SEND_PORT = 8557;
    else if(NODENAME == "TRAS") TCP_SEND_PORT = 8558;


  //Connect to the WiFi network
  while(WiFi.status() != WL_CONNECTED)
  connectToWiFi(networkName, networkPswd, TIMEOUT_WIFI_CONNECT_S);
#if (UDP_DEF)
  udp.begin(UDP_PORT);
#endif
  Serial.println("Setup done");

}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Espera istrução para iniciar Scan
  if(!ready_to_send_data){
#if (UDP_DEF)

#elif TCPDEF == 1
  #ifdef IPTOSEND_DEF
    IPAddress host = IPTOSEND_DEF; // ip or dns
  #else
    IPAddress host = WiFi.gatewayIP(); // IP do AP conectado
  #endif
  if(client.connected()){
  }
  else{
  while (!client.connect(host, TCP_PORT)) { // Fica tentando se conectar num servidor TCP
    while(WiFi.status() != WL_CONNECTED) connectToWiFi(networkName, networkPswd, TIMEOUT_WIFI_CONNECT_S);
    Serial.println("Connection failed.");
    Serial.println("Waiting 5 seconds before retrying...");
    delay(5000);
    //return;
  }
  }
  #define CONNECT_MESSAGE "$CONNECTED," NODENAME ",\n"
  //SendTCPMessage(host,TCP_PORT,CONNECT_MESSAGE,false);  //Envia mensagem para dizer que se conectou.
  if(client.connected()){
  client.print(CONNECT_MESSAGE);
  Serial.println(CONNECT_MESSAGE);
  }
  msg_types_t typeMsg = type_unknown;
	typeMsg = wait_for_client();
	switch(typeMsg){
			case type_loratxt:
				//serialize_message_to_LoRa_queue(msg_loratxt);
			break;
      case type_sensorstatus:
        if(client.connected()){
            client.print(CONNECT_MESSAGE);
            Serial.println(CONNECT_MESSAGE);
        }
      break;
      case type_unknown:
          while(WiFi.status() != WL_CONNECTED)
            connectToWiFi(networkName, networkPswd, TIMEOUT_WIFI_CONNECT_S);
      break;
      case type_begincoleta:
        begin_coleta = true;
        disconnectToWiFi();
        #if (CSI_ENABLE)
          csiMessage ="device,n_scan,mac,reqID,time,id_coleta,len,first_word_invalid,noise_floor,rssi,rate,sig_mode,cwb,stbc,channel,secondary_channel,rx_state\n";
          begin_csi_scan = true;
          send_csi_message = false;
          conta_csi = 0;
          #ifdef STATIC_CHANNEL
            set_static_channel = true;
          #endif
          setup_wifi_csi();
          time_out_csi = return_end_time(15*1e3);
        #endif
          break;
      case type_requestdata:
      if(fingerprintState.send_data_again == 1){
          client.stop();
          delay(500);
          while (!client.connect(host, TCP_SEND_PORT)){
            while(WiFi.status() != WL_CONNECTED) connectToWiFi(networkName, networkPswd, TIMEOUT_WIFI_CONNECT_S);
            Serial.println("Connection failed.");
            Serial.println("Waiting 5 seconds before retrying...");
            delay(5000);
            //return;
          }
          client.flush();

          #define COLETA_FINALIZADA "$COLETAFINISHED," NODENAME ",\n"
          client.print(COLETA_FINALIZADA);
          //SendTCPMessage(host,TCP_PORT,CONNECT_MESSAGE,false);  //Envia mensagem para dizer que se conectou.
          Serial.println("REENVIANDO DADOS!");
          delay(1000);
    #if SCANS_RSSI_ENABLE == 1
          client.print(dataMessage);
          Serial.println("Tamanho enviado dataMessage:\t"+String(dataMessage.length()));
          //Serial.println(dataMessage);
          delay(1000);
    #endif

    #if (CSI_ENABLE)
        //bool sent_csi = false;
        // while(!sent_csi){
        //   sent_csi = SendTCPMessage(host,TCP_PORT,csiMessage,false);
        // }
        client.print(csiMessage);
        send_csi_message = false;
        Serial.println("Tamanho enviado csiMessage:\t"+String(csiMessage.length()));
        Serial.println(csiMessage);
        delay(1000);          
    #endif
        client.stop();
        fingerprintState.send_data_again = 0;
      }
        delay(1000);
          break;
				default: break;
			}

#endif
  if(begin_coleta){
    wifi_list();
  }
  if(begin_csi_scan){
    csi_coleta();
  }
  }
    else{
#if KEEP_CONNECTED_WHILE_SCANNING == 0
  #if (UDP_DEF)
    //
  #elif TCPDEF == 1
    bool sent_data = false;
    //WiFiClient client;
    #ifdef IPTOSEND_DEF
      IPAddress host = IPTOSEND_DEF; // ip or dns
    #else
      IPAddress host = WiFi.gatewayIP(); // ip or dns
    #endif
    while (!client.connect(host, TCP_SEND_PORT)){
        while(WiFi.status() != WL_CONNECTED) connectToWiFi(networkName, networkPswd, TIMEOUT_WIFI_CONNECT_S);
        Serial.println("Connection failed.");
        Serial.println("Waiting 5 seconds before retrying...");
        delay(5000);
        //return;
      }
    client.flush();

    #define COLETA_FINALIZADA "$COLETAFINISHED," NODENAME ",\n"
    client.print(COLETA_FINALIZADA);
    //SendTCPMessage(host,TCP_PORT,CONNECT_MESSAGE,false);  //Envia mensagem para dizer que se conectou.
    Serial.println(COLETA_FINALIZADA);
    delay(1000);

#if SCANS_RSSI_ENABLE == 1
    //while(!sent_data){
    //  sent_data = SendTCPMessage(host,TCP_PORT,dataMessage,false);
    //}
    //if(NODENAME == "FRENTE") delay(100); 
    //else if(NODENAME == "DIR") delay(4000);
    //else if(NODENAME == "ESQ") delay(8000);
    //else if(NODENAME == "TRAS") delay(12000);
    client.print(dataMessage);
    Serial.println("Tamanho enviado dataMessage:\t"+String(dataMessage.length()));
    Serial.println(dataMessage);
    delay(1000);
#endif

#if (CSI_ENABLE)
    bool sent_csi = false;
    // while(!sent_csi){
    //   sent_csi = SendTCPMessage(host,TCP_PORT,csiMessage,false);
    // }
    client.print(csiMessage);
    send_csi_message = false;
    Serial.println("Tamanho enviado csiMessage:\t"+String(csiMessage.length()));
    Serial.println(csiMessage);
    delay(1000);          
#endif

  #endif
#endif
    client.stop();
    delay(1000);
    ready_to_send_data = false;
    }
  }
  else{
#if (CSI_ENABLE)
  if(ready_to_send_data && send_csi_message){
    esp_wifi_set_promiscuous(false);
    while(WiFi.status() != WL_CONNECTED)
    connectToWiFi(networkName, networkPswd, TIMEOUT_WIFI_CONNECT_S);
  }else if(begin_csi_scan){
//#ifdef STATIC_CHANNEL
//    if(set_static_channel){
//      printf("Static Channel:%d\n",STATIC_CHANNEL);
//      esp_wifi_set_channel(STATIC_CHANNEL, WIFI_SECOND_CHAN_NONE);
//      set_static_channel = false;
//    }
//#else
//    if(curChannel > 11){ 
//        curChannel = 1;
//      }
//    printf("Changed channel:%d\n",curChannel);
//    esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
//    delay(500);
//    curChannel++;
//#endif
  //NOVO CSI SCAN
  //for(int bandwith=0; bandwith<2;bandwith++){ // HT40- is not avaible for all channels
  while(!send_csi_message){
   for(int bandwith=0; bandwith<2;bandwith++){ // HT40- is not avaible for all channels
			//int ht_chan=0;
			switch (bandwith){
				case 0:
					ht_chan=WIFI_SECOND_CHAN_NONE;
					break;
				case 1:
					ht_chan=WIFI_SECOND_CHAN_ABOVE;
					break;
				case 2:
					ht_chan=WIFI_SECOND_CHAN_BELOW;
					break;
				default:
					ht_chan=WIFI_SECOND_CHAN_NONE;
			}
#ifdef STATIC_CHANNEL	
			//esp_wifi_set_channel(STATIC_CHANNEL, ht_chan);
      esp_wifi_set_channel(STATIC_CHANNEL, WIFI_SECOND_CHAN_NONE);
      printf("Switching channel to %d with bandwith [None/above/bellow]=%d\n", STATIC_CHANNEL, WIFI_SECOND_CHAN_NONE);
      Serial.println("Disconnect!");
			ESP_ERROR_CHECK(esp_wifi_disconnect());
			usleep(300*1000);
			Serial.println("Connect!");
			ESP_ERROR_CHECK(esp_wifi_connect());
			usleep(1500*1000);
			//ESP_ERROR_CHECK(ret);
#else
			for (int chan=1;chan<12;chan++){
				printf("Switching channel to %d with bandwith [None/above/bellow]=%d\n", chan, bandwith);
				//ret=esp_wifi_set_channel(chan, ht_chan);
        esp_wifi_set_channel(chan, ht_chan);
				//ESP_ERROR_CHECK(ret);
        usleep(1*1000*1000);
			}
#endif
      usleep(100*1000);
  }
  }
  //
  }
#else
    if(ready_to_send_data){
        while(WiFi.status() != WL_CONNECTED)
        connectToWiFi(networkName, networkPswd, TIMEOUT_WIFI_CONNECT_S);
      }
#endif
    if(!begin_coleta){
       while(WiFi.status() != WL_CONNECTED)
        connectToWiFi(networkName, networkPswd, TIMEOUT_WIFI_CONNECT_S);
    }
  }
}



void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  
  wakeup_reason = esp_sleep_get_wakeup_cause(); //recupera a causa do despertar

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0  : 
		Serial.println("\n Wakeup caused by external signal using EXT0\n");
	break;
    // case ESP_SLEEP_WAKEUP_EXT0     : Serial.println("\nWakeup caused by external signal using RTC_CNTL\n"); break;
    case ESP_SLEEP_WAKEUP_TIMER    : Serial.println("\nWakeup caused by timer\n"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("\nWakeup caused by touchpad\n"); break;
    case ESP_SLEEP_WAKEUP_ULP      : Serial.println("\nWakeup caused by ULP program\n"); break;
    default : {
		Serial.println("\nWakeup was not caused by deep sleep\n");
#if ENABLE_FW_OTA == 1
		if (update_fw()){
			while(WiFi.status() == WL_CONNECTED){
				//timerWrite(timer, 0); //reset timer (feed watchdog)
				server_ota.handleClient();
			}
		}
#endif
		wakeup_from_reset = true;
		
	}
	 break;
  }	
}