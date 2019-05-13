


/*
 * Login page
 */

#if AWS_AUTO_UPDATE == 1
	#include <Update.h>
#endif

#define LOGIN_STR "<html style="" lang=\"en\">"\
"<head>"\
"<title>Pagina de Login - PHYGITALL</title>"\
"</head>"\
"<body style='background-image: url(http://www.startuprio.rj.gov.br/demoday/images/logos/phy.jpg);width: 100vw;height: 100vh;background-size: cover;'>"\
"<form name='loginForm'>"\
"<br>"\
"<br>"\
"<br>"\
"<br>"\
"<br>"\
"<br>"\
"<table width='20%' align='center' style='background-color: lightblue;'>"\
"<tbody>"\
"<tr>"\
"<td colspan='2'>"\
"<center>"\
"<font size='4'>"\
"<b>PHYGITALL Login Page</b>"\
"</font>"\
"</center>"\
"<br>"\
"</td>"\
"</tr>"\
"<tr>"\
"<td>Usuario:</td>"\
"<td><input type='text' size='25' name='userid'><br></td>"\
"</tr>"\
"<tr><td>Senha:</td>"\
"<td>"\
"<input type='Password' size='25' name='pwd'>"\
"<br>"\
"</td>"\
"</tr>"\
"<tr>"\
"<td>"\
"<input type='submit' onclick='check(this.form)' value='Login'>"\
"</td></tr>"\
"</tbody>"\
"</table>"\
"</form>"\
"<script>function check(form){if(form.userid.value=='" LOGIN_FW_OTA "' && form.pwd.value=='" SENHA_FW_OTA "'){window.open('/serverIndex')}else{ alert('Error Password or Username')/*displays error message*/}}</script>"\
"</body>"\
"</html>"

const char* loginIndex = LOGIN_STR;

 
/*
 * Server Index Page
 */
