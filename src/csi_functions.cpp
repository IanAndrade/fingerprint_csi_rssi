#include "csi_functions.h"
//#include <Arduino.h>
#include "soc/timer_group_struct.h"  //Adicionado para evitar erro do wdt     
#include "soc/timer_group_reg.h"    //Adicionado para evitar erro do wdt
#include "functions.h"
int curChannel = 1;

#define WIFI_COUNTRY() { \
    .cc = "US",\
    .schan = 1,\
    .nchan = 11\
};

//static esp_err_t event_handler(void *ctx, system_event_t *event);

bool first_config = true;

//FingerprintState_t fingerprintState;
// [ADDED] This part of the code i got from an example code to use esp_wifi_80211_tx()
//NOVAS MODIFICAÇÕES CSI
esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq); //[ADDED] This part of the code i got from an example code to use esp_wifi_80211_tx()
esp_err_t ret;
wifi_second_chan_t ht_chan;
static bool can_print=1;

uint8_t beacon_raw[] = {
	0x80, 0x00,							// 0-1: Frame Control
	0x00, 0x00,							// 2-3: Duration
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff,				// 4-9: Destination address (broadcast)
	0xba, 0xde, 0xaf, 0xfe, 0x00, 0x06,				// 10-15: Source address
	0xba, 0xde, 0xaf, 0xfe, 0x00, 0x06,				// 16-21: BSSID
	0x00, 0x00,							// 22-23: Sequence / fragment number
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,			// 24-31: Timestamp (GETS OVERWRITTEN TO 0 BY HARDWARE)
	0x64, 0x00,							// 32-33: Beacon interval
	0x31, 0x04,							// 34-35: Capability info
	0x00, 0x00, /* FILL CONTENT HERE */				// 36-38: SSID parameter set, 0x00:length:content
	0x01, 0x08, 0x82, 0x84,	0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24,	// 39-48: Supported rates
	0x03, 0x0A, 0x0A,						// 49-51: DS Parameter set, current channel 1 (= 0x01),
	0x05, 0x04, 0x01, 0x02, 0x00, 0x00,				// 52-57: Traffic Indication Map
	
};

char *rick_ssids[] = {
	"01 Never gonna give you up",
	"02 Never gonna let you down",
	"03 Never gonna run around",
	"04 and desert you",
	"05 Never gonna make you cry",
	"06 Never gonna say goodbye",
	"07 Never gonna tell a lie",
	"08 and hurt you"
};

// [ADDED] This part of the code i got from an example code to use esp_wifi_80211_tx()
#define BEACON_SSID_OFFSET 38
#define SRCADDR_OFFSET 10
#define BSSID_OFFSET 16
#define SEQNUM_OFFSET 22
#define TOTAL_LINES (sizeof(rick_ssids) / sizeof(char *))
uint8_t line = 0;
// Keep track of beacon sequence numbers on a per-songline-basis
uint16_t seqnum[TOTAL_LINES] = { 0 };

unsigned long time_out_csi = 0;
//[CHANGED] this I pulled out because this was crashing the code when I was using esp like Station. 
//Also I could not connect to an AP (using esp as STA)
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
            ESP_LOGI(TAG, "Got IP: %s\n",
                     ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        default:
            break;
    }
    return ESP_OK;
}



// [/ADDED]

//

#define LEN_MAC_ADDR 20
bool begin_csi_scan = false;
bool send_csi_message = false;
uint64_t ultimo_mac;
uint64_t mac_lido;

uint64_t MAC_KNOWN_TABLE[10]{
  0x009D6B362493, //NOTE9-IAN
  0x981E19AD2E6A, //ROTEADOR CASA
	0x0E017CFE2221,
	0x4CD08A8E5DCA,
	0x4ED08A8E5DCB,
	0x84A423A03D2E,
	0x90B134C5AABB,
	0x802994DEDF90,
	0xF4F26D9766B0,
  0x48F8B3C3BB6B
};
//Tratamento de dados via serial

String recebido="";
String csiMessage = "0";
int conta_csi=0;


/*
 * This function receive all frames, would they contain CSI preamble or not.
 * It gets the content of the frame, not the preamble.
 */

