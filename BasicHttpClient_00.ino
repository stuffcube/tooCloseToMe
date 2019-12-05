/**

  04dic19 | test del wifi
 
*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

const char* ssid = "TIM-";
const char* password = "";


ESP8266WiFiMulti WiFiMulti;

int st, et, sd, ed;
String url;

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  st = 0;
  et = 0;
  sd = 0;
  ed = 0;


  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");


    st ++;
    et ++;
    sd ++;
    ed ++;
    
    url  = "http://led4run.altervista.org/00_metrica_ciclista/comandi.php?cmd=add_riga";
    url += "&start_time=";
    url += st;
    url += "&end_time=";
    url += et;
    url += "&start_distance=";
    url += sd;
    url += "&end_distance=";
    url += ed;
    Serial.println(url);
//    if (http.begin(client, "http://jigsaw.w3.org/HTTP/connection.html")) {  // HTTP
    if (http.begin(client, url)) {  // HTTP


      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }

  delay(10000);
}
