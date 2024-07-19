#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include "SPIFFS.h"
#include "FS.h"
#include "ArduinoJson.h"

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
  }
  WiFi.mode(WIFI_AP);
  WiFi.softAP("TESTE-INTERCEPT");
  delay(100);
  Serial.println("wifi iniciado!");
  IPAddress Ip(172, 217, 28, 1);    //setto IP Access Point same as gateway
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/index.html", String(),false);
  });
  server.on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/jquery.min.js", "text/javascript");
  }); 
  server.on("/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/bootstrap.min.js", "text/javascript");
  });
  server.on("/loc", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/loc.json", "text/javascript");
  });
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:

}
