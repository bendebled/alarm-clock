#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <EEPROM.h>
#include <CapacitiveSensor.h>
#include "ClickButton.h"
#include "Tlc5940.h"

// Pins
#define SCREEN_SWITCH_PIN 22
#define SNOOZE_PIN 24
#define TIMER_PIN 26
#define RADIO_PIN 28
#define MINUTES_PIN 30
#define HOURS_PIN 32
#define ALARM_SWITCH_PIN 34
#define ALARM2_PIN 36
#define ALARM1_PIN 38
#define LED_RELAY_5V_PIN 41
#define LED_RELAY_220V_PIN 40
#define SOUND_RELAY_PIN 12
#define MOSI 51
#define SCK 52
#define DISPLAY_PIN 53
#define SNOOZE_SEND 44
#define SNOOZE_RECEIVE 45
#define LIGHT_SENSOR_PIN A0
#define SD_SS 4
#define SOUND_PIN 11

// Constants
#define TIME_BEFORE_SAVING_ALARM 3 //in seconds
#define TIME_STATE 0
#define ALARM1_STATE 1
#define ALARM2_STATE 2
#define ADDRESS_ALARM1_HOUR 0
#define ADDRESS_ALARM1_MINUTE 1
#define ADDRESS_ALARM2_HOUR 2
#define ADDRESS_ALARM2_MINUTE 3
#define DISPLAY_TIME 0
#define DISPLAY_ALARM1 1
#define DISPLAY_ALARM2 2
#define DISPLAY_RADIO 4
#define DISPLAY_NUMBERS 5
#define PROGRESSIVE_WAKE_UP_TIME 30 //in minutes
#define ALARM_OFF 0
#define ALARM_ON 1
#define ALARM_PREON 2
#define ALARM_SNOOZE 3
#define ALARM_SECOND 2
#define SNOOZE_MINUTES 5
#define CAP_SINGLE 0
#define CAP_LONG 1

// Debounce Variable per button
long lastScreenSwtichDebounceTime = 0;
long lastSnoozeDebounceTime = 0;
long lastTimerDebounceTime = 0;
long lastRadioDebounceTime = 0;
long lastMinutesDebounceTime = 0;
long lastHoursDebounceTime = 0;
long lastAlarmSwitchebounceTime = 0;
long lastAlarm2DebounceTime = 0;
long lastAlarm1DebounceTime = 0;


// Variables 
long debounceDelay = 150;
time_t t;
long lastAccessToAlarm1Mode = 0;
long lastAccessToAlarm2Mode = 0;
boolean alarm1Changed = false;
boolean alarm2Changed = false;
tmElements_t alarm1 = loadAlarm1FromEEPROM();
tmElements_t alarm2 = loadAlarm2FromEEPROM();
tmElements_t alarm1Snooze;
int currentDisplay = getState();
float oldLedBrightness = 0;
float ledBrightness = 0;
int alarm1State = ALARM_OFF;
int alarm2State = ALARM_OFF;
int nmbCapTouch = 0;
long lastCapTouch = 0;
int oldValCap = 0;
long bTimeCap = 0;
CapacitiveSensor   cs_4_2 = CapacitiveSensor(SNOOZE_SEND,SNOOZE_RECEIVE);

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);
  
  //Initizalize input/output :
  pinMode(SCREEN_SWITCH_PIN, INPUT);
  pinMode(SNOOZE_PIN, INPUT);
  pinMode(TIMER_PIN, INPUT);
  pinMode(RADIO_PIN, INPUT);
  pinMode(MINUTES_PIN, INPUT);
  pinMode(HOURS_PIN, INPUT);
  pinMode(ALARM_SWITCH_PIN, INPUT);
  pinMode(ALARM2_PIN, INPUT);
  pinMode(ALARM1_PIN, INPUT);
  pinMode(LED_RELAY_220V_PIN, OUTPUT);
  pinMode(LED_RELAY_5V_PIN, OUTPUT);
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  Serial.println("Input/Output Inizialized");
  
  // Initialize relay
  digitalWrite(LED_RELAY_220V_PIN, HIGH);
  digitalWrite(LED_RELAY_5V_PIN, HIGH);
  Serial.println("Relay Inizialized");
  
  // Initialize 7 segments display
  Tlc.init();

  // Initialize DS1307
  setSyncProvider(RTC.get);
  setSyncInterval(3600);
  tmElements_t tm;
  while(!RTC.read(tm)){Serial.print(".");}
  setTime(tm.Hour, tm.Minute, tm.Second, tm.Day, tm.Month, tm.Year);
  Serial.println("DS1307 Inizialized");
  //setTime(5, 52,0, 15, 7, 2015);
  // Initialize Capacitive stuff
  cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);
  Serial.println("Capacitive Sensor Inizialized");
}

