/**************************************************************
 *
 * For this example, you need to install PubSubClient library:
 *   https://github.com/knolleary/pubsubclient
 *   or from http://librarymanager/all#PubSubClient
 *
 * TinyGSM Getting Started guide:
 *   https://tiny.cc/tinygsm-readme
 *
 * For more MQTT examples, see PubSubClient library
 *
 **************************************************************
 * Use Mosquitto client tools to work with MQTT
 *   Ubuntu/Linux: sudo apt-get install mosquitto-clients
 *   Windows:      https://mosquitto.org/download/
 *
 * Subscribe for messages:
 *   mosquitto_sub -h test.mosquitto.org -t GsmClientTest/init -t GsmClientTest/ledStatus -q 1
 * Toggle led:
 *   mosquitto_pub -h test.mosquitto.org -t GsmClientTest/led -q 1 -m "toggle"
 *
 * You can use Node-RED for wiring together MQTT-enabled devices
 *   https://nodered.org/
 * Also, take a look at these additional Node-RED modules:
 *   node-red-contrib-blynk-ws
 *   node-red-dashboard
 *
 **************************************************************/

// Select your modem:
#define TINY_GSM_MODEM_SIM808

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
//#define SerialAT Serial1

// or Software Serial on Uno, Nano
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(7, 8); // RX, TX
//SoftwareSerial SerialPy(12, 13); // RX, TX

// See all AT commands, if wanted
//#define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

#define SerialPy Serial

// Range to attempt to autobaud
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200

// Add a reception delay - may be needed for a fast processor at a slow baud rate
//#define TINY_GSM_YIELD() { delay(2); }

// Define how you're planning to connect to the internet
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
#define TINY_GSM_TEST_GPS false

// set GSM PIN, if any
#define GSM_PIN ""

#define topic_gps "gps"
#define topic_times_1 "times_1"
#define topic_times_2 "times_2"

char inData[80];
unsigned int i = 0;
unsigned int j = 0;

bool enviar_msg = false;
bool gps_fixstatus;

float gps_latitude, gps_longitude, gps_speed;
int gps_altitude, gps_view_satellites, gps_used_satellites;

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
String packet_times_1;
String packet_times_2;

// Your GPRS credentials, if any
#define apn_net "gprs.personal.com"
#define gprsUser  "datos"
#define gprsPass  "datos"

// MQTT details
#define broker  "broker.shiftr.io"
#define GsmClientName "SIM808"
#define mqtt_user  "cafbc6d8"
#define mqtt_pass  "bdf22c4fae433f9e"

#define topicInit  "init"

#include <TinyGsmClient.h>
#include <PubSubClient.h>

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif
TinyGsmClient client(modem);
PubSubClient mqtt(client);

unsigned long lastReconnectAttempt = 0;
unsigned long lastMillis = 0;

