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

#define LEDS_COUNT  2

#define RADIO_data  "/radio_conf.json"
#define NTRIP_data  "/ntrip_conf.json"
#define BOARD_data  "/board_conf.json"

#define size_radio  1024
#define size_ntrip  512
#define size_board  256

#define CF3     4
#define CF2     5
#define CF1     6
#define PDPG    7
#define RSSIP    15
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

String WIFI, WPAS, HOST, PORT, MNTP, USER, UPAS;
String CHTX, CHRX, SBUD, TPWR, TPRT;
String CTX0, CTX1, CTX2, CTX3, CTX4, CTX5, CTX6, CTX7, CTX8, CTX9;
String CRX0, CRX1, CRX2, CRX3, CRX4, CRX5, CRX6, CRX7, CRX8, CRX9;
String MODE, FLOW, FUPP;
String INPT, SERL, SREV, MODC, FIRC, READ, CONF;

String commands[11]={"TX","RX","PWR","SBAUD","PRT","SREV","SER","FLOW","FUPP","MODE"};
String TXR, RXR;

char cWIFI[32]; char cWPAS[64]; char cHOST[16]; char cPORT[6]; char cMNTP[81]; char cUSER[31]; char cUPAS[31]; 
char cCHTX[2]; char cCHRX[2]; char cSBUD[7]; char cTPWR[2]; char cTPRT[10];
char cCTX0[10]; char cCTX1[10]; char cCTX2[10]; char cCTX3[10]; char cCTX4[10]; char cCTX5[10]; char cCTX6[10]; char cCTX7[10]; char cCTX8[10]; char cCTX9[10];
char cCRX0[10]; char cCRX1[10]; char cCRX2[10]; char cCRX3[10]; char cCRX4[10]; char cCRX5[10]; char cCRX6[10]; char cCRX7[10]; char cCRX8[10]; char cCRX9[10];
char cMODE[7]; char cFLOW[4]; char cFUPP[4];
char cINPT[7]; char cSERL[13]; char cSREV[11]; char cMODC[10]; char cFIRC[10]; char cREAD[4]; char cCONF[2];

int baudrates[5]={9600,19200,38400,57600,115200};
int input;
int conf_radio = 0;
int conf_board = 0;
int conf_ntrip = 0;
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
    //Serial.print("baudrate: ");
    //Serial.println(baudrates[i]);
    Serial1.begin(baudrates[i], SERIAL_8N1, TXDT, RXDT);
    delay(50);
    //Serial.print("resposta do radio: ");
    //Serial.println(du2005read("SER"));
    du2005read("SER");
    if(du2005read("SER")!="ERRO1"){
      return String(baudrates[i]);
    }    
  }
}

