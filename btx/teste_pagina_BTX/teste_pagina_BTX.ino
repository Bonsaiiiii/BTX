#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "DNSServer.h"
#include "AsyncTCP.h"
#include "SPIFFS.h"
#include "FS.h"
#include "ArduinoJson.h"
#include <ArduinoUniqueID.h>

DNSServer dnsServer;
AsyncWebServer server(80);


class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    //request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html"); 
  }
};

void setup() {
  Serial.begin(115200);
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
  }
  WiFi.mode(WIFI_AP); 
  WiFi.softAP("HugenPLUS-RADIO",NULL,7,0,1);   //launch the access point
  Serial.println("Wait 100 ms for AP_START...");
  delay(100);
  //Serial.println("Setting the AP");
  IPAddress Ip(172, 217, 28, 1);    //setto IP Access Point same as gateway
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/logo.jpg", "image/jpg");
  });
  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "Erro 404!");
  });
  server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/bootstrap.min.css", "text/css");
  });
  server.on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/jquery.min.js", "text/javascript");
  }); 
  server.on("/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/bootstrap.min.js", "text/javascript");
  });
  server.on("/radio_conf.json", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/parametros.json", "text/javascript");
  });
  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("Pagina /scan acessada");
    String json = "["; // é aberto um colchete, para criação de um objeto JSON
    int varredura = WiFi.scanComplete(); // Obtém o resultado da varredura assíncrona
    if (varredura == -2)
    {
      if (WiFi.scanComplete() == -2) // Se a varredura não foi acionada (-2), ...
      {
        WiFi.scanNetworks(true); // Inicia a varredura em segundo plano e a função será encerrada sem aguardar o resultado (assíncrono), ...
      }
    } else if (varredura) // se varredura NÃO estiver ainda em andamento (-1) ela estará concluída e com o número de redes descobertas, ...
    {
      for (int i = 0; i < varredura; ++i) // faz obter os dados de todas as redes descobertas
      {
        if (i) json += ","; // se o índice da rede descoberta não for igual a 0 (tem de ser positivo), é adicioando uma vírgula no objeto JSON
        json += "{"; // abre uma chave, para criação de um array JSON
        json += "\"rssi\":" + String(WiFi.RSSI(i)); // passa o dado de potência do sinal
        json += ",\"ssid\":\"" + WiFi.SSID(i) + "\""; // passa o dado de nome da rede
        json += ",\"bssid\":\"" + WiFi.BSSIDstr(i) + "\""; // passa o MAC da rede
        json += ",\"channel\":" + String(WiFi.channel(i)); // passa o número do canal da rede
        json += ",\"secure\":" + String(WiFi.encryptionType(i)); // passa o tipo de encriptação da rede
        json += "}"; // fecha uma chave, da criação do array JSON
      }

      WiFi.scanDelete(); // Exclua o último resultado da verificação da memória.
      if (WiFi.scanComplete() == -2) // Se a varredura não está acionada (-2), ...
      {
        WiFi.scanNetworks(true); // Inicia a varredura em segundo plano e a função será encerrada sem aguardar o resultado (assíncrono), ...
      }
      Serial.println("Varredura acionada estágio 2");
    }
    json += "]";// é fechado um colchete, da criação do objeto JSON
    request->send(200, "application/json", json); // envia a resposta do tipo JSON com o código HTTP 200 (resposta de sucesso)
    json = String(); // limpa o objeto JSON, passando uma String vazia
  });
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
    
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();

}
