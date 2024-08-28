#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "DNSServer.h"
#include "AsyncTCP.h"
#include "SPIFFS.h"
#include "FS.h"
#include "ArduinoJson.h"
#include "NTRIPClient.h"
#include "Freenove_WS2812_Lib_for_ESP32.h"
#include "HardwareSerial.h"
#include "base64.hpp"

#define LEDS_COUNT  2

#define BTX_data  "/btx_conf.json"

#define size_btx  2048

#define CF3     4
#define CF2     5
#define CF1     6
#define PDPG    7
#define RSSIP   15
#define CONFP   16
#define V5EN    17
#define V5PG    18
#define ID      8
#define BT1     3
#define POUTEN  9
#define LEDIN   12
#define BT2     13
#define BYPASS  14
#define DET     21
#define TXCF    1
#define RXCF    2 
#define TXDT    43
#define RXDT    44
#define EXTINV  37
#define EXTOFF  36
#define EXTEN   35


uint16_t port = 2101;

const char* res_rev1 = "ICY 200 OK";
const char* res_rev2 = "HTTP/1.1 200 OK\r\n";
const char* res_rev1_ST = "SOURCETABLE 200 OK";

bool FLAG_Stable_ap;
bool FLAG_MP_ap;
bool flag_send_ap;
bool flag_ok_ap = false;
unsigned long currentMillis_ap;
unsigned long atualMillis_ap;
unsigned long wait_ap;
unsigned long int readcount_ap;

char ch_ap[1024];
char cliente_data_ap[1024];

//variaveis BTX

bool interruptFlag = false;
bool USBPowerIssueFlag = false;
int teste;
String MAC;

//var channels
String CTX0, CTX1, CTX2, CTX3, CTX4, CTX5, CTX6, CTX7, CTX8, CTX9;
String CRX0, CRX1, CRX2, CRX3, CRX4, CRX5, CRX6, CRX7, CRX8, CRX9;

char cCTX0[10]; char cCTX1[10]; char cCTX2[10]; char cCTX3[10]; char cCTX4[10]; char cCTX5[10]; char cCTX6[10]; char cCTX7[10]; char cCTX8[10]; char cCTX9[10];
char cCRX0[10]; char cCRX1[10]; char cCRX2[10]; char cCRX3[10]; char cCRX4[10]; char cCRX5[10]; char cCRX6[10]; char cCRX7[10]; char cCRX8[10]; char cCRX9[10];

//var serial
String CHTX, CHRX, SBUD, TPWR, TPRT;

char cCHTX[2]; char cCHRX[2]; char cSBUD[7]; char cTPWR[2]; char cTPRT[10];

//var cliente
String CHTXC, TPWRC, SBUDC, TPRTC, WIFI, WPAS, HOST, PORT, MNTP, USER, UPAS;
String Latitude, Longitude, Precisao, Altitude, Tempoutc, ChLoc;

char cCHTXC[2]; char cTPWRC[2]; char cSBUDC[7]; char cTPRTC[10]; char cWIFI[32]; char cWPAS[64]; char cHOST[16]; char cPORT[6]; char cMNTP[81]; char cUSER[31]; char cUPAS[31]; 
char cLatitude[12]; char cLongitude[12]; char cPrecisao[20]; char cAltitude[20]; char cTempoutc[7], cChLoc[2];

//var caster
String SBAUDL, WIFIL, WPASL, HOSTL, PORTL, MNTPL, USERL, UPASL;

char cSBAUDL[7]; char cWIFIL[32]; char cWPASL[64]; char cHOSTL[16]; char cPORTL[6]; char cMNTPL[81]; char cUSERL[31]; char cUPASL[31];

//var adv
String INPT, FLOW, FUPP, MODE, READ, CONF;  

char cINPT[7]; char cFLOW[4]; char cFUPP[4]; char cMODE[7]; char cREAD[4]; char cCONF[2]; 

//var info
String SERL, SREV, MODC, FIRC;

char cSERL[13]; char cSREV[11]; char cMODC[10]; char cFIRC[10];

String commands[11]={"TX","RX","PWR","SBAUD","PRT","SREV","SER","FLOW","FUPP","MODE"};
String TXR, RXR;

int baudrates[5]={9600,19200,38400,57600,115200};
int input;
int conf_btx = 0;
int conf_exit = 0;

unsigned long tempo;
char ch0[5000];
unsigned int readcount;
bool flag_send = false;
long readcount0;
long int size0;
unsigned long currentMillis;
unsigned long atualMillis;
int init1;
bool FlagOnly5V;

NTRIPClient ntrip_c;
Freenove_ESP32_WS2812 leds = Freenove_ESP32_WS2812(LEDS_COUNT, LEDIN, 0);

///TaskHandle_t task1;

// Create AsyncWebServer object on port 80
DNSServer dnsServer;
AsyncWebServer server(80);

WiFiServer wifiServer(port);
WiFiClient client = wifiServer.available();


//BDR FUNÇÔES////////////////////////////////////