void promi_cb(void *buff, wifi_promiscuous_pkt_type_t type) {
	if (can_print){
		can_print=0;
		wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
		wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
		wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;
		//uint8_t* my_ptr=ipkt;//
		//uint8_t* my_ptr = ipkt->payload; //[CHANGED]: This was not letting compile, but i comment this because i'm not using
		
		char senddMacChr[LEN_MAC_ADDR] = {0}; // Sender
		char recvdMacChr[LEN_MAC_ADDR] = {0}; // Receiver
		sprintf(recvdMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", hdr->addr1[0], hdr->addr1[1], hdr->addr1[2], hdr->addr1[3], hdr->addr1[4], hdr->addr1[5]);
		sprintf(senddMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", hdr->addr2[0], hdr->addr2[1], hdr->addr2[2], hdr->addr2[3], hdr->addr2[4], hdr->addr2[5]);
		
		
		//if (ppkt->rx_ctrl.sig_mode>0){
		//	printf("Received 'ht' packet from %s to %s\n", senddMacChr, recvdMacChr);
		//	printf("0000 ");
		//	for (int i=0;i<ppkt->rx_ctrl.sig_len;i++){
		//		printf("%02x ", my_ptr[i]);
		//	}
		//	printf("\n\n");
		//}
		
	}
	can_print=1;
	
}


/*
 * This function ONLY receive the CSI preamble of frames (if there is any)
 */

void receive_csi_cb(void *ctx, wifi_csi_info_t *data) {
  //TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;//Adicionado para evitar erro do wdt
  //TIMERG0.wdt_feed=1;//Adicionado para evitar erro do wdt
  //TIMERG0.wdt_wprotect=0;//Adicionado para evitar erro do wdt
							
/* 
 * Goal : Get Channel State Information Packets and fill fields accordingly
 * In : Contexte (null), CSI packet
 * Out : Null, Fill fields of corresponding AP
 * 
 */
String id_coleta = fingerprintState.id_coleta;
wifi_csi_info_t received = data[0];
	if (received.rx_ctrl.sig_mode==1){
		time_out_csi = return_end_time(30*1e3);
		Serial.println("Recebi pacote CSI!");
    //if((conta_csi<N_COLETAS) && (begin_csi_scan==true)){
		if((conta_csi<fingerprintState.number_of_scans) && (begin_csi_scan==true)){
			printf("Contador CSI:%d\n",conta_csi);
  	  
                //mac_lido = ((uint64_t)received.mac[0] << 40) |
  	  					//				   ((uint64_t)received.mac[1] << 32) |
  	  					//				   ((uint64_t)received.mac[2] << 24) |
  	  					//				   ((uint64_t)received.mac[3] << 16) |
  	  					//				   ((uint64_t)received.mac[4] << 8) |
  	  					//				   ((uint64_t)received.mac[5]);

      //for(int b=0;b<10;b++){
        //if((mac_lido==MAC_KNOWN_TABLE[b])||(SEM_FILTRO == 1)){  //pode repetir MACs seguidos
        //if(((mac_lido==MAC_KNOWN_TABLE[b])||(SEM_FILTRO == 1))&&(ultimo_mac!=mac_lido)){    //nao deixa repetir MACs seguidos

  	      char senddMacChr[LEN_MAC_ADDR] = {0}; // Sender
  	      // sprintf(senddMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", received.mac[0], received.mac[1], received.mac[2], received.mac[3], received.mac[4], received.mac[5]);
          sprintf(senddMacChr, "%02X%02X%02X%02X%02X%02X", received.mac[0], received.mac[1], received.mac[2], received.mac[3], received.mac[4], received.mac[5]);

          //csiMessage += "Following packet :";
          //csiMessage += "CSI from adress:";
          csiMessage += "#";
					csiMessage += NODENAME;
          csiMessage += ",";
					csiMessage += String(conta_csi);
          csiMessage += ",";
          csiMessage += senddMacChr;
          csiMessage += ",";
          csiMessage += fingerprintState.local_reference;
          csiMessage += ",";
          csiMessage += String(millis());
          csiMessage += ",";
					csiMessage += id_coleta;
          csiMessage += ",";
          csiMessage += String(received.len);
          csiMessage += ",";
          //csiMessage += String(received.last_word_invalid);
					csiMessage += String(received.first_word_invalid);
          csiMessage += ",";
          csiMessage += String(received.rx_ctrl.noise_floor);
          csiMessage += ",";
          csiMessage += String(received.rx_ctrl.rssi);
          csiMessage += ",";
          csiMessage += String(received.rx_ctrl.rate);
          csiMessage += ",";
          csiMessage += String(received.rx_ctrl.sig_mode);
          csiMessage += ",";
          csiMessage += String(received.rx_ctrl.cwb);
          csiMessage += ",";
          csiMessage += String(received.rx_ctrl.stbc);
          csiMessage += ",";
          csiMessage += String(received.rx_ctrl.channel);
          csiMessage += ",";
          csiMessage += String(received.rx_ctrl.secondary_channel);
          csiMessage += ",";
          csiMessage += String(received.rx_ctrl.rx_state);
          csiMessage += ",";
          csiMessage += "\n\r";

  #if DEBUG_CSI == 1
          printf("CSI from adress: %s\n", senddMacChr);
          printf("Length :%d\n",received.len);
          //printf("last four bytes of the CSI data is invalid or not:%d\n",received.last_word_invalid);
					printf("first four bytes of the CSI data is invalid or not:%d\n",received.first_word_invalid);
          printf("noise_floor:%d\n",received.rx_ctrl.noise_floor);
          printf("RSSI:%d\n",received.rx_ctrl.rssi);
  	      printf("rate :%d\n",received.rx_ctrl.rate);
  	      printf("sig_mode :%d -> ",received.rx_ctrl.sig_mode);
  	      if (received.rx_ctrl.sig_mode==0)
  	        printf("non HT(11bg)\n");
  	      if (received.rx_ctrl.sig_mode==1)
  	        printf("HT(11n)\n");
  	      if (received.rx_ctrl.sig_mode==2)
  	        printf("UNKNOWN!!!");
  	      if (received.rx_ctrl.sig_mode==3)
  	        printf("VHT(11ac)\n");
  	      printf("HT20 (0) or HT40 (1) : %d\n",received.rx_ctrl.cwb);
  	      printf("Space Time Block Code(STBC). 0: non STBC packet; 1: STBC packet : %d\n",received.rx_ctrl.stbc);
          printf("channel:%d\n",received.rx_ctrl.channel);
  	      printf("Secondary channel : 0: none; 1: above; 2: below:%d\n",received.rx_ctrl.secondary_channel);
  	      printf("Last word is invalid :%d\n",received.rx_ctrl.rx_state);
  	      printf("Following packet :\n");
  #endif
         
          //uint8_t* my_ptr = (data->buf);
          int8_t* my_ptr = (data->buf);
          //printf("0000 ");
  		    for(int i=0;i<data->len;i++){
  #if DEBUG_CSI == 1 
  		    	printf("%d,", my_ptr[i]);
  #endif
            csiMessage += my_ptr[i];
            csiMessage += ",";
  		    }
  		    printf("\n\n");
					if((conta_csi+1) < fingerprintState.number_of_scans)
          csiMessage += "\n\r";
					 //uint16_t max = received.len;
          //for(uint16_t j=0;j<(max);j++){
          //  wifi_csi_info_t received = data[j];
          //  printf("buffer of CSI data[%d",j);
          //  printf("]:%d\n", *received.buf);
          //}

          ultimo_mac = mac_lido;
          conta_csi++;
          //if(conta_csi==N_COLETAS){
          if(conta_csi== fingerprintState.number_of_scans){
						Serial.println("COLETA FINALIZADAAAAAAAAAAAAAAAAAA!!!!!");
            begin_csi_scan=false;
            send_csi_message = true;
						csiMessage += "\;\n\r";
          }
        //}
      //}
    }
  }
  else{
    //printf("This is invalid CSI until Espressif fix issue https://github.com/espressif/esp-idf/issues/2909\n", received.rx_ctrl.sig_mode); 
  }
}


void setup_wifi_csi(){
if(first_config){
	
} else ESP_ERROR_CHECK( esp_wifi_stop() );

	//ESP_ERROR_CHECK( ret );
    printf("my test\n");
    sleep(1);
    printf("my tes2t\n");
	Serial.println("MAC: " + WiFi.macAddress());
    
    tcpip_adapter_init();

  //ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL)); 
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	cfg.csi_enable = 1;	
	
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA)); // Forced to be in AP mode to send frame ...
	
	wifi_promiscuous_filter_t filer_promi;
	wifi_promiscuous_filter_t filer_promi_ctrl;
	
	uint32_t filter_promi_field=WIFI_PROMIS_FILTER_MASK_ALL;
	
	
	// WIFI_PROMIS_CTRL_FILTER_MASK_ALL == (0xFF800000) 
	uint32_t filter_promi_ctrl_field=(0xFF800000); // By setting it to 0xFFFFFFFF we can catch CSI ?! (but error)
	uint32_t filter_event=WIFI_EVENT_MASK_ALL;
   
	filer_promi.filter_mask = filter_promi_field;
	filer_promi_ctrl.filter_mask = filter_promi_ctrl_field;
	
	esp_wifi_set_promiscuous_filter(&filer_promi);
	esp_wifi_set_event_mask(filter_event);
	esp_wifi_set_promiscuous_ctrl_filter(&filer_promi_ctrl);
	
	//ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
	ESP_ERROR_CHECK(esp_wifi_start());
	esp_wifi_set_promiscuous_rx_cb(promi_cb);
	ESP_ERROR_CHECK(esp_wifi_set_csi(1));

	wifi_csi_config_t configuration_csi; // CSI = Channel State Information
	configuration_csi.lltf_en = 1;
	configuration_csi.htltf_en = 1;
	configuration_csi.stbc_htltf2_en = 1;
	configuration_csi.ltf_merge_en = 1; //[CHANGED]: This was not letting compile because of my framework version that is not updated
	configuration_csi.channel_filter_en = 1; //[CHANGED]: This was not letting compile because of my framework version that is not updated
	configuration_csi.manu_scale = 0; // Automatic scalling
	//configuration_csi.shift=15; // 0->15
	
	ESP_ERROR_CHECK(esp_wifi_set_csi_config(&configuration_csi));
	ESP_ERROR_CHECK(esp_wifi_set_csi_rx_cb(&receive_csi_cb, NULL));
	
	/*	
	[CHANGED] This part I removed because my goal is for ESP to connect to my router and 
	send random packets to receive CSI packets as a response. So I need it to be in Station mode.
	*/
	//wifi_config_t ap_config;
	//ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_AP, &ap_config));
	//ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config)); 
	//ESP_ERROR_CHECK(esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B| WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N));

	//[ADDED] Configuring ESP32 as STA
	wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));
	if(first_config){
		first_config = false;
		//ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	}else{
    strcpy(reinterpret_cast<char*>(wifi_config.sta.ssid), DEFAULT_SSID); //[ADDED] I did this because it was not compiling 
	strcpy(reinterpret_cast<char*>(wifi_config.sta.password), DEFAULT_PWD); //[ADDED] I did this because it was not compiling
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	ESP_ERROR_CHECK(esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B| WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N));
	// ESP_ERROR_CHECK(esp_wifi_start());
	ESP_ERROR_CHECK(esp_wifi_connect());
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
	delay(5000);
	}
}


