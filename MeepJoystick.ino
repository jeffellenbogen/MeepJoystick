// SoftwareSerial is used to communicate with the XBee
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

/*================
 * LCD CONNECTIONS:  (note...we're using 4 bit mode here...)
 *   1 to GND
 *   2 to 5V
 *   3 to the contrast control...I did a hardcoded voltage divider.
 *   4 to Arduino digital pin LCD_REG_SEL
 *   5 to GND
 *   6 to Arduino digital pin LCD_ENABLE
 *   7 (no connection)
 *   8 (no connection)
 *   9 (no connection)
 *   10 (no connection)
 *   11 to Arduino  digital pin LCD_D4
 *   12 to Arduino  digital pin LCD_D5
 *   13 to Arduino  digital pin LCD_D6
 *   14 to Arduino  digital pin LCD_D7
 *   15 to 5V
 *   16 to GND
 *====================*/
 
#define LCD_D7         A0 
#define LCD_D6         A1
#define LCD_D5         A2
#define LCD_D4         A3
#define LCD_ENABLE     A4
#define LCD_REG_SEL    A5

// Our LCD has 2 rows of 16 characters.
#define LCD_CHARS 16
#define LCD_ROWS 2

LiquidCrystal lcd(LCD_REG_SEL, LCD_ENABLE, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

SoftwareSerial XBee(2, 3); // Arduino RX, TX (XBee Dout, Din)

#define zippyy_switch_pin_1 6   //white     
#define zippyy_switch_pin_2 7   //green     
#define zippyy_switch_pin_3 8   //red     
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

  lcd.begin(LCD_CHARS, LCD_ROWS);
  lcd.clear();
  lcd.print("M33p!");
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
    lcd.setCursor(0, 1);
    lcd.print("SLOW MO ");
    
    Serial.println("Slow Mo Speed");
    XBee.print('S');
    analogWrite(LED_pinR, 255);
    analogWrite(LED_pinG, 0);
    analogWrite(LED_pinB, 0);
    delay(50);
    XBee.print('S');
    }

  if (Speed == 2){
    lcd.setCursor(0, 1);
    lcd.print("REGULAR ");
    Serial.println("Regular Speed!");
    XBee.print('R');
    analogWrite(LED_pinR, 210);
    analogWrite(LED_pinG, 150);
    analogWrite(LED_pinB, 0);
    delay(50);
    XBee.print('R');
    }       

    
  if (Speed == 3){
    lcd.setCursor(0, 1);
    lcd.print("TURBO!!!");
    Serial.println("Turbo Speed!");
    XBee.print('T');
    analogWrite(LED_pinR, 0);
    analogWrite(LED_pinG, 255);
    analogWrite(LED_pinB, 0);
    delay(50);
    XBee.print('T');
    }      
}
