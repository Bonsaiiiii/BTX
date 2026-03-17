// Inclusão das bibliotecas
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

// Instanciação do objeto da classe AsyncWebServer
AsyncWebServer server(80);

// Constantes das credenciais do WiFi
const char* ssid = "aaa"; // Substitua aqui com o SSID de sua rede
const char* password = "12345678"; // Substitua aqui com a SENHA de sua rede

void setup() {

  Serial.begin(115200);
  Serial.println("Firmware iniciado");
  // Conecta-se ao Ponto de acesso com as credenciais fornecidas
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


  // disponoboliza o url "/scan" para que quando acessado, mostre os dados das rede WiFi disponíveis
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

 // Route to handle the POST request
  server.on("/submit", HTTP_POST, [](AsyncWebServerRequest *request){
    // Read the JSON data from the request
    String jsonStr = request->arg("plain");
    Serial.println("Received JSON: " + jsonStr);

    // Parse the JSON data
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, jsonStr);

    // Check for parsing errors
    if (error) {
      Serial.print("Parsing failed: ");
      Serial.println(error.c_str());
      request->send(400, "text/plain", "Failed to parse JSON");
      return;
    }

    // Extract the data from the JSON object
    const char* senha = doc["senha"];

    // Do something with the data (e.g., print it)
    Serial.println("First name: " + String(senha));

    // Send a response back to the client
    request->send(200, "text/plain", "Data received successfully");
  });

  // Servidor começa à ouvir os clientes
  server.begin();
}

void loop() {
delay(10);  
}
