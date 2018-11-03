// SoftwareSerial is used to communicate with the XBee
// Adding LCD interface with Meep 4WD

#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <debouncer.h>

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

Debouncer debounceUp(zippyy_switch_pin_4);
Debouncer debounceDown(zippyy_switch_pin_1);
Debouncer debounceLeft(zippyy_switch_pin_3);
Debouncer debounceRight(zippyy_switch_pin_2);
Debouncer debounceSpeed(speedButton_pin);

#define LED_pinR 11
#define LED_pinG 12
#define LED_pinB 13

int tempDirection = 5;
int Speed = 2;
int Direction;

// Time, in ms, the joystick will wait until resending a command
#define ACK_TIMEOUT 250   

// if we haven't sent something to the meep in a while, send 
// a keep-alive command.  Note this time needs to be LESS than the
// keep-alive time on the meep side.
#define KEEP_ALIVE_MS 500

typedef struct
{
  bool          outstanding_dir;
  char          last_sent_dir;
  unsigned long last_sent_dir_time;

  bool          outstanding_speed;
  char          last_sent_speed;
  unsigned long last_sent_speed_time;
} ack_state_type;

static ack_state_type ack_state;

typedef struct
{
  int successful_acks;
  int resends;
} stats_type;

stats_type stats={0,0};

// These strings are used for debug prints out the serial port.
// The array index maps to the direction.
String dir_strings[] =
{
  "INVALID direction",         
  "Drive Forward Slight Left",
  "Drive Forward",
  "Drive Forward Slight Right",
  "Drive Left",
  "Stop",
  "Drive Right",
  "Drive Back Slight Left",
  "Drive Back",
  "Drive Back Slight Right"
};
/*=====================================================================
 * Function: init_ack_state
 */
void init_ack_state( void )
{
  ack_state.outstanding_dir = false;
  ack_state.last_sent_dir = '5';
  ack_state.last_sent_dir_time = 0;

  ack_state.outstanding_speed = false;
  ack_state.last_sent_speed = 'R';
  ack_state.last_sent_speed_time = 0;
  
}  // end of init_ack_state

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

  init_ack_state();
  
  lcd.begin(LCD_CHARS, LCD_ROWS);
  lcd.clear();
  lcd.print("Joystick");
  lcd.setCursor(0,1);
  lcd.print("On");
  
  Serial.println("Joystick initialized");
  
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
  up =    !debounceUp.read();
  down =  !debounceDown.read();
  left =  !debounceLeft.read();
  right = !debounceRight.read();
  
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
    // Send our new direction to the Meep.
    XBee.print(Direction);

    // and remember when we sent it...
    ack_state.outstanding_dir = true;
    ack_state.last_sent_dir = Direction + '0';  // convert # to char
    ack_state.last_sent_dir_time = millis();

    // Print a debug string out the serial port to show which way we're going.
    Serial.println(dir_strings[Direction]);

    // ...and remember which way we're going for next time.
    tempDirection = Direction;
    
  }  // end if direction changed
}  // end check_and_send_dir

/*=====================================================================
 * Function: check_and_send_speed
 */
void check_and_send_speed( void )
{
  static int last_speed_button_state = 1;  // looking for high to low transitions.
  int current_speed_button_state;
  
  current_speed_button_state = debounceSpeed.read();
  
  if ((last_speed_button_state == 1) && (current_speed_button_state == 0))
  {
    speedToggle();
  }

  last_speed_button_state = current_speed_button_state;

} // end of check_and_send_speed

/*=====================================================================
 * Function: loop
 */
void loop() {

  //Read ZIPPYY Joystick
  get_joystick_direction();
       
  //Check to see if joystick direction has changed.  If
  //it has, send the appropriate command to the Meep.
  check_and_send_dir();
  
  //Check the speed button to see if we need to send anew speed to the Meep.
  check_and_send_speed();
  
  // Does the MEEP have anything for us?
  check_meep();   

  // If any of our commands have timed out, resend them.
  check_for_resends();

  // Print any debounces on the LCD, for debug purposes.
  print_debounces();

  // If we haven't sent anything to the meep in a while, send a keep-alive.
  check_for_keep_alive();
          
}  // end of loop

/*=====================================================================
 * Function: speedToggle
 */
void speedToggle(){

  Speed++;
  if (Speed >3)
    Speed = 1;

  if (Speed == 1)
  {
    Serial.println("Slow Mo Speed");  // What the joystick is sending
    XBee.print('S');
    analogWrite(LED_pinR, 255);
    analogWrite(LED_pinG, 0);
    analogWrite(LED_pinB, 0);
    ack_state.last_sent_speed = 'S';
  }

  if (Speed == 2)
  {
    Serial.println("Regular Speed!");
    XBee.print('R');
    analogWrite(LED_pinR, 210);
    analogWrite(LED_pinG, 150);
    analogWrite(LED_pinB, 0);
    ack_state.last_sent_speed = 'R';
  }       

  if (Speed == 3)
  {
    Serial.println("Turbo Speed!");
    XBee.print('T');
    analogWrite(LED_pinR, 0);
    analogWrite(LED_pinG, 255);
    analogWrite(LED_pinB, 0);
    ack_state.last_sent_speed = 'T';
  }      

  // remember the last speed we sent
  ack_state.last_sent_speed_time = millis();
  ack_state.outstanding_speed = true;
  
} // of of Speed Toggle Function