#define SERVER_STR "<html style="" lang=\"en\"><head>"\
"<title>Pagina de mudança de FW - PHYGITALL</title>"\
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'>"\
"</script></head>"\
"<body style='background-image: url(http://www.startuprio.rj.gov.br/demoday/images/logos/phy.jpg);width: 100vw;height: 100vh;background-size: cover;'>"\
"<h2>Pagina de mudanca de Firmware - PHYLL</h2>"\
"<p>A versao atual da firmware e: " VERSAOFW "</p>"\
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"\
"<input type='file' name='update'><input type='submit' value='Update'>"\
"</form><div id='prg'>progress: 0%</div><header></header>"\
"<script>$('form').submit(function(e){e.preventDefault();var form = $('#upload_form')[0];var data = new FormData(form); $.ajax({url: '/update',type: 'POST',data: data,contentType: false,processData:false,xhr: function() {var xhr = new window.XMLHttpRequest();xhr.upload.addEventListener('progress', function(evt) {if (evt.lengthComputable) {var per = evt.loaded / evt.total;$('#prg').html('progress: ' + Math.round(per*100) + '%');}}, false);return xhr;},success:function(d, s) {console.log('success!')},error: function (a, b, c) {}});});</script>"\
"<p>Email para contato: <a href='mailto:contato@phygitall.rio'>contato@phygitall.rio</a>.</p>"\
"<p>Telefone para contato: (21)4109-0284</p>"\
"<a href='https://phygitall.com.br/'>Acesse a pagina da PHYGITALL</a>"\
"<p><button type='button' onclick= window.open('/deviceParameters')>Clique aqui para ver os parametros do sensor.</button></p>"\
"</body></html>"

const char* serverIndex = SERVER_STR;

//######### DEFINES PARA COLOCAR NA TABELA
#if FAKE_GPS == 0 
	#define GPS_STATUS "HABILITADO"
#else
	#define GPS_STATUS "DESABILITADO"
#endif
#if LORA_ENABLE == 1
	#define LORA_STATUS "HABILITADO"
#else
	#define LORA_STATUS "DESABILITADO"
#endif
#if WIFI_ENABLE == 1
	#define WIFI_STATUS "HABILITADO"
#else
	#define WIFI_STATUS "DESABILITADO"
#endif
#if RFID_ENABLE == 1
	#define RFID_STATUS "HABILITADO"
#else
	#define RFID_STATUS "DESABILITADO"
#endif
#if MPU_ODOMETER_ENABLE == 1
	#define ACELEROMETRO_STATUS "HABILITADO"
#else
	#define ACELEROMETRO_STATUS "DESABILITADO"
#endif
#if ABP == 0
	#define LORA_TYPE "OTAA"
#else
	#define LORA_TYPE "ABP"
#endif
#if LORA_SUB_BAND == 0
	#define LORA_GROUP "GRUPO 1"
#else
	#define LORA_GROUP "GRUPO 2"
#endif
#if ENABLE_MERCURY_SWITCH == 1
	#define SVIB_STATUS "HABILITADO"
#else 
	#define SVIB_STATUS "DESABILITADO"
#endif
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

//#if 
//	#define
//#else 
//	#define 
//#endif
//#if 
//	#define
//#else 
//	#define 
//#endif
//#if 
//	#define
//#else 
//	#define 
//#endif
//#########

#define DEVICE_PARAMETER_STR "<html lang=\"en\"><head>"\
"<title>Pagina de parametros PHYGITALL</title>"\
"<meta charset=\"utf-8\">"\
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"\
"<style>"\
"* {"\
"  box-sizing: border-box;"\
"}"\
""\
"body {"\
"  margin: 0;"\
"}"\
""\
".header {"\
"  background-color: #f1f1f1;"\
"color: #4b72b2;"\
"  padding: 20px;"\
"  text-align: center;"\
"}"\
""\
".topnav {"\
"  overflow: hidden;"\
"  background-color: #4b72b2;"\
"}"\
""\
".topnav a {"\
"  float: left;"\
"  display: block;"\
"  color: #f2f2f2;"\
"  text-align: center;"\
"  padding: 14px 16px;"\
"  text-decoration: none;"\
"}"\
""\
".topnav a:hover {"\
"  background-color: #ddd;"\
"  color: black;"\
"}"\
""\
".column {"\
"  float: left;"\
"  width: 50%;"\
"  padding: 15px;"\
"}"\
""\
".row:after {"\
"  content: \"\";"\
"  display: table;"\
"  clear: both;"\
"}"\
""\
"@media screen and (max-width:600px) {"\
"  .column {"\
"    width: 100%;"\
"  }"\
"}"\
""\
"table {"\
"  width:100%;"\
"}"\
"table, th, td {"\
"  border: 1px solid black;"\
"  border-collapse: collapse;"\
"}"\
"th, td {"\
"  padding: 15px;"\
"  text-align: left;"\
"}"\
"table tr:nth-child(even) {"\
"  background-color: #eee;"\
"}"\
"table tr:nth-child(odd) {"\
" background-color: #fff;"\
"}"\
"table th {"\
"  background-color: #fff;"\
"  color: black;"\
"}"\
"table#t01 tr:nth-child(even) {"\
"  background-color: #eee;"\
"}"\
"table#t01 tr:nth-child(odd) {"\
" background-color: #fff;"\
"}"\
"table#t01 th {"\
"  background-color: #fff;"\
"  color: black;"\
"}"\
"table#t02 tr:nth-child(even) {"\
"  background-color: #eee;"\
"}"\
"table#t02 tr:nth-child(odd) {"\
" background-color: #fff;"\
"}"\
"table#t02 th {"\
"  background-color: #fff;"\
"  color: black;"\
"}"\
"#tab1 {"\
"         display: none;   "\
"        }"\
""\
"        #tab2 {"\
"         display: none;"\
"        }"\
""\
"        #tab3 {"\
"         display: none;"\
"        }"\
"</style>"\
"<style id=\"__web-inspector-hide-shortcut-style__\" type=\"text/css\">"\
".__web-inspector-hide-shortcut__, .__web-inspector-hide-shortcut__ *, .__web-inspector-hidebefore-shortcut__::before, .__web-inspector-hideafter-shortcut__::after"\
"{"\
"    visibility: hidden !important;"\
"}"\
"</style></head>"\
"<body>"\
""\
"<div class=\"header\">"\
"  <h1>PHYGITALL - SOLUÇÕES EM INTERNET DAS COISAS</h1>"\
"  <p>Página de parametros do sensor "NODENAME"</p>"\
"</div>"\
""\
"<div class=\"topnav\">"\
"        <a href=\"#\" id=\"funcionalidade\">Funcionalidades</a>"\
"        <a href=\"#\" id=\"parametro\">Parametros</a>"\
"        <a id=\"chave\" href=\"#\">Chaves LoRa</a>"\
"</div>"\
""\
"<div id=\"tab1\" class=\"row\" style=\"display: none;\">"\
"  <div class=\"column\">"\
"  <h2>Funcionalidades do sensor</h2>"\
"    <table>"\
"<tbody><tr>"\
"<th>Funcionalidade</th>"\
"<th>Status</th> "\
"</tr>"\
"<tr>"\
"<td>GPS</td>"\
"<td>"GPS_STATUS"</td>"\
"</tr>"\
"<tr>"\
"<td>LoRa</td>"\
"<td>"LORA_STATUS"</td>"\
"</tr>"\
"<tr>"\
"<td>WiFi</td>"\
"<td>"WIFI_STATUS"</td>"\
"</tr>"\
"<tr>"\
"<td>RFID</td>"\
"<td>"RFID_STATUS"</td>"\
"</tr>"\
"<tr>"\
"<td>Acelerometro</td>"\
"<td>"ACELEROMETRO_STATUS"</td>"\
"</tr>"\
"<tr>"\
"<td>Chave de vibração</td>"\
"<td>"SVIB_STATUS"</td>"\
"</tr>"\
"</tbody></table>"\
"  </div>"\
"  </div>"\
"  <div class=\"row\" id=\"tab2\" style=\"display: none;\">"\
"  <div class=\"column\">"\
"    <h2>Parametros do sensor</h2>"\
"<table id=\"t01\">"\
"<tbody><tr>"\
"<th>Parametro</th>"\
"<th>Descricao</th>"\
"</tr>"\
"<tr>"\
"<td>Tempo de deepsleep</td>"\
"<td>STR(DEEPSLEEP_TIME_S)</td>"\
"</tr>"\
"<tr>"\
"<td>Grupo de transmissoes LoRa</td>"\
"<td>"LORA_GROUP"</td>"\
"</tr>"\
"<tr>"\
"<td>Tipo de Ativacao LoRa</td>"\
"<td>"LORA_TYPE"</td>"\
"</tr>"\
"<tr>"\
"<td>Login da rede WiFi</td>"\
"<td>"WIFINAME"</td>"\
"</tr>"\
"<tr>"\
"<td>Senha da rede WiFi</td>"\
"<td>"WIFIPASS"</td>"\
"</tr>"\
"</tbody></table>"\
"  </div>"\
"  </div>"\
"    "\
"    <div class=\"row\" id=\"tab3\" style=\"display: block;\">"\
"  <div class=\"column\">"\
"    <h2>Chaves LoRa - "NODENAME"</h2>"\
"<table id=\"t01\">"\
"  <tbody><tr>"\
"    <th>Parametro</th>"\
"    <th>Descricao</th>"\
"  </tr>"\
"  <tr>"\
"    <td>DevEUI</td>"\
"    <td>DEVEUI_STR</td>"\
"  </tr>"\
"  <tr>"\
"    <td>DevADDR</td>"\
"    <td>DEVADDR_STR</td>"\
"  </tr>"\
"  <tr>"\
"    <td>APPKEY</td>"\
"    <td>APPKEY_STR</td>"\
"  </tr>"\
"  <tr>"\
"    <td>APPEUI</td>"\
"    <td>APPEUI_STR</td>"\
"  </tr>"\
"  <tr>"\
"    <td>APPSKEY</td>"\
"    <td>APPSKEY_STR</td>"\
"  </tr>"\
"  <tr>"\
"    <td>NWKSKEY</td>"\
"    <td>NWKSKEY_STR</td>"\
"  </tr>"\
"</tbody></table>"\
"  </div>"\
"  </div>"\
"  "\
"    "\
""\
""\
""\
""\
"<script>var func = document.getElementById('funcionalidade');"\
"func.addEventListener('click', function() {"\
"            document.getElementById('tab1').style.display = 'block';"\
"            document.getElementById('tab2').style.display = 'none';"\
"            document.getElementById('tab3').style.display = 'none';"\
"        });"\
"var para = document.getElementById('parametro');"\
"para.addEventListener('click', function() {"\
"            document.getElementById('tab2').style.display = 'block';"\
"            document.getElementById('tab1').style.display = 'none';"\
"            document.getElementById('tab3').style.display = 'none';"\
"        });    "\
"var chave = document.getElementById('chave');"\
"chave.addEventListener('click', function() {"\
"            document.getElementById('tab3').style.display = 'block';"\
"            document.getElementById('tab2').style.display = 'none';"\
"            document.getElementById('tab1').style.display = 'none';"\
"        });  "\
""\
"    </script>"\
""\
""\
""\
"</body></html>"

//#########
//#define DEVICE_PARAMETER_STR "<html>"\
//"<head>"\
//"<style>"\
//"table {"\
//"  width:30%;"\
//"}"\
//"table, th, td {"\
//"  border: 1px solid black;"\
//"  border-collapse: collapse;"\
//"}"\
//"th, td {"\
//"  padding: 15px;"\
//"  text-align: left;"\
//"}"\
//"table#t01 tr:nth-child(even) {"\
//"  background-color: #eee;"\
//"}"\
//"table#t01 tr:nth-child(odd) {"\
//" background-color: #fff;"\
//"}"\
//"table#t01 th {"\
//"  background-color: #fff;"\
//"  color: black;"\
//"}"\
//"</style>"\
//"</head>"\
//"<body>"\
//"<h2>Tabela de Funcionalidades do sensor "NODENAME"</h2>"\
//"<table id='tab1'>"\
//"  <tr>"\
//"    <th>Funcionalidade</th>"\
//"    <th>Status</th> "\
//"  </tr>"\
//"  <tr>"\
//"    <td>GPS</td>"\
//"    <td>"GPS_STATUS"</td>"\
//"  </tr>"\
//"  <tr>"\
//"    <td>LoRa</td>"\
//"    <td>"LORA_STATUS"</td>"\
//"  </tr>"\
//"  <tr>"\
//"    <td>WiFi</td>"\
//"    <td>"WIFI_STATUS"</td>"\
//"  </tr>"\
//"  <tr>"\
//"    <td>RFID</td>"\
//"    <td>"RFID_STATUS"</td>"\
//"  </tr>"\
//"  <tr>"\
//"    <td>Acelerometro</td>"\
//"    <td>"ACELEROMETRO_STATUS"</td>"\
//"  </tr>"\
//"</table>"\
//"<br>"\
//"<h2>Tabela de Parametros do sensor "NODENAME"</h2>"\
//"<table id='t01'>"\
//"  <tr>"\
//"    <th>Parametro</th>"\
//"    <th>Descricao</th>"\
//"  </tr>"\
//"  <tr>"\
//"    <td>Tempo de deepsleep</td>"\
//"    <td>-</td>"\
//"  </tr>"\
//"  <tr>"\
//"    <td>Grupo de transmissoes LoRa</td>"\
//"    <td>"LORA_GROUP"</td>"\
//"  </tr>"\
//"  <tr>"\
//"    <td>Tipo de Ativacao LoRa</td>"\
//"    <td>"LORA_TYPE"</td>"\
//"  </tr>"\
//"  <tr>"\
//"    <td>Login da rede WiFi</td>"\
//"    <td>"WIFINAME"</td>"\
//"  </tr>"\
//"  <tr>"\
//"    <td>Senha da rede WiFi</td>"\
//"    <td>"WIFIPASS"</td>"\
//"  </tr>"\
//"</table></body></html>"
const char* deviceParameters = DEVICE_PARAMETER_STR;
