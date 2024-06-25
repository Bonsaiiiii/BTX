#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ESP32Ping.h>
#include "ArduinoJson.h"

#define WifiS "wifi_status"
#define size_wifis 256

String Conectado, Sshield, Espera, Sredes, Completo, Falha, ConexPerdida, Desconectado;

char cConectado[10]; char cSshield[10]; char cEspera[10]; char cSredes[10]; char cCompleto[10]; char cFalha[10]; char cConexPerdida[10]; char cDesconectado[10];

AsyncWebServer server(80);

bool loaddata(String filename){
  if (SPIFFS.begin(false) || SPIFFS.begin(true)){
    if (SPIFFS.exists(filename)){
      if(filename=="/wifi_status.json"){
        File configFile = SPIFFS.open(filename, "r");
          if (configFile){
            StaticJsonDocument<size_wifis> json;
            DeserializationError error = deserializeJson(json, configFile);
            if (!error){
              Conectado = strcpy(cConectado, json["Conectado"]);
              Sshield = strcpy(cSshield, json["Sshield"]);
              Espera = strcpy(cEspera, json["Espera"]);
              Sredes = strcpy(cSredes, json["Sredes"]);
              Completo = strcpy(cCompleto, json["Completo"]);
              Falha = strcpy(cFalha, json["Falha"]);
              ConexPerdida = strcpy(cConexPerdida, json["ConexPerdida"]);
              Desconectado = strcpy(cDesconectado, json["Desconectado"]);
              return true;
          }else{
              Serial.println("Failed to load wifi status");
          }
        }
      }
    }
  }
  return false;
}    

void saveConfigFile(String filename){
  if(filename=="/wifi_status.json"){
    StaticJsonDocument<size_wifis> json;
    json["Conectado"]    = Conectado.c_str();
    json["Sshield"]   = Sshield.c_str();
    json["Espera"]   = Espera.c_str();
    json["Sredes"]   = Sredes.c_str();
    json["Completo"]   = Completo.c_str();
    json["Falha"]   = Falha.c_str();
    json["ConexPerdida"]   = ConexPerdida.c_str();
    json["Desconectado"]   = Desconectado.c_str();
    File configFile = SPIFFS.open(filename, "w");
    if (!configFile){
      Serial.println("failed to open radio config file for writing");
    }
    if (serializeJson(json, configFile) == 0){
      Serial.println(F("Failed to write to file"));
    }
    configFile.close();
  }
} 

// Constantes das credenciais do WiFi
const char* ssid = "aoba"; // Substitua aqui com o SSID de sua rede
const char* password = "aobaaoba"; // Substitua aqui com a SENHA de sua rede
String novoSsid; // Variável global do novo ssid que é escolhido no site
String novaSenha; // Variável global da nova senha que é escolhida no site
IPAddress ipLocal; // Armazena o IP da nova rede para realizar um ping e verificar se foi conectado
String ipNovaRede; // Para armazenar o da rede conectada em uma String
bool flag_novarede=false; // Variável para zerar e definir a flag nova rede
bool flag_erro=false; // Variável para zerar e definit a flag erro
int estado=0; // Variável para zerar o estado da rede
char* remoteHost = "www.google.com";
bool flag_rede_indisponivel=false;
String novoPing;
const char* aoba = "aoba";

void setup() {
    Serial.begin(115200);
    Serial.println("\n[*] Creating AP");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial.print("[+] AP Created with IP Gateway ");
    Serial.println(WiFi.softAPIP());
    Serial.println(WiFi.localIP());

  // Inicializa a SPIFFS, formatando-à se houver falha na inicialização
  if (!SPIFFS.begin(true)) {
    Serial.println("Ocorreu um erro ao montar SPIFFS");
    return;
  }
  
  loaddata(WifiS);

  server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/bootstrap.min.css", "text/css");
  });
  server.on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/jquery.min.js", "text/javascript");
  }); 
  server.on("/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/bootstrap.min.js", "text/javascript");
  });
  server.on("/wifi_status.json", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/wifi_status.json", "text/javascript");
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

    server.on("/", HTTP_POST, [](AsyncWebServerRequest * request) {
    // Verifique se o parâmetro POST existe ou não
    if (request->hasParam("ssid", true)) {
      AsyncWebParameter* l = request->getParam("ssid", true);
    //  Serial.printf("O parâmetro POST %s existe e possui o valor %s\n", l->name(), l->value());
    if (request->hasParam("senha", true)) {
      AsyncWebParameter* s = request->getParam("senha", true);
    //  Serial.printf("O parâmetro POST %s existe e possui o valor %s\n", s->name(), s->value());

    // Armazena os valores recebidos em variáveis
    novoSsid = l->value();
    novaSenha = s->value();
    
    flag_novarede = true; // Essa flag sempre é acionada ao receber uma nova rede enviada pelo site
    
    // Atualiza as variáveis globais (se necessário)
    //ssid = novoSsid;
    //password = novaSenha;

    // Imprime as informações recebidas
    //Serial.printf("SSID atualizado para: %s\n", novoSsid);
    //Serial.printf("Senha atualizada para: %s\n", novaSenha);

    // Atualiza a conexão Wi-Fi com as novas credenciais
    //WiFi.begin(ssid, password); // Reconecta com o novo SSID e senha
    
    }}
    request->send(200);
  });

  // Servidor começa à ouvir os clientes
  server.begin();
}