String scrtbl(const char* mountpoint, String ip, uint16_t port){
  String body = "STR;";
  body = body + mountpoint + String(";;;;0;;HUGEN;BRA;0.00;0.00;0;0;sNTRIP;none;N;N;0;none;\r\n"
            "NET;HUGEN;;N;N;None;") + ip + String(":") + port + String(";None;;\r\n"
            "ENDSOURCETABLE\r\n");
  uint16_t tamanho = body.length();
  String header = "Server:"; 
  header = header + String(" Hugenplus ntrip simple ntrip caster/R1.0\r\n"
            //"Date: ") + ntp.getFormattedDate() + String("\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: ") + String(tamanho) + String("\r\n");
  return header + String("\r\n") + body + String("\r\n");
}

int check_aut(String data, String user, String pwr){
  char base64[50];
  unsigned char string[50];
  //Serial.print("user: ");Serial.println(user);
  //Serial.print("pwr: ");Serial.println(pwr);
  int posicao = data.indexOf(": Basic");
  posicao = posicao + 8;
  String usuario_encoded = data.substring(posicao);
  usuario_encoded.toCharArray(base64,50);
  unsigned int string_length = decode_base64((unsigned char *)base64, string);
  string[string_length] = '\0';
  String usuario_decoded = String((char *)string);
  int separador = usuario_decoded.indexOf(":");
  String cliente_usuario = usuario_decoded.substring(0,separador);
  separador ++;
  String cliente_senha = usuario_decoded.substring(separador);
  //Serial.print("cliente_usuario: ");Serial.println(cliente_usuario);
  //Serial.print("cliente_senha: ");Serial.println(cliente_senha);
  if(cliente_usuario == user && cliente_senha == pwr){
    return 8;// senha correta
  }else{
    return 9;// senha incorreta
  }
}

int check_ver(String data){
  int posicao_inicial = data.indexOf("HTTP");
  int posicao_final = data.indexOf("\r\n");
  if (posicao_inicial < 0){
    return 4; //ERRO1 
  }
  String versao_http = data.substring(posicao_inicial, posicao_final);
  //Serial.println(versao_http);
  if(versao_http == "HTTP/1.0"){
    return 1; //REV1
  }
  if(versao_http == "HTTP/1.1"){
    return 2; //REV2
  }
}

int check_mountpoint(String data, String mountpoint){
  int posicao_inicial = data.indexOf("/");
  int posicao_final = data.indexOf("HTTP");
  posicao_final--;
  String cliente_mountpoint = data.substring(posicao_inicial,posicao_final);
  String mp = "/";
  mp = mp + mountpoint;
  //Serial.println(cliente_mountpoint);
  //Serial.println(mp);
  if(cliente_mountpoint == mp){
    return 5; //igual
  }
  if(cliente_mountpoint == "/SOURCETABLE.TXT " || cliente_mountpoint == "/"){
    return 6; //solicita
  }
  return 7; //ERRO2
} 

//BTX FUNÇÔES/////////////////////////////////////// 

bool usbpd(int volt){
  switch(volt){
    case 5:
      digitalWrite(CF1,HIGH);
      digitalWrite(CF2,LOW);
      digitalWrite(CF3,LOW);
    break;
    case 9:
      digitalWrite(CF1,LOW);
      digitalWrite(CF2,LOW);
      digitalWrite(CF3,LOW);
    break;
    case 12:
      digitalWrite(CF1,LOW);
      digitalWrite(CF2,LOW);
      digitalWrite(CF3,HIGH);
    break;
    case 15:
      digitalWrite(CF1,LOW);
      digitalWrite(CF2,HIGH);
      digitalWrite(CF3,HIGH);
    break;
    case 20:
      digitalWrite(CF1,LOW);
      digitalWrite(CF2,HIGH);
      digitalWrite(CF3,LOW);
    break;
  }
  return digitalRead(!PDPG);
}

void init_gpio(){
  pinMode(CF3, OUTPUT);
  pinMode(CF2, OUTPUT);
  pinMode(CF1, OUTPUT);
  pinMode(CONFP, OUTPUT);
  pinMode(V5EN, OUTPUT);
  pinMode(POUTEN, OUTPUT);
  pinMode(EXTOFF, OUTPUT);
  pinMode(EXTEN, OUTPUT);
  pinMode(PDPG,INPUT);
  pinMode(RSSIP,INPUT);
  pinMode(V5PG,INPUT);
  pinMode(ID,INPUT);
  pinMode(BT1,INPUT);
  pinMode(BT2,INPUT);
  pinMode(DET,INPUT);
  pinMode(EXTINV,INPUT);
  pinMode(BYPASS,OUTPUT);
  pinMode(TXCF,OUTPUT);
}

bool ext_com(bool state){
  switch(state){
    case 1:
      digitalWrite(EXTEN, LOW);
      digitalWrite(EXTOFF, HIGH);
      break;
    case 0:
      digitalWrite(EXTEN, HIGH);
      digitalWrite(EXTOFF, LOW);
      break;
  }
  return digitalRead(EXTINV);
}

bool v5p(bool state){
  digitalWrite(V5EN,state);
  return digitalRead(V5PG);
}

String du2005read(String command){
  int retries = 0;
  again:
  if(retries==3){
    return String("ERRO1");
  }
  Serial.println(command);
  if(command=="PRT"){   
    digitalWrite(CONFP,LOW);
    String prt, airbaud;
    for(int i=0;i<=2;i++){
      if(i==2){
        if(prt=="TRIMTALK"){
          if(airbaud=="4800 "){
            return String("TRIMTALK1");
          }else if(airbaud=="9600 "){
            return String("TRIMTALK2");
          }
        }
        if(prt=="TRIMMK 3"){
          if(airbaud=="4800 "){
            return String("TRIMMK31");
          }else if(airbaud=="19200"){
            return String("TRIMMK32");
          }
        }
        if(prt=="TRANSEOT"){
          if(airbaud=="4800 "){
            return String("TRANSEOT1");
          }else if(airbaud=="9600 "){
            return String("TRANSEOT2");
          }
        }
        if(prt=="SATEL   "){
          if(airbaud=="4800 "){
            return String("SATEL1");
          }else if(airbaud=="19200"){
            return String("SATEL2");
          }else if(airbaud=="9600 "){
            return String("SATEL3");
          }   
        }
        if(prt=="SOUTH   "){
          if(airbaud=="4800 "){
            return String("SOUTH1");
          }else if(airbaud=="9600 "){
            return String("SOUTH2");
          }else if(airbaud=="19200"){
            return String("SOUTH3");
          }
        }
      }
      if(i==0){
        Serial1.println("PRT");
      }else if(i==1){
        Serial1.println("BAUD");
      }
      delay(20);
      int size = 0;
      char resp[Serial1.available() +1];
      while(Serial1.available()){
        resp[size]=Serial1.read();
        delay(1);
        size++;
      }
      String resposta = resp;     
      if(i==0){
        int pos1 = resposta.indexOf("PRT")  + 4;
        int pos2 = resposta.indexOf(">") - 2;
        prt = resposta.substring(pos1,pos2);
        Serial.println(prt);
      }else{
        int pos1 = resposta.indexOf("BAUD") + 5;
        int pos3 = resposta.indexOf(">") - 2;
        airbaud = resposta.substring(pos1,pos3);
        Serial.println(airbaud);
      }
    }
  }else{
    int size = 0;
    digitalWrite(CONFP,LOW);
    Serial1.println(command);
    delay(20);
    char resp[Serial1.available() +1];
    while(Serial1.available()){
      resp[size]=Serial1.read();
      delay(1);
      size++;
    }
    String resposta = resp;
    int pos1;
    int pos2;
    if(command == "SER"){pos1 = resposta.indexOf(command) + command.length() + 3;}
    else if(command == "PWR"||command == "SREV"){pos1 = resposta.indexOf(command) + command.length();} 
    else{pos1 = resposta.indexOf(command) + command.length() + 1;}
    if(command == "TX"||command == "RX"){pos2 = resposta.indexOf(">") - 6;}
    else if(command == "SREV"){pos2 = resposta.indexOf(">") - 4;}
    else if(command == "FLOW"||command == "FUPP"){pos2 = resposta.indexOf(">") - 3;}
    else{pos2 = resposta.indexOf(">") - 2;}
    String check = resposta.substring(pos1,pos2);
    if (check.length() >= 1){
      Serial.println(check);
      return check;
    }
    retries++;
    goto again;
  }

}

String du2005conf(String command, String valor){
  Serial.print(command);Serial.print(": ");Serial.println(valor);
  int size2 = 0;
  int c=0;
  if(command=="PRT"){
    if(valor=="TRIMTALK1"){
      digitalWrite(CONFP,LOW);
      for(int i=0; i<2; i++){
        if(i==0){
          Serial1.println("PRT TRIMTALK");
        }else if(i==1){
          Serial1.println("BAUD 4800");
        }else if (i==2){
          if(c==2){
            //Serial.println("programado");
            return String("PROGRAMADO");
          }
        }
        delay(50);
        char resp2[Serial1.available() +1];
        while(Serial1.available()){
          resp2[size2]=Serial1.read();
          delay(1);
          size2++;
        }
        String resposta2 = resp2;
        //Serial2.println(resposta2);
        if(resposta2.indexOf("PROGRAMMED OK")>0){
          c++;
          //return String("PROGRAMADO");
        }else if(resposta2.indexOf("INCORRECT ENTRY")>0){
          return String("ERRO2");
        }else{
          return String("ERRO1");
        }
      }
    }else if(valor=="TRIMTALK2"){
      digitalWrite(CONFP,LOW);
      for(int i=0; i<2; i++){
        if(i==0){
          Serial1.println("PRT TRIMTALK");
        }else if(i==1){
          Serial1.println("BAUD 9600");
        }else if (i==2){
          if(c==2){
            //Serial.println("programado");
            return String("PROGRAMADO");
          }
        }
        delay(50);
        char resp2[Serial1.available() +1];
        while(Serial1.available()){
          resp2[size2]=Serial1.read();
          delay(1);
          size2++;
        }
        String resposta2 = resp2;
        //Serial2.println(resposta2);
        if(resposta2.indexOf("PROGRAMMED OK")>0){
          c++;
          //return String("PROGRAMADO");
        }else if(resposta2.indexOf("INCORRECT ENTRY")>0){
          return String("ERRO2");
        }else{
          return String("ERRO1");
        }
      }
    }else if(valor=="TRIMMK31"){
      digitalWrite(CONFP,LOW);
      for(int i=0; i<2; i++){
        if(i==0){
          Serial1.println("PRT TRIMMK3");
        }else if(i==1){
          Serial1.println("BAUD 4800");
        }else if (i==2){
          if(c==2){
            //Serial.println("programado");
            return String("PROGRAMADO");
          }
        }
        delay(50);
        char resp2[Serial1.available() +1];
        while(Serial1.available()){
          resp2[size2]=Serial1.read();
          delay(1);
          size2++;
        }
        String resposta2 = resp2;
        //Serial2.println(resposta2);
        if(resposta2.indexOf("PROGRAMMED OK")>0){
          c++;
          //return String("PROGRAMADO");
        }else if(resposta2.indexOf("INCORRECT ENTRY")>0){
          return String("ERRO2");
        }else{
          return String("ERRO1");
        }
      }
    }else if(valor=="TRIMMK32"){
      digitalWrite(CONFP,LOW);
      for(int i=0; i<2; i++){
        if(i==0){
          Serial1.println("PRT TRIMMK3");
        }else if(i==1){
          Serial1.println("BAUD 19200");
        }else if (i==2){
          if(c==2){
            //Serial.println("programado");
            return String("PROGRAMADO");
          }
        }
        delay(50);
        char resp2[Serial1.available() +1];
        while(Serial1.available()){
          resp2[size2]=Serial1.read();
          delay(1);
          size2++;
        }
        String resposta2 = resp2;
        //Serial2.println(resposta2);
        if(resposta2.indexOf("PROGRAMMED OK")>0){
          c++;
          //return String("PROGRAMADO");
        }else if(resposta2.indexOf("INCORRECT ENTRY")>0){
          return String("ERRO2");
        }else{
          return String("ERRO1");
        }
      }
    }else if(valor=="TRANSEOT1"){
      digitalWrite(CONFP,LOW);
      for(int i=0; i<2; i++){
        if(i==0){
          Serial1.println("PRT TRANSEOT");
        }else if(i==1){
          Serial1.println("BAUD 4800");
        }else if (i==2){
          if(c==2){
            //Serial.println("programado");
            return String("PROGRAMADO");
          }
        }
        delay(50);
        char resp2[Serial1.available() +1];
        while(Serial1.available()){
          resp2[size2]=Serial1.read();
          delay(1);
          size2++;
        }
        String resposta2 = resp2;
        //Serial2.println(resposta2);
        if(resposta2.indexOf("PROGRAMMED OK")>0){
          c++;
          //return String("PROGRAMADO");
        }else if(resposta2.indexOf("INCORRECT ENTRY")>0){
          return String("ERRO2");
        }else{
          return String("ERRO1");
        }
      }   
    }else if(valor=="TRANSEOT2"){
      digitalWrite(CONFP,LOW);
      for(int i=0; i<2; i++){
        if(i==0){
          Serial1.println("PRT TRANSEOT");
        }else if(i==1){
          Serial1.println("BAUD 9600");
        }else if (i==2){
          if(c==2){
            //Serial.println("programado");
            return String("PROGRAMADO");
          }
        }
        delay(50);
        char resp2[Serial1.available() +1];
        while(Serial1.available()){
          resp2[size2]=Serial1.read();
          delay(1);
          size2++;
        }
        String resposta2 = resp2;
        //Serial2.println(resposta2);
        if(resposta2.indexOf("PROGRAMMED OK")>0){
          c++;
          //return String("PROGRAMADO");
        }else if(resposta2.indexOf("INCORRECT ENTRY")>0){
          return String("ERRO2");
        }else{
          return String("ERRO1");
        }
      }
    }else if(valor=="SATEL1"){
      digitalWrite(CONFP,LOW);
      for(int i=0; i<2; i++){
        if(i==0){
          Serial1.println("PRT SATEL");
        }else if(i==1){
          Serial1.println("BAUD 4800");
        }else if (i==2){
          if(c==2){
            //Serial.println("programado");
            return String("PROGRAMADO");
          }
        }
        delay(50);
        char resp2[Serial1.available() +1];
        while(Serial1.available()){
          resp2[size2]=Serial1.read();
          delay(1);
          size2++;
        }
        String resposta2 = resp2;
        //Serial2.println(resposta2);
        if(resposta2.indexOf("PROGRAMMED OK")>0){
          c++;
          //return String("PROGRAMADO");
        }else if(resposta2.indexOf("INCORRECT ENTRY")>0){
          return String("ERRO2");
        }else{
          return String("ERRO1");
        }
      }
    }else if(valor=="SATEL2"){
      digitalWrite(CONFP,LOW);
      for(int i=0; i<2; i++){
        if(i==0){
          Serial1.println("PRT SATEL");
        }else if(i==1){
          Serial1.println("BAUD 9600");
        }else if (i==2){
          if(c==2){
            //Serial.println("programado");
            return String("PROGRAMADO");
          }
        }
        delay(50);
        char resp2[Serial1.available() +1];
        while(Serial1.available()){
          resp2[size2]=Serial1.read();
          delay(1);
          size2++;
        }
        String resposta2 = resp2;
        //Serial2.println(resposta2);
        if(resposta2.indexOf("PROGRAMMED OK")>0){
          c++;
          //return String("PROGRAMADO");
        }else if(resposta2.indexOf("INCORRECT ENTRY")>0){
          return String("ERRO2");
        }else{
          return String("ERRO1");
        }
      }
    }else if(valor=="SATEL3"){
      digitalWrite(CONFP,LOW);
      for(int i=0; i<2; i++){  
        if(i==0){
          Serial1.println("PRT SATEL");
        }else if (i==1){
          Serial1.println("BAUD 19200");
        }else if (i==2){
          if(c==2){
            //Serial.println("programado");
            return String("PROGRAMADO");
          }
        }
        delay(50);
        char resp2[Serial1.available() +1];
        while(Serial1.available()){
          resp2[size2]=Serial1.read();
          delay(1);
          size2++;
        }
        String resposta2 = resp2;
        //Serial.println(resposta2);
        if(resposta2.indexOf("PROGRAMMED OK")>0){
          c++;
          //return String("PROGRAMADO");
        }else if(resposta2.indexOf("INCORRECT ENTRY")>0){
          return String("ERRO2");
        }else{
          return String("ERRO1");
        }       
      }
    }else if(valor=="SOUTH1"){
      digitalWrite(CONFP,LOW);
      for(int i=0; i<2; i++){
        if(i==0){
          Serial1.println("PRT SOUTH");
        }else if(i==1){
          Serial1.println("BAUD 4800");
        }else if (i==2){
          if(c==2){
            //Serial.println("programado");
            return String("PROGRAMADO");
          }
        }
        delay(50);
        char resp2[Serial1.available() +1];
        while(Serial1.available()){
          resp2[size2]=Serial1.read();
          delay(1);
          size2++;
        }
        String resposta2 = resp2;
        //Serial2.println(resposta2);
        if(resposta2.indexOf("PROGRAMMED OK")>0){
          c++;
          //return String("PROGRAMADO");
        }else if(resposta2.indexOf("INCORRECT ENTRY")>0){
          return String("ERRO2");
        }else{
          return String("ERRO1");
        }
      }
    }else if(valor=="SOUTH2"){
      digitalWrite(CONFP,LOW);
      for(int i=0; i<2; i++){
        if(i==0){
          Serial1.println("PRT SOUTH");
        }else if(i==1){
          Serial1.println("BAUD 9600");
        }else if (i==2){
          if(c==2){
            //Serial.println("programado");
            return String("PROGRAMADO");
          }
        }
        delay(50);
        char resp2[Serial1.available() +1];
        while(Serial1.available()){
          resp2[size2]=Serial1.read();
          delay(1);
          size2++;
        }
        String resposta2 = resp2;
        //Serial2.println(resposta2);
        if(resposta2.indexOf("PROGRAMMED OK")>0){
          c++;
          //return String("PROGRAMADO");
        }else if(resposta2.indexOf("INCORRECT ENTRY")>0){
          return String("ERRO2");
        }else{
          return String("ERRO1");
        }
      }
    }else if(valor=="SOUTH3"){
      digitalWrite(CONFP,LOW);
      for(int i=0; i<2; i++){
        if(i==0){
          Serial1.println("PRT SOUTH");
        }else if(i==1){
          Serial1.println("BAUD 19200");
        }else if (i==2){
          if(c==2){
            //Serial.println("programado");
            return String("PROGRAMADO");
          }
        }
        delay(50);
        char resp2[Serial1.available() +1];
        while(Serial1.available()){
          resp2[size2]=Serial1.read();
          delay(1);
          size2++;
        }
        String resposta2 = resp2;
        //Serial2.println(resposta2);
        if(resposta2.indexOf("PROGRAMMED OK")>0){
          c++;
          //return String("PROGRAMADO");
        }else if(resposta2.indexOf("INCORRECT ENTRY")>0){
          return String("ERRO2");
        }else{
          return String("ERRO1");
        }
      }
    }  
    
  }else{
    digitalWrite(CONFP,LOW);
    String confg = command + String(" ") + valor;
    Serial1.println(confg);
    delay(50);
    char resp2[Serial1.available() +1];
    while(Serial1.available()){
      resp2[size2]=Serial1.read();
      delay(1);
      size2++;
    }
    String resposta2 = resp2;
    //Serial2.println(resposta2);
    if(resposta2.indexOf("PROGRAMMED OK")>0){
      return String("PROGRAMADO");
    }else if(resposta2.indexOf("INCORRECT ENTRY")>0){
      return String("ERRO2");
    }else{
      return String("ERRO1");
    }
  }
}

String searchbaud(){
  for (int i = 0; i <= 5; i++){
    if(i==5){
      return String("ERRO3");
    }
    delay(20);
    Serial1.begin(baudrates[i], SERIAL_8N1, TXDT, RXDT);
    delay(50);
    du2005read("SER");
    if(du2005read("SER")!="ERRO1"){
      return String(baudrates[i]);
    }    
  }
}

int programall(){
  if(INPT="SERIAL"){
    for(int e=0;e<=10;e++){
      switch(e){
        case 0: 
          if(CHTX=="0"){
            du2005conf(commands[e], CTX0);
          }
          if(CHTX=="1"){
            du2005conf(commands[e], CTX1);
          }
          if(CHTX=="2"){
            du2005conf(commands[e], CTX2);
          }
          if(CHTX=="3"){
            du2005conf(commands[e], CTX3);
          }
          if(CHTX=="4"){
            du2005conf(commands[e], CTX4);
          }
          if(CHTX=="5"){
            du2005conf(commands[e], CTX5);
          }
          if(CHTX=="6"){
            du2005conf(commands[e], CTX6);
          }
          if(CHTX=="7"){
            du2005conf(commands[e], CTX7);
          }
          if(CHTX=="8"){
            du2005conf(commands[e], CTX8);
          }
          if(CHTX=="9"){
            du2005conf(commands[e], CTX9);
          }
          break;
        case 1: 
          if(CHRX=="0"){
            du2005conf(commands[e], CRX0);
          }
          if(CHRX=="1"){
            du2005conf(commands[e], CRX1);
          }
          if(CHRX=="2"){
            du2005conf(commands[e], CRX2);
          }
          if(CHRX=="3"){
            du2005conf(commands[e], CRX3);
          }
          if(CHRX=="4"){
            du2005conf(commands[e], CRX4);
          }
          if(CHRX=="5"){
            du2005conf(commands[e], CRX5);
          }
          if(CHRX=="6"){
            du2005conf(commands[e], CRX6);
          }
          if(CHRX=="7"){
            du2005conf(commands[e], CRX7);
          }
          if(CHRX=="8"){
            du2005conf(commands[e], CRX8);
          }
          if(CHRX=="9"){
            du2005conf(commands[e], CRX9);
          }
          break;
        case 2: 
          du2005conf(commands[e],TPWR);
          break;
        case 3: 
          du2005conf(commands[e],SBUD);
          break;
        case 4: 
          du2005conf(commands[e],TPRT);
          break;
        case 5:
          break;
        case 6:
          break;
        case 7:
          du2005conf(commands[e],FLOW);
          break;
        case 8:
          du2005conf(commands[e],FUPP);
          break;
        case 9:
          du2005conf(commands[e],MODE);
          break;
      } 
    }
  }else if(INPT="CLIENT"){
    for(int e=0;e<=10;e++){
      switch(e){
        case 0: 
          if(CHTX=="0"){
            du2005conf(commands[e], CTX0);
          }
          if(CHTX=="1"){
            du2005conf(commands[e], CTX1);
          }
          if(CHTX=="2"){
            du2005conf(commands[e], CTX2);
          }
          if(CHTX=="3"){
            du2005conf(commands[e], CTX3);
          }
          if(CHTX=="4"){
            du2005conf(commands[e], CTX4);
          }
          if(CHTX=="5"){
            du2005conf(commands[e], CTX5);
          }
          if(CHTX=="6"){
            du2005conf(commands[e], CTX6);
          }
          if(CHTX=="7"){
            du2005conf(commands[e], CTX7);
          }
          if(CHTX=="8"){
            du2005conf(commands[e], CTX8);
          }
          if(CHTX=="9"){
            du2005conf(commands[e], CTX9);
          }
          break;
        case 1: 
          break;
        case 2: 
          du2005conf(commands[e],TPWR);
          break;
        case 3: 
          du2005conf(commands[e],SBUD);
          break;
        case 4: 
          du2005conf(commands[e],TPRT);
          break;
        case 5:
          break;
        case 6:
          break;
        case 7:
          du2005conf(commands[e],FLOW);
          break;
        case 8:
          du2005conf(commands[e],FUPP);
          break;
        case 9:
          du2005conf(commands[e],MODE);
          break;
      } 
    }
  }
  return 1;  
}

int readall(){
  for(int e=0;e<=10;e++){
    Serial.println(e);
    switch(e){
      case 0: 
        TXR = du2005read(commands[e]);  
        if(TXR=="ERRO1"){
          return 0;
        } 
        for(int i=0;i<=10;i++){
          switch(i){
            case 0: 
              if(CTX0==TXR){
                CHTX="0";
                CHTXC="0";
              }
              break;
            case 1: 
              if(CTX1==TXR){
                CHTX="1";
                CHTXC="1";
              }
              break;
            case 2: 
              if(CTX2==TXR){
                CHTX="2";
                CHTXC="2";
              }
              break;
            case 3: 
              if(CTX3==TXR){
                CHTX="3";
                CHTXC="3";
              }
              break;
            case 4: 
              if(CTX4==TXR){
                CHTX="4";
              }
              break;
            case 5: 
              if(CTX5==TXR){
                CHTX="5";
                CHTXC="4";
              }
              break;
            case 6: 
              if(CTX6==TXR){
                CHTX="6";
                CHTXC="6";
              }
              break;
            case 7: 
              if(CTX7==TXR){
                CHTX="7";
                CHTXC="7";
              }
              break;
            case 8: 
              if(CTX8==TXR){
                CHTX="8";
                CHTXC="8";
              }
              break;
            case 9: 
              if(CTX9==TXR){
                CHTX="9";
                CHTXC="9";
              }
              break;
            case 10:
              if(CTX0==""){
                CTX0==TXR;
                CHTX="0";
                CHTXC="0";
              }else{
                du2005conf("TX",CTX0);
                CHTX="0";
                CHTXC="0";
              }
              break;
          }
        }
        break;
      case 1: 
        RXR = du2005read(commands[e]);
        if(RXR=="ERRO1"){
          return 0;
        }
        for(int i=0;i<=10;i++){
          switch(i){
            case 0: 
              if(CRX0==RXR){
                CHRX="0";
              }
              break;
            case 1: 
              if(CRX1==RXR){
                CHRX="1";
              }
              break;
            case 2: 
              if(CRX2==RXR){
                CHTX="2";
              }
              break;
            case 3: 
              if(CRX3==RXR){
                CHRX="3";
              }
              break;
            case 4: 
              if(CRX4==RXR){
                CHRX="4";
              }
              break;
            case 5: 
              if(CRX5==RXR){
                CHRX="5";
              }
              break;
            case 6: 
              if(CRX6==RXR){
                CHRX="6";
              }
              break;
            case 7: 
              if(CRX7==RXR){
                CHRX="7";
              }
              break;
            case 8: 
              if(CRX8==RXR){
                CHRX="8";
              }
              break;
            case 9: 
              if(CRX9==RXR){
                CHRX="9";
              }
              break;
            case 10:
              if(CRX0==""){
                CRX0==RXR;
                CHRX="0";
              }else{
                du2005conf("RX",CRX0);
                CHRX="0";
              }
              break;
          }
        }
        break;
      case 2: 
        TPWR = du2005read(commands[e]);
        TPWR = TPWRC;
        if(TPWR=="ERRO1"){
          return 0;
        }
        break;
      case 3: 
        SBUD = du2005read(commands[e]);
        SBUD = SBUDC;
        if(SBUD=="ERRO1"){
          return 0;
        }
        break;
      case 4: 
        TPRT = du2005read(commands[e]);
        TPRT = TPRTC;
        if(TPRT=="ERRO1"){
          return 0;
        }
        break;
      case 5:
        SREV = du2005read(commands[e]);
        if(SREV=="ERRO1"){
          return 0;
        }
        break;
      case 6:
        SERL = du2005read(commands[e]);
        if(SERL=="ERRO1"){
          return 0;
        }
        break;
      case 7:
        FLOW = du2005read(commands[e]);
        if(FLOW=="ERRO1"){
          return 0;
        }
        break;
      case 8:
        FUPP = du2005read(commands[e]);
        if(FUPP=="ERRO1"){
          return 0;
        }
        break;
      case 9:
        MODE = du2005read(commands[e]);
        if(MODE=="ERRO1"){
          return 0;
        }
        break;
    } 
  }
  return 1;
}

int du2005begin(){
  long int SERIALBAUD;
  /* se tiver modificação para funcionar somente com 5V
  if(FlagOnly5V==true){
    v5p(LOW);
    digitalWrite(BYPASS, HIGH);
  }else{
    digitalWrite(BYPASS, LOW);
    usbpd(12);
    v5p(HIGH);
  }*/
  digitalWrite(BYPASS, LOW);
  usbpd(12);
  v5p(HIGH);
  //Serial.println("inicia pesquisa");
  String pesquisa = searchbaud();
  //Serial.print("baudrate do radio: ");
  //Serial.println("pesquisa");
  if(pesquisa=="ERRO3"){
    Serial.println("ERRO3");
    return 0;
  }else{
    SERIALBAUD = pesquisa.toInt();
    //Serial.println("pesquisa completa");
  }
  Serial1.begin(SERIALBAUD, SERIAL_8N1, TXDT, RXDT);
  delay(10);
  if(READ=="SIM"){
    int ler = readall();
    Serial.println("leitura concluida");
    if(ler == 1){
      digitalWrite(CONFP,HIGH);
      return 1;
    }
  }else if(READ=="NAO"){
    Serial.println("programa o radio");
    int escreve = programall();
    if(escreve == 1){
      digitalWrite(CONFP,HIGH);
      return 1;
    }
  }
  return 0;
}

String processor(const String& var){
  if(var == "model"){
    return MODC;
  }
  if(var == "codser"){
    return SERL;
  }
  if(var == "firver"){
    return SREV;
  }
  if(var == "controlver"){
    return FIRC;
  }
  return String();
}

bool loaddata(String filename){
  if (SPIFFS.begin(false) || SPIFFS.begin(true)){
    if (SPIFFS.exists(filename)){
      if(filename=="/btx_conf.json"){
        File configFile = SPIFFS.open(filename, "r");
          if (configFile){
            StaticJsonDocument<size_btx> json;
            DeserializationError error = deserializeJson(json, configFile);
            if (!error){
              //canais de transmissão
              JsonObject tx = json["tx_ch"][0];
              CTX0 = strcpy(cCTX0, tx["TX0"]);
              CTX1 = strcpy(cCTX1, tx["TX1"]);
              CTX2 = strcpy(cCTX2, tx["TX2"]);
              CTX3 = strcpy(cCTX3, tx["TX3"]);
              CTX4 = strcpy(cCTX4, tx["TX4"]);
              CTX5 = strcpy(cCTX5, tx["TX5"]);
              CTX6 = strcpy(cCTX6, tx["TX6"]);
              CTX7 = strcpy(cCTX7, tx["TX7"]);
              CTX8 = strcpy(cCTX8, tx["TX8"]);
              CTX9 = strcpy(cCTX9, tx["TX9"]);
              //canais de recepção
              JsonObject rx = json["rx_ch"][0];
              CRX0 = strcpy(cCRX0, rx["RX0"]);
              CRX1 = strcpy(cCRX1, rx["RX1"]);
              CRX2 = strcpy(cCRX2, rx["RX2"]);
              CRX3 = strcpy(cCRX3, rx["RX3"]);
              CRX4 = strcpy(cCRX4, rx["RX4"]);
              CRX5 = strcpy(cCRX5, rx["RX5"]);
              CRX6 = strcpy(cCRX6, rx["RX6"]);
              CRX7 = strcpy(cCRX7, rx["RX7"]);
              CRX8 = strcpy(cCRX8, rx["RX8"]);
              CRX9 = strcpy(cCRX9, rx["RX9"]);
              //modo serial
              JsonObject serial = json["serial"][0];
              CHTX = strcpy(cCHTX, serial["TX"]);
              CHRX = strcpy(cCHRX, serial["RX"]);
              TPWR = strcpy(cTPWR, serial["PWR"]);
              SBUD = strcpy(cSBUD, serial["SBAUD"]);
              TPRT = strcpy(cTPRT, serial["PRT"]);
              //modo cliente
              JsonObject cliente = json["client"][0];
              CHTXC = strcpy(cCHTXC, cliente["TXC"]);
              TPWRC = strcpy(cTPWRC, cliente["PWRC"]);
              SBUDC = strcpy(cSBUDC, cliente["SBAUDC"]);
              TPRTC = strcpy(cTPRTC, cliente["PRTC"]);
              WIFI = strcpy(cWIFI, cliente["WIFI"]);
              WPAS = strcpy(cWPAS, cliente["WPAS"]);
              HOST = strcpy(cHOST, cliente["HOST"]);
              PORT = strcpy(cPORT, cliente["PORT"]);
              MNTP = strcpy(cMNTP, cliente["MNTP"]);
              USER = strcpy(cUSER, cliente["USER"]);
              UPAS = strcpy(cUPAS, cliente["UPAS"]);
              ChLoc = strcpy(cChLoc, cliente["chLoc"]);
              Latitude = strcpy(cLatitude, cliente["latitude"]);
              Longitude = strcpy(cLongitude, cliente["longitude"]);
              Precisao = strcpy(cPrecisao, cliente["altitude"]);
              Altitude = strcpy(cAltitude, cliente["precisao"]);
              Tempoutc = strcpy(cTempoutc, cliente["tempoutc"]);
              //modo caster local
              JsonObject local = json["local"][0];
              SBAUDL = strcpy(cSBAUDL, local["SBAUDL"]);
              WIFIL = strcpy(cWIFIL, local["WIFIL"]);
              WPASL = strcpy(cWPASL, local["WPASL"]);
              HOSTL = strcpy(cHOSTL, local["HOSTL"]);
              PORTL = strcpy(cPORTL, local["PORTL"]);
              MNTPL = strcpy(cMNTPL, local["MNTPL"]);
              USERL = strcpy(cUSERL, local["USERL"]);
              UPASL = strcpy(cUPASL, local["UPASL"]);
              //adv
              JsonObject adv = json["adv"][0];
              INPT = strcpy(cINPT, adv["INPT"]);
              FLOW = strcpy(cFLOW, adv["FLOW"]);
              FUPP = strcpy(cFUPP, adv["FUPP"]);
              MODE = strcpy(cMODE, adv["MODE"]);
              READ = strcpy(cREAD, adv["READ"]);
              CONF = strcpy(cCONF, adv["CONF"]);
              //info
              JsonObject info = json["info"][0];
              SERL = strcpy(cSERL, info["SERL"]);
              SREV = strcpy(cSREV, info["SREV"]);
              MODC = strcpy(cMODC, info["MODC"]);
              FIRC = strcpy(cFIRC, info["FIRC"]);
              return true;
          }else{
              Serial.println("Failed to load btx_conf.json");
          }
        }
      }
    }
  }else{
    Serial.println("Failed to mount FS");
  }
  return false;
}

void saveConfigFile(String filename){
  if(filename=="/btx_conf.json"){
    StaticJsonDocument<size_btx> json;
    JsonObject tx_ch = json["tx_ch"].createNestedObject();
    tx_ch["TX0"] = CTX0.c_str();
    tx_ch["TX1"] = CTX1.c_str();
    tx_ch["TX2"] = CTX2.c_str();
    tx_ch["TX3"] = CTX3.c_str();
    tx_ch["TX4"] = CTX4.c_str();
    tx_ch["TX5"] = CTX5.c_str();
    tx_ch["TX6"] = CTX6.c_str();
    tx_ch["TX7"] = CTX7.c_str();
    tx_ch["TX8"] = CTX8.c_str();
    tx_ch["TX9"] = CTX9.c_str();
    JsonObject rx_ch = json["rx_ch"].createNestedObject();
    rx_ch["RX0"] = CRX0.c_str();
    rx_ch["RX1"] = CRX1.c_str();
    rx_ch["RX2"] = CRX2.c_str();
    rx_ch["RX3"] = CRX3.c_str();
    rx_ch["RX4"] = CRX4.c_str();
    rx_ch["RX5"] = CRX5.c_str();
    rx_ch["RX6"] = CRX6.c_str();
    rx_ch["RX7"] = CRX7.c_str();
    rx_ch["RX8"] = CRX8.c_str();
    rx_ch["RX9"] = CRX9.c_str();
    JsonObject serial = json["serial"].createNestedObject();
    serial["TX"] = CHTX.c_str();
    serial["RX"] = CHRX.c_str();
    serial["PWR"] = TPWR.c_str();
    serial["SBAUD"] = SBUD.c_str();
    serial["PRT"] = TPRT.c_str();
    JsonObject cliente = json["client"].createNestedObject();
    cliente["TXC"] = CHTXC.c_str();
    cliente["PWRC"] = TPWRC.c_str();
    cliente["SBAUDC"] = SBUDC.c_str();
    cliente["PRTC"] = TPRTC.c_str();
    cliente["WIFI"] = WIFI.c_str();
    cliente["WPAS"] = WPAS.c_str();
    cliente["HOST"] = HOST.c_str();
    cliente["PORT"] = PORT.c_str();
    cliente["MNTP"] = MNTP.c_str();
    cliente["USER"] = USER.c_str();
    cliente["UPAS"] = UPAS.c_str();
    cliente["chLoc"] = ChLoc.c_str();
    cliente["latitude"] = Latitude.c_str();
    cliente["longitude"] = Longitude.c_str();
    cliente["altitude"] = Altitude.c_str();
    cliente["precisao"] = Precisao.c_str();
    cliente["tempoutc"] = Tempoutc.c_str();
    JsonObject local = json["local"].createNestedObject();
    local["SBAUDL"] = SBAUDL.c_str();
    local["WIFIL"] = WIFIL.c_str();
    local["WPASL"] = WPASL.c_str();
    local["HOSTL"] = HOSTL.c_str();
    local["PORTL"] = PORTL.c_str();
    local["MNTPL"] = MNTPL.c_str();
    local["USERL"] = USERL.c_str();
    local["UPASL"] = UPASL.c_str();
    JsonObject adv = json["adv"].createNestedObject();
    adv["INPT"] = INPT.c_str();
    adv["FLOW"] = FLOW.c_str();
    adv["FUPP"] = FUPP.c_str();
    adv["MODE"] = MODE.c_str();
    adv["READ"] = READ.c_str();
    adv["CONF"] = CONF.c_str();
    JsonObject info = json["info"].createNestedObject();
    info["SERL"] = SERL.c_str();
    info["SREV"] = SREV.c_str();
    info["MODC"] = MODC.c_str();
    info["FIRC"] = FIRC.c_str();
    File configFile = SPIFFS.open(filename, "w");
    if (!configFile){
      Serial.println("failed to open btx_conf.json file for writing");
    }
    if (serializeJson(json, configFile) == 0){
      Serial.println(F("Failed to write to file"));
    }
    configFile.close();
  }
}

String CriaGGA(String lati, String logi, String alti, String tutc){
  String p = "GNGGA,";
  String p1 = "";
  String p2 = "";
  String newLati, NorS, EorW, newLong;
  float decimalCoord, minutos;
  int graus;
  for(int i = 0; i < 2; i++){
    if(i==0){
      decimalCoord = lati.toFloat();   
      graus = (int)decimalCoord;
      minutos = (abs(decimalCoord) - abs(graus)) * 60;
      char resultado[15]; // buffer para armazenar o resultado formatado
      // Formatar a string no formato desejado
      snprintf(resultado, sizeof(resultado), "%02d%09.6f", graus, minutos);
      String newResult = String(resultado);
      if (newResult.indexOf("-")<=0){
        NorS = "S";
        newLati = newResult.substring(newResult.indexOf("-")+1);
        p1 = "";
        p1 = newLati + String(",")+ NorS;
      }else{
        NorS = "N";
        p1 = "";
        p1 = resultado + String(",")+ NorS;
      }
    }else if(i==1){
      decimalCoord = logi.toFloat();   
      graus = (int)decimalCoord;
      minutos = (abs(decimalCoord) - abs(graus)) * 60;
      char resultado1[15]; // buffer para armazenar o resultado formatado
      // Formatar a string no formato desejado
      snprintf(resultado1, sizeof(resultado1), "%02d%09.6f", graus, minutos);
      String newResult1 = String(resultado1);
      if (newResult1.indexOf("-")<=0){
        EorW = "W";
        newLong = newResult1.substring(newResult1.indexOf("-")+1);
        p2 = "";
        p2 = newLong + String(",")+ EorW;
      }else{
        EorW = "E";
        p2 = "";
        p2 = resultado1 + String(",")+ EorW;
      }
    }
  }
  String newalt = alti.substring(0,6);
  p = p + tutc + String(",") + p1 + String(",0") + p2 + String(",1,00,1.0,")+ newalt + String(",M,0.0,M,1.0,0000");
  int checksum = 0;
  for(int e = 0; e < p.length(); e++){
    checksum ^= p[e];
  }
  String checksumHex = String(checksum,HEX);
  if (checksumHex.length() < 2) {
    checksumHex = "0" + checksumHex;
  }
  checksumHex.toUpperCase();
  String GNGGA = "$" + p + "*" + checksumHex;
  return GNGGA;
}

void IRAM_ATTR handleInterrupt() {
    interruptFlag = true;
}

void setup(){
  Serial.begin(115200);
  init_gpio();
  ext_com(LOW);
  leds.begin();

  for(int l = 0; l < 255; l++) {
    leds.setLedColorData(0, l, l, l);
    leds.show();
    delay(4);
  }

  usbpd(12);
  delay(500);

  if (digitalRead(PDPG) == HIGH) {
    USBPowerIssueFlag = true;
  }

  if (!SPIFFS.begin(true)) {
    handleSpiFFSError();
  }

  loaddata(BTX_data);
  Serial.println("inicia radio");
  init1 = du2005begin();
  if (init1 == 0) {
    handleRadioError();
  }

  if (CONF == "1") {
    setupWiFiAndServer();
  }

  pinMode(BT1, INPUT);
  attachInterrupt(digitalPinToInterrupt(BT1), handleInterrupt, CHANGE);    
}

void handleSpiFFSError() {
  while (true) {
    delay(1);
    leds.setLedColorData(0, 0, 255, 255);
    leds.show();
    delay(1);
    leds.setLedColorData(1, 0, 255, 255);
    leds.show();
    delay(1000);
    leds.setLedColorData(0, 0, 0, 0);
    leds.show();
    delay(1);
    leds.setLedColorData(1, 0, 0, 0);
    leds.show();
    delay(1000);
  }
}

void handleRadioError() {
  while (true) {
    delay(1);
    leds.setLedColorData(0, 128, 0, 128);
    leds.show();
    delay(1);
    leds.setLedColorData(1, 128, 0, 128);
    leds.show();
    delay(1000);
    leds.setLedColorData(0, 0, 0, 0);
    leds.show();
    delay(1);
    leds.setLedColorData(1, 0, 0, 0);
    leds.show();
    delay(1000);
  }
}

void setupWiFiAndServer() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("HugenPLUS-RADIO", NULL, 7, 0, 10);
  Serial.println("Wait 100 ms for AP_START...");
  delay(100);
  
  IPAddress Ip(192, 168, 0, 1);
  IPAddress Iplocal(192, 168, 0, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Iplocal, Ip, NMask);
  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(53, "btx02.local", Ip);
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/index.html", String(), false, processor);
      conf_btx = 0;
    });
    server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(204);
    });
    server.on("/btx_conf.json", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/btx_conf.json", "text/javascript");
    });
    server.on("/ntrip_app.json", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200);
    });     
    server.on("/client", HTTP_POST, [](AsyncWebServerRequest *request){
      // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
      if (request->hasParam("txc", true)){
        AsyncWebParameter* ba = request->getParam("txc", true);   
        AsyncWebParameter* bc = request->getParam("pwrc", true);
        AsyncWebParameter* bd = request->getParam("prtc", true);
        AsyncWebParameter* be = request->getParam("wifi", true);
        AsyncWebParameter* bf = request->getParam("wifipass", true);
        AsyncWebParameter* bg = request->getParam("IP", true);
        AsyncWebParameter* bh = request->getParam("port", true);
        AsyncWebParameter* bi = request->getParam("mountpt", true);
        AsyncWebParameter* bj = request->getParam("user", true);
        AsyncWebParameter* bk = request->getParam("userpass", true);
        AsyncWebParameter* bl = request->getParam("Latitude", true);
        AsyncWebParameter* bm = request->getParam("Longitude", true);
        AsyncWebParameter* bn = request->getParam("Precisao", true);
        AsyncWebParameter* bo = request->getParam("Altitude", true);
        AsyncWebParameter* bp = request->getParam("tempoutc", true);
        AsyncWebParameter* bq = request->getParam("ChLoc", true);

        CHTXC = ba->value();
        TPWRC = bc->value();
        TPRTC = bd->value();
        WIFI =  be->value();
        WPAS =  bf->value();
        HOST =  bg->value();
        PORT =  bh->value();
        MNTP =  bi->value();
        USER =  bj->value();
        UPAS =  bk->value();
        Latitude =  bl->value();
        Longitude = bm->value();
        Precisao =  bn->value();
        Altitude =  bo->value();
        Tempoutc =  bp->value();
        ChLoc =     bq->value();
      }
      INPT = "CLIENT";
      SBUDC = "115200";
      conf_btx = 1;
      request->send(200);
      //request->send(SPIFFS, "/index.html", String(), false, processor);
    });
    server.on("/local", HTTP_POST, [](AsyncWebServerRequest *request){
      // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
      if (request->hasParam("sbaudl", true)){
        AsyncWebParameter* ya = request->getParam("sbaudl", true);
        AsyncWebParameter* yb = request->getParam("wifil", true);
        AsyncWebParameter* yc = request->getParam("wifipassl", true);
        AsyncWebParameter* yd = request->getParam("IPl", true);
        AsyncWebParameter* ye = request->getParam("mountptl", true);
        AsyncWebParameter* yf = request->getParam("userl", true);
        AsyncWebParameter* yg = request->getParam("userpassl", true);

        SBAUDL = ya->value();
        WIFIL = yb->value();
        WPASL = yc->value();
        HOSTL = yd->value();
        MNTPL = ye->value();
        USERL = yf->value();
        UPASL = yg->value();
      }
      INPT = "LOCAL";
      conf_btx = 1;
      request->send(200);
    });
    server.on("/serial", HTTP_POST, [](AsyncWebServerRequest *request){
      if (request->hasParam("tx", true)){
        AsyncWebParameter* aa = request->getParam("tx", true);
        AsyncWebParameter* ab = request->getParam("rx", true);
        AsyncWebParameter* ac = request->getParam("sbaud", true);
        AsyncWebParameter* ad = request->getParam("pwr", true);
        AsyncWebParameter* ae = request->getParam("prt", true);
    // Armazena os valores recebidos em variáveis
        CHTX = aa->value();
        CHRX = ab->value();
        SBUD = ac->value();
        TPWR = ad->value();
        TPRT = ae->value();
      }
      INPT = "SERIAL";
      conf_btx = 1;
      request->send(200);
    }); 
    server.on("/txChannel", HTTP_POST, [](AsyncWebServerRequest *request){
      if (request->hasParam("ch_0_tx",true)){
        AsyncWebParameter* sa = request->getParam("ch_0_tx", true);
        AsyncWebParameter* sb = request->getParam("ch_1_tx", true);
        AsyncWebParameter* sc = request->getParam("ch_2_tx", true);
        AsyncWebParameter* sd = request->getParam("ch_3_tx", true);
        AsyncWebParameter* se = request->getParam("ch_4_tx", true);
        AsyncWebParameter* sf = request->getParam("ch_5_tx", true);
        AsyncWebParameter* sg = request->getParam("ch_6_tx", true);
        AsyncWebParameter* sh = request->getParam("ch_7_tx", true);
        AsyncWebParameter* si = request->getParam("ch_8_tx", true);
        AsyncWebParameter* sj = request->getParam("ch_9_tx", true);
        CTX0 = sa->value();
        CTX1 = sb->value();
        CTX2 = sc->value();
        CTX3 = sd->value();
        CTX4 = se->value();
        CTX5 = sf->value();
        CTX6 = sg->value();
        CTX7 = sh->value();
        CTX8 = si->value();
        CTX9 = sj->value();
      }
      else {
        Serial.println("Credenciais não encontradas");
      }
      conf_btx=1;
      request->send(200);
    }); 
    server.on("/rxChannel", HTTP_POST, [](AsyncWebServerRequest *request){
      if (request->hasParam("ch_0_rx",true)){
        AsyncWebParameter* da = request->getParam("ch_0_rx", true);
        AsyncWebParameter* db = request->getParam("ch_1_rx", true);
        AsyncWebParameter* dc = request->getParam("ch_2_rx", true);
        AsyncWebParameter* dd = request->getParam("ch_3_rx", true);
        AsyncWebParameter* de = request->getParam("ch_4_rx", true);
        AsyncWebParameter* df = request->getParam("ch_5_rx", true);
        AsyncWebParameter* dg = request->getParam("ch_6_rx", true);
        AsyncWebParameter* dh = request->getParam("ch_7_rx", true);
        AsyncWebParameter* di = request->getParam("ch_8_rx", true);
        AsyncWebParameter* dj = request->getParam("ch_9_rx", true);
        CRX0 = da->value();
        CRX1 = db->value();
        CRX2 = dc->value();
        CRX3 = dd->value();
        CRX4 = de->value();
        CRX5 = df->value();
        CRX6 = dg->value();
        CRX7 = dh->value();
        CRX8 = di->value();
        CRX9 = dj->value();
      }
      else {
        Serial.println("Credenciais não encontradas");
      }
      conf_btx=1;
      request->send(200);
    });
    
    server.on("/adv", HTTP_POST, [](AsyncWebServerRequest *request){
      if (request->hasParam("modo",true)){
        AsyncWebParameter* fa = request->getParam("modo", true);
        AsyncWebParameter* fb = request->getParam("flow", true);
        AsyncWebParameter* fc = request->getParam("fupp", true);
        AsyncWebParameter* fd = request->getParam("read", true);
        MODE = fa->value();
        FLOW = fb->value();
        FUPP = fc->value();
        READ = fd->value();
      }
      else {
        Serial.println("Credenciais não encontradas");
      }
      conf_btx=1;
      request->send(200);
    });
    server.on("/closeconf", HTTP_POST, [](AsyncWebServerRequest *request){
      conf_btx=1;
      conf_exit=1;
      request->send(200);
    });
    server.on("/apprequest", HTTP_POST, [](AsyncWebServerRequest *request){
      MAC = WiFi.softAPmacAddress();
      request->send(200, "application/json", MAC);
    });
  
  server.begin();
}

