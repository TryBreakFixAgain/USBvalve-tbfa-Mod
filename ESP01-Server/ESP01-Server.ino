#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include "LittleFS.h"
#include "webcalls.h"
#define TRIGGER_PIN 0

//#define DEBUGIT

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";
const char* PARAM_INPUT_5 = "reset";

//Variables to save values from HTML form
String ssid;
String pass;
String ip;
String gateway;
String enterPortal;

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";
const char* enterPortalctrl = "/portal.txt";

//IPAddress localIP;
IPAddress localIP(192, 168, 42, 1); // hardcoded

// Set your Gateway IP address
//IPAddress localGateway;
IPAddress localGateway(192, 168, 42, 1); //hardcoded
IPAddress subnet(255, 255, 2550, 0);

// Timer variables
unsigned long previousMillis = 0;
const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)

boolean restart = false;
String incomingSerial = "";
int visitor = 0;

// Initialize LittleFS
void initFS() {
#if defined(DEBUGIT)  
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else{
    Serial.println("LittleFS mounted successfully");
  }
#else
  LittleFS.begin();
#endif
}

// Read File from LittleFS
String readFile(fs::FS &fs, const char * path){
#if defined(DEBUGIT) 
  Serial.printf("Reading file: %s\r\n", path);
#endif
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
#if defined(DEBUGIT)  
    Serial.println("- failed to open file for reading");
#endif
    return String();
  }

  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;
  }
  file.close();
  return fileContent;
}

// Write file to LittleFS
void writeFile(fs::FS &fs, const char * path, const char * message){
#if defined(DEBUGIT)   
  Serial.printf("Writing file: %s\r\n", path);
#endif
  File file = fs.open(path, "w");
  if(!file){
#if defined(DEBUGIT) 
    Serial.println("- failed to open file for writing");
#endif
    return;
  }
#if defined(DEBUGIT) 
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
#else
file.print(message);
#endif
  file.close();
}

// Initialize WiFi
bool initWiFi() {
  if(enterPortal=="yes"){
    return false;
  }
  if(enterPortal==""){
    writeFile(LittleFS, enterPortalctrl, "no");
    enterPortal="no";
  }
  if(ssid==""){
#if defined(DEBUGIT)
    Serial.println("Undefined SSID");
#endif
    return false;
  }

  WiFi.mode(WIFI_STA);
  if(ip==""){
    WiFi.begin(ssid.c_str(), pass.c_str());
  }else{
    localIP.fromString(ip.c_str());
    localGateway.fromString(gateway.c_str());

    if (!WiFi.config(localIP, localGateway, subnet)){
#if defined(DEBUGIT)
      Serial.println("STA Failed to configure");
#endif
      return false;
    }
    WiFi.begin(ssid.c_str(), pass.c_str());
  }
#if defined(DEBUGIT)
  Serial.println("Connecting to WiFi...");
#endif
  delay(20000);
  if(WiFi.status() != WL_CONNECTED) {
#if defined(DEBUGIT)
    Serial.println("Failed to connect.");
#endif
    return false;
  }
  String outip = WiFi.localIP().toString();
  Serial.println("IP: " + outip);
  return true;
}


void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  pinMode(TRIGGER_PIN, INPUT);
  initFS();
  
  // Load values saved in LittleFS
  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);
  ip = readFile(LittleFS, ipPath);
  gateway = readFile (LittleFS, gatewayPath);
  enterPortal = readFile (LittleFS, enterPortalctrl);
#if defined(DEBUGIT)
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);
  Serial.println(enterPortal);
