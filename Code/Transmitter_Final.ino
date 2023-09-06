/*This is the program for the transmitter circuit which reads data from the accelerometer and the buttons on the LCD shield
 * and sends these reading through the Bluetooth serial to the receiver. The program also receives
 * data from the receiver, such as the LED brightness and the states of the window and fan, to be displayed
 * on the LCD screen.
 */

#include <SoftwareSerial.h>

//include accelerometer library
#include "SparkFunLIS3DH.h"
#include "Wire.h"
#include "SPI.h"

SoftwareSerial BTserial(2, 3); // Arduino-TX | Arduino-RX 
const long baudRate = 9600; 
LIS3DH myIMU(SPI_MODE, 10); // constructed with parameters for SPI and cs pin number
String LEDbrightness,fanState,WindowState;
float y = 0, x = 0, z = 0;
int button = 0, button2 = 0, button3 = 0, i, flag;
const int buttonPin = 7, button2Pin = 8;

//include LCD library
#include <LiquidCrystal.h>
// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
// read the buttons
int t ;
int read_LCD_buttons()
{
 adc_key_in = analogRead(0);      // read the value from the sensor 
 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 /*if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 // For V1.1 us this threshold
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 250)  return btnUP; 
 if (adc_key_in < 350)  return btnDOWN; 
 if (adc_key_in < 500)  return btnLEFT; 
 if (adc_key_in < 650)  return btnSELECT;  
*/
 // For V1.0 comment the other threshold and use the one below:
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 195)  return btnUP; 
 if (adc_key_in < 380)  return btnDOWN; 
 if (adc_key_in < 555)  return btnLEFT; 
 if (adc_key_in < 790)  return btnSELECT;   



 return btnNONE;  // when all others fail, return this...
}


void setup() {
  Serial.begin(baudRate);
  BTserial.begin(baudRate);
  pinMode(buttonPin, INPUT); //set pin connected to LCD buttons as input
  myIMU.begin(); //start accelerometer library
  
 lcd.begin(16, 2);              // start the LCD library
 lcd.setCursor(0,0);
 lcd.print("Push the buttons"); // print a simple message


}
void loop() {

  lcd_key = read_LCD_buttons();  // read the buttons
  switch (lcd_key)               // depending on which button was pushed, we perform an action
  {


   //case for window
   case btnRIGHT:
     {
      button3 = 1;

     lcd.setCursor(0,0);
     lcd.print("Window:         ");
      if(flag==1){
      lcd.setCursor(0,1);
      lcd.print("                ");   
      flag = 0;
      }
       if(BTserial.available()>0){  
       BTserial.setTimeout(50);
       WindowState = BTserial.readString();//read window state
       //print window state to LCD
       if (WindowState.toInt()<=70){
        lcd.setCursor(0,1);
        lcd.print("Closed");
        
       }else if(WindowState.toInt() >70){
         lcd.setCursor(0,1);
         lcd.print("Open  ");
       }
     }
     break;
     }

     //case for fan
   case btnLEFT:
     {
      flag = 1;
      lcd.setCursor(0,0);
      button2 = 1 ;
     lcd.print("Fan :           ");
     BTserial.setTimeout(50);
     fanState = BTserial.readString();//read fan state
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      //print fan state to LCD
      if (fanState[0] == '1'){
        
        lcd.print("ON ");
      }else {
        lcd.print("OFF");
      }
     break;
     }
   case btnUP:
     {
     break;
     }

     //case for LEDs
   case btnDOWN:
     {
      Serial.println(LEDbrightness);
      lcd.setCursor(0,0);
      
      lcd.print("Lights:          "); // print a simple message
    //  lcd.setCursor(0,1);
      button = 1;
       if(BTserial.available()>0){  
       BTserial.setTimeout(50);
       LEDbrightness = BTserial.readString();//read LED brightness
       //print LED prightness to LCD
       i = map(LEDbrightness.toInt(),0,180,0,15);
       for (t=0;t<16;t++){
        if ((t<=i)&&(LEDbrightness.toInt()>5)){
        lcd.setCursor(t,1);
        lcd.write(255);
        }else{
          lcd.setCursor(t,1);
        lcd.print(" ");
        }
       }
     }
     break;
     }
     case btnNONE:
     {
     //lcd.print("NONE  ");
//     lcd.clear();
//      lcd.setCursor(0,0);
//      lcd.print("Push the buttons"); // print a simple message}
     button = 0;
     button2 = 0;
     button3 = 0;
     break;
     }
 }
    //read accelerometer
    y = myIMU.readFloatAccelY()* 10000;
  x = myIMU.readFloatAccelX()* 10000;
  z = myIMU.readFloatAccelZ()* 10000;
  char Y[6]; 
  char X[6]; 
  char Z[6];
  dtostrf(x,5,0,X);
  dtostrf(y,5,0,Y);
  dtostrf(z,5,0,Z);

  //send data via bluetooth serial
  BTserial.print(String(X)); 
  BTserial.print(String(Y));
  BTserial.print(String(Z));
  BTserial.print(String(button));
  BTserial.print(String(button2));
  BTserial.print(String(button3));
  if(BTserial.available()>0){  
  BTserial.setTimeout(50);
  }
  delay(100);
}
