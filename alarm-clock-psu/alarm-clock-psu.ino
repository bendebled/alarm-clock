String key = "";
String value = "";
bool keySet = false;

bool psuOn = false;

#define PSU_PIN 2
#define LED_PIN 3

void setup() {
  Serial.begin(9600);

  pinMode(PSU_PIN, OUTPUT);
  digitalWrite(PSU_PIN, HIGH);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(PSU_PIN, LOW);
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
}

void doAction(byte key, byte value){
  if(key == 0){
      if(value == 0 && psuOn){
        turnPSU(false);
      }
      else if (value == 1 && !psuOn){
        turnPSU(true);
      }
  }
  else if(key == 1){
    if(value > 0 && !psuOn){
      turnPSU(true);
    }
    analogWrite(LED_PIN, value);
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

