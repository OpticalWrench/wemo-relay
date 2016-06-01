#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "FS.h"

// comment/remove DEBUG to disable serial info output
//#define DEBUG


// comment/remove STATICIP definition to use DHCP and get IP address automatically
//#define STATICIP

/*
IMPORTANT: Make sure the SPIFFS size is set large enough to contain all the files in /data directory !!
*/

const char *softAP_ssid = "ssid";
const char *softAP_password = "********";

const char *wifiClient_ssid = "ssid";
const char *wifiClient_password = "********";

const char *myHostname = "wemo-relay";
char hostString[16] = {0};

// Web server on port 80
ESP8266WebServer server(80);

void clientWiFI() {
  // connect to local wifi network

  WiFi.mode(WIFI_STA);

  WiFi.begin(wifiClient_ssid, wifiClient_password);

  #ifdef DEBUG
    Serial.print("Connnecting to wifi network...");
  #endif

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    #ifdef DEBUG
      Serial.print(".");
    #endif
  }

  #ifdef STATICIP
    IPAddress ip(192, 168, 1, 2);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.config(ip, gateway, subnet);
  #endif

  #ifdef DEBUG
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(wifiClient_ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  #endif
}

void accessPointWiFi() {
  // set up as an access point

  #ifdef DEBUG
    Serial.print("Configuring access point...");
  #endif
  /* You can remove the password parameter if you want the AP to be open. */

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(myHostname, softAP_password);
  WiFi.disconnect();
  delay(500); // Without delay I've seen the IP address blank
  #ifdef DEBUG
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
  #endif

}

const int relayPin = D1;
const long buttonPressTime = 1000;  // simulate button press for one second

void activateGarageDoorButton() {

  digitalWrite(relayPin, HIGH); // turn on relay with voltage HIGH
  delay(buttonPressTime);              // pause
  digitalWrite(relayPin, LOW);  // turn off relay with voltage LOW

}

void setup() {

  #ifdef DEBUG
    Serial.begin(115200);
    delay(100);
    Serial.println();
  #endif

  clientWiFI(); // connect to local wifi network

  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "wemorelay1.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin("wemorelay1")) {
    ;
    #ifdef DEBUG
      Serial.println("Error setting up MDNS responder!");
    #endif

  }

  if (!SPIFFS.begin()) {
    ;
    #ifdef DEBUG
      Serial.println("Failed to mount file system");
    #endif
  }

  //Setup endpoints
  server.on("/ls", handleDirectoryListing);
  server.on("/lslinks", handleDirectoryListingWithLinks);
  server.on("/viewfile", handleViewFile);
  server.on("/", handleRoot);
  server.on("/pressthebutton", handleGarageButton);
  server.onNotFound ( handleEndPointNotFound ); // serve up file requests that do not have a specific endpoint programmed
  server.begin(); // Web server start

  #ifdef DEBUG
    Serial.println("HTTP server started");
  #endif

  delay(1);

  pinMode(relayPin, OUTPUT);

}

void loop() {

  //HTTP
  server.handleClient();
  delay(10);

}