void loop(){
  t = now();
   // If in Time mode
  if (getState() == TIME_STATE){
    //Check if Vol+ button is pressed
    if ((millis() - lastHoursDebounceTime) > debounceDelay && digitalRead(HOURS_PIN)) {
      Serial.println("vol+");
      lastHoursDebounceTime = millis();
    }
      
    //Check if Vol+ button is pressed
    if ((millis() - lastMinutesDebounceTime) > debounceDelay && digitalRead(MINUTES_PIN)) {
      Serial.println("vol-");
      lastMinutesDebounceTime = millis();
    }
  }
  
  
  // If in Alarm1 mode
  else if (getState() == ALARM1_STATE){
    if (digitalRead(HOURS_PIN) && digitalRead(MINUTES_PIN)) {
      alarm1.Hour = 5;
      alarm1.Minute = 0;
      alarm1Changed = true;
      lastHoursDebounceTime = millis();
      lastMinutesDebounceTime = millis();
    }
    else{
      if ((millis() - lastHoursDebounceTime) > debounceDelay && digitalRead(HOURS_PIN)) {
        alarm1.Hour = (alarm1.Hour + 1) % 24;
        alarm1Changed = true;
        lastHoursDebounceTime = millis();
      }
      if ((millis() - lastMinutesDebounceTime) > debounceDelay && digitalRead(MINUTES_PIN)) {
        alarm1.Minute = (alarm1.Minute + 1) % 60;
        alarm1Changed = true;
        lastMinutesDebounceTime = millis();
      }
    }
    lastAccessToAlarm1Mode = millis();
  }
  
  
  // If in Alarm2 mode
  else{
    if ( alarm2State == ALARM_OFF ){
      alarm2.Hour = hour();
      alarm2.Minute = minute();
    }
    if ((digitalRead(HOURS_PIN) && digitalRead(MINUTES_PIN))) {
      alarm2.Hour = hour();
      alarm2.Minute = minute();
      alarm2Changed = true;
      alarm2State = ALARM_OFF;
      lastHoursDebounceTime = millis();
      lastMinutesDebounceTime = millis();
    }
    else{
      if ((millis() - lastHoursDebounceTime) > debounceDelay && digitalRead(HOURS_PIN)) {
        alarm2.Hour = (alarm2.Hour + 1) % 24;
        alarm2Changed = true;
        if (alarm2.Hour == hour() && alarm2.Minute == minute()){
          alarm2State = ALARM_OFF;
        }
        else{
          alarm2State = ALARM_PREON;
        }
        lastHoursDebounceTime = millis();
      }
      if ((millis() - lastMinutesDebounceTime) > debounceDelay && digitalRead(MINUTES_PIN)) {
        alarm2.Minute = (alarm2.Minute + 1) % 60;
        alarm2Changed = true;
        if (alarm2.Hour == hour() && alarm2.Minute == minute()){
          alarm2State = ALARM_OFF;
        }
        else{
          alarm2State = ALARM_PREON;
        }
        lastMinutesDebounceTime = millis();
      }
    }
    lastAccessToAlarm2Mode = millis();
  }
  
  
  // Eventually save alarm 1
  if (alarm1Changed && millis() - lastAccessToAlarm1Mode > TIME_BEFORE_SAVING_ALARM*1000){
    alarm1Changed = false;
    saveAlarm1ToEEPROM();
  }
  
  // Eventually save alarm 2
  if (alarm2Changed && millis() - lastAccessToAlarm2Mode > TIME_BEFORE_SAVING_ALARM*1000){
    alarm2Changed = false;
    saveAlarm2ToEEPROM();
  }
  
  // Any states:
  //Check if Radio button is pressed
  if ((millis() - lastRadioDebounceTime) > debounceDelay && digitalRead(RADIO_PIN)) {
    Serial.println("Radio");
    lastRadioDebounceTime = millis();
  }
  
  // Eventually trigger ALARM 1
  if ( alarm1State == ALARM_OFF && (alarm1.Hour*60 + alarm1.Minute - PROGRESSIVE_WAKE_UP_TIME  == hour()*60 + minute()) && second() < ALARM_SECOND){
    alarm1State = ALARM_PREON;
    Serial.println("Alarm 1 pre on activated");
  }
  if ( alarm1State == ALARM_PREON && alarm1.Hour == hour() && alarm1.Minute == minute() && second() < ALARM_SECOND) { 
    alarm1State = ALARM_ON;
    Serial.println("Alarm 1 on activated");
  }
  if (alarm1State == ALARM_SNOOZE && alarm1Snooze.Hour == hour() && alarm1Snooze.Minute == minute() && second() < ALARM_SECOND){
    alarm1State = ALARM_ON;
    Serial.println("Alarm 1 on activated from snooze");
  }
  // Eventually trigger ALARM 2
  if ( alarm2State == ALARM_PREON && alarm2.Hour == hour() && alarm2.Minute == minute() && second() < ALARM_SECOND ){
    alarm2State = ALARM_ON;
    Serial.println("Alarm 2 on activated");
  }
  if (alarm2State = ALARM_SNOOZE && alarm2.Hour == hour() && alarm2.Minute == minute() && second() < ALARM_SECOND){
    alarm2State = ALARM_ON;
    Serial.println("Alarm 2 on activated from snooze");
  }
  
  if (alarm1State == ALARM_PREON){
    ledBrightness = ( (hour()*60 + minute() - (alarm1.Hour * 60 + alarm1.Minute - PROGRESSIVE_WAKE_UP_TIME) ) / float(PROGRESSIVE_WAKE_UP_TIME));
    // Start music
    Serial.println("playing");
  }
  
  if (alarm1State == ALARM_ON){
    // alarm sound
    // IF SNOOZE BUTTON ACTIVATED ==> ALARM IN SNOOZE MODE
    if (getCapacitiveStatus() == CAP_SINGLE){
      alarm1Snooze.Hour = alarm1.Hour;
      alarm1Snooze.Minute = alarm1.Minute + SNOOZE_MINUTES;
      if(alarm1Snooze.Minute >= 60){
        alarm1Snooze.Hour++;
        alarm1Snooze.Minute = alarm1Snooze.Minute - 60;
      }
      alarm1State = ALARM_SNOOZE;
    }
    if (getCapacitiveStatus() == CAP_LONG){
      alarm1State = ALARM_OFF;
    }
  }

  if (alarm2State == ALARM_ON){
    // play sound + turn on slighly led
    // IF SNOOZE BUTTON ACTIVATED ==> ALARM IN SNOOZE MODE
  }
  setLedBrightness();
  displayer();
  Tlc.setIntensity(getRoomBrightness());
  delay(50);
  Tlc.updateDigits();
}

