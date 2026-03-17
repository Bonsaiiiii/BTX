#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ESP32Ping.h>

// Instanciação do objeto da classe AsyncWebServer
AsyncWebServer server(80);

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

void setup() {
    Serial.begin(115200);
    Serial.println("\n[*] Creating AP");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial.print("[+] AP Created with IP Gateway ");
    Serial.println(WiFi.softAPIP());
    Serial.println(WiFi.localIP());
    
    server.on("/", HTTP_POST, [](AsyncWebServerRequest * request) {
    // Verifique se o parâmetro POST existe ou não
    if (request->hasParam("ssid", true)) {
      AsyncWebParameter* l = request->getParam("ssid", true);
    //  Serial.printf("O parâmetro POST %s existe e possui o valor %s\n", l->name().c_str(), l->value().c_str());
    if (request->hasParam("senha", true)) {
      AsyncWebParameter* s = request->getParam("senha", true);
    //  Serial.printf("O parâmetro POST %s existe e possui o valor %s\n", s->name().c_str(), s->value().c_str());

    // Armazena os valores recebidos em variáveis
    novoSsid = l->value();
    novaSenha = s->value();
    
    flag_novarede = true; // Essa flag sempre é acionada ao receber uma nova rede enviada pelo site
    
    // Atualiza as variáveis globais (se necessário)
    //ssid = novoSsid.c_str();
    //password = novaSenha.c_str();

    // Imprime as informações recebidas
    //Serial.printf("SSID atualizado para: %s\n", novoSsid.c_str());
    //Serial.printf("Senha atualizada para: %s\n", novaSenha.c_str());

    // Atualiza a conexão Wi-Fi com as novas credenciais
    //WiFi.begin(ssid, password); // Reconecta com o novo SSID e senha
    
    }}
    request->send(200);
  });

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

  server.begin();
}

void loop() {
  if(flag_novarede){ // Quando a flag é acionada o wifi é disconectado e é tentado conectar em uma nova rede
    WiFi.disconnect();
    delay(100);
    WiFi.begin(novoSsid.c_str(), novaSenha.c_str());
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
          }
          break;
          case WL_NO_SHIELD:
          if(estado!=2){
            Serial.println("sem shield wifi");
            estado=2;
          }
          break;
          case WL_IDLE_STATUS:
          if(estado!=3){
            Serial.println("wifi em espera");
            estado=3;
          }
          break;
          case WL_NO_SSID_AVAIL:
          if(estado!=4){
            Serial.println("sem redes disponiveis");
            estado=4;
          }
          break;
          case WL_SCAN_COMPLETED:
          if(estado!=5){
            Serial.println("escaneamento completado");
            estado=5;
          }
          break;
          case WL_CONNECT_FAILED:
          if(estado!=6){
            Serial.println("conexão falhou");
            estado=6;
          }
          break;
          case WL_CONNECTION_LOST:
          if(estado!=7){
            Serial.println("conexão perdida");
            estado=7;
          }
          break;
          case WL_DISCONNECTED:
          if(estado!=8){
            Serial.println("wifi desconectado");
            estado=8;
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
