#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define FAN 4

long fanTime =0;

#define USE_SERIAL Serial
ESP8266WiFiMulti WiFiMulti;
WebSocketsServer webSocket = WebSocketsServer(81); 
ESP8266WebServer server(80);
String myIP = "";

void switchRelay(int pin, long pinTime){
  long diff = millis() - pinTime;
  if(diff < 3000){
    Serial.println(diff);
    return;
  }
  if(digitalRead(pin) == HIGH){
    digitalWrite(pin, LOW);
  }else {
    digitalWrite(pin, HIGH);
  }
}
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        
                // send message to client
                webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
        {
            USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);
            String device = (char*)payload;
            USE_SERIAL.println(device);
            if(device.equals("pankha")){
              switchRelay(FAN, fanTime);
              fanTime = millis();
            }if(device.equals("Light")){
              switchRelay(LIGHT, lightTime);
              lightTime = millis();
            }
            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
        }
            break;
        case WStype_BIN:
            USE_SERIAL.printf("[%u] get binary length: %u\n", num, length);
            hexdump(payload, length);

            // send message to client
            // webSocket.sendBIN(num, payload, length);
            break;
    }

}



/********************************************************
 *    start_ws(String ws_ip)  Connect to websocket
 ********************************************************/
void start_ws(){  
  webSocket.begin();
  // event handler
  webSocket.onEvent(webSocketEvent);
  Serial.println("Websocket started");
}
/********************************************************
 *    /connectip?server_ip=...  Connect to IP IP
 ********************************************************/
void connectToIp() {
    String message = "Number of args received:";  
    if(server.args()>0){
      message = server.arg(0);
      Serial.print("ConnectToIP : ");
      Serial.println(message);
      start_ws();
    }
    else{
      message= "Error in setting IP";  
    }
    String cnnctPage = "<!DOCTYPE html><html><head> <title>Minion - Configuration</title> <link rel='stylesheet' type='text/css' href='http://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/css/bootstrap.min.css'/> <script src='https://ajax.googleapis.com/ajax/libs/jquery/2.1.3/jquery.min.js'></script> <script type='text/javascript' src='http://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/js/bootstrap.min.js'></script> <script>$(function(){$('.spinner .btn:first-of-type').on('click', function(){var btn=$(this); var input=btn.closest('.spinner').find('input'); if (input.attr('max')==undefined || parseInt(input.val()) < parseInt(input.attr('max'))){input.val(parseInt(input.val(), 10) + 1);}else{btn.next('disabled', true);}}); $('.spinner .btn:last-of-type').on('click', function(){var btn=$(this); var input=btn.closest('.spinner').find('input'); if (input.attr('min')==undefined || parseInt(input.val()) > parseInt(input.attr('min'))){input.val(parseInt(input.val(), 10) - 1);}else{btn.prev('disabled', true);}});}) </script> <style>@import url(http://netdna.bootstrapcdn.com/font-awesome/4.0.3/css/font-awesome.css); .spinner input{text-align: right;}.input-group-btn-vertical{position: relative; white-space: nowrap; width: 2%; vertical-align: middle; display: table-cell;}.input-group-btn-vertical>.btn{display: block; float: none; width: 100%; max-width: 100%; padding: 8px; margin-left: -1px; position: relative; border-radius: 0;}.input-group-btn-vertical>.btn:first-child{border-top-right-radius: 4px;}.input-group-btn-vertical>.btn:last-child{margin-top: -2px; border-bottom-right-radius: 4px;}.input-group-btn-vertical i{position: absolute; top: 0; left: 4px;}</style></head><body> <div class='container'> <div class='page-header'> <h1>Server IP :</h1> <h4>"+ message +"</h4> </div><div class='col-md-12'> <div class='alert alert-info'> <h1>Connected</h1> </div></div></div></body></html>";
    
    server.send(200, "text/html", cnnctPage);
 }

/*********************************************************/
void setup() {
    delay(100); // Wait for sensors to get ready

    Serial.begin(115200);
    /*________________________________________________________________________________________*/
    // We start by connecting to a WiFi network
    WiFiMulti.addAP("HomeX", "122333444456");
        
    Serial.println();Serial.println();
    Serial.print("Wait for WiFi... ");
        
    while(WiFiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
        
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");   //Serial.println(WiFi.localIP());
    IPAddress ipA = WiFi.localIP();
    myIP = String(ipA[0]) + "." + String(ipA[1])  + "." + String(ipA[2])  + "." + String(ipA[3]) ;
    Serial.println(myIP);
    /**************Begin Webserver*****************/
    if (MDNS.begin("esp8266")) {
      USE_SERIAL.println("MDNS responder started");
    }
          
    server.on("/connectip",connectToIp);
          
    server.begin();
          
    // Add service to MDNS
    MDNS.addService("http", "tcp", 80);
    start_ws();
    pinMode(FAN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
}
void loop() {
  
  server.handleClient();
  webSocket.loop();

}