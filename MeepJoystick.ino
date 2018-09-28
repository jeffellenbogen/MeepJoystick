// SoftwareSerial is used to communicate with the XBee
#include <SoftwareSerial.h>

SoftwareSerial XBee(2, 3); // Arduino RX, TX (XBee Dout, Din)

#define zippyy_switch_pin_1 6   //white     
#define zippyy_switch_pin_2 7   //red     
#define zippyy_switch_pin_3 8   //green     
#define zippyy_switch_pin_4 9   //blue
#define speedButton_pin 10

#define LED_pinR 11
#define LED_pinG 12
#define LED_pinB 13

int tempDirection = 5;
int Speed = 2;
int Direction;

void setup() {  
  Serial.begin(9600); // Start serial communication
  XBee.begin(9600);  
  //with XBee at 9600 baud
  delay(10);
  pinMode(zippyy_switch_pin_1,INPUT_PULLUP); 
  pinMode(zippyy_switch_pin_2,INPUT_PULLUP);
  pinMode(zippyy_switch_pin_3,INPUT_PULLUP); 
  pinMode(zippyy_switch_pin_4,INPUT_PULLUP);
  pinMode(speedButton_pin,INPUT_PULLUP); 
  pinMode(LED_pinR,OUTPUT); 
  pinMode(LED_pinG,OUTPUT); 
  pinMode(LED_pinB,OUTPUT);
  analogWrite(LED_pinR, 210);
  analogWrite(LED_pinG, 150); 
  analogWrite(LED_pinB, 0);
}

void loop() {

//Read ZIPPYY Joystick
  
      if ((digitalRead(zippyy_switch_pin_4) == 0) && 
      (digitalRead(zippyy_switch_pin_2) == 0))
        Direction = 3;
      else if ((digitalRead(zippyy_switch_pin_1) == 0) && 
      (digitalRead(zippyy_switch_pin_2) == 0))
        Direction = 9;
      else if ((digitalRead(zippyy_switch_pin_1) == 0) && 
      (digitalRead(zippyy_switch_pin_3) == 0))
        Direction = 7;
      else if ((digitalRead(zippyy_switch_pin_3) == 0) && 
      (digitalRead(zippyy_switch_pin_4) == 0))
        Direction = 1;    
      else if(digitalRead(zippyy_switch_pin_1) == 0)
        Direction = 8;
      else if(digitalRead(zippyy_switch_pin_4) == 0)
        Direction = 2;
      else if(digitalRead(zippyy_switch_pin_2) == 0)
        Direction = 6;
      else if(digitalRead(zippyy_switch_pin_3) == 0)
        Direction = 4;
      else
        Direction = 5;  
       delay (100);
       
//Check to see if joystick direction has changed

      if(Direction != tempDirection){
         if (Direction == 1)
            driveForwardSlightLeft();
         else if (Direction == 2)
            driveForward();
         else if (Direction == 3)
            driveForwardSlightRight();           
         else if (Direction == 4)
            driveLeft();
         else if (Direction == 5)
            stopDriving();        
         else if (Direction == 6)
            driveRight();     
         else if (Direction == 7)
            driveBackSlightLeft();
         else if (Direction == 8)
            driveBack();       
         else if (Direction == 9)
            driveBackSlightRight();       
           
         tempDirection = Direction;
      }

      if (digitalRead(speedButton_pin) == 0){
        speedToggle();
        delay(100);
      }
         
      
}

void stopDriving(){
  Serial.println("Stop");
  XBee.print('5');
}

void driveForward(){
  Serial.println("Drive Forward");
  XBee.print('2');
}

void driveBack(){
  Serial.println("Drive Back");
  XBee.print('8');
}

void driveLeft(){
  Serial.println("Drive Left");
  XBee.print('4');
}

void driveRight(){
  Serial.println("Drive Right");
  XBee.print('6');
}

void driveForwardSlightLeft(){
  Serial.println("Drive Forward Slight Left");
  XBee.print('1');
}

void driveForwardSlightRight(){
  Serial.println("Drive Forward Slight Right");
  XBee.print('3');
}

void driveBackSlightRight(){
  Serial.println("Drive Back Slight Right");
  XBee.print('9');
}

void driveBackSlightLeft(){
  Serial.println("Drive Back Slight Left");
  XBee.print('7');
}

void speedToggle(){

  Speed++;
  if (Speed >3)
    Speed = 1;

  if (Speed == 1){
    Serial.println("Slow Mo Speed");
    XBee.print('S');
    analogWrite(LED_pinR, 255);
    analogWrite(LED_pinG, 0);
    analogWrite(LED_pinB, 0);
    delay(50);
    XBee.print('S');
    }

  if (Speed == 2){
    Serial.println("Regular Speed!");
    XBee.print('R');
    analogWrite(LED_pinR, 210);
    analogWrite(LED_pinG, 150);
    analogWrite(LED_pinB, 0);
    delay(50);
    XBee.print('R');
    }       

    
  if (Speed == 3){
    Serial.println("Turbo Speed!");
    XBee.print('T');
    analogWrite(LED_pinR, 0);
    analogWrite(LED_pinG, 255);
    analogWrite(LED_pinB, 0);
    delay(50);
    XBee.print('T');
    }      
}