void loop() {
  if (CONF == "1") {
    dnsServer.processNextRequest();

    if (conf_btx == 1) {
      saveConfigFile(BTX_data);
      conf_btx = 0;
    }

    if (conf_exit == 1 && conf_btx == 0) {
      CONF = "0";
      saveConfigFile(BTX_data);
      delay(1000);
      ESP.restart();
    }

    if (USBPowerIssueFlag == false) {
      handleLedAnimation();
    } else if (USBPowerIssueFlag == true) {
      handleUsbPowerIssue();
    }
    
    handleInterruptProcessingConfig();
  } else if (CONF == "0") {
    if (INPT == "SERIAL") {
      mserial();
    } else if (INPT == "CLIENT") {
      cliente();
    } else if (INPT == "LOCAL") {
      local();
    }
  }
}

void handleLedAnimation() {
  static int j = 0;
  leds.setLedColorData(1, leds.Wheel(j));
  leds.show();
  delay(9);
  if (j < 252) {
    j += 2;
  } else {
    j = 0;
  }
}

void handleUsbPowerIssue() {
  INPT = "LOCAL";
  saveConfigFile(BTX_data);
  server.on("/USBPowerIssue", HTTP_GET, [](AsyncWebServerRequest * request){
      request->send(200);
  });
  if (CONF == "1") {
    leds.setLedColorData(0, 255, 255, 0);
    leds.show();
    handleLedAnimation();
  } else if (CONF = "0")  {
    return;
  }
}

