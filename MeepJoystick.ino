// SoftwareSerial is used to communicate with the XBee
// Adding LCD interface with Meep 4WD

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

/*=====================================================================
 * Function: setup
 */
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
  lcd.print("Joystick On");
} // end of setup

/*=====================================================================
 * Function: get_joystick_direction
 */
void get_joystick_direction( void )
{
  int up;
  int down;
  int left;
  int right;

  // the joystick pins are active low; therefore to 
  // get the logical direction, we need to invert what
  // we read.
  up =    !digitalRead(zippyy_switch_pin_4);
  down =  !digitalRead(zippyy_switch_pin_1);
  left =  !digitalRead(zippyy_switch_pin_3);
  right = !digitalRead(zippyy_switch_pin_2);

  if (up)
  {
    if (left)       Direction = 1;
    else if (right) Direction = 3;
    else            Direction = 2;
  }
  else if (down)
  {
    if (left)       Direction = 7;
    else if (right) Direction = 9;
    else            Direction = 8;
  }
  else if (left)    Direction = 4;
  else if (right)   Direction = 6;
  else              Direction = 5;
  
} // end of get_joystick_direction

/*=====================================================================
 * Function: check_and_send_dir
 */
void check_and_send_dir( void )
{
  if(Direction != tempDirection)
  {
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
  }  // end if direction changed
}  // end check_and_send_dir

/*=====================================================================
 * Function: loop
 */
void loop() {

  //Read ZIPPYY Joystick
  get_joystick_direction();
  
  delay (100);
       
  //Check to see if joystick direction has changed.  If
  //it has, send the appropriate command to the Meep.
  check_and_send_dir();
  

  //Check the speed button.  If it's changed, send the new speed to the Meep.
  if (digitalRead(speedButton_pin) == 0)
  {
    speedToggle();
    delay(100);
  }

  // Does the MEEP have anything for us?
  check_meep();   
          
}  // end of loop

/*=====================================================================
 * Function: stopDriving
 */
void stopDriving(){
  Serial.println("Stop");
  XBee.print('5');
}

/*=====================================================================
 * Function: driveForward
 */
void driveForward(){
  Serial.println("Drive Forward");
  XBee.print('2');
}

/*=====================================================================
 * Function: driveBack
 */
void driveBack(){
  Serial.println("Drive Back");
  XBee.print('8');
}
/*=====================================================================
 * Function: driveLeft
 */
void driveLeft(){
  Serial.println("Drive Left");
  XBee.print('4');
}

/*=====================================================================
 * Function: driveRight
 */
void driveRight(){
  Serial.println("Drive Right");
  XBee.print('6');
}

/*=====================================================================
 * Function: driveForwardSlightLeft
 */
void driveForwardSlightLeft(){
  Serial.println("Drive Forward Slight Left");
  XBee.print('1');
}

/*=====================================================================
 * Function: driveForwardSlightRight
 */
void driveForwardSlightRight(){
  Serial.println("Drive Forward Slight Right");
  XBee.print('3');
}

/*=====================================================================
 * Function: driveBackSlightRight
 */
void driveBackSlightRight(){
  Serial.println("Drive Back Slight Right");
  XBee.print('9');
}

/*=====================================================================
 * Function: driveBackSlightLeft
 */
void driveBackSlightLeft(){
  Serial.println("Drive Back Slight Left");
  XBee.print('7');
}

/*=====================================================================
 * Function: speedToggle
 */
void speedToggle(){

  Speed++;
  if (Speed >3)
    Speed = 1;

  if (Speed == 1){
    Serial.println("Slow Mo Speed");  // What the joystick is sending
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
} // of of Speed Toggle Function

/*=====================================================================
 * Function: check_meep
 */
void check_meep()
{
    char c;
    if (XBee.available())
    {
       c = XBee.read();
       switch(c)
       {
         case 'R': 
            lcd.setCursor(0, 1);
            lcd.print("REGULAR ");
         break;
         case 'T': 
            lcd.setCursor(0, 1);
            lcd.print("TURBO!!! ");
         break;
         case 'S': 
            lcd.setCursor(0, 1);
            lcd.print("SLOW MO ");
         break; 
         case '1': 
            lcd.setCursor(0, 0);
            lcd.print("Forw. Slgt. LEFT");
         break;
         case '2': 
            lcd.setCursor(0, 0);
            lcd.print("FORWARD         ");
         break; 
         case '3': 
            lcd.setCursor(0, 0);
            lcd.print("Forw. Slgt. RGHT");
         break;  
         case '4': 
            lcd.setCursor(0, 0);
            lcd.print("LEFT            ");
         break;   
         case '5': 
            lcd.setCursor(0, 0);
            lcd.print("STOP            ");
         break;    
         case '6': 
            lcd.setCursor(0, 0);
            lcd.print("RIGHT           ");
         break;  
         case '7': 
            lcd.setCursor(0, 0);
            lcd.print("Back  Slgt. LEFT");
         break;
         case '8': 
            lcd.setCursor(0, 0);
            lcd.print("BACKWARD        ");
         break; 
         case '9': 
            lcd.setCursor(0, 0);
            lcd.print("Back Slgt. RIGHT");
         break;                                                                      
       } //end of switch on c
    }  // end of if XBee.available
    
}  // end of check_meep
