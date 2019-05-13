#include "WiFi_Functions.h"
#include "functions.h"

#if ENABLE_FW_OTA == 1
	#include "EEPROM.h"
	#include "html_ota_codes.h"
	const char* ssid_OTA_UPDATE = WIFIOTA;
	const char* password_OTA_UPDATE = WIFIPASSOTA;
	#if AWS_AUTO_UPDATE == 1
		bool read_eeprom = false;
		String version_id;
		bool igual = true;
		int addr_eeprom = 0;
		#define EEPROM_SIZE 32
		char fw_atual_version[EEPROM_SIZE];
		char fw_new_version[EEPROM_SIZE];
		WiFiClient client_aws;
		// Variables to validate response from S3
		int contentLength = 0;
		bool isValidContentType = false;
		// S3 Bucket Config
		String host_aws = "firmwarephyll.s3-website-sa-east-1.amazonaws.com";//
		int port_aws = 80;// Non https. For HTTPS 443. As of today, HTTPS doesn't work.
		#define FW_BIN "/" NODENAME ".bin"
		String bin = FW_BIN; // bin file name with a slash in front.
	#endif
#endif
const char* host = HOSTNAME;//node_name.c_str();
boolean connected = false;
//The udp library class
WiFiUDP udp;
//int len;
static WiFiServer server(TCP_PORT);
//static WiFiClient serverClients[MAX_SRV_CLIENTS];
//static WiFiMulti wifiMulti;
FingerprintState_t fingerprintState;

