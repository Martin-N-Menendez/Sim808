#include <ESP8266WiFi.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

#include "secrets.h"


SoftwareSerial mySerial(14, 12, false, 256);


const char* ssid     = WIFI_SSID;
const char* password = WIFI_PASS;

WiFiClient WiFiclient;
MQTTClient client;

unsigned long lastMillis = 0;

char inData[80];
byte i = 0;
byte j = 0;

String emisor =  "Aparato_1";

String Id;
String data_gps;
String time_gps;
String lat_gps , long_gps;
String hs_s , hs_p , hs_e;
String dp_prom , dp_max , dp_min;
String ps_prom , ps_max , ps_min;
String pd_prom , pd_max , pd_min;
String state;

String packet_gps;
String packet_times;



void setup() {
  Serial.begin(9600);
  delay(10);
  mySerial.begin(9600);
  delay(10);
  mySerial.println("Connect! - Conexion SOFTWARESERIAL");
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(2000);

  Serial.print("connecting to MQTT broker...");
  client.begin("broker.shiftr.io", WiFiclient);
  connect();
}

void connect() {
  while (!client.connect(MQTT_NAME, MQTT_ID , MQTT_KEY)) {
    Serial.print(".");
  }

  Serial.println("\nconnected!");
  //client.subscribe("CDP");
}

void parsear_datos() {

  while (mySerial.available())
  {
    char aChar = mySerial.read();
    //Serial.write(aChar);
    if (aChar == '|' or aChar == '\n')
    {
      Serial.println(inData);
      cargar_variable(inData, j);
      i = 0;
      j++;
      inData[i] = NULL;

      if (aChar == '\n')
      {
        j = 0;
        leer_gps();
        publicar_datos();
        break;
      }
    }
    else
    {
      inData[i] = aChar;
      i++;
      inData[i] = '\0'; // Keep the string NULL terminated
    }
  }
}

void paquetizar_JSON(){
  
  StaticJsonDocument<500> gps;

  gps["i"] = Id;
  //gps["t"] = state;

  JsonArray data = gps.createNestedArray("g");
   //data.add(data_gps.toFloat());
  //data.add(time_gps.toFloat());
  data.add(state.toFloat());
  data.add(lat_gps.toFloat());
  data.add(long_gps.toFloat());

  StaticJsonDocument<500> times;

  times["i"] = Id;
    
  JsonArray hs = times.createNestedArray("h");
  hs.add(hs_s.toFloat());
  hs.add(hs_p.toFloat());
  hs.add(hs_e.toFloat());

  JsonArray dp = times.createNestedArray("p");
  dp.add(dp_max.toFloat());
  dp.add(dp_prom.toFloat());
  dp.add(dp_min.toFloat());

  JsonArray ps = times.createNestedArray("s");
  ps.add(ps_max.toFloat());
  ps.add(ps_prom.toFloat());
  ps.add(ps_min.toFloat());

  JsonArray pd = times.createNestedArray("d");
  pd.add(pd_max.toFloat());
  pd.add(pd_prom.toFloat());
  pd.add(pd_min.toFloat());
  
  //serializeJsonPretty(root, Serial);
  serializeJson(gps,   packet_gps);
  serializeJson(times, packet_times);
  
  
}

void loop() {
  client.loop();
  if (!client.connected()) {
    connect();
  }

  //parsear_datos();

  //if (Serial.available())
  //  Serial.print(Serial.read());

  //if (mySerial.available())
  //    Serial.write(mySerial.read());
  //if (Serial.available())
  //    mySerial.write(Serial.read());

  if (millis() - lastMillis > 10 * 1000) {
    lastMillis = millis();
    //char c;

    //Serial.println("Entrare aca alguna vez?");
    leer_gps();
    variables_random();
    paquetizar_JSON();
    publicar_paquete();
    //publicar_datos();
  }
}

void leer_gps() {
  data_gps = "Hoy";
  time_gps = "Ahora";
  lat_gps = "-34.6729089";
  long_gps = "-58.335208";
}