#endif
  if(initWiFi()) {
#if defined(DEBUGIT)
    Serial.println("Setting Static");
#endif
    // Route for root / web page
    // Serial.println("Webportal");
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        visitor = 1;
        request->send_P(200, PSTR("text/html"), Web_page);
      });
      server.on("/valveout", HTTP_GET, [](AsyncWebServerRequest *request){
        if(incomingSerial != ""){
          request->send(200, "text/plain", String(incomingSerial)+"\n");
          incomingSerial="";
        }
      });
    server.begin();
  }else{
#if defined(DEBUGIT)
    Serial.println("Setting AP (Access Point)");
#endif
    if(enterPortal=="yes"){
      WiFi.softAP("USBvalveConfig", NULL);
      writeFile(LittleFS, enterPortalctrl, "no");
    }else{
      WiFi.softAP("USBvalve", NULL);
    }
      
    WiFi.softAPConfig(localIP, localGateway, subnet);
    IPAddress IP = WiFi.softAPIP();
#if defined(DEBUGIT)
    Serial.print("AP IP address: ");
#endif
    String outip = IP.toString();;
    Serial.println("IP: " + outip);
    // Web Server Root URL

    if(enterPortal=="yes"){
      // Serial.println("Configportal");
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, PSTR("text/html"), config_portal);
      });

      server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
        int params = request->params();
        for(int i=0;i<params;i++){
          AsyncWebParameter* p = request->getParam(i);
          if(p->isPost()){
            // HTTP POST ssid value
            if (p->name() == PARAM_INPUT_1) {
              ssid = p->value().c_str();
#if defined(DEBUGIT)
              Serial.print("SSID set to: ");
              Serial.println(ssid);
#endif
              // Write file to save value
              writeFile(LittleFS, ssidPath, ssid.c_str());
            }
            // HTTP POST pass value
            if (p->name() == PARAM_INPUT_2) {
              pass = p->value().c_str();
#if defined(DEBUGIT)
              Serial.print("Password set to: ");
              Serial.println(pass);
#endif
              // Write file to save value
              writeFile(LittleFS, passPath, pass.c_str());
            }
            // HTTP POST ip value
            if (p->name() == PARAM_INPUT_3) {
              ip = p->value().c_str();
#if defined(DEBUGIT)
              Serial.print("IP Address set to: ");
              Serial.println(ip);
#endif
              // Write file to save value
              writeFile(LittleFS, ipPath, ip.c_str());
            }
            // HTTP POST gateway value
            if (p->name() == PARAM_INPUT_4) {
              gateway = p->value().c_str();
#if defined(DEBUGIT)
              Serial.print("Gateway set to: ");
              Serial.println(gateway);
#endif
              // Write file to save value
              writeFile(LittleFS, gatewayPath, gateway.c_str());
            }
            //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
          }
        }
        restart = true;
        request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      });

      server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
        int params = request->params();
        for(int i=0;i<params;i++){
          AsyncWebParameter* p = request->getParam(i);
          if(p->isPost()){
            // HTTP POST ssid value
            if (p->name() == PARAM_INPUT_5) {
              writeFile(LittleFS, passPath, "");
              writeFile(LittleFS, ipPath, "");
              writeFile(LittleFS, gatewayPath, "");
              writeFile(LittleFS, enterPortalctrl, "");
              writeFile(LittleFS, ssidPath, "");
            }
          }
        }
        restart = true;
        request->send(200, "text/plain", "Settings resetted. USBvalve will restart");
      });                           

    }else{
      // Serial.println("Webportal");
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        visitor = 1;
        request->send_P(200, PSTR("text/html"), Web_page);
      });
      server.on("/valveout", HTTP_GET, [](AsyncWebServerRequest *request){
        if(incomingSerial != ""){
          request->send(200, "text/plain", String(incomingSerial)+"\n");
          incomingSerial="";
        }
      });
    }      
    server.begin();
  }

}

void loop() {
  if ( digitalRead(TRIGGER_PIN) == LOW ) {
    writeFile(LittleFS, enterPortalctrl, "yes");
    delay(50);
    restart = true;
  }
  if (restart){
    delay(1000);
    ESP.restart();
  }
  if (Serial.available() > 0) {
    String tempser = Serial.readString();
    if(tempser.indexOf("ESPRESET") > -1 ){
        delay(100);
        ESP.restart();
        visitor = 0;
    }
    if(visitor == 1){
      if(tempser.indexOf("ESPSEROFF") > -1 ){
        visitor = 0;
      }else{
        incomingSerial += tempser;
      }
    }else{
      if(tempser.indexOf("ESPSERON") > -1 ){
        visitor = 1;
      }
    }
  }
}
