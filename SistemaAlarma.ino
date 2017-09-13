/*
 * HACKERS CLUB ACADEMY - PRÁCTICA FINAL -  Iván Marcos Pazos
 * ==========================================================
 * Realizar una simulación de un sistema de alarma real 
 * Se  utilizarán  diversos  sensores  que  variarán  su  funcionalidad según diferentes condiciones 
 * Se  debe  utilizar  la  comunicación  serial  para  mostrar  mensajes por pantalla 
 */

#include <SPI.h>
#include <MFRC522.h>
#include <Ultrasonic.h>

#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);
Ultrasonic ultrasonic(7,8);

const int pushButton = 4; //pin botón
const int buzzer = 2; //zumbador
const int ledR = 3; //Pin para el LED rojo
const int ledG = 5; //Pin para el LED verde
const byte accessCard[4] = {0xC9, 0xB3, 0x86, 0xAB}; //tarjeta blanca autorizada
const int freq = 1000; //frecuencia en Hz a la que sonara el buzzer
const int interval = 1000; //intervalo de tiempo en milisegundo para el blink

const String title = "HACKERS CLUB ACADEMY - PRÁCTICA FINAL"; //mensajes para mostrar por el monitor serial
const String alarmOff = "-- Alarma desactivada --";
const String alarmOn = "-- Alarma activada --";
const String cardInvalid = "-- Intento de suplantación --";
const String ultrasonicSensor = "-- Detector de movimiento, ¡INTRUSO! --";

byte readCard[4]; //almacena la ultima tarjeta leida
boolean suplantacion; //si es verdadero han pasado una tarjeta no autorizada
boolean alarmaActivada; //si es verdadero la alarma esta activada
boolean alarmaSonando; //cuando es verdadero ha saltado la alarma
int distanciaSegura; //almacena el valor en cm de la lectura de distancia minima permitida por el sensor

void setup () {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  inicioDePines();
  Serial.println("HACKERS CLUB ACADEMY - PRÁCTICA FINAL");
  Serial.println(alarmOff);
}


/*
 * 1. comprobamos el boton
 * 2. calibramos el sensor de ultrasonidos
 * 3. comprobamos el sensor de proximidad
 * 4. comprobamos el sensor NFC
 * 5. comprobamos si se ha producido intento de suplantación
 */
void loop (){

  if (digitalRead(pushButton) == LOW) {
    
    Serial.println(alarmOn);
    alarmaActivada = true;
    
    distanciaSegura = getDistancia() - 5;
    
    while(alarmaActivada) {
      
      if (getDistancia() < distanciaSegura) {     
        Serial.println(ultrasonicSensor);
        secuenciaAlarma();
      }
      
      if (getUID(alarmaSonando)) {
        Serial.println(alarmOff);
        avisoAlarmaDesactivada();
        alarmaActivada = false;
      }
      
      if (suplantacion) {
        Serial.println(cardInvalid);
        suplantacion = false;
        secuenciaAlarma();
      }
                  
    }
        
  }
    
}

/*
 * Iniciamos los pines de botón, altavoz y LED rojo y verde que necesitamos.
 */
void inicioDePines() {
  pinMode(pushButton, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
}

/*
 * devuelve el valor actual del sensor de ultrasonidos en centimetros
 */
int getDistancia() {
  return ultrasonic.Ranging(CM);
}

/*
 * Cuando no pasamos tarjeta devuelve false,
 * cuando no puede ser leida devuelve false,
 * cuando lee una tarjeta:
 * autorizada devuelve verdadero,
 * cuando lee una tarjeta no autorizada devuelve falso
 * y ademas en este caso, si la alarma no esta sonando 
 * pondra la variable suplantacion a true.
 */
boolean getUID(boolean alarmaSonando) {
  if (! rfid.PICC_IsNewCardPresent()) {
    return false;
  }
  
  if (! rfid.PICC_ReadCardSerial()) {
    return false;
  }

  for (int i = 0; i < 4; i++) {
    readCard[i] = rfid.uid.uidByte[i];
    if(readCard[i] != accessCard[i]) {      
      if (!alarmaSonando){
          suplantacion = true; 
      }
      return false;
    }
  }

  rfid.PICC_HaltA();
  return true;
}

void setColor(int R, int G){
  analogWrite(ledR, R);
  analogWrite(ledG, G);
}

/*
 * La alarma sonara hasta que el lector RFID lea la tarjeta blanca autorizada.
 * Cuando se cumpla lo anterior, volveremos en el estado inicial.
 */
void secuenciaAlarma(){
  alarmaSonando = true;
            
  do {
    blink();
    Serial.println(alarmOn);
  } while(!getUID(alarmaSonando));
    
  Serial.println(alarmOff);
  avisoAlarmaDesactivada();
  alarmaSonando = false;
  alarmaActivada = false;
}

//alerta de la alarma
void blink() {
  tone(buzzer, freq);
  setColor(255, 0);
  delay(interval);
  noTone(buzzer); 
  setColor(0, 0);
  delay(interval);   
}

//aviso de alarma desactivada
void avisoAlarmaDesactivada(){
  tone(buzzer, freq);
  setColor(0, 255);
  delay(333);
  noTone(buzzer);
  delay(333);
  tone(buzzer, freq);
  delay(333);
  noTone(buzzer);
  setColor(0, 0);
}
