#include "SoftwareSerial.h"
SoftwareSerial SIM808(14, 12); //Seleccionamos los pines 7 como Rx y 8 como Tx

// Rx = D5 y Tx = D6

#define dbg

unsigned int Counter = 0;
unsigned long datalength, checksum, rLength;
unsigned short topiclength;
unsigned short topiclength2;
unsigned char topic[30];
char str[250];
unsigned char encodedByte;
int X;  

unsigned short MQTTProtocolNameLength;
unsigned short MQTTClientIDLength;
unsigned short MQTTUsernameLength;
unsigned short MQTTPasswordLength;
unsigned short MQTTTopicLength;

const char * MQTTHost = "broker.shiftr.io";
const char * MQTTPort = "1883";
const char * MQTTClientID = "SIM808";
const char * MQTTTopic = "SampleTopic";
const char * MQTTTopic2 = "SampleTopic2";
const char * MQTTProtocolName = "MQIsdp";
const char MQTTLVL = 0x03;
const char MQTTFlags = 0xC2;
const unsigned int MQTTKeepAlive = 60;
//const char * MQTTUsername = "try";
//const char * MQTTPassword = "try";
const char * MQTTUsername = "cafbc6d8";
const char * MQTTPassword = "bdf22c4fae433f9e";
const char MQTTQOS = 0x00;
const char MQTTPacketID = 0x0001;

void conectar_red();
void enviar_AT(String comando);
void suscribirse_mqtt();
int conectar_mqtt();
void conectar_red();
void leer_gps();

void setup(){
  //Begin nodemcu serial-0 channel
  SIM808.begin(9600);
  Serial.begin(19200);

  //enviar_sms();

  //leer_gps();

  conectar_red();
  
  conectar_mqtt();
  readServerResponse("AT+CIPRXGET=2,1024", "OK", "ERROR", 10000);
  //suscribirse_mqtt();
  //readServerResponse("AT+CIPRXGET=2,1024", "OK", "ERROR", 10000);
}

void enviar_AT(String comando) {
  //Serial.println(comando);
  SIM808.print(comando);  //Start Configuring GSM Module
  delay(3000);         //One second delay
  SIM808.println();

  leer_respuesta();

}

void suscribirse_mqtt(){

  Serial.println("###################### Subscribirse a MQTT");
  
  enviar_AT("AT+CIPSEND");

  memset(str, 0, 250);
    topiclength2 = strlen(MQTTTopic2);
    datalength = 2 + 2 + topiclength2 + 1;
    delay(1000);

    SIM808.write(0x82);
    X = datalength;
    do
    {
      encodedByte = X % 128;
      X = X / 128;
      // if there are more data to encode, set the top bit of this byte
      if ( X > 0 ) {
        encodedByte |= 128;
      }
      SIM808.write(encodedByte);
    }
    while ( X > 0 );
    SIM808.write(MQTTPacketID >> 8);
    SIM808.write(MQTTPacketID & 0xFF);
    SIM808.write(topiclength2 >> 8);
    SIM808.write(topiclength2 & 0xFF);
    SIM808.write(MQTTTopic2);
    SIM808.write(MQTTQOS);

    SIM808.write(0x1A);

    SIM808.println();

    delay(7000);
    Serial.println("\r\n###################### Subscripto a MQTT");
}