void handleInterruptProcessingConfig() {
  if (interruptFlag) {
    if (digitalRead(BT1) == 1) {
      for (int r = 0; r < 2; r++) {
        for (int lh = 0; lh < 200; lh++) {
          delay(3);
          leds.setLedColorData(0, 0, 0, lh);
          leds.show();
          delay(3);
          leds.setLedColorData(1, 0, 0, lh);
          leds.show();
        }
        for (int ll = 200; ll > 0; ll--) {
          delay(3);
          leds.setLedColorData(0, 0, 0, ll);
          leds.show();
          delay(3);
          leds.setLedColorData(1, 0, 0, ll);
          leds.show();
        }
      }
      if (digitalRead(BT1) == 1) {
        CONF = "0";
        saveConfigFile(BTX_data);
        delay(100);
        ESP.restart();
      } else {
        delay(1);
        leds.setLedColorData(0, 250, 250, 250);
        leds.show();
        delay(1);
      }
    }
    interruptFlag = false;
  }
}

void handleInterruptProcessingModes() {
  if (interruptFlag) {
    if (digitalRead(BT1) == 1) {
      for (int r = 0; r < 2; r++) {
        for (int lh = 0; lh < 200; lh++) {
          delay(3);
          leds.setLedColorData(0, 0, 0, lh);
          leds.show();
          delay(3);
          leds.setLedColorData(1, 0, 0, lh);
          leds.show();
        }
        for (int ll = 200; ll > 0; ll--) {
          delay(3);
          leds.setLedColorData(0, 0, 0, ll);
          leds.show();
          delay(3);
          leds.setLedColorData(1, 0, 0, ll);
          leds.show();
        }
      }
      if (digitalRead(BT1) == 1) {
        CONF = "1";
        saveConfigFile(BTX_data);
        delay(100);
        ESP.restart();
      } else {
        delay(1);
        leds.setLedColorData(0, 250, 250, 250);
        leds.show();
        delay(1);
      }
    }
    interruptFlag = false;
  }
}

