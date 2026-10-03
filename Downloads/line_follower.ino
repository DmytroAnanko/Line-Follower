  #include <QTRSensors.h>
  #define AIN1 7
  #define AIN2 8

  #define PWMA 5
  #define BIN1 9
  #define BIN2 10
  #define PWMB 6

  #define STBY 4

  #define Lsensor A0
  #define Csensor A1
  #define Rsensor A2
  #define LED_PIN LED_BUILTIN
  QTRSensors qtr;
  const uint8_t SensorCount = 3;
  uint16_t sensorValues[SensorCount];
  bool lastTurn;
  unsigned long lastActivityTime = 0;
  const unsigned long delay_Time = 10000;
  void setup() {
      Serial.begin(9600);
      Serial.println("Start");
      qtr.setTypeAnalog();
      qtr.setSensorPins((const uint8_t[]){A0, A1, A2}, SensorCount);
      pinMode(LED_PIN,OUTPUT);
      pinMode(BIN1, OUTPUT);
      pinMode(BIN2, OUTPUT);
      pinMode(PWMB,OUTPUT);
      pinMode(PWMA,OUTPUT);
      pinMode(AIN1, OUTPUT);
      pinMode(AIN2, OUTPUT);
      pinMode(STBY, OUTPUT);
      digitalWrite(STBY,HIGH);
      lastActivityTime = millis();
}


/*обязательно добавить калибровку роботу*/
//Reads and outputs sensors current values
void readSensors () {
qtr.read(sensorValues);
  for (int i = 0; i < SensorCount; i++) {
      Serial.println((sensorValues[i]) + " ");
  }
}

/*разобраться с калибровкой*/
 bool lineDetacted () {
  if (sensorValues[0] > 275 && sensorValues[0] < 800 || sensorValues[1] > 275 && sensorValues[1] < 800 || sensorValues[2] > 275 && sensorValues[2] < 800 ) {
      return true;
  }
  return false;
}

//Created for simplify code and  manage them in one function 
void motor (int leftspeed,int rightspeed) {
  /*проверить какой мотор к pwmb а какой к pwma*/
  if (rightspeed > 0) {
    analogWrite(PWMB,rightspeed);
    digitalWrite(BIN1,HIGH);
    digitalWrite(BIN2,LOW);
  }
   else if (rightspeed == 0) {
     analogWrite(PWMB,rightspeed);
     digitalWrite(BIN1,LOW);
     digitalWrite(BIN2,LOW);
  }
  else if (rightspeed < 0) {
    analogWrite(PWMB,rightspeed);
    rightspeed = -(rightspeed);
    digitalWrite(BIN1,LOW);
    digitalWrite(BIN2,HIGH);
  }
 
  if (leftspeed > 0) {
    analogWrite(PWMA,leftspeed);
    digitalWrite(AIN1,HIGH);
    digitalWrite(AIN2,LOW);
  }
  else if (leftspeed == 0) {
    analogWrite(PWMA,leftspeed);
    digitalWrite(AIN1,LOW);
    digitalWrite(AIN2,LOW);
  }
  else if(leftspeed < 0){
    analogWrite(PWMA,leftspeed);
    leftspeed = -(leftspeed);
    digitalWrite(AIN1,LOW);
    digitalWrite(AIN2,HIGH);
  }
}
/*The idea is to search for line in case if the robot lost it from the vision*/
void searchLine () {
   if (lastTurn == true) {
     motor(100,180);
   }
      else if (lastTurn == false) {
        motor(180,100);
      }
}
//Main Logic for robot
void followLine () {
    if (sensorValues[0] < 300 && sensorValues[1] > 300 && sensorValues[2] > 300) {
      /*в этом случаи ему нужно просто немножко левее поворачивать,а вот если только один выдит тогда совсем по другому должен действовать*/
        lastTurn = true;
        motor(180, 80);
    }
    else if (sensorValues[0] > 300 && sensorValues[1] > 300 && sensorValues[2] < 300) {
        lastTurn = false;
        motor(80, 180);
    }
    else if (sensorValues[0] < 300 && sensorValues[1] < 300 && sensorValues[2] < 300) {
      /*если потерял линию*/
        searchLine();  
    }
    else if (sensorValues[0] < 300 && sensorValues[1] > 300 && sensorValues[2] < 300) {
        motor(200,200);
    }
    else if (sensorValues[0] < 300 && sensorValues[1] < 300 && sensorValues[2] > 300) {
        lastTurn = true;
        motor(180,90);
    }
    else if (sensorValues[0] > 300 && sensorValues[1] < 300 && sensorValues[2] < 300) {
        //если только левый выдит
        lastTurn = false;
        motor(90,180);
    }
    else if (sensorValues[0] > 300 && sensorValues[1] > 300 && sensorValues[2] > 300) {
      /*это означает что линию сильная большая для всех датчиков и надо либо ее либо просто чтобы ехал дальше*/
        motor(200,200);
    }
    else if (sensorValues[0] > 300 && sensorValues[1] < 300 && sensorValues[2] > 300) {
      motor(200,200);
    }
}

void loop () {
  readSensors();
  followLine();
  
  if (lineDetacted()) {
    lastActivityTime = millis();
    digitalWrite(LED_PIN,HIGH);
  }
  else if (millis() - lastActivityTime >= delay_Time) {//verify is robot lost line for more than 10 sec
    motor(0,0);
    digitalWrite(LED_PIN,LOW);
    Serial.println("Line follower is stopped");
    return;
  }
}