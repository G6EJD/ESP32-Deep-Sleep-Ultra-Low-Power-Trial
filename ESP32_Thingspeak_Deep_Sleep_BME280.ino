/* ESP32 takes a large current pulse to get started, so won't work with a USB programmer PSU, need a direct power feed at 3.3V
   ESP32 in Deep Sleep takes 0.0056mA or 5.6uA, giving a typical battery life with a 2600mAh battery of : 220 days at 15-min update intervals
 Typical battery capacities:
   AAA 1200 (Alkaline) or  800–1000 (NiMH) 
   AA 2700 (alkaline) or 3000 (Lithium-Rechargeable) or 1700–2900 (NiMH) 
   C 8000 (alkaline) or 4500–6000 (NiMH) 
   D 12000 (alkaline) or 2200–12000 (NiMH) or 19000 (Lithium-Primary) 3.6V 
   9V Transistor 565 (alkaline) or 175–300 (NiMH)
   6V Lantern 26000 (alkaline) 
   CR2032 240 (Lithium-Primary) 3.6V 
   CR2016 90 (Lithium-Primary) 3.6V 
   4 Farad Cap 1 (loses 1 volt in 1 hour at 1mA)
   
*/
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
WiFiClient client; // wifi client object

const char *ssid     = "yourSSID";
const char *password = "yourPASSWORD";

char ThingSpeakAddress[] = "api.thingspeak.com";      // Thingspeak address
String api_key           = "yourTHINGSPEAK_API_KEY";  // Thingspeak API WRITE Key for your channel
const int UpdateInterval = 10 * 60 * 1000000;  // e.g. 15 * 60 * 1000000; for a 15-Min update interval (15-mins x 60-secs * 1000000uS)
#define ADC_input_pin   36  // also known as SVP ADC-0 or SVN Pin-39 
#define pressure_offset 3.9 // Compensates for this location being 40M asl  
Adafruit_BME280 bme;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Start WiFi");
  while (WiFi.status() != WL_CONNECTED ) {Serial.print("."); delay(500); }
  Wire.begin(17,16); // (sda,scl) 
  if (!bme.begin()) {
    Serial.println("Could not find a sensor, check wiring!");
  } 
  else
  {
    Serial.println("Found a sensor continuing");
    while (isnan(bme.readPressure())) {};//Serial.println(bme.readPressure());}
  }

  float temperature = bme.readTemperature(); // 3 example variables, ideally supplied by a sensor, see my examples for the BMP180, BME280 or DS18B20
  float humidity    = bme.readHumidity();
  float pressure    = bme.readPressure() / 100.0F + pressure_offset; // Result is in hPA
  float VBat        = float(analogRead(ADC_input_pin)) / 4096.0f * 3.3;    // Using ADC Channel-0
  UpdateThingSpeak("field1="+String(temperature)+"&field2="+String(humidity)+"&field3="+String(pressure)+"&field4="+String(VBat)); //Send the data as text
  esp_deep_sleep_enable_timer_wakeup(UpdateInterval);
  BME280_Sleep();
  Serial.println("Going to sleep now...");
  esp_deep_sleep_start();
}

void loop() {
  //Do nothing as it will never get here!
}

void UpdateThingSpeak(String DataForUpload) {
  WiFiClient client;
  if (!client.connect(ThingSpeakAddress, 80)) {
     Serial.println("connection failed");
     return;
  }
  else
  {
    Serial.println(DataForUpload);
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + api_key + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(DataForUpload.length());
    client.print("\n\n");
    client.print(DataForUpload);
  }
  client.stop();
}

void BME280_Sleep() {
  //Serial.println("BME280 to Sleep mode");
  Wire.beginTransmission(0x76); // Check your I2C address, they vary, could be 0x77
  Wire.write((uint8_t)BME280_REGISTER_CONTROL);
  Wire.write((uint8_t)0b00);
  Wire.endTransmission();
}