void handleInterruptProcessingNstop() {
  if (interruptFlag) {
    if(digitalRead(BT1)==1){
           for(int r=0;r<2;r++){
              for(int lh=0;lh<200;lh++){
                delay(3);
                leds.setLedColorData(0, 0, 0, lh);
                leds.show();
                delay(3);
                leds.setLedColorData(1, 0, 0, lh);
                leds.show();
              }
              for(int ll=200;ll>0;ll--){
                delay(3);
                leds.setLedColorData(0, 0, 0, ll);
                leds.show();
                delay(3);
                leds.setLedColorData(1, 0, 0, ll);
                leds.show();
              }
          }
          if(digitalRead(BT1)==1){
            CONF="1";
            saveConfigFile(BTX_data);
            delay(100);
            ntrip_c.stop();
            ESP.restart();
          }
          delay(1);
          leds.setLedColorData(0, 255, 135, 0);
          leds.show();
          delay(1);
       }
   } interruptFlag = false;
}

void cliente(){
  pinMode(BT1, INPUT);
  attachInterrupt(digitalPinToInterrupt(BT1), handleInterrupt, CHANGE);
  WiFi.mode(WIFI_STA);
  WiFi.begin(cWIFI, cWPAS);
  int t=0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    //Serial.println("Connecting to WiFi..");
    if(t>5){
      for(int wait=0;wait<10;wait++){
      leds.setLedColorData(0, 250, 130, 0);
      leds.show();
      delay(1);
      leds.setLedColorData(1, 0, 0, 0);
      leds.show();
      delay(500);
      leds.setLedColorData(0, 0, 0, 0);
      leds.show();
      delay(1);
      leds.setLedColorData(1, 250, 130, 0);
      leds.show();
      delay(500);
      handleInterruptProcessingModes();
    }
    ESP.restart();
    }else{
      t++;
    }
  }
  Serial.println(WiFi.localIP());
  int iPORT = strtol(PORT.c_str(), NULL, 0);
  Serial.println("Requesting SourceTable.");
  if(ntrip_c.reqSrcTbl(cHOST,iPORT)){
    char buffer[512];
    delay(1000);
    while(ntrip_c.available()){
      ntrip_c.readLine(buffer,sizeof(buffer));
      Serial.print(buffer); 
    }
  }else{
    Serial.println("SourceTable request error");
  }
  Serial.print("Requesting SourceTable is OK\n");
  ntrip_c.stop(); //Need to call "stop" function for next request.
  Serial.println("Requesting MountPoint's Raw data");     
  if(ChLoc=="S"){
    if(Latitude!=""&&Longitude!=""&&Altitude!=""&&Tempoutc!=""){
      String nmeaGGA = CriaGGA(Latitude,Longitude,Altitude,Tempoutc);
      if(!ntrip_c.reqRaw(cHOST,iPORT,cMNTP,cUSER,cUPAS,nmeaGGA,ChLoc)){
      Serial.println("unable to connect with ntrip caster");
      delay(100);
      for(int wait=0;wait<10;wait++){
        leds.setLedColorData(0, 100, 0, 0);
        leds.show();
        delay(1);
        leds.setLedColorData(1, 0, 0, 0);
        leds.show();
        delay(1);
        leds.setLedColorData(0, 100, 0, 0);
        leds.show();
        delay(1);
        leds.setLedColorData(1, 0, 0, 0);
        leds.show();
        delay(500);
        leds.setLedColorData(0, 0, 0, 0);
        leds.show();
        delay(1);
        leds.setLedColorData(1, 100, 0, 0);
        leds.show();
        delay(1);
        leds.setLedColorData(0, 0, 0, 0);
        leds.show();
        delay(1);
        leds.setLedColorData(1, 100, 0, 0);
        leds.show();
        delay(500);
        handleInterruptProcessingNstop();
      }
      ntrip_c.stop();
      ESP.restart();
      }
    }
  }else if (ChLoc=="N"){
    if(!ntrip_c.reqRaw(cHOST,iPORT,cMNTP,cUSER,cUPAS,"","")){
      Serial.println("unable to connect with ntrip caster");
      delay(100);
      for(int wait=0;wait<10;wait++){
        leds.setLedColorData(0, 100, 0, 0);
        leds.show();
        delay(1);
        leds.setLedColorData(1, 0, 0, 0);
        leds.show();
        delay(1);
        leds.setLedColorData(0, 100, 0, 0);
        leds.show();
        delay(1);
        leds.setLedColorData(1, 0, 0, 0);
        leds.show();
        delay(500);
        leds.setLedColorData(0, 0, 0, 0);
        leds.show();
        delay(1);
        leds.setLedColorData(1, 100, 0, 0);
        leds.show();
        delay(1);
        leds.setLedColorData(0, 0, 0, 0);
        leds.show();
        delay(1);
        leds.setLedColorData(1, 100, 0, 0);
        leds.show();
        delay(500);
        handleInterruptProcessingNstop();
      }
      ntrip_c.stop();
      ESP.restart();
    }
  }
  leds.setLedColorData(0, 0, 200, 0);
  leds.show();
  delay(1);
  input = 1;
  digitalWrite(CONFP,HIGH);
  while(true){
    if(ntrip_c.available()){
      currentMillis = millis();
      delay(1);
      leds.setLedColorData(1, 0, 0, 200);
      leds.show();
      while(ntrip_c.available()){     
        readcount = ntrip_c.readBytes(ch0, ntrip_c.available());
        //readcount = ntrip_c.readLine(ch0, 2500);
        Serial1.write(ch0, readcount);
        //Serial.println(ch0);
        if(ntrip_c.available()==0){
          Serial.println(readcount);
          readcount = 0;
          return;
        }
        if(digitalRead(BT1)==1){
          readcount = 0;
          return;
        }
      }
    }
    atualMillis = millis();
    if(atualMillis - currentMillis > 30000){
      for(int n=0; n<2; n++){
        delay(1);
        leds.setLedColorData(0, 0, 0, 0);
        leds.show();
        delay(500);
        leds.setLedColorData(0, 0, 200, 0);
        leds.show();
        delay(500);
      }
      ntrip_c.stop();
      ESP.restart();
    }
    delay(1);
    leds.setLedColorData(1, 0, 0, 0);
    leds.show();
    handleInterruptProcessingNstop();
    if(digitalRead(BT2)==1){
      if(TPWR=="H"){
        for(int y=0;y<2;y++){
          delay(1);
          leds.setLedColorData(0, 250,0,0);
          leds.show();
          delay(250);
          leds.setLedColorData(0, 0,0,0);
          leds.show();
          delay(250);
        }
        if(digitalRead(BT2)==1){
          TPWR="L";
          saveConfigFile(BTX_data);
          du2005conf("PWR",TPWR);
          digitalWrite(CONFP,HIGH);
          for(int v=0;v<2;v++){
            delay(1);
            leds.setLedColorData(0, 0,0,250);
            leds.show();
            delay(250);
            leds.setLedColorData(0, 0,0,0);
            leds.show();
            delay(250);
          }
        }
      }
      if(TPWR=="M"){
        for(int z=0;z<2;z++){
          delay(1);
          leds.setLedColorData(0, 0,250,0);
          leds.show();
          delay(250);
          leds.setLedColorData(0, 0,0,0);
          leds.show();
          delay(250);
        }
        if(digitalRead(BT2)==1){
          TPWR="H";
          saveConfigFile(BTX_data);
          du2005conf("PWR",TPWR);
          digitalWrite(CONFP,HIGH);
          for(int x=0;x<2;x++){
            delay(1);
            leds.setLedColorData(0,250,0,0);
            leds.show();
            delay(250);
            leds.setLedColorData(0, 0,0,0);
            leds.show();
            delay(250);
          }
        }
      }
      if(TPWR=="L"){
        for(int b=0;b<2;b++){
          delay(1);
          leds.setLedColorData(0, 0,0,250);
          leds.show();
          delay(250);
          leds.setLedColorData(0, 0,0,0);
          leds.show();
          delay(250);
        }
        if(digitalRead(BT2)==1){
          TPWR="M";
          saveConfigFile(BTX_data);
          du2005conf("PWR",TPWR);
          digitalWrite(CONFP,HIGH);
          for(int k=0;k<2;k++){
            delay(1);
            leds.setLedColorData(0, 0,250,0);
            leds.show();
            delay(250);
            leds.setLedColorData(0, 0,0,0);
            leds.show();
            delay(250);
          }
        }
      }
      ntrip_c.stop();
      ESP.restart();
    }
  }
}

