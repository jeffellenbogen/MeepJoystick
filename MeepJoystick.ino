// SoftwareSerial is used to communicate with the XBee
#include <SoftwareSerial.h>

#if !defined (TRUE)
#define TRUE 1
#endif

#if !defined (FALSE)
#define FALSE 0
#endif

SoftwareSerial XBee(2, 3); // Arduino RX, TX (XBee Dout, Din)

#define zippyy_switch_pin_1 6   //white     
#define zippyy_switch_pin_2 7   //red     
#define zippyy_switch_pin_3 8   //green     
#define zippyy_switch_pin_4 9   //blue
#define speedButton_pin 10

#define JOYSTICK_UP_PIN       zippyy_switch_pin_4
#define JOYSTICK_DOWN_PIN     zippyy_switch_pin_1
#define JOYSTICK_LEFT_PIN     zippyy_switch_pin_3
#define JOYSTICK_RIGHT_PIN    zippyy_switch_pin_2

#define LED_pinR 11
#define LED_pinG 12
#define LED_pinB 13

int tempDirection = 5;
int Speed = 2;
int Direction;

typedef enum
{
  DIR_INVALID    = 0,
  DIR_FWD_LEFT   = 1,
  DIR_FWD        = 2,
  DIR_FWD_RIGHT  = 3,
  DIR_LEFT       = 4,
  DIR_STOP       = 5,
  DIR_RIGHT      = 6,
  DIR_BACK_LEFT  = 7,
  DIR_BACK       = 8,
  DIR_BACK_RIGHT = 9
} dir_type;

char *drive_string[] =
{
  "INVALID drive!",
  "Drive Forward Left",
  "Drive Forward",
  "Drive Forward Right",
  "Drive Left",
  "Stop Driving",
  "Drive Right",
  "Drive Back Left",
  "Drive Back"
  "Drive Back Right"
};

typedef enum
{
  SPEED_INVALID = 0,
  SPEED_SLOW_MO = 1,
  SPEED_REGULAR = 2,
  SPEED_TURBO   = 3
} speed_type;

static dir_type joystick_dir=DIR_STOP;  // current joystick orientation.

typedef enum
{
  RX_IDLE,
  WAIT_FOR_CMD,
  WAIT_FOR_DATA
} parse_state_type;

typedef struct
{
  dir_type         last_sent_dir;
  bool             last_dir_acked;
  unsigned long    last_sent_dir_time;
  speed_type       last_sent_speed;
  bool             last_speed_acked;
  unsigned long    last_sent_speed_time;
  parse_state_type parse_state;
  
} ack_state_type;

ack_state_type ack_state;

#define ACK_TIMEOUT   1000  // 250 ms for an ack to timeout before resending.

#define ACK_CMD 'A'


void init_ack_state( void )
{
  ack_state.last_sent_dir = DIR_STOP;
  ack_state.last_sent_dir_time = 0;
  ack_state.last_dir_acked = TRUE;

  ack_state.last_sent_speed = SPEED_REGULAR;
  ack_state.last_sent_speed_time = 0;
  ack_state.last_speed_acked = TRUE;

  ack_state.parse_state = RX_IDLE;
}

void setup() 
{  
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

  Serial.println("Joystick setup");
}

void read_joystick_direction(void)
{
  int up;
  int down;
  int left;
  int right;

  // read the joystick switches.  Note these are active low, so we're inverting.
  up = !digitalRead(JOYSTICK_UP_PIN);
  down = !digitalRead(JOYSTICK_DOWN_PIN);
  left = !digitalRead(JOYSTICK_LEFT_PIN);
  right = !digitalRead(JOYSTICK_RIGHT_PIN);

  if (up)
  {
    if (left)       Direction = DIR_FWD_LEFT;
    else if (right) Direction = DIR_FWD_RIGHT;
    else            Direction = DIR_FWD;
  }
  else if (down)
  {
    if (left)       Direction = DIR_BACK_LEFT;
    else if (right) Direction = DIR_BACK_RIGHT;
    else            Direction = DIR_BACK;
  }
  else if (left)    Direction = DIR_LEFT;
  else if (right)   Direction = DIR_RIGHT;
  else              Direction = DIR_STOP;
}

void send_direction(dir_type dir)
{
  char *serial_string;
  
  ack_state.last_sent_dir = dir;
  ack_state.last_dir_acked = FALSE;
  ack_state.last_sent_dir_time = millis();
  ack_state.parse_state = WAIT_FOR_CMD;

  // For some reason, Arduino didn't like:
  //   Serial.println(drive_string[dir]);
  // Some weird macro substitution thing going on maybe?
  serial_string = drive_string[dir];
  Serial.println(serial_string); 
  
  XBee.print(dir);
}

void check_for_resends( void )
{
  unsigned int current_time;

  current_time = millis();

  // do we have an outstanding direction command?
  if (ack_state.last_dir_acked == FALSE)
  {
    // is it time to resend?
    if (current_time > ack_state.last_sent_dir_time + ACK_TIMEOUT)
    {
      Serial.println("RESEND!");
      send_direction(ack_state.last_sent_dir);
    }
  }

  // do we have an outstanding speed command?
  // currently unimplemented...
}

void check_for_acks( void )
{
  char rx_char;

  while (XBee.available())
  {
    rx_char = XBee.read();

    Serial.print("*** MEEP: ");
    Serial.println(rx_char);
    
    switch (ack_state.parse_state)
    {
      case WAIT_FOR_CMD:
        // in this state, we're waiting on the "ack" command.
        if (rx_char == ACK_CMD)
        {
          ack_state.parse_state = WAIT_FOR_DATA;      
        }
        else
        {
          // got something other than an "ack" command, and that's the only 
          // thing currently impelemented.  Flag the unexpected input.
          Serial.print("Unexpected data over XBee whilst waiting for Ack: ");
          Serial.println(rx_char);
        }
      break;

      case WAIT_FOR_DATA:
        // in this state, we're waiting for either the direction or speed character

        // An annoying bit:  the received character is...wait for it...a character.
        // The "last_sent_dir" is an integer.  Need to convert to compare...hence the +'0'
        if (rx_char == ack_state.last_sent_dir + '0')
        {
          ack_state.last_dir_acked = TRUE;
          ack_state.parse_state = RX_IDLE;
        }
        else if (rx_char == ack_state.last_sent_speed)
        {
          ack_state.last_speed_acked = TRUE;
          ack_state.parse_state = RX_IDLE;
        }
        else
        {
           // this ack didn't match either our last sent speed or dir.
           // Flag the unexpected input...and that means we're waiting for a full cmd again.
           Serial.print("Unexpeced ack from MEEP: ");
           Serial.println(rx_char);
           ack_state.parse_state = WAIT_FOR_CMD;
        }
      break;

      case RX_IDLE:
        // In this state, we had no outstanding command to the meep, so we're not
        // expecting anything back.
        Serial.print("Unexpected data whilst idle: ");
        Serial.println(rx_char);
      break;
        
    }
  }
}

void loop() 
{

  //Read ZIPPYY Joystick
  read_joystick_direction();
       
  // Has our joystick direction changed?
  if (Direction != ack_state.last_sent_dir)
  {
    // send direction, and mark the current time.
    send_direction(Direction);
  }

  // Speed not currenly being acked...add soon.
  if (digitalRead(speedButton_pin) == 0)
  {
    speedToggle();
  }

  check_for_acks();
  check_for_resends();

  // this delay is going to act as a temporary debounce...
  delay(100);
  
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
