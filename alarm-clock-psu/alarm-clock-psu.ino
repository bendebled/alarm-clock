#include <SoftwareSerial.h>

String key = "";
String value = "";
bool keySet = false;

bool psuOn = false;
bool rPiOn = false;

bool failSafe = false;
unsigned long failSafeTime = 0;

#define PSU_PIN 2
#define LED_PIN 3
#define RPI_PIN 4
#define BUZZER_PIN 6

SoftwareSerial rPiSerial(10, 11); // RX, TX

void setup() {
  //Set serials
  Serial.begin(9600);
  rPiSerial.begin(9600);
  Serial.println("--------------------------------------");

  //Set Pins
  pinMode(PSU_PIN, OUTPUT);
  digitalWrite(PSU_PIN, HIGH);
  
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, 255);
  
  pinMode(RPI_PIN, OUTPUT);
  digitalWrite(RPI_PIN,HIGH);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

void loop() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    if(!keySet){
      key += inChar;
    }
    else{
      value += inChar;
    }
    
    if (inChar == ',') {
      keySet = true;
    }
    else if (inChar == '\n') {
      doAction((byte)key.toInt(), (byte)value.toInt());
      key = "";
      value = "";
      keySet = false;
    }
  }
  failSafeHandler();
  
}

void doAction(byte key, byte value){
  if(key == 0){
    //
  }
  else if(key == 1){
    if(value < 255 && !psuOn){
      turnPSU(true);
    }
    analogWrite(LED_PIN, value);
  }
  else if(key == 2){
    if(value == 0){ //turn on wake up music
      turnOnWakeUpMusic();
    }
  }
}

void turnOnWakeUpMusic(){
  if(!rPiOn){
    turnRPi(true);
  }
  unsigned long t = millis();
  while((!rPiSerial.available() && t + (long)2*60*1000 > millis()) || (rPiSerial.available() && (char)rPiSerial.read() != '1')){
    delay(500);
  }
  if(t + (long)2*60*1000 > millis()){ //Raspberry pi turned on with no problem
    rPiSerial.println(0); //Turn on wake up music
    Serial.println("Turn on wake up music");
  }
  else{
    failSafe = true;
    failSafeTime = millis();
    Serial.println("Failsafe activated");
  }
}

void turnRPi(bool on){
  if(!psuOn){
    turnPSU(true);
    delay(1000);
  }
  if(on){
    digitalWrite(RPI_PIN, LOW);
    rPiOn = true;
  }
  else{
    digitalWrite(RPI_PIN, HIGH);
    rPiOn = false;
  }
}

void turnPSU(bool on){
  if(on){
    digitalWrite(PSU_PIN, LOW);
    delay(100);
    digitalWrite(PSU_PIN, HIGH);
    delay(100);
    digitalWrite(PSU_PIN, LOW);
    psuOn = true;
  }
  else{
    digitalWrite(PSU_PIN, HIGH);
    psuOn = false;
  }
}

void failSafeHandler(){
  if(failSafe && failSafeTime + (long)30*60*1000 < millis()){
    Serial.println("failsafe buzzing");
    if((millis()/1000) % 2 == 0){
      digitalWrite(BUZZER_PIN, LOW);
    }
    else{
      digitalWrite(BUZZER_PIN, HIGH);
    }
  }
}

