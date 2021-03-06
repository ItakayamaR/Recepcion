#include <Arduino.h>
#include <SPI.h>
#include "SX1278.h"
#include "Pines.h"

//Definiciones para la libreria
#define LORA_MODE  4
#define LORA_CHANNEL  915E6
#define LORA_ADDRESS  4
#define LORA_SEND_TO_ADDRESS  2

byte MODO = 0;
byte MODO_ANT = 0;

byte e;
char my_packet[100];

void setup()
{
    //Inicializamos los pines de LED y selección
  pinMode(LED, OUTPUT);
  pinMode(SEL1, INPUT);
  pinMode(SEL2, INPUT);

  //Inicializamos pines del módulo 1
  pinMode(DIO0_1, INPUT);
  pinMode(DIO1_1, INPUT);
  pinMode(DIO2_1, INPUT);
  pinMode(RST1, OUTPUT);
  pinMode(SS1, OUTPUT);
  digitalWrite(SS1,HIGH);

  //Inicializamos pines del módulo 2
  pinMode(DIO0_2, INPUT);
  pinMode(DIO1_2, INPUT);
  pinMode(DIO2_2, INPUT);
  pinMode(RST2, OUTPUT);
  pinMode(SS2, OUTPUT);
  digitalWrite(SS2,HIGH);

  //Inicializamos pines del módulo 3
  pinMode(AUX, INPUT);
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);

  // Abrimos comunicaciones para observar 
  Serial.begin(115200); 

  //Iniciamos comunicación por UART (Módulo 3)
  Serial1.begin(9600, SERIAL_8N1, RX, TX);
  
  //Iniciamos los modulos en reset
  digitalWrite(RST1,0);
  digitalWrite(RST2,0);
  digitalWrite(M0,1);
  digitalWrite(M1,1);
  
}

void Ini_LoraModule(byte m) 
{
  //Inicializamos SPI en los pines correspondientes
  if (m==1){
    digitalWrite(RST1,1);
    SPI.begin(SCK, MISO, MOSI, SS1);    

  } else if (m==2) {
    digitalWrite(RST2,1);
    SPI.begin(SCK, MISO, MOSI, SS2);
  }

  // Mensaje de comprobacion por serial
  Serial.println(F("sx1278 module and Arduino: send two packets (One to an addrees and another one in broadcast)"));

  // Inicializamos el modulo
  if (sx1278.ON() == 0) {
    Serial.println(F("Setting power ON: SUCCESS "));
  } else {
    Serial.println(F("Setting power ON: ERROR "));
  }

  // Seteamos el modo de transmisión a Lora
  if (sx1278.setMode(LORA_MODE) == 0) {
    Serial.println(F("Setting Mode: SUCCESS "));
  } else {
    Serial.println(F("Setting Mode: ERROR "));
  }

  // Activamos el envío de Header
  if (sx1278.setHeaderON() == 0) {
    Serial.println(F("Setting Header ON: SUCCESS "));
  } else {
    Serial.println(F("Setting Header ON: ERROR "));
  }

  // Seleccionamos la frecuencia del canal
  if (sx1278.setChannel(LORA_CHANNEL) == 0) {
    Serial.println(F("Setting Channel: SUCCESS "));
  } else {
    Serial.println(F("Setting Channel: ERROR "));
  }

  // Activamos el envío de crc
  if (sx1278.setCRC_ON() == 0) {
    Serial.println(F("Setting CRC ON: SUCCESS "));
  } else {
    Serial.println(F("Setting CRC ON: ERROR "));
  }

  // Seleccionamos la potencia de envío (Max, High, Intermediate or Low)
  if (sx1278.setPower('M') == 0) {
    Serial.println(F("Setting Power: SUCCESS "));
  } else {
    Serial.println(F("Setting Power: ERROR "));
  }

  // Definimos la dirección del nodo de envio
  if (sx1278.setNodeAddress(LORA_ADDRESS) == 0) {
    Serial.println(F("Setting node address: SUCCESS "));
  } else {
    Serial.println(F("Setting node address: ERROR "));
  }

  // Mensaje de comprobación
  Serial.println(F("sx1278 configured finished"));
  Serial.println();

}

void terminar_spi(){
  digitalWrite(RST1,1);
  digitalWrite(RST2,1);
  SPI.end();
}

void iniciar_uart(){
  digitalWrite(M0,1);
  digitalWrite(M1,1);
}

void terminar_uart(){
  digitalWrite(M0,1);
  digitalWrite(M1,1);  
}


void EnableDevice(byte m){
  terminar_spi();
  terminar_uart();
  
  switch(m)
  {
    case 0:
      Serial.println("modo 0");
      break;

    case 1:
      Serial.println("modo 1");
      Ini_LoraModule(m);
      break;
    case 2:
      Serial.println("modo 2");
      Ini_LoraModule(m);
      break;

    case 3:
      Serial.println("modo 3");
      iniciar_uart(); 
      break;
  }
}

void loop(void)
{ 
  byte i;

  // Leemos el modo
  MODO = ( (digitalRead(SEL2)<<1) + digitalRead(SEL1) );
  
  if (MODO_ANT != MODO){
    EnableDevice(MODO);
    MODO_ANT=MODO;
  }

  if (MODO==1 || MODO==2){
    // Esperamos el envío de un mensaje por 10 segundos
    e = sx1278.receivePacketTimeout(10000);
    if (e == 0) {
      digitalWrite(LED, HIGH);
      delay(500);
      digitalWrite(LED, LOW);
        
      Serial.println(F("Package received!"));

      for (unsigned int i = 0; i < sx1278.packet_received.length; i++) {
        my_packet[i] = (char)sx1278.packet_received.data[i];
      }
      
      Serial.print(F("Message: "));
      Serial.println(my_packet);
    } else {
      Serial.print(F("Package received ERROR\n"));
    }
  } else if (MODO == 3){
      if (Serial1.available() > 0) {
        i=0;
        Serial.println(F("Package received!"));
        while (Serial1.available() > 0){
          // Leemos el byte recibido:
          my_packet[i]=Serial1.read();
          i++;
        }
        //Parpadeamos el led
        digitalWrite(LED, HIGH);
        delay(500);
        digitalWrite(LED, LOW);

        //Imprimimos el mensaje recibido
        Serial.print("Message: ");
        Serial.println(my_packet);
      }
  }
  delay(1000);
}