#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <ArduinoJson.h>

#include <SPI.h>
#include <LoRa.h>


#define ss 32
#define rst 5
#define dio0 33

// ===========================
// Enter your WiFi credentials
// ===========================
const char* ssid = "Team . NET";
const char* password = "Nepo913913";

String serverName = "http://103.180.240.24:8091/v1/api/sensors";

char* strPacket[6];
char* ptr = NULL;
const char s[2] = "|";


void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;
  delay(500);
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  Serial.println("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
  delay(500);

  Serial.println("LoRa Receiver");
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
  LoRa.setSyncWord(0xF3);
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    // read packet
    while (LoRa.available()) {
      Serial.println("Lora Started.");

      String str = LoRa.readString();
      char charArray[str.length() + 1];
      str.toCharArray(charArray, str.length() + 1);
      byte index = 0;
      ptr = strtok(charArray, "|");

      while (ptr != NULL) {
        strPacket[index] = ptr;
        index++;
        ptr = strtok(NULL, "|");
      }
      sendRestApiData(strPacket[0], strPacket[1], strPacket[2], strPacket[3]);
    }
  }
}
void sendRestApiData(char* nodeId, char* deviceId, char* sensorData, char* sensorTimestamp) {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");

    HTTPClient http;
    String serverPath = serverName + "/log";
    http.begin(serverPath.c_str());
    http.addHeader("accept", "application/json");
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> json;

    json["nodeId"] = nodeId;
    json["deviceId"] = deviceId;
    json["sensorData"] = sensorData;
    json["sensorTimestamp"] = sensorTimestamp;

    String requestBody;
    serializeJson(json, requestBody);

    Serial.println(requestBody);
    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode == 200) {

      String payload = http.getString();
      Serial.println(payload);
      Serial.println("Data sent");
    } else {
      Serial.printf("Error occurred while sending HTTP POST: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi dis-connected");
  }
}