int freeRam(){
 extern int __heap_start, *__brkval;
 int v;
 return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

boolean mqttConnect() {
  SerialMon.print(F("Broker > Conectandose ... "));
  SerialMon.print(F(broker));

  // Connect to MQTT Broker
  //boolean status = mqtt.connect(GsmClientTest);

  // Or, if you want to authenticate MQTT:
  boolean status = mqtt.connect(GsmClientName, mqtt_user, mqtt_pass);

  if (status == false) {
    SerialMon.println(F("> Conexion fallida"));
    return false;
  }
  SerialMon.println(F("> Conexion exitosa"));
  mqtt.publish(topicInit, "GsmClientTest started");
  //mqtt.subscribe(topicLed);
  return mqtt.connected();
}

void setup() {
  // Set console baud rate
  SerialMon.begin(38400);
  delay(10);

  SerialMon.println(F("-----> Iniciando..."));

  // Set GSM module baud rate
  // TinyGsmAutoBaud(SerialAT,GSM_AUTOBAUD_MIN,GSM_AUTOBAUD_MAX);
  SerialAT.begin(38400);
  delay(10);
  SerialPy.begin(38400);
  delay(10);
  
  //SerialAT.listen();

  #if TINY_GSM_TEST_GPS
    bool gps_status = modem.enableGPS();
    SerialMon.print(F("GPS > activado: "));
    SerialMon.println(gps_status?"Ok":"Error");
  #endif

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println(F("Modem > Iniciando"));
  //modem.restart();
  modem.init();

  String modemInfo = modem.getModemInfo();
  SerialMon.print(F("Modem : "));
  SerialMon.println(modemInfo);

#if TINY_GSM_USE_GPRS
  // Unlock your SIM card with a PIN if needed
  if ( GSM_PIN && modem.getSimStatus() != 3 ) {
    modem.simUnlock(GSM_PIN);
  }
#endif


#if TINY_GSM_USE_GPRS
  // GPRS connection parameters are usually set after network registration
    SerialMon.print(F("GPRS > Conectando a "));
    SerialMon.print(apn_net);
    if (!modem.gprsConnect(apn_net, gprsUser, gprsPass)) {
      SerialMon.println(F(" Falla"));
      delay(10000);
      return;
    }
    SerialMon.println(F(" Exito"));

  if (modem.isGprsConnected()) {
    SerialMon.println(F("GPRS > Conectado"));
  }
#endif

  // MQTT Broker setup
  mqtt.setServer(broker, 1883);
  //mqtt.setCallback(mqttCallback);
  
}

void loop() {

  parsear_datos();
  leer_gps();
  
  mqtt.loop();
  if (!mqtt.connected()) {
    SerialMon.println(F("=== MQTT NOT CONNECTED ==="));
    // Reconnect every 10 seconds
    unsigned long t = millis();
    if (t - lastReconnectAttempt > 10*1000L) {
      lastReconnectAttempt = t;
      if (mqttConnect()) {
        lastReconnectAttempt = 0;
      }
    }
    delay(100);
    return;
  }
  
  if (millis() - lastMillis > 10 * 1000) {
    lastMillis = millis();
    SerialMon.println(F(">>> Loop"));
    SerialMon.println(freeRam());
    //leer_gps();
    //mqtt.publish(topicLedStatus, ledStatus ? "1" : "0");
  }

  if (enviar_msg)
  {
    SerialMon.println(F(">>> Datos"));

    publicar_paquete();
  }
  
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

void parsear_datos() {
  
  //SerialPy.listen();

  if(enviar_msg)
  {
    SerialMon.println(F("Parser > Abortando!"));
    return;
  } 
  
  delay(10);
  while (SerialPy.available())
  {
    char aChar = SerialPy.read();
    //SerialMon.write(aChar);
    
    if (aChar == '\r')
      continue;
      
    if (aChar == '|' or aChar == '\n' )
    {
      //SerialMon.println(inData);
      cargar_variable(inData, j);
      i = 0;
      j++;
      inData[i] = NULL;

      if (aChar == '\n')
      {
        j = 0;
        SerialMon.println(F("Parser > Nuevo dato!"));
        publicar_parseado();
        paquetizar_JSON();
        Serial.println(F("Publicar > PERMITIDO"));
        SerialPy.flush();
        enviar_msg = true;
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
  //delay(10);
  //SerialAT.listen();
  //delay(10);
}

void publicar_parseado(){
  SerialMon.print("ID:");
  SerialMon.println(Id);
  SerialMon.print("State:");
  SerialMon.println(state);
  
  SerialMon.print("Hs:[");
  SerialMon.print(hs_s);
  SerialMon.print(",");
  SerialMon.print(hs_p);
  SerialMon.print(",");
  SerialMon.print(hs_e);
  SerialMon.println("]");

  SerialMon.print("Ps:[");
  SerialMon.print(ps_max);
  SerialMon.print(",");
  SerialMon.print(ps_prom);
  SerialMon.print(",");
  SerialMon.print(ps_min);
  SerialMon.println("]");

  SerialMon.print("Pd:[");
  SerialMon.print(pd_max);
  SerialMon.print(",");
  SerialMon.print(pd_prom);
  SerialMon.print(",");
  SerialMon.print(pd_min);
  SerialMon.println("]");

  SerialMon.print("Dp:[");
  SerialMon.print(dp_max);
  SerialMon.print(",");
  SerialMon.print(dp_prom);
  SerialMon.print(",");
  SerialMon.print(dp_min);
  SerialMon.println("]");
}

void publicar_paquete(void) {
  Serial.println(F("Subiendo paquetea la nube"));
  Serial.println(packet_gps);
  Serial.println(packet_gps.length());
  Serial.println(packet_times_1);
  Serial.println(packet_times_1.length());
  Serial.println(packet_times_2);
  Serial.println(packet_times_2.length());

  mqtt.publish(topic_gps, packet_gps.c_str());
  packet_gps = " ";
  delay(3*1000);
  
  mqtt.publish(topic_times_1, packet_times_1.c_str());
  packet_times_1 = " ";
  delay(3*1000);
  
  mqtt.publish(topic_times_2, packet_times_2.c_str());
  packet_times_2 = " ";

  Serial.println(F("Publicar > PROHIBIDO"));
  enviar_msg = false;
}

void leer_gps() {
  
  data_gps = "Hoy";
  time_gps = "Ahora";
  lat_gps = "-34.6729089";
  long_gps = "-58.335208";

  //modem.enableGPS();
  //delay(1000);
  //String gps_raw = modem.getGPSraw();

  /*
  gps_fixstatus = modem.getGPS(&gps_latitude, &gps_longitude);
  
  //modem.disableGPS();
  
  if ( gps_fixstatus ) {
    SerialMon.print(F("GPS > LATITUD : "));
    SerialMon.println(gps_latitude);
  }
  */
}

void paquetizar_JSON(){
  
  packet_gps = "{\"i\":\""+Id+"\",\"g\":["+state+","+lat_gps+","+long_gps+"]}";

  packet_times_1 = "{\"i\":\""+Id+"\",\"h\":[\""+hs_s+"\",\""+hs_p+"\",\""+hs_e+"\"],\"s\":[\""+ps_max+"\",\""+ps_prom+"\",\""+ps_min+"\"]}";

  packet_times_2 = "{\"i\":\""+Id+"\",\"p\":[\""+pd_max+"\",\""+pd_prom+"\",\""+pd_min+"\"],\"d\":[\""+dp_max+"\",\""+dp_prom+"\",\""+dp_min+"\"]}";
}