void mserial(){
  pinMode(BT1, INPUT);
  attachInterrupt(digitalPinToInterrupt(BT1), handleInterrupt, CHANGE);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  leds.setLedColorData(0, 255, 135, 0);
  leds.show();
  delay(1);
  input = 0;
  Serial1.end();
  pinMode(TXDT,INPUT);
  pinMode(RXDT,INPUT);
  ext_com(HIGH);
  digitalWrite(CONFP,HIGH);
  if(FlagOnly5V==false){
    digitalWrite(POUTEN,HIGH);
  }
  while(true){
    delay(1);
    if(digitalRead(RXDT)==0){
      leds.setLedColorData(1, 0, 0, 200);
      leds.show();
      while(true){
        if(digitalRead(RXDT)==1){
          return;
        }
        if(digitalRead(BT1)==1){
          return;
        }
      }
    }
    delay(1);
    leds.setLedColorData(1, 0, 0, 0);
    leds.show();
    handleInterruptProcessingNstop();
    if(digitalRead(BT2)==1){
      if(TPWR=="H"){
        for(int y=0;y<2;y++){
          delay(1);
          leds.setLedColorData(0, 250,0,0);
          leds.show();
          delay(250);
          leds.setLedColorData(0, 0,0,0);
          leds.show();
          delay(250);
        }
        if(digitalRead(BT2)==1){
          TPWR="L";
          saveConfigFile(BTX_data);
          du2005conf("PWR",TPWR);
          digitalWrite(CONFP,HIGH);
          for(int v=0;v<2;v++){
            delay(1);
            leds.setLedColorData(0, 0,0,250);
            leds.show();
            delay(250);
            leds.setLedColorData(0, 0,0,0);
            leds.show();
            delay(250);
          }
        }
      }
      if(TPWR=="M"){
        for(int z=0;z<2;z++){
          delay(1);
          leds.setLedColorData(0, 0,250,0);
          leds.show();
          delay(250);
          leds.setLedColorData(0, 0,0,0);
          leds.show();
          delay(250);
        }
        if(digitalRead(BT2)==1){
          TPWR="H";
          saveConfigFile(BTX_data);
          du2005conf("PWR",TPWR);
          digitalWrite(CONFP,HIGH);
          for(int x=0;x<2;x++){
            delay(1);
            leds.setLedColorData(0,250,0,0);
            leds.show();
            delay(250);
            leds.setLedColorData(0, 0,0,0);
            leds.show();
            delay(250);
          }
        }
      }
      if(TPWR=="L"){
        for(int b=0;b<2;b++){
          delay(1);
          leds.setLedColorData(0, 0,0,250);
          leds.show();
          delay(250);
          leds.setLedColorData(0, 0,0,0);
          leds.show();
          delay(250);
        }
        if(digitalRead(BT2)==1){
          TPWR="M";
          saveConfigFile(BTX_data);
          du2005conf("PWR",TPWR);
          digitalWrite(CONFP,HIGH);
          for(int k=0;k<2;k++){
            delay(1);
            leds.setLedColorData(0, 0,250,0);
            leds.show();
            delay(250);
            leds.setLedColorData(0, 0,0,0);
            leds.show();
            delay(250);
          }
        }
      }
      ESP.restart();
    }
  }
}