int conectar_mqtt() {

  Serial.println("\r\n###################### Iniciando MQTT");

  while (sendATcommand2("AT+CIPSEND",">","ERROR",2000) ==0 );
  
  SIM808.write(0x10);
  //Serial.write(0x10);
  MQTTProtocolNameLength = strlen(MQTTProtocolName);
  MQTTClientIDLength = strlen(MQTTClientID);
  MQTTUsernameLength = strlen(MQTTUsername);
  MQTTPasswordLength = strlen(MQTTPassword);
  datalength = MQTTProtocolNameLength + 2 + 4 + MQTTClientIDLength + 2 + MQTTUsernameLength + 2 + MQTTPasswordLength + 2;
  X = datalength;

    do
    {
      encodedByte = X % 128;
      X = X / 128;
      // if there are more data to encode, set the top bit of this byte
      if ( X > 0 ) {
        encodedByte |= 128;
      }
      SIM808.write(encodedByte);
      Serial.write(encodedByte);
    }
    while ( X > 0 );
    SIM808.write(MQTTProtocolNameLength >> 8);
    SIM808.write(MQTTProtocolNameLength & 0xFF);
    SIM808.write(MQTTProtocolName);
    
    SIM808.write(MQTTLVL); // LVL
    SIM808.write(MQTTFlags); // Flags
    SIM808.write(MQTTKeepAlive >> 8);
    SIM808.write(MQTTKeepAlive & 0xFF);

    SIM808.write(MQTTClientIDLength >> 8);
    SIM808.write(MQTTClientIDLength & 0xFF);
    SIM808.print(MQTTClientID);

    SIM808.write(MQTTUsernameLength >> 8);
    SIM808.write(MQTTUsernameLength & 0xFF);
    SIM808.print(MQTTUsername);

    SIM808.write(MQTTPasswordLength >> 8);
    SIM808.write(MQTTPasswordLength & 0xFF);
    SIM808.print(MQTTPassword);
    
    SIM808.write(0x1A);
    
    if (sendATcommand2("", "SEND OK", "SEND FAIL", 5000)) {
      Serial.println(F("CONNECT PACKET SUCCESS"));
      return 1;
    }
    else return 0;
    
    leer_respuesta();

  delay(7000);  
  Serial.println("\r\n###################### MQTT iniciado");
}

void conectar_red() {

  Serial.println("\r\n###################### Conectando GPRS");

  while (sendATcommand("AT","OK",2000) == 0);
  while (sendATcommand("AT+CREG?","+CREG: 0,1",5000) == 0 );
  
  while (sendATcommand("AT+CIPMUX=0","OK",2000) == 0 );   // siempre falla, 0 o 1
  while (sendATcommand("AT+CIPRXGET=2","OK",3000) == 0 );
  while (sendATcommand("AT+CIPMODE=0","OK",3000) == 0 );
  while (sendATcommand("AT+CIPSRIP=0","OK",2000) == 0 );
  while (sendATcommand("AT+CGATT=1","OK",2000) == 0 );
  while (sendATcommand("AT+CIPSTATUS","OK",2000) ==0 );
  sendATcommand("AT+CSTT=\"gprs.personal.com\",\"datos\",\"datos\"","OK",10000);
  while (sendATcommand("AT+CIPSTATUS","OK",2000) ==0 );
  while (sendATcommand("AT+CIICR","OK",3000) ==0 );
  while (sendATcommand("AT+CIPSTATUS","OK",2000) ==0 );
  enviar_AT("AT+CIFSR");
  while (sendATcommand("AT+CIPSTATUS","OK",2000) ==0 );
  sendATcommand("AT+CIPSTART=\"TCP\",\"broker.shiftr.io\",\"1883\"","OK",2000);
  
  /*
  enviar_AT("AT");
  enviar_AT("AT+CREG?");
  enviar_AT("AT+CIPMUX=0");
  enviar_AT("AT+CIPRXGET=2");
  enviar_AT("AT+CIPMODE=0");
  enviar_AT("AT+CIPSRIP=0");
  enviar_AT("AT+CGATT=1");
  enviar_AT("AT+CIPSTATUS");
  
  enviar_AT("AT+CSTT=\"gprs.personal.com\",\"datos\",\"datos\"");
  enviar_AT("AT+CIPSTATUS");
  enviar_AT("AT+CIICR");
  enviar_AT("AT+CIPSTATUS");
  enviar_AT("AT+CIFSR");
  enviar_AT("AT+CIPSTATUS");

  enviar_AT("AT+CIPSTART=\"TCP\",\"broker.shiftr.io\",\"1883\"");
  */
  
  delay(1000);
  Serial.println("\r\n###################### GPRS conectado");
}

void leer_gps() {

  Serial.println("Conectando GPS");
  enviar_AT("AT");
  enviar_AT("AT+CGPSPWR=1");  // activar GPS
  enviar_AT("AT+CGPSSTATUS?");;
  enviar_AT("AT+CGPSOUT=255");
  enviar_AT("AT+CGPSPWR=0");
  Serial.println("Desconectando GPRS");
}