int programall(){
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
              }
              break;
            case 1: 
              if(CTX1==TXR){
                CHTX="1";
              }
              break;
            case 2: 
              if(CTX2==TXR){
                CHTX="2";
              }
              break;
            case 3: 
              if(CTX3==TXR){
                CHTX="3";
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
              }
              break;
            case 6: 
              if(CTX6==TXR){
                CHTX="6";
              }
              break;
            case 7: 
              if(CTX7==TXR){
                CHTX="7";
              }
              break;
            case 8: 
              if(CTX8==TXR){
                CHTX="8";
              }
              break;
            case 9: 
              if(CTX9==TXR){
                CHTX="9";
              }
              break;
            case 10:
              if(CTX0==""){
                CTX0==TXR;
                CHTX="0";
              }else{
                du2005conf("TX",CTX0);
                CHTX="0";
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
        if(TPWR=="ERRO1"){
          return 0;
        }
        break;
      case 3: 
        SBUD = du2005read(commands[e]);
        if(SBUD=="ERRO1"){
          return 0;
        }
        break;
      case 4: 
        TPRT = du2005read(commands[e]);
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
      if(filename=="/radio_conf.json"){
        File configFile = SPIFFS.open(filename, "r");
          if (configFile){
            StaticJsonDocument<size_radio> json;
            DeserializationError error = deserializeJson(json, configFile);
            if (!error){
              CHTX = strcpy(cCHTX, json["TX"]);
              CTX0 = strcpy(cCTX0, json["TX0"]);
              CTX1 = strcpy(cCTX1, json["TX1"]);
              CTX2 = strcpy(cCTX2, json["TX2"]);
              CTX3 = strcpy(cCTX3, json["TX3"]);
              CTX4 = strcpy(cCTX4, json["TX4"]);
              CTX5 = strcpy(cCTX5, json["TX5"]);
              CTX6 = strcpy(cCTX6, json["TX6"]);
              CTX7 = strcpy(cCTX7, json["TX7"]);
              CTX8 = strcpy(cCTX8, json["TX8"]);
              CTX9 = strcpy(cCTX9, json["TX9"]);
              CHRX = strcpy(cCHRX, json["RX"]);
              CRX0 = strcpy(cCRX0, json["RX0"]);
              CRX1 = strcpy(cCRX1, json["RX1"]);
              CRX2 = strcpy(cCRX2, json["RX2"]);
              CRX3 = strcpy(cCRX3, json["RX3"]);
              CRX4 = strcpy(cCRX4, json["RX4"]);
              CRX5 = strcpy(cCRX5, json["RX5"]);
              CRX6 = strcpy(cCRX6, json["RX6"]);
              CRX7 = strcpy(cCRX7, json["RX7"]);
              CRX8 = strcpy(cCRX8, json["RX8"]);
              CRX9 = strcpy(cCRX9, json["RX9"]);
              TPWR = strcpy(cTPWR, json["PWR"]);
              SBUD = strcpy(cSBUD, json["SBAUD"]);
              TPRT = strcpy(cTPRT, json["PRT"]);
              FLOW = strcpy(cFLOW, json["FLOW"]);
              FUPP = strcpy(cFUPP, json["FUPP"]);
              MODE = strcpy(cMODE, json["MODE"]);
              return true;
          }else{
              Serial.println("Failed to load radio conf");
          }
        }
      }
      if(filename=="/ntrip_conf.json"){
        File configFile = SPIFFS.open(filename, "r");
        if (configFile){
          StaticJsonDocument<size_ntrip> json;
          DeserializationError error = deserializeJson(json, configFile);
          if (!error){
            WIFI = strcpy(cWIFI, json["WIFI"]);
            WPAS = strcpy(cWPAS, json["WPAS"]);
            HOST = strcpy(cHOST, json["HOST"]);
            PORT = strcpy(cPORT, json["PORT"]);
            MNTP = strcpy(cMNTP, json["MNTP"]);
            USER = strcpy(cUSER, json["USER"]);
            UPAS = strcpy(cUPAS, json["UPAS"]);
            return true;
          }else{
              Serial.println("Failed to load ntrip conf");
          }
        }
      }
      if(filename=="/board_conf.json"){
        File configFile = SPIFFS.open(filename, "r");
        if (configFile){
          StaticJsonDocument<size_board> json;
          DeserializationError error = deserializeJson(json, configFile);
          if (!error){
            INPT = strcpy(cINPT, json["INPT"]);
            SERL = strcpy(cSERL, json["SERL"]);
            SREV = strcpy(cSREV, json["SREV"]);
            MODC = strcpy(cMODC, json["MODC"]);
            FIRC = strcpy(cFIRC, json["FIRC"]);
            READ = strcpy(cREAD, json["READ"]);
            CONF = strcpy(cCONF, json["CONF"]);
            return true;
          }else{
              Serial.println("Failed to load board conf");
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
  if(filename=="/radio_conf.json"){
    StaticJsonDocument<size_radio> json;
    json["TX"]    = CHTX.c_str();
    json["TX0"]   = CTX0.c_str();
    json["TX1"]   = CTX1.c_str();
    json["TX2"]   = CTX2.c_str();
    json["TX3"]   = CTX3.c_str();
    json["TX4"]   = CTX4.c_str();
    json["TX5"]   = CTX5.c_str();
    json["TX6"]   = CTX6.c_str();
    json["TX7"]   = CTX7.c_str();
    json["TX8"]   = CTX8.c_str();
    json["TX9"]   = CTX9.c_str();
    json["RX"]    = CHRX.c_str();
    json["RX0"]   = CRX0.c_str();
    json["RX1"]   = CRX1.c_str();
    json["RX2"]   = CRX2.c_str();
    json["RX3"]   = CRX3.c_str();
    json["RX4"]   = CRX4.c_str();
    json["RX5"]   = CRX5.c_str();
    json["RX6"]   = CRX6.c_str();
    json["RX7"]   = CRX7.c_str();
    json["RX8"]   = CRX8.c_str();
    json["RX9"]   = CRX9.c_str();
    json["PWR"]   = TPWR.c_str();
    json["SBAUD"] = SBUD.c_str();
    json["PRT"]   = TPRT.c_str();
    json["FLOW"]  = FLOW.c_str();
    json["FUPP"]  = FUPP.c_str();
    json["MODE"]  = MODE.c_str();
    File configFile = SPIFFS.open(filename, "w");
    if (!configFile){
      Serial.println("failed to open radio config file for writing");
    }
    if (serializeJson(json, configFile) == 0){
      Serial.println(F("Failed to write to file"));
    }
    configFile.close();
  }
  if(filename=="/ntrip_conf.json"){
    StaticJsonDocument<size_ntrip> json;
    json["WIFI"]  = WIFI.c_str();
    json["WPAS"]  = WPAS.c_str();
    json["HOST"]  = HOST.c_str();
    json["PORT"]  = PORT.c_str();
    json["MNTP"]  = MNTP.c_str();
    json["USER"]  = USER.c_str();
    json["UPAS"]  = UPAS.c_str();
    File configFile = SPIFFS.open(filename, "w");
    if (!configFile){
      Serial.println("failed to open ntrip config file for writing");
    }
    if (serializeJson(json, configFile) == 0){
      Serial.println(F("Failed to write to file"));
    }
    configFile.close();
  }
  if(filename=="/board_conf.json"){
    StaticJsonDocument<1536> json;
    json["INPT"]  = INPT.c_str();
    json["SERL"]  = SERL.c_str();
    json["SREV"]  = SREV.c_str();
    json["MODC"]  = MODC.c_str();
    json["FIRC"]  = FIRC.c_str();
    json["READ"]  = READ.c_str();
    json["CONF"]  = CONF.c_str();
    File configFile = SPIFFS.open(filename, "w");
    if (!configFile){
      Serial.println("failed to open config file for writing");
    }
    if (serializeJson(json, configFile) == 0){
      Serial.println(F("Failed to write to file"));
    }
    configFile.close();
  }
}

class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    //request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", String(), false, processor); 
  }
};

void setup(){
  Serial.begin(115200);
  init_gpio();
  ext_com(LOW);
  leds.begin();
  for(int l=0;l<255;l++){
    leds.setLedColorData(0, l, l, l);
    leds.show();
    delay(4);
  }
  usbpd(12);
  delay(500);
  /* se tiver modificação para funcionar somente com 5V
  if(digitalRead(PDPG)==HIGH){
    //Serial.println("Not usb pd");
    //modo 5v
    FlagOnly5V = true;
  }else{
    FlagOnly5V = false;
  }
  Serial.println("initing");
  */
  if(digitalRead(PDPG)==HIGH){
    while(true){
      delay(1);
      leds.setLedColorData(0, 255, 135, 0);
      leds.show();
      delay(1);
      leds.setLedColorData(1, 255, 135, 0);
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
   //Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    while(true){
      delay(1);
      leds.setLedColorData(0, 0, 250, 250);
      leds.show();
      delay(1);
      leds.setLedColorData(1, 0, 250, 250);
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
  loaddata(RADIO_data);
  loaddata(NTRIP_data);
  loaddata(BOARD_data);
  Serial.println("inicia radio");
  init1 = du2005begin();
  if(init1==0){
    Serial.println("radio not found");
    while(true){
      delay(1);
      leds.setLedColorData(0, 250, 0, 250);
      leds.show();
      delay(1);
      leds.setLedColorData(1, 250, 0, 250);
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
  saveConfigFile(RADIO_data);
  saveConfigFile(BOARD_data);
  if(CONF=="1"){
    WiFi.mode(WIFI_AP); 
    WiFi.softAP("HugenPLUS-RADIO",NULL,7,0,1);   //launch the access point
    Serial.println("Wait 100 ms for AP_START...");
    delay(100);
    //Serial.println("Setting the AP");
    IPAddress Ip(172, 217, 28, 1);    //setto IP Access Point same as gateway
    IPAddress NMask(255, 255, 255, 0);
    WiFi.softAPConfig(Ip, Ip, NMask);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/index.html", String(), false, processor);
    });
    server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/index.html", String(), false, processor);
    });
    server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "Erro 404!");
    });
    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/logo.jpg", "image/jpg");
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
      request->send(SPIFFS, "/radio_conf.json", "text/javascript");
    });
    server.on("/ntrip_conf.json", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/ntrip_conf.json", "text/javascript");
    });
    server.on("/board_conf.json", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/board_conf.json", "text/javascript");
    });
    server.on("/ntrip", HTTP_GET, [](AsyncWebServerRequest *request){
      // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
      if (request->hasParam("wifi")){
        WIFI = request->getParam("wifi")->value();
      }
      if (request->hasParam("wifipass")){
        WPAS = request->getParam("wifipass")->value();
      }
      if (request->hasParam("IP")){
        HOST = request->getParam("IP")->value();
      }
      if (request->hasParam("port")){
        PORT = request->getParam("port")->value();
      }
      if (request->hasParam("mountpt")){
        MNTP = request->getParam("mountpt")->value();
      }
      if (request->hasParam("user")){
        USER = request->getParam("user")->value();
      }
      if (request->hasParam("userpass")){
        UPAS = request->getParam("userpass")->value();
      }
      conf_ntrip = 1;
      request->send(SPIFFS, "/index.html", String(), false, processor);
    });
    server.on("/radio", HTTP_GET, [](AsyncWebServerRequest *request){
      if (request->hasParam("input")){
        INPT = request->getParam("input")->value();     
      }
      if (request->hasParam("tx")){
        CHTX = request->getParam("tx")->value();
      }
      if (request->hasParam("rx")){
        CHRX = request->getParam("rx")->value();
      }
      if (request->hasParam("sbaud")){
        SBUD = request->getParam("sbaud")->value();
      }
      if (request->hasParam("pwr")){
        TPWR = request->getParam("pwr")->value();
      }
      if (request->hasParam("prt")){
        TPRT = request->getParam("prt")->value();
      }
      conf_radio = 1;
      conf_board = 1;
      request->send(SPIFFS, "/index.html", String(), false, processor);
    }); 
    server.on("/txchanels", HTTP_GET, [](AsyncWebServerRequest *request){
      if (request->hasParam("ch_0_tx")){
        CTX0 = request->getParam("ch_0_tx")->value();
      }
      if (request->hasParam("ch_1_tx")){
        CTX1 = request->getParam("ch_1_tx")->value();
      }
      if (request->hasParam("ch_2_tx")){
        CTX2 = request->getParam("ch_2_tx")->value();
      }
      if (request->hasParam("ch_3_tx")){
        CTX3 = request->getParam("ch_3_tx")->value();
      }
      if (request->hasParam("ch_4_tx")){
        CTX4 = request->getParam("ch_4_tx")->value();
      }
      if (request->hasParam("ch_5_tx")){
        CTX5 = request->getParam("ch_5_tx")->value();
      }
      if (request->hasParam("ch_6_tx")){
        CTX6 = request->getParam("ch_6_tx")->value();
      }
      if (request->hasParam("ch_7_tx")){
        CTX7 = request->getParam("ch_7_tx")->value();
      }
      if (request->hasParam("ch_8_tx")){
        CTX8 = request->getParam("ch_8_tx")->value();
      }
      if (request->hasParam("ch_9_tx")){
        CTX9 = request->getParam("ch_9_tx")->value();
      }
      conf_radio=1;
      request->send(SPIFFS, "/index.html", String(), false, processor);
    }); 
    server.on("/rxchanels", HTTP_GET, [](AsyncWebServerRequest *request){
      if (request->hasParam("ch_0_rx")){
        CRX0 = request->getParam("ch_0_rx")->value();
      }
      if (request->hasParam("ch_1_rx")){
        CRX1 = request->getParam("ch_1_rx")->value();
      }
      if (request->hasParam("ch_2_rx")){
        CRX2 = request->getParam("ch_2_rx")->value();
      }
      if (request->hasParam("ch_3_rx")){
        CRX3 = request->getParam("ch_3_rx")->value();
      }
      if (request->hasParam("ch_4_rx")){
        CRX4 = request->getParam("ch_4_rx")->value();
      }
      if (request->hasParam("ch_5_rx")){
        CRX5 = request->getParam("ch_5_rx")->value();
      }
      if (request->hasParam("ch_6_rx")){
        CRX6 = request->getParam("ch_6_rx")->value();
      }
      if (request->hasParam("ch_7_rx")){
        CRX7 = request->getParam("ch_7_rx")->value();
      }
      if (request->hasParam("ch_8_rx")){
        CRX8 = request->getParam("ch_8_rx")->value();
      }
      if (request->hasParam("ch_9_rx")){
        CRX9 = request->getParam("ch_9_rx")->value();
      }
      conf_radio=1;
      request->send(SPIFFS, "/index.html", String(), false, processor);
    });
    server.on("/adv", HTTP_GET, [](AsyncWebServerRequest *request){
      if (request->hasParam("modo")){
        MODE = request->getParam("modo")->value();
      }
      if (request->hasParam("flow")){
        FLOW = request->getParam("flow")->value();
      }
      if (request->hasParam("fupp")){
        FUPP = request->getParam("fupp")->value();
      }
      if (request->hasParam("read")){
        READ = request->getParam("read")->value();
      }
      conf_radio=1;
      conf_board=1;
      request->send(SPIFFS, "/index.html", String(), false, processor);
    });
    server.on("/closeconf", HTTP_GET, [](AsyncWebServerRequest *request){
      CONF = "0";
      conf_board=1;
      conf_exit=1;
      request->send(SPIFFS, "/index.html", String(), false, processor);
    });
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", WiFi.softAPIP());
    server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
    
    server.begin();
    int j =0;
    while(true){
      dnsServer.processNextRequest();
      if(conf_ntrip==1){
        saveConfigFile(NTRIP_data);
        conf_ntrip=0;
      }
      if(conf_radio==1){
        saveConfigFile(RADIO_data);
        conf_radio=0;
      }
      if(conf_board==1){
        saveConfigFile(BOARD_data);
        conf_board=0;
      }
      if(conf_exit==1&&conf_ntrip==0&&conf_radio==0&&conf_board==0){
        ESP.restart();
      }
      delay(1);
      leds.setLedColorData(1, leds.Wheel(j));
      leds.show();
      delay(9);
      if(j<252){
        j+=2;
      }else{
        j=0;
      }
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
          CONF="0";
          saveConfigFile(BOARD_data);
          delay(100);
          ESP.restart();
        }else{
          delay(1);
          leds.setLedColorData(0,250,250,250);
          leds.show();
        }
      }
    }  
  }else if(CONF=="0"){ 
    if(INPT=="CLIENT"){
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
              saveConfigFile(BOARD_data);
              delay(100);
              ESP.restart();
            }
          }
        }
        ESP.restart();
        }else{
          t++;
        }
      }
      //Serial.println(WiFi.localIP());
      int iPORT = strtol(PORT.c_str(), NULL, 0);
      //Serial.println("Requesting SourceTable.");
      if(ntrip_c.reqSrcTbl(cHOST,iPORT)){
        char buffer[512];
        delay(5);
        while(ntrip_c.available()){
          ntrip_c.readLine(buffer,sizeof(buffer));
          //Serial.print(buffer); 
        }
      }else{
        //Serial.println("SourceTable request error");
      }
      //Serial.print("Requesting SourceTable is OK\n");
      ntrip_c.stop(); //Need to call "stop" function for next request.
      //Serial.println("Requesting MountPoint's Raw data");
      if(!ntrip_c.reqRaw(cHOST,iPORT,cMNTP,cUSER,cUPAS)){
        //Serial.println("unable to connect with ntrip caster");
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
              saveConfigFile(BOARD_data);
              delay(100);
              ESP.restart();
            }
          }
        }
        ESP.restart();
      }
      leds.setLedColorData(0, 0, 200, 0);
      leds.show();
      delay(1);
      //Serial.println("Requesting MountPoint is OK");
      input = 1;
      digitalWrite(CONFP,HIGH);
    }else if (INPT=="SERIAL"){
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
    }
  }
}
 
