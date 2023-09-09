#include <SoftwareSerial.h>
SoftwareSerial BTserial(12, 13); //TX | RX
const long baudRate = 9600;

#include"PWMServo.h"
PWMServo fan;

#include "SparkFun_TB6612.h"
#define AIN1 5 //output pins 
#define AIN2 4
#define PWMA 10 //pin 3 PWM
#define STBY 6 //standby pin is pin 6
const int offsetA = 1;
Motor DC_motor = Motor( AIN1, AIN2, PWMA, offsetA, STBY );
volatile int counter = 0;
int prev_counter = 0;
const int Bpin = 2;
const int Apin = 3;
int BState = 0;
int AState = 0;

String reading, readingCopy, X, Y, Z;
int x = 0, y = 0, z = 0, inRoom = 0, window = 0, Override = 0;
int LEDpin = 11, LEDbrightness = 0;
int IRpin = 1, IRreading;
int tempPin = 0, temp;
int fanPin = 9, fanSpeed = 0;
char button, button2, button3;
int lastReading = 0 ;

void setup()
{
  Serial.begin(baudRate);
  BTserial.begin(baudRate);
  
  pinMode(LEDpin, OUTPUT); // LED PWM pin set to output
  pinMode(Bpin, INPUT_PULLUP);
  pinMode(Apin, INPUT_PULLUP); // put your setup code here, to run once:
  attachInterrupt(digitalPinToInterrupt(Apin), channelA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(Bpin), channelB, CHANGE);
  
  fan.attach(fanPin);
  fan.write(90);


}


double counts_per_sec;
unsigned long timer;
unsigned long period = 20000;
double dt = 0.02;
void channelA() {                                   // encoder setup
  if (digitalRead(Apin) == HIGH) {      
    if (digitalRead(Bpin) == LOW)counter++;
    else counter--;
  }
  else {
    if (digitalRead(Bpin) == HIGH)counter++;
    else counter--;
  }
}
void channelB() {
  if (digitalRead(Bpin) == HIGH) {
    if (digitalRead(Apin) == HIGH)counter++;
    else counter--;
  }
  else {
    if (digitalRead(Apin) == LOW)counter++;
    else counter--;
  }
}
int counts = 0;
double time = 0;
int value = 0;
int abs_value = 0;
const double Countperdegree = 9.9333;
double angle = 0;
int kdaWkda ;
double angular_velocity = 0;


void loop()
{
  IRreading = analogRead(IRpin);    
  if (IRreading >= 270 && !inRoom) {    // infra red sensor with distance 270 which is the door
    analogWrite(11, 255);
    inRoom = 1;
  }
  if (BTserial.available() > 0)   // check bluetooth serial is available
  {
    BTserial.setTimeout(50); // Setting time out to terminate readString()
    reading = BTserial.readString(); // Read potentiometer reading as string
    X = reading.substring(0, 5);      // read x, y and z
    Y = reading.substring(5, 10);
    Z = reading.substring(10, 15);
    button = reading[15];           // button reading
    button2 = reading[16];
    button3 = reading[17];
    x = X.toInt();        // convert x, y and z to integer
    y = Y.toInt();
    z = Z.toInt();
       
    if (z < 9700 && z > 10 && x < -250 && x > -9000 && y < 3000 && y > -3000 && button == '1') {
      if (x>-5000){
      LEDbrightness = map(x, -250, -5000, 0, 70);        // if accelerometer rotates around x-axis map LED brightness to angle of rotation
      }else{
      LEDbrightness = map(x, -5000, -9000, 71, 255);
      }
      inRoom = 1;
      BTserial.print(LEDbrightness);    // send LED brightness to bluetooth module
      analogWrite(11, LEDbrightness);   // change LED brightness
        
    } else if (z < 9700 && x >= -250 && y < 3000 && y > -3000 && button == '1') {
      LEDbrightness = 0;      // LED brightness equal to 0 if rotation is outside margin
      inRoom = 1;
      BTserial.print(LEDbrightness);
      analogWrite(11, LEDbrightness);
    }


    
      if (z < 9700 && z > 10 && x < -250 && x > -10000 && y < 3000 && y > -3000 && button2 == '1') {
      fan.write(180);     // if accelerometer detect rotation to the left, fan should be on, otherwise fan should be off
      Override = 1;
      BTserial.print(1);
    } else if (z < 9700 && x >= -250 && y < 3000 && y > -3000 && button2 == '1') {
      fan.write(90);
      Override = 1;
      BTserial.print(0);
    }

    counts=(counter-prev_counter);
    angle = ((double)counter / Countperdegree);
    prev_counter=counter;
    //Serial.println(angle);
    if (angle>0 && z < 9700 && z > 10 && y > -3000 && x < 3000 && x > -3000 && button3 == '1') {
      while(angle>0  && z < 9700 && z > 10 && y > -3000 && x < 3000 && x > -3000 && button3 == '1'){      // MOTOR pulls window up if accelerometer reading is above the required threshold
        DC_motor.drive(-255);
        counts=(counter-prev_counter);
        angle = ((double)counter / Countperdegree);
        prev_counter=counter;
        Serial.println(angle);
        BTserial.print(angle);
        reading = BTserial.readString(); // Read potentiometer reading as string
        X = reading.substring(0, 5);
        Y = reading.substring(5, 10);
        Z = reading.substring(10, 15);
        button = reading[15];
        button2 = reading[16];
        button3 = reading[17];
        x = X.toInt();
        y = Y.toInt();
        z = Z.toInt();
      }
    } 
    else if (z < 9700 && y <= -3000 && y > -6000 && x > -3000 &&  x < 3000 && button3 == '1') {
     DC_motor.brake();
    }
    else if(angle<780 && z < 9700 && y <= -6000 && x > -3000 &&  x < 3000 && button3 == '1') {
      while(angle<780 && z < 9700 && y <= -6000 && x > -3000 &&  x < 3000 && button3 == '1'){
        DC_motor.drive(255);
        counts=(counter-prev_counter);
        angle = ((double)counter / Countperdegree);
        prev_counter=counter;
        Serial.println(angle);
        BTserial.print(angle);
        reading = BTserial.readString(); // Read potentiometer reading as string
        X = reading.substring(0, 5);
        Y = reading.substring(5, 10);
        Z = reading.substring(10, 15);
        button = reading[15];
        button2 = reading[16];
        button3 = reading[17];        // read buttons
        x = X.toInt();
        y = Y.toInt();        // convert to integer
        z = Z.toInt();
      }
    }
    else{
     DC_motor.brake();    // stop motor
    }

  
  }
}
