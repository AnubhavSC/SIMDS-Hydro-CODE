#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

//SSID and Password to your ESP Access Point
const char* ssid = "Smart irrigation";
const char* password = "password";


#define Motor  4 // Motor  GPIO2(D4)

String command;           

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);

  pinMode(Motor, OUTPUT); 
 
  
// Connecting WiFi

  WiFi.mode(WIFI_AP);           //Only Access point
  WiFi.softAP(ssid, password);  //Start HOTspot removing password will disable security

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
 
 // Starting WEB-server 
     server.on ( "/", HTTP_handleRoot );
     server.onNotFound ( HTTP_handleRoot );
     server.begin();    
}

void loop() {
    server.handleClient();
    
      command = server.arg("State");
           if (command == "W")digitalWrite(Motor, HIGH); // light is on
      else if (command == "w")digitalWrite(Motor, LOW);  // light is off
}

void HTTP_handleRoot(void) {

if( server.hasArg("State") ){
       Serial.println(server.arg("State"));
  }
  server.send ( 200, "text/html", "" );
  delay(1);
}