#if ENABLE_FW_OTA == 1
bool update_fw()
{
	connectToWiFi(ssid_OTA_UPDATE, password_OTA_UPDATE,TIME_WAITING_FOR_CONNECT_FW_OTA_S);
    if (WiFi.status() == WL_CONNECTED)
    {
		#if AWS_AUTO_UPDATE == 1
		// Execute AWS S3 OTA Update
  			execOTA();
		#endif
    	// WiFi.setHostname(host);
        // if (!MDNS.begin(host))
        // { //http://esp32.local
        //     Serial.println("Error setting up MDNS responder!");
        //     return false;
        // }
        Serial.println("mDNS responder started");
        /*return index page which is stored in serverIndex */
        server_ota.on("/", HTTP_GET, []() {
            server_ota.sendHeader("Connection", "close");
            server_ota.send(200, "text/html", loginIndex);
        });
        server_ota.on("/serverIndex", HTTP_GET, []() {
            server_ota.sendHeader("Connection", "close");
            server_ota.send(200, "text/html", serverIndex);
        });
		server_ota.on("/deviceParameters", HTTP_GET, []() {
            server_ota.sendHeader("Connection", "close");
            server_ota.send(200, "text/html", deviceParameters);
        });
        /*handling uploading firmware file */
        server_ota.on("/update", HTTP_POST, []() {
        server_ota.sendHeader("Connection", "close");
        server_ota.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart(); }, []() {
        HTTPUpload& upload = server_ota.upload();
        if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
            Update.printError(Serial);
        }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
        /* flashing firmware to ESP*/
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
        } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
            Update.printError(Serial);
        }
        } });
        server_ota.begin();
		return true;
    }
    return false;
}
	#if AWS_AUTO_UPDATE == 1
		// Utility to extract header value from headers
		String getHeaderValue(String header, String headerName) {
		  return header.substring(strlen(headerName.c_str()));
		}
		// OTA Logic 
		bool execOTA() {
		  
		  Serial.println("Connecting to: " + String(host_aws));
		  // Connect to S3
		  if (client_aws.connect(host_aws.c_str(), port_aws)) {
			// Verifico o ID versão da firmware atual que está na EEPROM
			
			
  			if (!EEPROM.begin(EEPROM_SIZE) && (!read_eeprom))
  			{
  			  Serial.println("failed to initialise EEPROM"); 
				//delay(1000000);
			}else{
			Serial.println("Iniciando a leitura da EEPROM do ID da versão da firmware ATUAL");
  			Serial.print("o ID da versão atual é:");
  			for (int i = 0; i < EEPROM_SIZE; i++)
  			{
				  fw_atual_version[i] = (char)EEPROM.read(i);
				  Serial.print(fw_atual_version[i]); Serial.print(" ");
  			}
				read_eeprom =true;
				Serial.println();
			}
  			//
		    // Connection Succeed.
		    Serial.println("Fetching Bin: " + String(bin));// Fecthing the bin
		    // Get the contents of the bin file
		    client_aws.print(String("GET ") + bin + " HTTP/1.1\r\n" +
		                 "Host: " + host_aws + "\r\n" +
		                 "Cache-Control: no-cache\r\n" +
		                 "Connection: close\r\n\r\n");
		    // Check what is being sent
		    //    Serial.print(String("GET ") + bin + " HTTP/1.1\r\n" +
		    //                 "Host: " + host + "\r\n" +
		    //                 "Cache-Control: no-cache\r\n" +
		    //                 "Connection: close\r\n\r\n");
		    unsigned long timeout = millis();
		    while (client_aws.available() == 0) {
		      if (millis() - timeout > 5000) {
		        Serial.println("client_aws Timeout !");
		        client_aws.stop();
		        return false;
		      }
		    }
		    
		    while (client_aws.available()) {
		      String line = client_aws.readStringUntil('\n');
			  
		      line.trim(); // remove space, to check if the line is end of headers
		      // if the the line is empty,
		      // this is end of headers
		      // break the while and feed the
		      // remaining `client_aws` to the
		      // Update.writeStream();
		      if (!line.length()) {
		        //headers ended
		        break; // and get the OTA started
		      }
				
				//Serial.println(line);Serial.println(); //debug da resposta recebida do servidor
		      // Check if the HTTP Response is 200 else break and Exit Update
		      if (line.startsWith("HTTP/1.1")) {
		        if (line.indexOf("200") < 0) {
		          Serial.println("Got a non 200 status code from server. Exiting OTA Update.");
				  client_aws.stop();
				  return false;
		        }
		      }
			if (line.startsWith("x-amz-version-id: ")) {
		        version_id = getHeaderValue(line, "x-amz-version-id: ");
		        Serial.println("A versao atual na AWS é: " + version_id);
				strcpy(fw_new_version, version_id.c_str());
				
				for (int i = 1; i < EEPROM_SIZE; i++){
					//Serial.print("fw_atual_version: "); Serial.println(fw_atual_version[i]);
					//Serial.print("fw_new_version: "); Serial.println(fw_new_version[i]);
					if(fw_new_version[i] == fw_atual_version[i]) {
						//Serial.println("igual!");
					} else {
						//Serial.println("diferente!");
						igual = false;
					}
					
				}
				if(igual) {
					Serial.println("A versao ja e a mais atualizada!!!");
				}
				else Serial.println("Versao nova encontrada!! Sera feita uma atualizacao!!");
			}
		      // extract headers here
		      // Start with content length
		      if (line.startsWith("Content-Length: ")) {
		        contentLength = atoi((getHeaderValue(line, "Content-Length: ")).c_str());
		        Serial.println("Got " + String(contentLength) + " bytes from server");
		      }
		      // Next, the content type
		      if (line.startsWith("Content-Type: ")) {
		        String contentType = getHeaderValue(line, "Content-Type: ");
		        Serial.println("Got " + contentType + " payload.");
		        if (contentType == "application/octet-stream") {
		          isValidContentType = true;
		        }
		      }
		    }
		  } else {
		    // Connect to S3 failed
		    // May be try?
		    // Probably a choppy network?
		    Serial.println("Connection to " + String(host) + " failed. Please check your setup");
			return false;
		    // retry??
		    // execOTA();
		  }
		  // Check what is the contentLength and if content type is `application/octet-stream`
		  Serial.println("contentLength : " + String(contentLength) + ", isValidContentType : " + String(isValidContentType));
		  // check contentLength and content type
		  if (contentLength && isValidContentType && !igual) {
		    // Check if there is enough to OTA Update
		    bool canBegin = Update.begin(contentLength);
			
		    // If yes, begin
		    if (canBegin) {
		      Serial.println("Begin OTA. This may take 2 - 5 mins to complete. Things might be quite for a while.. Patience!");
		      // No activity would appear on the Serial monitor, So be patient. This may take 2 - 5mins to complete 
		      size_t written = Update.writeStream(client_aws);
		      if (written == contentLength) {
		        Serial.println("Written : " + String(written) + " successfully");
		      } else {
		        Serial.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?" );
		        // retry??
		        // execOTA();
		      }
		      if (Update.end()) {
		        Serial.println("OTA done!");
		        if (Update.isFinished()) {
		          	Serial.println("Update successfully completed. Rebooting.");
					// advance to the next address.  there are 512 bytes in the EEPROM, so go back to 0 when we hit 512. save all changes to the flash.
					//Escreve o ID da versão da firmware atual na EEPROM
					//Serial.print("Qtd de caracteres do ID versao:"); Serial.println(version_id.length());
					for(int i = 0; i < (version_id.length());i++){	
  						EEPROM.write(i, byte(fw_new_version[i]));
  						//Serial.print(fw_new_version[i]); Serial.print(" ");
					}
					EEPROM.commit();
					
  					Serial.print((version_id.length()));
  					Serial.println(" bytes written on Flash . Values are:");
  					for (int i = 0; i < (version_id.length()); i++)
  					{
  					  Serial.print((char)EEPROM.read(i)); Serial.print(" ");
					//Serial.print(byte(EEPROM.read(i))); Serial.print(" ");
  					}
  					Serial.println(); Serial.println("----------------------------------");
		          	ESP.restart();
		        } else {
		          Serial.println("Update not finished? Something went wrong!");
		        }
		      } else {
		        Serial.println("Error Occurred. Error #: " + String(Update.getError()));
		      }
		    } else {
		      // not enough space to begin OTA, Understand the partitions and, space availability
		      Serial.println("Not enough space to begin OTA");
		      client_aws.flush();
		    }
		  } else {
			if(!igual) Serial.println("There was no content in the response");
		    //client_aws.flush();
			//delay(500);
			client_aws.stop();
			return false;
		  }
		}
	#endif
