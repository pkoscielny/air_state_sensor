/*
 * IDE: Arduino IDE 1.8.13
 * Board: LOLIN(WEMOS) D1 R2 & mini.
 *
 * Hardware:
 * - ESP8266 WeMos D1 mini board.
 * - DHT21 sensor (AM2301).
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>

const char* ssid   = "****";
const char* passwd = "****";
String iot_server_url = "http://your_zeroiot_server_address:3000/air_state"; // zeroiot
String device_id = "esp8266_wemos_2";
String location = "kitchen";

#define DHTTYPE DHT21 
DHT dht(D2, DHTTYPE);

// For proper working section for avoiding data contamination
// I need remember previous measurement results.
float prev_temp;
float prev_hum;

void setup() {
  Serial.begin(115200);

  // Hide SSID itself.
  WiFi.enableAP(0);
  
  WiFi.begin(ssid, passwd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Waiting for connection");
  }
    
  // Initialize DHT sensor.
  dht.begin();
  delay(3000); // sensor needs at least 2sec between measurements to proper work.
  
  prev_temp = dht.readTemperature();
  prev_hum  = dht.readHumidity();
  delay(3000); 
}



const int main_delay = 1 * 60 * 1000; // 1 minute delay.
void loop() {
  
  // Read air parameters.
  float current_temp = dht.readTemperature();
  float current_hum  = dht.readHumidity();

  // Check if any problems appear.
  if (isnan(current_temp) || isnan(current_hum)) {
    Serial.println("Failed to read from sensor");
  } 
  // Sometimes the sensor gives e.g. temperature -26 degrees in one measurement
  // but previous measurements gave proper values ~ 21.5 degrees.
  // It is very rately situation so I will filter out these measurements
  // to avoid data contamination.
  // I happens with DHT11 sensor.
  else if ( 
       abs(current_temp - prev_temp) > 10
      || abs(current_hum - prev_hum) > 30 
    ) {
    Serial.println("Observing strange values from sensor");
  } 
  else {
    Serial.print("temperature: "); Serial.print(current_temp);
    Serial.print(", humidity: "); Serial.println(current_hum);
    
    // Create JSON and send it to IoT server.
    send_temp_and_hum_to_iot_server(current_temp, current_hum);

    prev_temp = current_temp;
    prev_hum = current_hum;
  }

  delay(main_delay);
}


void send_temp_and_hum_to_iot_server(float temperature, float humidity) {


  // If lost connection then WiFi library should automatically reconnect to WiFi network.

  String attributes = String("{")
    + "\"temperature\":\"" + String(temperature) + "\""
    + ",\"humidity\":\""   + String(humidity)    + "\"" 
    + ",\"location\":\""   + location            + "\""
    + ",\"device\":\""     + device_id           + "\""    
    + "}";                                                                                                                                                                                                                                                                     
                                                                                                                                                                                                                                                                                
  String data = String("{")                                                                                                                                                                                                                                                     
    + "\"type\":\"air_state\""                                                                                                                                                                                                                                                
    + ",\"attributes\":" + attributes                                                                                                                                                                                                                                            
    + "}";                                                                                                                                                                                                                                                                      
                                                                                                                                                                                                                                                                                
  String json_to_send = String("{")                                                                                                                                                                                                                                             
    + "\"data\":" + data                                                                                                                                                                                                                                                        
    + "}";                                                                                                                                                                                                                                                                          
                                                                                                                                                                                                                                                                                
  //Serial.print("json_to_send: "); Serial.println(json_to_send);   
  
  HTTPClient http;
  http.begin(iot_server_url);
  http.addHeader("Content-Type", "application/vnd.api+json");
  int http_code = http.POST(json_to_send);
  
  //Serial.print("http code: "); Serial.println(http_code);
  //String payload = http.getString();
  //Serial.print("payload: "); Serial.println(payload);
  
  http.end();
}