/*=====================================================================
 * Function: check_meep
 */
void check_meep()
{

    char c;
    while (XBee.available())
    {
       c = XBee.read();

       Serial.print("RX Char: ");
       Serial.println(c);

       // check for speed acks
       if ((ack_state.outstanding_speed) && (ack_state.last_sent_speed == c))
       {
          ack_state.outstanding_speed = false;
          stats.successful_acks++;
          Serial.println("speed acked");
       }

       // then check for dir acks.   Note only one of these really should fire...
       if ((ack_state.outstanding_dir) && (ack_state.last_sent_dir == c))
       {
         ack_state.outstanding_dir = false;
         stats.successful_acks++; 
         Serial.println("dir acked");
       }

       // now print what we've got on the LCD.
       switch(c)
       {
         case 'R': 
            lcd.setCursor(0, 1);
            lcd.print("REGULAR ");
         break;
         
         case 'T': 
            lcd.setCursor(0, 1);
            lcd.print("TURBO!!!");
         break;
         
         case 'S': 
            lcd.setCursor(0, 1);
            lcd.print("SLOW MO ");
         break; 
         
         case '1': 
            lcd.setCursor(0, 0);
            lcd.print("F-S-LEFT");
         break;

         case '2': 
            lcd.setCursor(0, 0);
            lcd.print("FORWARD ");
         break; 
         
         case '3': 
            lcd.setCursor(0, 0);
            lcd.print("F-S-RGHT");
         break;  
         
         case '4': 
            lcd.setCursor(0, 0);
            lcd.print("LEFT    ");
         break;   
         
         case '5': 
            lcd.setCursor(0, 0);
            lcd.print("STOP    ");
         break;    
         
         case '6': 
            lcd.setCursor(0, 0);
            lcd.print("RIGHT   ");
         break;  
         
         case '7': 
            lcd.setCursor(0, 0);
            lcd.print("B-S-LEFT");
         break;
         
         case '8': 
            lcd.setCursor(0, 0);
            lcd.print("BACKWARD");
         break; 
         
         case '9': 
            lcd.setCursor(0, 0);
            lcd.print("B-S-RGHT");
         break;                                                                      
       } //end of switch on c
    }  // end of if XBee.available
    
}  // end of check_meep

/*=====================================================================
 * Function: check_for_resends
 */
void check_for_resends( void )
{
  unsigned long current_time;

  current_time = millis();
  
  if ((ack_state.outstanding_dir) &&
      (current_time > ack_state.last_sent_dir_time + ACK_TIMEOUT))
  {
    XBee.print(ack_state.last_sent_dir);
    ack_state.last_sent_dir_time = current_time;

    stats.resends++;

    print_resends();
    
    Serial.print("Resend dir: ");
    Serial.println(ack_state.last_sent_dir);
  }

  if ((ack_state.outstanding_speed) &&
      (current_time > ack_state.last_sent_speed_time + ACK_TIMEOUT))
  {
    XBee.print(ack_state.last_sent_speed);
    ack_state.last_sent_speed_time = current_time;

    stats.resends++;

    print_resends();

    Serial.print("Resend speed: ");
    Serial.println(ack_state.last_sent_speed);
  }
 
}

/*=====================================================================
 * Function: print_resends
 */
void print_resends( void )
{
  lcd.setCursor(11, 1);
  lcd.print(stats.resends);
}

/*=====================================================================
 * Function: print_debounces
 */
void print_debounces( void )
{
  int speed_debounces;
  int joystick_debounces;
  
  joystick_debounces =
    debounceUp.getNumBounces() +
    debounceDown.getNumBounces() +
    debounceLeft.getNumBounces() +
    debounceRight.getNumBounces();

  // For LCD display purposes, we only want 3 digits for debounce counts.
  if (joystick_debounces > 999) joystick_debounces = 999;

  speed_debounces = debounceSpeed.getNumBounces();

  // Technically, the debouncer currently limits us at 999...but I'm putting this in
  // here as a safety check in case that implementation ever changes.
  if (speed_debounces > 999) speed_debounces = 999;

  lcd.setCursor(10, 0);
  lcd.print(joystick_debounces);
  lcd.setCursor(14, 0);
  lcd.print(speed_debounces);
  
}

/*=====================================================================
 * Function: check_for_keep_alive
 */
void check_for_keep_alive( void )
{
  unsigned long        current_time;
  bool                 keep_alive_needed=true;
  static unsigned long last_sent_keep_alive=0;

  current_time = millis();

   if ((current_time > ack_state.last_sent_dir_time + KEEP_ALIVE_MS) &&
       (current_time > ack_state.last_sent_speed_time + KEEP_ALIVE_MS) &&
       (current_time > last_sent_keep_alive + KEEP_ALIVE_MS))
   {
     Serial.println("Sending Keep Alive");
     XBee.print('K');
     last_sent_keep_alive = current_time;
   }
}