void setLedBrightness(){
  //if(ledBrightness != oldLedBrightness){
    Serial2.print("1,");
    Serial2.println(ledBrightness*255);
    oldLedBrightness = ledBrightness;
    Serial.println(ledBrightness);
  //}
  if(ledBrightness > 0){
    Serial2.println("0,1");
    Serial.println("turning on");
  }
}

void displayer(){
  if(getState() == TIME_STATE){
    Tlc.setDigits(hour()/10,hour()%10,minute()/10,minute()%10,false, true, false, false, 500);
  }
  else if(getState() == ALARM1_STATE){
    Tlc.setDigits(alarm1.Hour/10,alarm1.Hour%10,alarm1.Minute/10,alarm1.Minute%10,false, true, false, false, 500);
  }
  else{
    int timeInMinToDisplay = alarm2.Hour*60 + alarm2.Minute - (hour()*60 + minute());
    int timeInMinToDisplayHour = timeInMinToDisplay / 60;
    int timeInMinToDisplayMinute = timeInMinToDisplay % 60;
    
    Tlc.setDigits(timeInMinToDisplayHour/10,timeInMinToDisplayHour%10,timeInMinToDisplayMinute/10,timeInMinToDisplayMinute%10,false, true, false, false, 500);
  }
}


int getState(){
  if(digitalRead(ALARM1_PIN)){
    return ALARM1_STATE;
  }
  else if (digitalRead(ALARM2_PIN)){
    return ALARM2_STATE;
  }
  else{
    return TIME_STATE;
  }
}