// void setup_wifi_csi(){
//   Serial.println("Setup_wifi_csi");
//   wifi_country_t wifi_country = WIFI_COUNTRY();
//   //nvs_flash_init();
//   //tcpip_adapter_init();
//   //ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
//   wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//   cfg.csi_enable = 1;	//add
//   esp_wifi_init(&cfg);
//   ESP_ERROR_CHECK( esp_wifi_set_country(&wifi_country) ); /* set country for channel range [1, 11] */
//   esp_wifi_set_storage(WIFI_STORAGE_RAM);
//   esp_wifi_set_mode(WIFI_MODE_APSTA);//WIFI_MODE_NULL); (OLD)
//   esp_wifi_start();
//   esp_wifi_set_promiscuous(true);
//   esp_wifi_set_csi(true);//add
//   esp_wifi_set_promiscuous_rx_cb(promi_cb);
//   esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);

  
//   wifi_csi_config_t configuration_csi; // CSI = Channel State Information
// 	configuration_csi.lltf_en = 1;
// 	configuration_csi.htltf_en = 1;
// 	configuration_csi.stbc_htltf2_en = 1;
// 	//configuration_csi.channel_filter_en = 0;
// 	configuration_csi.manu_scale = 0; // Automatic scalling
// 	//configuration_csi.shift=15; // 0->15
  
//   ESP_ERROR_CHECK(esp_wifi_set_csi_config(&configuration_csi));
//   ESP_ERROR_CHECK(esp_wifi_set_csi_rx_cb(&receive_csi_cb, NULL));
//  // esp_wifi_set_promiscuous_filter(&filt);
//   //esp_wifi_set_promiscuous_rx_cb(&sniffer);
  
//   //esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
//   //wifi_config_t ap_config;
// 	//ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_AP, &ap_config));
// 	//ap_config.ap.ssid_hidden=1; // Never broadcast name
// 	//ap_config.ap.ssid_len=0; // Auto-find the right len
// 	//for (int i=0;i<30;i++)
// 	//	ap_config.ap.ssid[i]=(uint8_t)esp_random(); // Random name
// 	//ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config)); 
// }