void loop(){
  if(input==1){
    if(ntrip_c.available()){
      currentMillis = millis();
      delay(1);
      leds.setLedColorData(1, 0, 0, 200);
      leds.show();
      while(ntrip_c.available()){     
        readcount = ntrip_c.readBytes(ch0, ntrip_c.available());
        //readcount = ntrip_c.readLine(ch0, 2500);
        Serial1.write(ch0, readcount);
        if(ntrip_c.available()==0){
          //Serial.println(readcount);
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
    if(atualMillis - currentMillis > 15000){
      for(int n=0; n<2; n++){
        delay(1);
        leds.setLedColorData(0, 0, 0, 0);
        leds.show();
        delay(500);
        leds.setLedColorData(0, 0, 200, 0);
        leds.show();
        delay(500);
      }
      ESP.restart();
    }
  }else if(input==0){
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
  }
  delay(1);
  leds.setLedColorData(1, 0, 0, 0);
  leds.show();
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
      saveConfigFile(BOARD_data);
      delay(100);
      ESP.restart();
    }
    if(input==1){
      delay(1);
      leds.setLedColorData(0, 0, 200, 0);
      leds.show();
      delay(1);
    }else if(input==0){
      delay(1);
      leds.setLedColorData(0, 255, 135, 0);
      leds.show();
      delay(1);
    }
  } 
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
        saveConfigFile(RADIO_data);
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
        saveConfigFile(RADIO_data);
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
        saveConfigFile(RADIO_data);
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
    /*if(input==1){
      delay(1);
      leds.setLedColorData(0, 0, 200, 0);
      leds.show();
      delay(1);
    }
    if(input==0){
      delay(1);
      leds.setLedColorData(0, 255, 135, 0);
      leds.show();
      delay(1);
    }*/
  }
}
