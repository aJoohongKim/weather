#include "DHT.h"
#include <SoftwareSerial.h>

const int dhtPin = 2;
DHT dht;

// 자신의 thingspeak 채널의 API key 입력
String apiKey = "BKU14NX6Q4BH7VFL";

SoftwareSerial esp8266(11, 10); // RX/TX 설정, serial 객체생성

void setup() {  
  Serial.begin(9600);
  Serial.println();
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)");

  dht.setup(dhtPin);

  esp8266.begin(9600);        //소프트웨어 시리얼 시작
  esp8266.println("AT+RST");  //ESP8266 Reset
}

void loop() {
  delay(dht.getMinimumSamplingPeriod());

  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("\t\t");
  Serial.println(dht.toFahrenheit(temperature), 1);

  // String 변환
  char humidity_buffer[16];
  String strHumi = dtostrf(humidity, 4, 1, humidity_buffer);
  char temperature_buffer[16];
  String strTemp = dtostrf(temperature, 4, 1, temperature_buffer);

  // TCP 연결
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com 접속 IP
  cmd += "\",80";           // api.thingspeak.com 접속 포트, 80
  esp8266.println(cmd);
   
  if(esp8266.find("Error")){
    Serial.println("AT+CIPSTART error");
    return;
  }
  
  // GET 방식으로 보내기 위한 String, Data 설정
  String getStr = "GET /update?api_key=";
  getStr += apiKey;
  getStr +="&field1=";
  getStr += String(strTemp);
  getStr +="&field2=";
  getStr += String(strHumi);
  getStr += "\r\n\r\n";
 
  // Send Data
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  esp8266.println(cmd);
 
  if(esp8266.find(">")) esp8266.print(getStr);
  else {
    esp8266.println("AT+CIPCLOSE");
    Serial.println("AT+CIPCLOSE");  // alert user
  }

  delay(20000); // Thingspeak 최소 업로드 간격 15초를 맞추기 위한 delay
}