void enviar_sms() {
  sendATcommand("AT","OK",2000);
  sendATcommand("AT+CMGF=1","OK",2000);
  sendATcommand("AT+CMGS=\"1168035565\"",">",2000);
  SIM808.print("hola_mundo"); // SMS body - Sms Text
  Serial.println("hola_mundo");
  SIM808.write(26);                //CTRL+Z Command to send text and end session
}

void loop()
{
  //Envíamos y recibimos datos
  //if (Serial.available() > 0)
  //  SIM808.write(Serial.read());

  //leer_respuesta();

  //sendATcommand("AT+CIPMUX=1","OK",2000);
}

void leer_respuesta() {

  //Serial.print(comando);
  
  while (SIM808.available() > 0)
  {
    char c = SIM808.read();
    Serial.print(c);
  }
  Serial.print(" : ");
}

int8_t readServerResponse(char* ATcommand, char* expected_answer1, char* expected_answer2, unsigned int timeout) {
  unsigned long nowMillis = millis();
  SIM808.println(ATcommand);
  delay(3000);
 
  if (SIM808.available()) {
    while (char(Serial.read()) != 0x24) {
      if ((millis() - nowMillis) > 2000) {
        Serial.println("NO DATA RECEIVED FROM REMOTE");
        break;
      }
    }
    nowMillis=(millis());
    while (SIM808.available()) {
      Serial.print(char(SIM808.read()));
    }
  }

}

int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout) {

  uint8_t x = 0,  answer = 0;
  char response[500];
  unsigned long previous;
  char* str;
  uint8_t index = 0;

  memset(response, '\0', 100);    // Initialize the string

  delay(100);

  while ( SIM808.available() > 0) SIM808.read();   // Clean the input buffer

  SIM808.println(ATcommand);    // Send the AT command
#ifdef dbg
  Serial.println(ATcommand);    // Send the AT command
#endif


  x = 0;
  previous = millis();

  // this loop waits for the answer
  do {
    if (SIM808.available() != 0) {
      // if there are data in the UART input buffer, reads it and checks for the asnwer
      response[x] = SIM808.read();
      //Serial2.print(response[x]);
      x++;
      // check if the desired answer  is in the response of the module
      if (strstr(response, expected_answer) != NULL)
      {
        answer = 1;

      }
    }
  }
  // Waits for the asnwer with time out
  while ((answer == 0) && ((millis() - previous) < timeout));

#ifdef dbg
  Serial.println(response);    // Send the AT command
#endif
  return answer;
}

int8_t sendATcommand2(char* ATcommand, char* expected_answer1, char* expected_answer2, unsigned int timeout) {

  uint8_t x = 0,  answer = 0;
  char response[100];
  unsigned long previous;

  memset(response, '\0', 100);    // Initialize the string

  delay(100);

  SIM808.flush();
  SIM808.println(ATcommand);    // Send the AT command
  //if(strstr(ATcommand, "AT+CIPSEND")!=NULL) Serial2.write(0x1A);

#ifdef dbg
  Serial.println(ATcommand);    // Send the AT command
#endif

  x = 0;
  previous = millis();

  // this loop waits for the answer
  do {
    // if there are data in the UART input buffer, reads it and checks for the asnwer
    if (SIM808.available() != 0) {
      response[x] = SIM808.read();
      x++;
      // check if the desired answer 1  is in the response of the module
      if (strstr(response, expected_answer1) != NULL)
      {
        answer = 1;
        while (Serial.available()) {
          response[x] = SIM808.read();
          x++;
        }
      }
      // check if the desired answer 2 is in the response of the module
      else if (strstr(response, expected_answer2) != NULL)
      {
        answer = 2;
        while (Serial.available()) {
          response[x] = SIM808.read();
          x++;
        }
      }

    }
  }
  // Waits for the asnwer with time out
  while ((answer == 0) && ((millis() - previous) < timeout));
#ifdef dbg
  Serial.println(response);
#endif
  return answer;
}