void loop() {
  if(flag_novarede){ // Quando a flag é acionada o wifi é disconectado e é tentado conectar em uma nova rede
    WiFi.disconnect();
    delay(100);
    WiFi.begin(novoSsid, novaSenha);
    int i = 0;
    while (WiFi.status() != WL_CONNECTED&&flag_erro==false) {  // Enquanto o status for diferente de conectado e a flag erro for falsa
      //Serial.println("Connecting to WiFi..");                // o código tentará se conectar 50 vezes e notificará qualquer mudança de estado
      delay(100);                                              // se ele não conseguir se conectar após 50 tentativas a flag erro virará true
      switch(WiFi.status()){
          case WL_CONNECTED:
          if(estado!=1){
            Serial.println("conectado");
            estado = 1;
            IPAddress ipLocal = WiFi.localIP();
            ipNovaRede = ipLocal.toString();
            Conectado = true;
            saveConfigFile(WifiS);
          }
          break;
          case WL_NO_SHIELD:
          if(estado!=2){
            Serial.println("sem shield wifi");
            estado=2;
            Sshield = true;
            saveConfigFile(WifiS);
          }
          break;
          case WL_IDLE_STATUS:
          if(estado!=3){
            Serial.println("wifi em espera");
            estado=3;
            Espera = true; 
            saveConfigFile(WifiS);
          }
          break;
          case WL_NO_SSID_AVAIL:
          if(estado!=4){
            Serial.println("sem redes disponiveis");
            estado=4;
            Sredes = true;
            saveConfigFile(WifiS);
          }
          break;
          case WL_SCAN_COMPLETED:
          if(estado!=5){
            Serial.println("escaneamento completado");
            estado=5;
            Completo = true;
            saveConfigFile(WifiS);
          }
          break;
          case WL_CONNECT_FAILED:
          if(estado!=6){
            Serial.println("conexão falhou");
            estado=6;
            Falha = true;
            saveConfigFile(WifiS);
          }
          break;
          case WL_CONNECTION_LOST:
          if(estado!=7){
            Serial.println("conexão perdida");
            estado=7;
            ConexPerdida = true;
            saveConfigFile(WifiS);
          }
          break;
          case WL_DISCONNECTED:
          if(estado!=8){
            Serial.println("wifi desconectado");
            estado=8;
            Desconectado = true;
            saveConfigFile(WifiS);
          }
          break;
        }
      if(i>50){
        flag_erro = true;
      }
      i++;
    }
    if(flag_erro){                                   // caso a flag erro for true ele retornará para a rede inicial
      WiFi.disconnect();
      novoSsid.clear();
      novaSenha.clear();
      Serial.println("reconecta a rede padrão!");
      Serial.println(WiFi.softAPIP());   // printará o ip caso voltar a rede original
    }else{
      Serial.println(WiFi.localIP());    // printará o ip da nova rede
      Serial.println(WiFi.softAPIP());
    }
    flag_novarede=false;                 // variáveis para zerar as coisas assim que se conectar em uma nova rede
    flag_erro=false;
    estado=0;
  }
    Serial.println("\nPing em " + String(ipNovaRede) + "...");

    
  //Ping no remoteHost
  if (Ping.ping(ipLocal, 3)) {      //Parâmetros: (remoteHost, quantidade_pings (default=5))
    Serial.print(Ping.averageTime());  //Retorna o tempo médio dos pings
    Serial.println(" ms");
  } else {
    Serial.println("Erro.");
    flag_rede_indisponivel = true;
  }

    Serial.println("\nPing em " + String(remoteHost) + "...");

  //Ping no remoteHost
  if (Ping.ping(remoteHost, 3)) {      //Parâmetros: (remoteHost, quantidade_pings (default=5))
    Serial.print(Ping.averageTime());  //Retorna o tempo médio dos pings
    Serial.println(" ms");
  } else {
    Serial.println("Erro.");
  }
  delay(3000);
} 