#endif



bool SendTCPMessage(IPAddress IPToSend, unsigned int PortToCommunicate, String MessageToSend, bool stop_connection_after = true)
{
#define DEBUG_EN 1
	bool TCPStatus = false;
	WiFiClient client;
	if (client.connected())
	{
		client.print(MessageToSend);
		TCPStatus = true;
	}
	else if (client.connect(IPToSend, PortToCommunicate))
	{
#if DEBUG_EN > 0
		Serial.println("connected!");
#endif
#if DEBUG_EN > 1
		Serial.println("Message:");
		Serial.println(MessageToSend);
#endif
		client.print(MessageToSend);
		TCPStatus = true;
	}
#if DEBUG_EN > 0
	else
		Serial.println("Can't Send TCP Msg!");
#endif
	if (stop_connection_after) 
		client.stop();
	
	delay(100);
	
	return TCPStatus;
}

// Funcao de ennvio de dados via UDP
void SendUDPMessage(IPAddress IPToSend, unsigned int PortToSend, String MessageToSend)
{
	udp.beginPacket(IPToSend, PortToSend);
	udp.println(MessageToSend);
	udp.endPacket();
}

void generateWiFiAP(const char *ssid, const char *pwd, int channel = 1, int ssid_hidden = 0, int max_connection = 4)
{
#define DEBUG_EN 1
    WiFi.softAP(ssid, pwd, channel, ssid_hidden, max_connection);//,1,1);
    IPAddress myIP = WiFi.softAPIP();
#if DEBUG_EN > 0
    Serial.print("AP IP address: ");
    Serial.println(myIP);
#endif
    connected = true;
}

bool connectToWiFi(const char *ssid, const char *pwd,unsigned long time_out_seconds)
{
#define DEBUG_EN 1
	WiFi.mode(WIFI_STA);
	delay(100);
#if DEBUG_EN > 0
	Serial.println("Connecting to WiFi network: " + String(ssid));
#endif
	WiFi.disconnect(true);
	WiFi.begin(ssid, pwd);
	WiFi.setHostname(host);
        
    unsigned long timeout = millis();
    
    while (WiFi.status() != WL_CONNECTED) {
        //if (millis() - timeout > (TIMEOUT_WIFI_CONNECT_S*1e3)){
		if (millis() - timeout > (time_out_seconds*1e3)){
#if DEBUG_EN > 0
            Serial.println("WiFi not connected.");
#endif
            connected = false;
            return false;
        }
        delay(500);
#if DEBUG_EN > 0
        Serial.print(".");
#endif
    }
    connected = true;
#if DEBUG_EN > 0
    Serial.println();
    Serial.print("WiFi connected! IP address: ");
	Serial.println(WiFi.localIP());
#endif
    return true;
}