void saveAlarm1ToEEPROM(){
  EEPROM.write(ADDRESS_ALARM1_HOUR, alarm1.Hour);
  EEPROM.write(ADDRESS_ALARM1_MINUTE, alarm1.Minute);
  Serial.println("alarm 1 saved to EEPROM");
}

void saveAlarm2ToEEPROM(){
  EEPROM.write(ADDRESS_ALARM2_HOUR, alarm2.Hour);
  EEPROM.write(ADDRESS_ALARM2_MINUTE, alarm2.Minute);
  Serial.println("alarm 2 saved to EEPROM");
}

tmElements_t loadAlarm1FromEEPROM(){
  tmElements_t ret;
  ret.Hour = EEPROM.read(ADDRESS_ALARM1_HOUR);
  ret.Minute = EEPROM.read(ADDRESS_ALARM1_MINUTE);
  return ret;
}

tmElements_t loadAlarm2FromEEPROM(){
  tmElements_t ret;
  ret.Hour = 0;
  ret.Minute = 0;
  return ret;
}

int getCapacitiveStatus(){  
  int ret = -1;
  int val = cs_4_2.capacitiveSensor(30);
  if (abs(oldValCap) < 300 and abs(val) >= 300){
    bTimeCap = millis();
  }
  if (abs(val) >= 300 && abs(oldValCap) >= 300 && now() - bTimeCap >= 300){
    ret = CAP_LONG;
  }
  if (abs(val) < 300 && abs(oldValCap) >= 300 && millis() - bTimeCap < 100){
    ret =  CAP_SINGLE;
  }
  oldValCap = val;
  return ret;
}

float getRoomBrightness(){
  // Values read from the ldr are from 350 to 750
  // if the value read is 350, we return 15
  // if the value read is 750, we return 1
  int val = analogRead(A0);
  if (val <= 350) {return 15;}
  else if (val <= 325) {return 1;}
  else if (val <= 350) {return 0.9;}
  else if (val <= 375) {return 0.8;}
  else if (val <= 400) {return 0.7;}
  else if (val <= 425) {return 0.6;}
  else if (val <= 450) {return 0.5;}
  else if (val <= 475) {return 0.4;}
  else if (val <= 500) {return 0.3;}
  else if (val <= 525) {return 0.2;}
  else if (val <= 550) {return 0.1;}
  else if (val <= 575) {return 0.08;}
  else if (val <= 600) {return 0.06;}
  else if (val <= 625) {return 0.04;}
  else {return 0.01;}
  
  /*int res = int(abs(val-350-400)/26.66);
  return res;*/
}