void local(){
  pinMode(BT1, INPUT);
  attachInterrupt(digitalPinToInterrupt(BT1), handleInterrupt, CHANGE); 
  handleInterruptProcessingModes();
  //WiFi.disconnect();
  Serial.println("modo local");
   WiFi.mode(WIFI_AP); 
   WiFi.softAP(cWIFIL,cWPASL,7,0,10);   //launch the access point
   Serial.println("Wait 100 ms for AP_START...");
   delay(100);
   //Serial.println("Setting the AP");
   IPAddress Ip(192, 168, 0, 1);    //setto IP Access Point same as gateway
   IPAddress Iplocal(192, 168, 0, 1); 
   IPAddress NMask(255, 255, 255, 0);
   WiFi.softAPConfig(Iplocal, Ip, NMask);
   delay(100);
  wifiServer.begin();
  v5p(LOW);
  digitalWrite(CONFP,LOW);
  Serial.print("IP: ");Serial.println(WiFi.softAPIP());
  Serial1.begin(SBAUDL.toInt(), SERIAL_8N1, RXDT, TXDT);
  digitalWrite(TXCF,HIGH);
  delay(100);
  delay(1);
  ext_com(HIGH);
  if(FlagOnly5V==false){
    digitalWrite(POUTEN,HIGH);
  }  
  leds.setLedColorData(0, 255, 0, 0);
  leds.show();
  leds.setLedColorData(1, 0, 0, 0);
  leds.show();
  Serial.println("esperando por cliente");
  while(true){
  handleInterruptProcessingModes();
    WiFiClient client = wifiServer.available();
    //Serial.print(".");
    if (client) {
      //Serial.println(client);
   
      while (client.connected()) {
        int counter = 0;
        handleInterruptProcessingModes();
        
        while (client.available()>0) {
         handleInterruptProcessingModes();
         
          cliente_data_ap[counter] = client.read();
          counter++;
          if(client.available()==0){        
            
            String c_data_s = String(cliente_data_ap);
  
            if(c_data_s.indexOf("$GPGGA")==-1){
              
              currentMillis_ap = millis();
              switch(check_ver(c_data_s)*check_mountpoint(c_data_s,MNTPL)*check_aut(c_data_s,USERL,UPASL)){
                case 40:  Serial.println("REV1 + mount point correto + autentificação correta");// REV1 + mount point correto + autentificação correta
                          client.println(res_rev1);
                          flag_ok_ap = true;
                          delay(1);
                          leds.setLedColorData(0, 0, 255, 0);
                          leds.show();
                  break;
                case 48:  Serial.println("REV1 + solicita mount point + autentificação correta");// REV1 + solicita mount point + autentificação correta
                          client.println(res_rev1_ST);
                          client.println(scrtbl(cMNTPL,WiFi.softAPIP().toString(),port));
                          //Serial.println(scrtbl(cMNTPL,WiFi.softAPIP().toString(),port));
                          client.stop();
                  break;
                case 56:  Serial.println("REV1 + mount point incorreto + autentificação correta");// REV1 + mount point incorreto + autentificação correta
                          flag_ok_ap = false;
                  break;
                case 45:  Serial.println("REV1 + mount point correto + autentificação incorreta");// REV1 + mount point correto + autentificação incorreta
                          flag_ok_ap = false;
                  break;
                case 54:  Serial.println("REV1 + solicita mount point + autentificação incorreta");// REV1 + solicita mount point + autentificação incorreta
                          flag_ok_ap = false;
                  break; 
                case 63:  Serial.println("REV1 + mount point incorreto + autentificação incorreta");// REV1 + mount point incorreto + autentificação incorreta
                          flag_ok_ap = false;
                  break;
  
                case 80:  Serial.println("REV2 + mount point correto + autentificação correta");// REV2 + mount point correto + autentificação correta
                          client.write(res_rev2);
                          flag_ok_ap = true;
                          //Serial1.begin(SBAUDL.toInt(), SERIAL_8N1, RXDT, TXDT);
                          //digitalWrite(TXCF,HIGH);
                          delay(100);
                          delay(1);
                          leds.setLedColorData(0, 0, 255, 0);
                          leds.show();
                  break;
                case 96:  Serial.println("REV2 + solicita mount point + autentificação correta");// REV2 + solicita mount point + autentificação correta
                          client.println(res_rev1_ST);
                          client.println(scrtbl(cMNTPL,WiFi.softAPIP().toString(),port));
                          //Serial.println(scrtbl(cMNTPL,WiFi.softAPIP().toString(),port));
                          client.stop();
                  break;
                case 112: Serial.println("REV2 + mount point incorreto + autentificação correta");// REV2 + mount point incorreto + autentificação correta
                          flag_ok_ap = false;
                  break;
                case 90:  Serial.println("REV2 + mount point correto + autentificação incorreta");// REV2 + mount point correto + autentificação incorreta
                          flag_ok_ap = false;
                  break;
                case 108: Serial.println("REV2 + solicita mount point + autentificação incorreta");// REV2 + solicita mount point + autentificação incorreta
                          flag_ok_ap = false;
                  break;
                case 126: Serial.println("REV2 + mount point incorreto + autentificação incorreta");// REV2 + mount point incorreto + autentificação incorreta
                          flag_ok_ap = false;
                  break;
  
                case 160: Serial.println("NAO HTTP + mount point correto + autentificação correta");// NAO HTTP + mount point correto + autentificação correta
                          flag_ok_ap = false;
                  break;
                case 192: Serial.println("NAO HTTP + solicita mount point + autentificação correta");// NAO HTTP + solicita mount point + autentificação correta
                          flag_ok_ap = false;
                  break;
                case 224: Serial.println("NAO HTTP + mount point incorreto + autentificação correta");// NAO HTTP + mount point incorreto + autentificação correta
                          flag_ok_ap = false;
                  break;
                case 180: Serial.println("NAO HTTP + mount point correto + autentificação incorreta");// NAO HTTP + mount point correto + autentificação incorreta
                          flag_ok_ap = false;
                  break;
                case 216: Serial.println("NAO HTTP + solicita mount point + autentificação incorreta");// NAO HTTP + solicita mount point + autentificação incorreta
                          flag_ok_ap = false;
                  break;
                case 252: Serial.println("NAO HTTP + mount point incorreto + autentificação incorreta");// NAO HTTP + mount point incorreto + autentificação incorreta
                          flag_ok_ap = false;
                  break;
              }
              long int size = sizeof(cliente_data_ap);
              memset(cliente_data_ap,' ',size*sizeof(char));
            }
            else{
              currentMillis_ap = millis();
              flag_send_ap=true;
            }
          } 
          
        }
        atualMillis_ap = millis();
         if(atualMillis_ap - currentMillis_ap > 2500 && flag_ok_ap == true && currentMillis_ap != 0 && flag_send_ap==true){
            client.stop();
            Serial.println(atualMillis_ap - currentMillis_ap);
            currentMillis_ap = 0;
            atualMillis_ap = 0;
            Serial.println("Client disconnected timeout");
            
            long int size = sizeof(cliente_data_ap);
            memset(cliente_data_ap,' ',size*sizeof(char));
            delay(1);
            leds.setLedColorData(0, 255, 0, 0);
            leds.show();
            delay(200);
            flag_send_ap=false;
            //ESP.restart();
        }      
        while (Serial1.available() && flag_ok_ap == true) {
          delay(1);
          leds.setLedColorData(1, 0, 0, 200);
          leds.show();
          readcount_ap = 0;
          while (Serial1.available()) {
            ch_ap[readcount_ap] = Serial1.read();
            readcount_ap++;
            if (readcount_ap > 511)break;
          }//buffering
          
          client.write((uint8_t*)ch_ap, readcount_ap);
          //Serial.println(readcount_ap);
          delay(1);
          leds.setLedColorData(1, 0, 0, 0);
          leds.show();
        } 
      }  
      delay(300);
      client.stop();
      Serial.println("Client disconnected");
      flag_ok_ap == false;
      long int size = sizeof(cliente_data_ap);
      currentMillis_ap = 0;
      atualMillis_ap = 0;
      memset(cliente_data_ap,' ',size*sizeof(char));
      delay(1);
      leds.setLedColorData(0, 255, 0, 0);
      leds.show();
      flag_send_ap=false;
    }
  } 
  handleInterruptProcessingModes();
}
