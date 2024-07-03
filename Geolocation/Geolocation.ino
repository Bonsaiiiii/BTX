#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <FS.h>
#include <SPIFFS.h>

AsyncWebServer server(80);

// Constantes das credenciais do WiFi
const char* ssid = "Starlink 2.4GHz"; // Substitua aqui com o SSID de sua rede
const char* password = "32851112"; // Substitua aqui com a SENHA de sua rede
String Latitude; // Variável global do novo ssid que é escolhido no site
String Longitude; // Variável global da nova senha que é escolhida no site
String Precisão; // Para armazenar o da rede conectada em uma String

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi..");
    delay(100);
  }
  //  Configura a taxa de transferência em bits por segundo (baud rate) para transmissão serial
  Serial.println(WiFi.localIP());

  // Inicializa a SPIFFS, formatando-à se houver falha na inicialização
  if (!SPIFFS.begin(true)) {
    Serial.println("Ocorreu um erro ao montar SPIFFS");
    return;
  }
  
  server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/bootstrap.min.css", "text/css");
  });
  server.on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/jquery.min.js", "text/javascript");
  }); 
  server.on("/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/bootstrap.min.js", "text/javascript");
  });

  // disponibiliza o url "/" para que quando acessada, a redireciona para a url "/index.html"
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("Pagina / acessada");
    request->redirect("/index.html"); // redireciona para index.html
  });

  // disponibiliza o url "/index.html" com o conteúdo da página "/index.html" da SPIFFS
  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("Pagina /index.html acessada");
    request->send(SPIFFS, "/index.html", "text/html"); // //Envia página "/index.html" como texto html
  });

    server.on("/", HTTP_POST, [](AsyncWebServerRequest * request) {
    // Verifique se o parâmetro POST existe ou não
    if (request->hasParam("Latitude", true)) {
      AsyncWebParameter* la = request->getParam("Latitude", true);
      //Serial.printf("O parâmetro POST %s existe e possui o valor %s\n", la->name(), la->value());
    if (request->hasParam("Longitude", true)) {
      AsyncWebParameter* lo = request->getParam("Longitude", true);
      //Serial.printf("O parâmetro POST %s existe e possui o valor %s\n", lo->name(), lo->value());
    if (request->hasParam("Precisão", true)) {
      AsyncWebParameter* pr = request->getParam("Precisão", true);
      //Serial.printf("O parâmetro POST %s existe e possui o valor %s\n", lo->name(), lo->value());

    // Armazena os valores recebidos em variáveis
    Latitude = la->value();
    Longitude = lo->value();
    Precisão = pr->value();

    // Imprime as informações recebidas
    Serial.printf("Latitude têm o valor de: %s\n", Latitude);
    Serial.printf("Longitude têm o valor de: %s\n", Longitude);
    Serial.printf("Com a precisão de: %s\n", Precisão, " metros");
    
    }}}
    else {
      Serial.println("Credenciais não encontradas");
    }
    request->send(200);
  });

  // Servidor começa à ouvir os clientes
  server.begin();
}

void loop() {
} 