void connectToWiFiEvent(const char *ssid, const char *pwd)
{
#define DEBUG_EN 1
	WiFi.mode(WIFI_STA);
	// delay(100);
	vTaskDelay(100);
#if DEBUG_EN > 0
	Serial.println("Connecting to WiFi network: " + String(ssid));
	Serial.println("Waiting for WIFI connection...");
#endif
	WiFi.disconnect(true);
	WiFi.onEvent(WiFiEvent);
	WiFi.begin(ssid, pwd);
}
void disconnectToWiFi()
{
	WiFi.disconnect(true);
	WiFi.mode(WIFI_OFF);
	connected = false;
}

void WiFiEvent(WiFiEvent_t event)
{
#define DEBUG_EN 1	
	switch (event)
	{
	case SYSTEM_EVENT_STA_GOT_IP:
#if DEBUG_EN > 0
		Serial.print("WiFi connected! IP address: ");
		Serial.println(WiFi.localIP());
#endif
		connected = true;
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
#if DEBUG_EN > 1
		Serial.println("WiFi lost connection");
#endif
        WiFi.reconnect();
		connected = false;
		break;
	default: break;	
	}
}
void Start_tcp_server(uint16_t port = TCP_PORT)
{
#define DEBUG_EN 1	
	server.begin();
	server.setNoDelay(true);
#if DEBUG_EN > 0
	Serial.print("Ready! Use 'telnet ");
	Serial.print(WiFi.localIP());
	Serial.print(" port ");
	Serial.print(port);
	Serial.println(" to connect");
#endif
}

bool ReceiveUDPMessage(unsigned int PortToCommunicate) {
  // buffer for incoming packets
  int packetSize = udp.parsePacket();
  if (packetSize)
  {
    // receive incoming udp packets
    len = udp.read(receivePacket, 255);
    if (len > 0)
    {
      receivePacket[len] = 0;
    }
    Serial.print("recebido: ");
    Serial.println(receivePacket);
    return true;
  } else return false;
}

bool ReceiveTCPMessage(unsigned int PortToCommunicate) {
  // buffer for incoming packets
	//client.read(receivePacket,255);
  //int packetSize = client.parsePacket();
	String receiveMsg = client.readStringUntil('\r');
	int packetSize = receiveMsg.length();
  if (packetSize)
  {
    // receive incoming udp packets
    len = receiveMsg.length();
    //if (len > 0)
    //{
    //  receivePacket[len] = 0;
    //}
		receiveMsg.toCharArray(receivePacket,len);
    Serial.print("recebido: ");
    Serial.println(receivePacket);
    return true;
  } else return false;
}