void publicar_paquete(void) {

  Serial.println("Subiendo paquetea la nube");
  Serial.println(packet_gps);
  Serial.println(packet_gps.length());
  Serial.println(packet_times);
  Serial.println(packet_times.length());
  
  client.publish("gps", packet_gps);
  client.publish("times", packet_times);
  
  packet_gps = "";
  packet_times = "";
}

void publicar_datos(void) {

  Serial.println("Subiendo a la nube");

  client.publish(Id + "/GPS/data",     data_gps);
  client.publish(Id + "/GPS/time",     time_gps);
  client.publish(Id + "/GPS/latitud",  lat_gps);
  client.publish(Id + "/GPS/longitud", long_gps);

  client.publish(Id + "/horas/marcha", hs_s);
  client.publish(Id + "/horas/parada", hs_p);
  client.publish(Id + "/horas/error",  hs_e);

  client.publish(Id + "/dp/promedio",  dp_prom);
  client.publish(Id + "/dp/max",       dp_max);
  client.publish(Id + "/dp/min",       dp_min);

  client.publish(Id + "/ps/promedio",  ps_prom);
  client.publish(Id + "/ps/max",       ps_max);
  client.publish(Id + "/ps/min",       ps_min);

  client.publish(Id + "/pd/promedio",  pd_prom);
  client.publish(Id + "/pd/max",       pd_max);
  client.publish(Id + "/pd/min",       pd_min);

  client.publish(Id + "/state",        state);
}

void cargar_variable(String inData, byte j) {

  switch (j) {
    case 0:
      Id = inData;
      break;
    case 1:
      hs_s = inData;
      break;
    case 2:
      hs_p = inData;
      break;
    case 3:
      hs_e = inData;
      break;
    case 4:
      state = inData;
      break;
    case 5:
      ps_max = inData;
      break;
    case 6:
      ps_prom = inData;
      break;
    case 7:
      ps_min = inData;
      break;
    case 8:
      pd_max = inData;
      break;
    case 9:
      pd_prom = inData;
      break;
    case 10:
      pd_min = inData;
      break;
    case 11:
      dp_max = inData;
      break;
    case 12:
      dp_prom = inData;
      break;
    case 13:
      dp_min = inData;
      break;
    default:
      break;
  }

}

void variables_random(void) {

  float aux;
  char charBuf[15];

  Id = "FF011C000";
  //data_gps = String(random(1, 10));
  //time_gps = String(random(1, 10));

  //lat_gps  = String(random(1, 10));
  //long_gps = String(random(1, 10));

  aux = random(1, 1000) / 100.0;
  dtostrf(aux, 3, 1, charBuf);
  hs_s     = String(charBuf);

  aux = random(1, 1000) / 100.0;
  dtostrf(aux, 3, 1, charBuf);
  hs_p     = charBuf;

  aux = random(1, 1000) / 100.0;
  dtostrf(aux, 3, 1, charBuf);
  hs_e     = charBuf;

  aux = random(1, 1000) / 100.0;
  dtostrf(aux, 3, 1, charBuf);
  dp_prom     = String(charBuf);

  aux = random(1, 1000) / 100.0;
  dtostrf(aux, 3, 1, charBuf);
  dp_max     = charBuf;

  aux = random(1, 1000) / 100.0;
  dtostrf(aux, 3, 1, charBuf);
  dp_min     = charBuf;

  aux = random(1, 1000) / 100.0;
  dtostrf(aux, 3, 1, charBuf);
  ps_prom     = String(charBuf);

  aux = random(1, 1000) / 100.0;
  dtostrf(aux, 3, 1, charBuf);
  ps_max     = charBuf;

  aux = random(1, 1000) / 100.0;
  dtostrf(aux, 3, 1, charBuf);
  ps_min     = charBuf;

  aux = random(1, 1000) / 100.0;
  dtostrf(aux, 3, 1, charBuf);
  pd_prom     = String(charBuf);

  aux = random(1, 1000) / 100.0;
  dtostrf(aux, 3, 1, charBuf);
  pd_max     = charBuf;

  aux = random(1, 1000) / 100.0;
  dtostrf(aux, 3, 1, charBuf);
  pd_min     = charBuf;

  state = String(random(0, 2));

}