msg_types_t wait_for_client()
{
#define DEBUG_EN 2

	msg_types_t typeMsg = type_unknown;
	uint16_t n_msg = 0;
	
	if (client)
	{								   
#if DEBUG_EN > 0
		Serial.println("New Client."); 
#endif
		char strbuf[MAX_BUF_SIZE];	  
		//unsigned long timeout_client = 0;
#if DEBUG_EN > 1
		//Serial.print("Received data from: ");
		//Serial.print(TCP_Client.IP);
		//Serial.print(":");
		//Serial.println(TCP_Client.port);
#endif		
		//timeout_client = return_end_time(TIMEOUT_CLIENT_TCP); 
		// while (client.connected())
		//while (client.connected()&&(millis()<timeout_client))
		while (client.connected())
		{ 
			// if (millis()>timeout_client) break;
			while (client.available()) //&&(millis()<timeout_client))
			{							
				char c = client.read(); 
				Serial.write(c);		
				if ((c == '\n')||(c == '#'))
				{
					strbuf[n_msg] = '\0';
                    typeMsg = getMsgType(strbuf, n_msg);
					if (typeMsg == type_unknown)
					{
						
						#define ERRO_MSG "$ERROR," NODENAME ",\n"
						client.println(ERRO_MSG);
						//client.println("$ERROR");
#if DEBUG_EN > 0
						Serial.println(ERRO_MSG);
#endif
					}
					else if (typeMsg == type_reset)
					{
						client.println("$ACK_RESET");
						delay(100);
						client.flush();
						client.stop();
#if DEBUG_EN > 0
						Serial.println("$ACK_RESET");
						delay(250);
#endif
						esp_restart();
						delay(1000);	
					}
					else if(typeMsg == type_begincoleta){
						#define ACK_COLETA "$ACK_STARTCOLETA," NODENAME ",\n"
						client.println(ACK_COLETA);
						Serial.println(ACK_COLETA);
						//client.println("$ACK_STARTCOLETA");
						delay(100);
						client.println(ACK_COLETA);
						if(NODENAME == "FRENTE") delay(100);
    					else if(NODENAME == "DIR") delay(200);
    					else if(NODENAME == "ESQ") delay(300);
   				 		else if(NODENAME == "TRAS") delay(400);
						client.println(ACK_COLETA);
						// client.flush();
						// client.readStringUntil('\r');
						client.stop();
						delay(100);
						return type_begincoleta;

					}
#if (ENABLE_FW_OTA == 1 && AWS_AUTO_UPDATE == 1)
					else if (typeMsg == type_updatefw)
					{
						client.println("$ACK_UPDATEFW");
						Serial.println("$ACK_UPDATEFW");
						delay(100);
						client.flush();
						client.stop();
						//flagupdatefw = true;
						//if (xSemaphoreTake(SensorState.semaphore, 1000) == pdTRUE)
    				//	{  
						//	SensorState.updateRequest = true;
						//    xSemaphoreGive(SensorState.semaphore);
    				//	} else Serial.println("[ERROR] SensorState.semaphore");

						execOTA();
					}
#endif
					else if(typeMsg == type_sensorstatus){
							Serial.println("Recebi MSG de status.");
						return typeMsg;
					}
					else if(typeMsg == type_requestdata){
							Serial.println("Recebi MSG de REQUEST DATA.");
							printf("fingerprintState.send_data_again: %d",fingerprintState.send_data_again);
						return typeMsg;
					}					
					else
					{
						//TCP_Client.received = true;
						client.println("$ACK_WIFI");

#if DEBUG_EN > 0
						Serial.println("$ACK_WIFI");
#endif
					}
					client.flush();
					
					client.stop();
#if DEBUG_EN > 0
					Serial.println("Client Disconnected.");
#endif
				}
				else if (c != '\r')
				{					   
					
					if (n_msg < MAX_BUF_SIZE){
						strbuf[n_msg] = c; 
						n_msg++;
					}
					else 
					{
#if DEBUG_EN > 0
						Serial.println();
						Serial.println("$OVERFLOW");
						Serial.println("Client Disconnected.");
#endif
						client.println("$OVERFLOW");
						client.flush();
						client.stop();
						return type_unknown;
					}

				}
			}
			//vTaskDelay(10);
		}
		//close the connection:
		client.stop();
#if DEBUG_EN > 0
		Serial.println("Client Disconnected.");
#endif
	}
	return typeMsg;
}

msg_types_t getMsgType(char strbuf[MAX_BUF_SIZE], int lenstrbuf)
{
	String argstrbuf;
	char aux_type[20];
	int i = 0;
	int k = 0;
	int n_msg;
	uint8_t countSeparator = 0;
	Serial.print("DEBUG strbuf: ");
	Serial.println(strbuf);
	while (strbuf[i] != ',')
	{
        aux_type[i] = strbuf[i];
		i++;
        if (i>lenstrbuf) return type_unknown;
	}
	aux_type[i] = ',';
	aux_type[i + 1] = '\0';
	argstrbuf = String(aux_type);
	Serial.print("DEBUG argstrbuf: ");
	Serial.println(argstrbuf);
	if (argstrbuf.startsWith("$LORATXT,"))
	{
		n_msg = lenstrbuf - argstrbuf.length();
        Serial.print("n_msg=");
		Serial.println(n_msg);
		for (i = 0; i < lenstrbuf; i++)
		{
			if (countSeparator < 3)
			{
				if (strbuf[i] == ',')
				{
					countSeparator++;
					switch (countSeparator)
					{
					case 1:
						argstrbuf = "";
						break;
					case 2:
						argstrbuf = "";
						break;
					case 3:
						//msg_loratxt.msg_nr = argstrbuf.toInt();
						argstrbuf = "";
						break;
					}
				}
				else
					argstrbuf += strbuf[i];
			}
			else
			{
				//msg_loratxt.data[k] = strbuf[i];
				k++;
			}
		}
		//msg_loratxt.data_len = k;
		        
		// if (k < MAX_LORATXT)
		// {
		// 	send_lora_message = true;
		// 	if (k > 8)
		// 	{
		// 		send_lora_message_incomplete = true;
		// 	}
		// }
		// else
		// {
		// 	send_lora_message_incomplete = false;
		// 	send_lora_message = false;
		// }
		// Serial.print("DEBUG data: ");
		// Serial.println(msg_loratxt.data);
		// Serial.print("DEBUG msg_nr: ");
		// Serial.println(msg_loratxt.msg_nr);
		// Serial.print("DEBUG data_len: ");
		// Serial.println(msg_loratxt.data_len);
		return type_loratxt;
	}
	else if(argstrbuf.startsWith("$RESET,"))
  {
		return type_reset;
	}
  else if(argstrbuf.startsWith("$UPDATEFW,"))
  {
		return type_updatefw;
	}
	else if(argstrbuf.startsWith("$BEGINCOLETA,"))
  {
		for (i = 0; i < lenstrbuf; i++)
		{
			// if (countSeparator < 3)
			if (countSeparator < 5)
			{
				if (strbuf[i] == ',')
				{
					countSeparator++;
					switch (countSeparator)
					{
					case 1:
						argstrbuf = "";
						break;
					case 2:
						Serial.print("[DEBUG number_of_scans]:");Serial.println(argstrbuf);
						fingerprintState.number_of_scans = argstrbuf.toInt();
						argstrbuf = "";
						break;
					case 3:
						Serial.print("[DEBUG local_reference]:");Serial.println(argstrbuf);
						fingerprintState.local_reference = argstrbuf;
						//msg_loratxt.msg_nr = argstrbuf.toInt();
						argstrbuf = "";
						break;
					case 4:
						Serial.print("[DEBUG roda_meters]:");Serial.println(argstrbuf);
						fingerprintState.roda_distance = argstrbuf;
						argstrbuf = "";
						break;
					case 5:
						Serial.print("[DEBUG ID_Coleta]:");Serial.println(argstrbuf);
						fingerprintState.id_coleta = argstrbuf;
						argstrbuf = "";
						break;
					}
				}
				else
					argstrbuf += strbuf[i];
			}
			//else
			//{
			//	//msg_loratxt.data[k] = strbuf[i];
			//	k++;
			//}
		}
		return type_begincoleta;
	} else if(argstrbuf.startsWith("$REQUESTDATA,")){
		//fingerprintState.send_data_again
		for (i = 0; i < lenstrbuf; i++)
		{
			// if (countSeparator < 3)
			if (countSeparator < 5)
			{
				if (strbuf[i] == ',')
				{
					countSeparator++;
					switch (countSeparator)
					{
					case 1:
						argstrbuf = "";
						break;
					case 2:
						if(NODENAME == "FRENTE"){
							fingerprintState.send_data_again = argstrbuf.toInt();
							printf("Sou o FRENTE, REENVIAR DATA: %d",fingerprintState.send_data_again);
						}
						argstrbuf = "";
						break;
					case 3:
						if(NODENAME == "DIR"){
							fingerprintState.send_data_again = argstrbuf.toInt();
							printf("Sou o DIR, REENVIAR DATA: %d",fingerprintState.send_data_again);
						}
						argstrbuf = "";
						break;
					case 4:
						if(NODENAME == "ESQ"){
							fingerprintState.send_data_again = argstrbuf.toInt();
							printf("Sou o ESQ, REENVIAR DATA: %d",fingerprintState.send_data_again);
						}
						argstrbuf = "";
						break;
					case 5:
						if(NODENAME == "TRAS"){
							fingerprintState.send_data_again = argstrbuf.toInt();
							printf("Sou o TRAS, REENVIAR DATA: %d",fingerprintState.send_data_again);
						}
						argstrbuf = "";
						break;
					}
				}
				else
					argstrbuf += strbuf[i];
			}
		}

		return type_requestdata;
	}
	else if((argstrbuf.startsWith("$CONNECTED,")) || (argstrbuf.startsWith("$STATUSCOLETA,"))){

		return type_sensorstatus;
	}
	else
	{
		return type_unknown;
	}
}