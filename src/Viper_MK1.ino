/***************************************************
//Web: http://www.buydisplay.com
EastRising Technology Co.,LTD
Examples for ER-OLEDM0.42-1
Display is Hardward I2C 2-Wire I2C Interface 
Tested and worked with:
Works with Arduino 1.6.0 IDE  
Test OK : Arduino DUE,Arduino mega2560,Arduino UNO Board 
****************************************************/
#include <Wire.h>
#include "er_oled.h"
#include <EEPROM.h>
#include "DFPlayer.h"

// Program Modes determined by number of button presses
// Consol LED flashes mode number
// 1 - Night Light On/Off or adjust LED level
//      Quick press - NL On/Off
//      Hold - adjusts LED light levle up and down
// 2 - Disable motion sensor input
//      Enable/Disable motion interrupts
// 3 - Adjust volume level while playing BSG theme song
//        3 quick press - Volume +/- 5, while playing BSG Theme song clip
//        2 quick press and hold on third - adjusts audio level up and down while playing BSG Theme song
// 4 - Sleep mode, minimal light, night light/Display mode
//        4 quick press - Shuts down most systems (enhanced night light mode) except NightLight for base, Helmet 
//                        and Engines reduced to low power, motion sensor is disabled. Repeat mode selection to power back up.
// Display will go in to sleep mode (Blank display) after about 20 seconds of no activity detected. It will wake up once motion
// is detected.
// 
//#define VIPER_DEBUG      /* Enables display data to the Serial port */

// DFPlayer.h settings
// DFP_RX_PIN               12     /* Pin Assigned for serial RX, set to -1 if not used  */
// DFP_TX_PIN                8     /* Pin Assigned for serial TX  */
// DFP_BUSY_PIN              0     /* Pin Assigned for busy detection, set to 0 if not used */

// OLED Panel
// SDA                      18
// SCL                      19

#define NIGHT_LIGHT_LED_OUT 6  /* Night Light LED, ATMeag328 pin xx, D6 (PWM) */
#define PROG_SW_IN          2  /* Program Switch, INT0, D2 */
#define CONSOLE_LED_OUT     13 /* Blink LED, D13 */ 
#define ENGINE_LED_OUT      9  /* Engine LED, ATMeag328 pin xx, D9 (PWM) */ 

#define MOTION_IN           3  /* Motion in, INT1/D3 */
#define WING_LED_OUT        4  /* Wing LED  */
#define FLASH_LED_OUT       5  /* Electrical short LED, Flasher D5 */
#define GUNS_LED_OUT        10 /* Guns LED */

#define PANEL1_LED_OUT      14  /* Cockpit panel 1 back light LED */
#define PANEL2_LED_OUT      15  /* Cockpit panel 2 back light LED */
#define PANEL3_LED_OUT      16  /* Cockpit panel 3 back light LED */
#define PANEL4_LED_OUT      17  /* Cockpit panel 4 back light LED */

#define POWER1_LED_OUT      7   /* Cockpit Helmet light LED */
#define POWER2_LED_OUT      11  /* Cockpit panel LED - Multicolors, solids, blinkers */

#define PROG_MODE_INT       0  /* Program Switch Interrupt Bit */
#define MOTION_INT          1  /* Motion Interrupt Bit */

#define NIGHT_LIGHT_ADDR     0    /* Memory address of night light setting 0=Off or up to 255 power level*/
#define NIGHT_LIGHT_LL_ADDR  2    /* Memory address of night light last level setting, last ON level used */
#define AUDIO_LEVEL_ADDR     4    /* Memory address of audio level setting */
#define PROG_MODE_MAX_DELAY  500  /* Allow up to 500ms to set one-button program mode, resets on each button press */
#define DFP_MAX_VOL          25   /* Maximum volume setting to use, 30 is DFP max but very loud */
#define DFP_INIT_VOL         15   /* Initial volume setting to use */

/*
  == Hardware connection for 4 PIN module==
    OLED   =>    Arduino
  *1. GND    ->    GND
  *2. VCC    ->    3.3V
  *3. SCL    ->    SCL  (Nano A5/D19)
  *4. SDA    ->    SDA  (Nano A4/D18)

 == Hardware connection for 7 PIN module==
 Note:The module needs to be jumpered to an I2C interface.
    OLED   =>    Arduino
  *1. GND    ->    GND
  *2. VCC    ->    3.3V
  *3. SCL    ->    SCL
  *4. SDA    ->    SDA
  *5. RES    ->    8 
  *6. DC     ->    GND 
  *7. CS     ->    GND  
*/

const uint8_t Bogeyx3_0816[16] PROGMEM = //Radar contact x 3
{
    0x00,0x80,0xC0,0xA0,0xC0,0x90,0x18,0x14,0x18,0x90,0xC0,0xA0,0xC0,0x80,0x00,0x00
};

const uint8_t BigBogey_0932[36] PROGMEM = //Radar contact x 1
{
    0x00,0x03,0xC0,0x00,0x00,0x0D,0xB0,0x00,0x00,0x72,0x4E,0x00,0x03,0xB3,0xCD,0xC0,
    0x3D,0x71,0x8E,0xBC,0xCA,0x7B,0xDE,0x53,0x7F,0xCD,0xB3,0xFE,0x00,0x4F,0xF2,0x00,
    0x00,0x31,0x8C,0x00
};


const uint8_t Tracking_0529[20] PROGMEM = //Tracking Message
{
    0xEC, 0x46, 0xAA, 0x98, 0x4A, 0xA8, 0xAA, 0xA0,
    0x4C, 0xE8, 0xCB, 0xB8, 0x4A, 0xA8, 0xAB, 0xA8,
    0x4A, 0xA6, 0xAA, 0xB8
};


const uint8_t Alert_0519[15] PROGMEM = //Alert Message
{
    0x48, 0xEC, 0xE0, 0xA8, 0x8A, 0x40, 0xE8, 0xCC, 
    0x40, 0xA8, 0x8A, 0x40, 0xAE, 0xEA, 0x40
};

/* 2 Byte Font (Size 2)                                                    */
/* Uses 4 bit nibbles to assemble char, Msb of each byte discarded */
/* ASCII 65 - 90, converts to upper case                           */
/* 0xTM, 0xMB: Top and Middle, Next Middle and Bottom              */

const uint8_t ViperFont43_s2[65][2] PROGMEM = {
    {0x00,0x00},/*" ",0*/
    {0x11,0x01},/*"!",1*/
    {0x55,0x00},/*""",2*/
    {0x27,0x72},/*"#",3*/
    {0x23,0x62},/*"$",4*/
    {0x06,0x73},/*"%",5*/
    {0x34,0x37},/*"&",6*/
    {0x12,0x00},/*"'",7*/
    {0x12,0x21},/*"(",8*/
    {0x21,0x12},/*")",9*/
    {0x02,0x00},/*"*",10*/
    {0x02,0x72},/*"+",11*/
    {0x00,0x01},/*",",12*/
    {0x00,0x30},/*"-",13*/
    {0x00,0x01},/*".",14*/
    {0x01,0x24},/*"/",15*/
    {0x25,0x52},/*"0",16*/
    {0x11,0x11},/*"1",17*/
    {0x71,0x27},/*"2",18*/
    {0x73,0x17},/*"3",19*/
    {0x55,0x71},/*"4",20*/
    {0x76,0x16},/*"5",21*/
    {0x34,0x77},/*"6",22*/
    {0x71,0x11},/*"7",23*/
    {0x77,0x77},/*"8",24*/
    {0x75,0x71},/*"9",25*/
    {0x01,0x01},/*":",26*/
    {0x01,0x02},/*";",27*/
    {0x01,0x21},/*"<",28*/
    {0x03,0x30},/*"=",29*/
    {0x02,0x12},/*">",30*/
    {0x25,0x32},/*"?",31*/
    {0x77,0x47},/*"@",32*/
    {0x25,0x75},/*"A",33*/
    {0x77,0x77},/*"B",34*/
    {0x34,0x43},/*"C",35*/
    {0x65,0x56},/*"D",36*/
    {0x76,0x67},/*"E",37*/
    {0x74,0x64},/*"F",38*/
    {0x34,0x73},/*"G",39*/
    {0x66,0x76},/*"H",40*/
    {0x72,0x27},/*"I",41*/
    {0x11,0x16},/*"J",42*/
    {0x56,0x75},/*"K",43*/
    {0x44,0x47},/*"L",44*/
    {0x77,0x55},/*"M",45*/
    {0x57,0x55},/*"N",46*/
    {0x25,0x52},/*"O",47*/
    {0x65,0x64},/*"P",48*/
    {0x25,0x53},/*"Q",49*/
    {0x77,0x65},/*"R",50*/
    {0x34,0x36},/*"S",51*/
    {0x72,0x22},/*"T",52*/
    {0x55,0x52},/*"U",53*/
    {0x55,0x72},/*"V",54*/
    {0x55,0x77},/*"W",55*/
    {0x52,0x25},/*"X",56*/
    {0x52,0x22},/*"Y",57*/
    {0x71,0x27},/*"Z",58*/
    {0x64,0x46},/*"[",59*/
    {0x04,0x21},/*"\",60*/
    {0x31,0x13},/*"]",61*/
    {0x25,0x00},/*"^",62*/
    {0x00,0x07},/*"_",63*/
    {0x42,0x00},/*"`",64*/
}; 

// String constants in flash memory - LCD Display
// Start up table 0-7, Display 1
static const char viperMk1Str[] PROGMEM = "Viper MK-1";
static const char startUpStr[] PROGMEM = "Startup Check";
static const char systemsStr[] PROGMEM = "Systems Powering";
static const char comsStr[] PROGMEM = "COM SYS ONLINE";
static const char vtLocStr[] PROGMEM = "VT location - OK";
static const char batteryStr[] PROGMEM = "BATTERY LVL - OK";
static const char thrustStr[] PROGMEM = "THRUST % - OK";
static const char fuelStr[] PROGMEM = "FUEL % - OK";

// Flight settings table 0-6, Display 2
static const char fltPointStr[] PROGMEM = "FLT POINTING L206 00.1";
static const char sunSStr[] PROGMEM = "SUN SOUTH    .434 327.5";
static const char sunNStr[] PROGMEM = "SUN NORTH    6.00 286.";
static const char comLvlStr[] PROGMEM = "COM LVL      2.00% 300.";
static const char vfVectStr[] PROGMEM = "VF VECT      .476";
static const char syncRetStr[] PROGMEM = "SYNC RET     .837 130.";
static const char startPriStr[] PROGMEM = "Start PRI SYS CHK";

// Flight bar graph table 0-3, Display 3
static const char priSysStatStr[] PROGMEM = "Pri Sys Stat Chk:";
static const char filterCtrlStr[] PROGMEM = "Filter Control OYD";
static const char oxySupplyStr[] PROGMEM = "Oxygen Supply OYD";
static const char fuelFlowStr[] PROGMEM = "Fuel Flow Rate";


// Initialize Table of Strings
const char* const startStrTbl[] PROGMEM = { viperMk1Str, startUpStr, systemsStr, comsStr, vtLocStr, batteryStr, thrustStr, fuelStr};
const char* const flightStrTbl[] PROGMEM = { fltPointStr, sunSStr, sunNStr, comLvlStr, vfVectStr, syncRetStr, startPriStr};
const char* const bargraphStrTbl[] PROGMEM = { priSysStatStr, filterCtrlStr, oxySupplyStr, fuelFlowStr};

char strBuffer[24]; // buffer for reading the string to (needs to be large enough to take the longest string


uint8_t oled_buf[SSD1306_WIDTH * SSD1306_HEIGHT_ER / 8];


// Variables:
byte nightLightLev;                  // Night Light LED level
byte nightLightLastOnLev;            // Last Night Light LED level Set
int nightLightInc = 5;               // Determine increment up/down
byte dfpVolumeLev =  DFP_INIT_VOL;   // Current or initial DFP volume level
int dfpVolumeInc = 1;                // Determine increment up/down

// Clean up program button state
//int progButtonState = 0;           // Track state of the night light button
volatile bool motionState = false;
volatile int progMode = 0;     
volatile unsigned long progTimer;    // Program button timer
unsigned long wingTimer;             // set to trigger every second 
unsigned long flashTimer;            // Set to trigger every 1 to 3 seconds, random values
unsigned long displayTimer;          // Set to trigger every 2 seconda
int displayTimerCount = 0;           // Count display timer cycles and sleep display after 10 cycles
int shortCircuitEnDelay = 2000;      // Initial random delay value

bool motionSenEn = true;             // Motion sensor enabled/disabled
int motionPhase = 0;                 // tracks what phase motion sequence is in

bool sleepEn = false;                // Tracks sleep mode, enhanced night light mode
int dfpQueryVal;                     // General result from DFP

SoftwareSerial dfpSerial (DFP_RX_PIN, DFP_TX_PIN); // RX, TX

/* Hardware Motion Input interrupt (ISR) */
void motionDetect() {
    motionState = true;

    // Disable motion sensor until sequence completed
    EIMSK &= ~(1 << MOTION_INT);
}

/* Hardware Program Button Input interrupt (ISR) */
void checkProgState (void) {
    if (progMode == 0) {
       // Disable motion sensor while program button is used
       EIMSK &= ~(1 << MOTION_INT);
    }

    // count the pushbutton input pin
    progMode++;
    progTimer = millis();
}

// Print associated char at location specified
void viper_oled_char(unsigned char x, unsigned char y, char acsii, char size, char mode, uint8_t* buffer)
{
    unsigned char i, j, chBit, y0=y, x0=x;
    char temp;
    unsigned char ch = acsii - ' ';

    for (i = 0;i<size;i++) {
        /* Size is 2 */
        if (ch > 64) {
            ch = ch - 32;
        }
        temp = pgm_read_byte(&ViperFont43_s2[ch][i]);

        for (j =0;j<8;j++) {
            if ((j==0) || (j==4)) {
                temp <<= 1;
                if (j==4) {
                    x = x0;
                    y++;  /* Advance to next row */
                }
                continue;    /* Skip unused bits */
            }

            chBit = temp & 0x80? 1 : 0; 
            er_oled_pixel(x, y, chBit, buffer);
            temp <<= 1;
            x++;               
        }
        y++;
        x = x0;
    }

    y = y0;
    x = x0 + 3;
} // end viper_oled_char

// Print associated string at location specified
void viper_oled_string(uint8_t x, uint8_t y, const char *pString, uint8_t Size, uint8_t Mode, uint8_t* buffer)
{
    while (*pString != '\0') {
        if ((x+3) > SSD1306_WIDTH) return;
        if ((y+4) > SSD1306_HEIGHT_ER) return;

        viper_oled_char(x, y, *pString, Size, Mode, buffer);
        x += 4;
        pString++;
    }
} // end viper_oled_string

// Draw basic patterns for Viper display
void viper_display_base(uint8_t* buffer, bool render) 
{
    er_oled_Line(buffer, 4, 4, 68, 36, 1, 0); /* Diag Line */
    er_oled_Line(buffer, 68, 4, 4, 36, 1, 0); /* Diag Line */
    er_oled_Line(buffer, 36, 2, 36, 38, 1, 0); /* Vert Y Line */
    er_oled_Line(buffer, 2, 21, 70, 21, 1, 0); /* Horiz X Line */
    er_oled_Circle(buffer, 36, 20, 8, 1, render);
}

// Draw out the scanner display console 
// If scanner is true, cycles through ellipse patterns for a scanning screen
void viper_scanner_display(uint8_t* buffer, bool scanner)
{
    er_oled_clear(buffer);
    viper_display_base(buffer, 0);

    /*    buffer to use  x & y centers  x & y radius  Color:0/1  Render:0/1 */
    er_oled_Ellipse(buffer, 36, 20, 35, 18, 1, 0);
    er_oled_Ellipse(buffer, 36, 20, 35, 12, 1, 1);  /* Render image */

    /* Provide forward and backward ellipse scans */
    if (scanner) {
        for (int i = 3; (i < 20) && !motionState; i= i +3) {
            er_oled_Ellipse(buffer, 36, 20, 35, i, 1, 1);

            viperDelay (300);
    
            if ((i != 12) && (i != 18)) {
                er_oled_Ellipse(buffer, 36, 20, 35, i, 0, 0);
            }
    
            viper_display_base(buffer, 1);
        }

        viper_display_base(buffer, 0);
    
        /*      buffer to use  x & y centers  x & y radius  Color:0/1  Render:0/1 */
        er_oled_Ellipse(buffer, 36, 20, 35, 18, 1, 0);
        er_oled_Ellipse(buffer, 36, 20, 35, 12, 1, 1);
    
        for (int i = 18; (i > 3) && !motionState; i= i - 3) {
            er_oled_Ellipse(buffer, 36, 20, 35, i, 1, 1);
            viperDelay (300);
            if ((i != 12) && (i != 18)) {
                er_oled_Ellipse(buffer, 36, 20, 35, i, 0, 0);
            }
            viper_display_base(buffer, 1);
        }
    
        viper_display_base(buffer, 0);
    
        /*      buffer to use  x & y centers  x & y radius  Color:0/1  Render:0/1 */
        er_oled_Ellipse(buffer, 36, 20, 35, 18, 1, 0);
        er_oled_Ellipse(buffer, 36, 20, 35, 12, 1, 1);
    } /* end if scanner */
} // end viper_scanner_display

/* Display BOGEY Alert to console */
void viper_bogey_alert_display(uint8_t* buffer)
{
    /* Display tracking Alert */
    er_oled_clear(buffer);
    er_oled_bitmap(8, 17, Tracking_0529, 29, 5, buffer); 
    er_oled_bitmap(47, 17, Alert_0519, 19, 5, buffer); 
    er_oled_display(buffer);

    er_oled_invert(TRUE);
    viperDelay(300);
    er_oled_invert(FALSE);
    viperDelay(300);
    er_oled_invert(TRUE);
    viperDelay(300);
    er_oled_invert(FALSE);
    viperDelay(500);
} // end viper_bogey_alert_display

/* Display BOGEY tracker */
void viper_bogey_tracker_display(uint8_t* buffer)
{
    /* Clear display and show bogey's */
    er_oled_clear(buffer);
    viper_scanner_display(buffer, 0);

    /* Clear display and show bogey's */
    er_oled_bitmap(20, 9, Bogeyx3_0816, 8, 16, buffer);   
    er_oled_display(buffer);

    viperDelay(400);

    er_oled_bitmap(15, 22, Bogeyx3_0816, 8, 16, buffer); 
    er_oled_display(buffer);

    viperDelay(400);

    er_oled_bitmap(5, 5, Bogeyx3_0816, 8, 16, buffer); 
    er_oled_display(buffer);


    viperDelay(400);
    er_oled_bitmap(0, 24, Bogeyx3_0816, 8, 16, buffer); 
    er_oled_display(buffer);

    viperDelay(500);
} // end viper_bogey_tracker_display

/* Display Cylon ship */
void viper_cylon_display(uint8_t* buffer)
{
    /* Clear display and show Cylon ship */
    er_oled_clear(buffer);

    viper_scanner_display(buffer, 0);
    // Move cylon ship around on screen
    er_oled_bitmap(20, 25, BigBogey_0932, 32, 9, buffer); 
    er_oled_display(buffer);

    viperDelay (300);

    er_oled_clear(buffer); 

    viper_scanner_display(buffer, 0);
    er_oled_bitmap(30, 22, BigBogey_0932, 32, 9, buffer); 
    er_oled_display(buffer);

    viperDelay (500);

    er_oled_clear(buffer); 

    viper_scanner_display(buffer, 0);
    er_oled_bitmap(15, 22, BigBogey_0932, 32, 9, buffer); 
    er_oled_display(buffer);

    viperDelay (300);
    er_oled_clear(buffer); 

    viper_scanner_display(buffer, 0);

    er_oled_bitmap(20, 18, BigBogey_0932, 32, 9, buffer); 

    er_oled_display(buffer);

  //  viperDelay (800);
} // end viper_cylon_display


/* Display Cylon ship explosion */
void viper_explosion_display(uint8_t* buffer)
{
    /* Explosion of Cylon ship */
    int16_t d, i, r;
    int16_t xc = 36;
    int16_t yc = 20;
    int16_t rm = 3;
    int16_t x;
    int16_t y;
    int16_t yt;
    int16_t ry_mx = 12;
    int16_t rx_mx = 25;

    er_oled_Circle(buffer, 36, 21, 2, 1, 0);
    er_oled_Circle(buffer, 36, 21, 3, 1, 0);
    er_oled_Circle(buffer, 36, 21, 4, 1, 0);

    er_oled_display(buffer);

    for (d = 0; d < 10; d++) {

      if (d==7) {
         ry_mx = 20;
         rx_mx = 36;
      }

      for (i = 0; i < 3; i++) {
         // x = random( 1, rx_mx);
          x = random( 1, rx_mx);
          y = random( 1, ry_mx);

          er_oled_pixel(xc + x, yc + y, 1, buffer);
          er_oled_pixel(xc - x, yc + y, 1, buffer);
          er_oled_pixel(xc + x, yc - y, 1, buffer);
          er_oled_pixel(xc - x, yc - y, 1, buffer);

          yt = yc + x;
          if (yt > SSD1306_HEIGHT) {
              yt = SSD1306_HEIGHT - random( rm,19);
          }

          er_oled_pixel(xc + y, yt, 1, buffer);
          er_oled_pixel(xc - y, yt, 1, buffer);

          yt = yc - x;
          if (yt < 0) {
              yt = random( rm,19);
          }

          er_oled_pixel(xc + y, yt, 1, buffer);
          er_oled_pixel(xc - y, yt, 1, buffer);

          er_oled_display(buffer);
      } /* end for 3 */

      er_oled_Circle(buffer, 36, 21, d + 5, 1, 0);
      er_oled_display(buffer);
      rm++;

      viperDelay(1);
  } /* end for 10 */

    delay (150);
} // end viper_explosion_display


/* Display Viper Startup sequence */
void viper_startup_seq(uint8_t* buffer)
{
    int yOff = 0;
    int scroll = 0;
    int scrollRows = 5;

    er_oled_clear(buffer); 
    /* Start Viper Console Display Data, Screen 1 */
    for (int i = 0; i <= 7; i++) {
        strcpy_P(strBuffer, (char*)pgm_read_dword(&(startStrTbl[i])));
        viper_oled_string(0, yOff, strBuffer, 2, 1, buffer);
        er_oled_display(buffer);

        yOff += 5;
    }

    /* Start cockpit console light up sequence */
    multi_blink_led (PANEL1_LED_OUT, 2, 100);
    digitalWrite(PANEL1_LED_OUT, HIGH); 

    delay (200);

    /* Start Viper Console Display Data, Screen 2 */
    for (int i = 0; i <= 6; i++) {
        strcpy_P(strBuffer, (char*)pgm_read_dword(&(flightStrTbl[i])));
        viper_oled_string(0, yOff, strBuffer, 2, 1, buffer);

        yOff += 5;

        if (i == 2) {
            yOff = 0;
        }

        if ((i == 2) || (i == 4) || (i == 6)) {
            er_oled_display(buffer); 
        }

        if ((i == 2) || (i == 3) || (i == 4) || (i == 5)) {
            scroll = er_oled_vertscroll(scroll, scrollRows); 
        }
    }

    multi_blink_led (PANEL2_LED_OUT, 2, 100);
    digitalWrite(PANEL2_LED_OUT, HIGH); 

    delay (200);

    /* Finish scrolling text */
    for (int i = 0; i < 5; ++i) {
        scroll = er_oled_vertscroll(scroll, scrollRows);
    }


    multi_blink_led (PANEL3_LED_OUT, 2, 100);
    digitalWrite(PANEL3_LED_OUT, HIGH); 

    delay(400);

    multi_blink_led (PANEL4_LED_OUT, 2, 100);
    digitalWrite(PANEL4_LED_OUT, HIGH); 
    delay (100);

    /* Light up remaining console LEDs */
    digitalWrite(POWER2_LED_OUT, HIGH); 

    /* Start Viper Console Display Bar Graph, Screen 3 */
    er_oled_clear(buffer);
    command(SSD1306_SETSTARTLINE); /* Reset startline */

    strcpy_P(strBuffer, (char*)pgm_read_dword(&(bargraphStrTbl[0])));
    viper_oled_string(0, 0, strBuffer, 2, 1, buffer);
    er_oled_display(buffer);

    strcpy_P(strBuffer, (char*)pgm_read_dword(&(bargraphStrTbl[1])));
    viper_oled_string(0, 7, strBuffer, 2, 1, buffer);
    er_oled_display(buffer);
    /* x1, y1, x2, y2, color,  divider,  fill */
    er_oled_Rectangle(buffer, 10, 12, 70, 15, 1, 1, 1);

    /* Prestart engines */
    analogWrite(ENGINE_LED_OUT, 2);
    delay(100);

    strcpy_P(strBuffer, (char*)pgm_read_dword(&(bargraphStrTbl[2])));
    viper_oled_string(0, 19, strBuffer, 2, 1, buffer);
    er_oled_display(buffer);
    /* x1, y1, x2, y2, color,  divider,  fill */
    er_oled_Rectangle(buffer, 10, 24, 70, 27, 1, 1, 1);

    /* Prestart engines */
    analogWrite(ENGINE_LED_OUT, 7);

    strcpy_P(strBuffer, (char*)pgm_read_dword(&(bargraphStrTbl[3])));
    viper_oled_string(0, 31, strBuffer, 2, 1, buffer);
    er_oled_display(buffer);
    /* x1, y1, x2, y2, color,  divider,  fill */
    er_oled_Rectangle(buffer, 10, 36, 70, 39, 1, 1, 1);

    /* Begin bar graph fills */
    /* Pass 1 */
    er_oled_bar_fill(buffer, 10, 12, 18, 15, 1, 1); /* Filter */ 
    er_oled_bar_fill(buffer, 10, 24, 27, 27, 1, 1); /* Oxygen */
    er_oled_bar_fill(buffer, 10, 36, 15, 39, 1, 1); /* Fuel */

    /* Engines warm up */
    for (int i = 10; i < 50; i++) {
        i = i + random(5, 20);
        analogWrite(ENGINE_LED_OUT, i);
        delay(200);  
    }

    /* Pass 2 */
    er_oled_bar_fill(buffer, 18, 12, 42, 15, 1, 1); /* Filter */ 
    er_oled_bar_fill(buffer, 27, 24, 37, 27, 1, 1); /* Oxygen */
    er_oled_bar_fill(buffer, 15, 36, 55, 39, 1, 1); /* Fuel */

    delay(200);

    /* Pass 3 */
    er_oled_bar_fill(buffer, 42, 12, 58, 15, 1, 1); /* Filter */ 
    er_oled_bar_fill(buffer, 37, 24, 63, 27, 1, 1); /* Oxygen */ 
    er_oled_bar_fill(buffer, 55, 36, 60, 39, 1, 1); /* Fuel */

    delay(200);

    /* Pass 4 */
    er_oled_bar_fill(buffer, 58, 12, 70, 15, 1, 1); /* Filter */ 
    er_oled_bar_fill(buffer, 63, 24, 70, 27, 1, 1); /* Oxygen */
    er_oled_bar_fill(buffer, 60, 36, 70, 39, 1, 1); /* Fuel */

    delay(1000);

    /* Clear display and show tracking screen */
    viper_scanner_display(buffer, 0);

    /* Continue engines */ 
    for (int i = 50; i < 200; i++) {
        i = i + random(5, 20);
        if (i > 230) {
            i = 230;
        }
        analogWrite(ENGINE_LED_OUT, i);
        delay(400);  
    }

    analogWrite(ENGINE_LED_OUT, 255);  /* Engine thrust */
    delay(3700);  

    analogWrite(ENGINE_LED_OUT, 220);  /* Engine nominal */
} // end Viper startup seq

/* Blink LED number of times and what delay between blinks */
void multi_blink_led (int ledPin, int cycles, int dVal) {
    for (int c = 0 ; c < cycles; c++) {
        digitalWrite(ledPin, HIGH); 
        delay(dVal);
        digitalWrite(ledPin, LOW); 
        if (c < (cycles - 1)) {
          delay(dVal);
        }
    }
} // end multi_blink_led


/* Activate Guns - LED and Sound */
void viperGuns () {
    dfpPlayTrackMP3(6);  /* 1.1 sec track  */
    delay (400);
    multi_blink_led (GUNS_LED_OUT, 3, 50);
} // end viperGuns


/* Varies flash period and PMW level of pin */
void flash_led (int ledPin, int flashes) {
    int llev;
    for (int c = 0 ; c < flashes; c++) {
        llev = random(5, 255);
        if (llev < 10) {llev = 10;}
        analogWrite(ledPin, llev);
        delay(random(20, 150));
    }

    analogWrite(ledPin, 0);
} // end flicker_led

/* Check program button mode and handle option */
void checkProgButton () {
    if (progMode == 0) {return;}

    // Check if program mode delay has expired
    if (millis() - progTimer > PROG_MODE_MAX_DELAY) {
      progTimer = millis();

#ifdef VIPER_DEBUG    
      Serial.println("Program Timer Elapsed"); // Debug
#endif

    } else {
        return;
    }

    multi_blink_led (CONSOLE_LED_OUT, progMode, 350);

#ifdef VIPER_DEBUG    
    Serial.print("Program Mode (CPB): "); // Debug
    Serial.println(progMode); // Debug
#endif

    // read the program pushbutton input pin:
    //progButtonState = digitalRead(PROG_SW_IN);
    // Check if NL button is pressed
    if (digitalRead(PROG_SW_IN)) {
        // Disable prog sw int while setting LED level
        EIMSK &= ~(1 << PROG_MODE_INT);

#ifdef VIPER_DEBUG    
        Serial.print("Program Button Press: "); // Debug
        Serial.println(digitalRead(PROG_SW_IN)); // Debug
     //   Serial.println(progButtonState); // Debug
#endif

        // Delay a little bit to check for holding
        delay(100);
    //    progButtonState = digitalRead(PROG_SW_IN);

        if (digitalRead(PROG_SW_IN)) {
            if (progMode == 3) {
                dfpPlayTrackMP3(1);
            }
          
            while (digitalRead(PROG_SW_IN)) {
                if (progMode == 1) {
                    if (nightLightLev == 255) {
                        nightLightInc = -5;
                    } else if (nightLightLev == 0) {
                        nightLightInc = 5;
                    }

                    nightLightLev = nightLightLev + nightLightInc;
                    analogWrite(NIGHT_LIGHT_LED_OUT, nightLightLev);
                } else if (progMode == 3) {
                    if (dfpVolumeLev == DFP_MAX_VOL) {
                        dfpVolumeInc = -1;
                    } else if (dfpVolumeLev == 0) {
                        dfpVolumeInc = 1;
                    }

                    dfpVolumeLev = dfpVolumeLev + dfpVolumeInc;
                    dfpSetVolume(dfpVolumeLev);

#ifdef VIPER_DEBUG
            Serial.print("DFP Volume 3h: "); // Debug
            Serial.println(dfpVolumeLev); // Debug
#endif
                }

                delay(100);
                // progButtonState = digitalRead(PROG_SW_IN);
            } /* end while */

            if (progMode == 1) {
                nightLightLastOnLev = nightLightLev;
                EEPROM.write(NIGHT_LIGHT_ADDR, nightLightLev);
                EEPROM.write(NIGHT_LIGHT_LL_ADDR, nightLightLastOnLev);
            } else if (progMode == 3) {
                EEPROM.write(AUDIO_LEVEL_ADDR, dfpVolumeLev);
                dfpQueryVal = dfpReadQuery(0);

#ifdef VIPER_DEBUG
            Serial.println("DFP Volume EPROM update"); // Debug
            Serial.print(F("Query Params PS:"));
            Serial.println(dfpQueryVal, HEX);
#endif
            }
      } // end if progButtonState

        // Enable prog sw
        EIMSK |= (1 << PROG_MODE_INT);
    } else {
        switch (progMode) {
            case 1:
                // One press, toggle light
                if (nightLightLev > 0) {
                    nightLightLev = 0;
                } else {
                    nightLightLev = nightLightLastOnLev;
                }

                analogWrite(NIGHT_LIGHT_LED_OUT, nightLightLev);
                EEPROM.write(NIGHT_LIGHT_ADDR, nightLightLev);
                break;
            case 2:
                motionSenEn = (motionSenEn == LOW)? HIGH : LOW; //toggle state

                if (motionSenEn) {
                    if (sleepEn) {
                        /* Wake up system, disable sleep and Re-start display */
                        viper_startup_seq(oled_buf);
                        sleepEn = LOW;
                    }
                } else {
                    // Disable motion sensor
                    EIMSK &= ~(1 << MOTION_INT);
                }

                // Play sound effect, guns for enabled/disable
                viperGuns();
                break;
            case 3:
                if (dfpVolumeLev == DFP_MAX_VOL) {
                    dfpVolumeInc = -1;
                } else if (dfpVolumeLev == 0) {
                   dfpVolumeInc = 1;
                }

                dfpVolumeLev = dfpVolumeLev + (dfpVolumeInc * 5);

                if (dfpVolumeLev > DFP_MAX_VOL) {
                    dfpVolumeLev = DFP_MAX_VOL;
                } else if (dfpVolumeLev < 0) {
                   dfpVolumeLev = 0;
                }

                dfpSetVolume(dfpVolumeLev);
                EEPROM.write(AUDIO_LEVEL_ADDR, dfpVolumeLev);

                dfpQueryVal = dfpGetStatus() & 0xFF;
                // If nothing playing, play or play again.
                if (dfpQueryVal != 1) {
                    dfpPlayTrackMP3(3);
                }

#ifdef VIPER_DEBUG 
                Serial.print("DFP Status: "); // Debug
                Serial.println(dfpQueryVal); // Debug
                Serial.print("DFP Volume3: "); // Debug
                Serial.println(dfpVolumeLev); // Debug
                Serial.println("DFP Volume EPROM update"); // Debug
#endif

                break;
            case 4:
                sleepEn = (sleepEn == LOW)? HIGH : LOW; //toggle state

                if (sleepEn) {
                    motionSenEn = LOW; // Disable motion
                    motionState = false;
                    // Disable motion sensor
                    EIMSK &= ~(1 << MOTION_INT);

                    /* Engines */
                    analogWrite(ENGINE_LED_OUT, 25);

                    /* cockpit console light shut down */
                    digitalWrite(PANEL4_LED_OUT, LOW); 
                    delay (200);
                    digitalWrite(PANEL3_LED_OUT, LOW); 
                    delay (200);
                    digitalWrite(PANEL2_LED_OUT, LOW); 
                    delay (200);
                    digitalWrite(PANEL1_LED_OUT, LOW);
                    delay (200);
                    digitalWrite(POWER2_LED_OUT, LOW); 
                } else {
                    /* Re-start display */
                    viper_startup_seq(oled_buf);
                    motionSenEn = HIGH; // Enable motion
                }
                break;
            default:
                break;
        } /* end switch */
    } /* end if-else progButtonState */

    if (motionSenEn) {
        // Enable motion sensor
        EIMSK |= (1 << MOTION_INT);
        displayTimerCount = 0;  /* Reset and wake up display */
    } 

#ifdef VIPER_DEBUG    
    Serial.print("Motion En (CPB): "); // Debug
    Serial.println(motionSenEn); // Debug
    Serial.print("Motion State (CPB): "); // Debug
    Serial.println(motionState); // Debug

    Serial.print("EIMSK Reg- PI: "); // Debug
    Serial.println(EIMSK, HEX);  // Debug
#endif

    progMode = 0;
} // end checkProgButton

void setup() {
#ifdef VIPER_DEBUG    
    Serial.begin(57600);  /* For debug */
#endif

    nightLightLev = EEPROM.read(NIGHT_LIGHT_ADDR);
    nightLightLastOnLev = EEPROM.read(NIGHT_LIGHT_LL_ADDR);

    dfpVolumeLev = EEPROM.read(AUDIO_LEVEL_ADDR);
    if (dfpVolumeLev > DFP_MAX_VOL) {
        dfpVolumeLev = DFP_INIT_VOL;   // Likely first run, set to a reasonable value
    }

    // Engine PWM Freq to slowest setting
    TCCR1B = TCCR1B & B11111000 | B00000101;  // Set to 0x5, 1024 divisor

    // LED's
    pinMode(CONSOLE_LED_OUT, OUTPUT); 
    pinMode(WING_LED_OUT, OUTPUT); 
    pinMode(FLASH_LED_OUT, OUTPUT); 
    pinMode(NIGHT_LIGHT_LED_OUT, OUTPUT);
    pinMode(ENGINE_LED_OUT, OUTPUT);
    pinMode(GUNS_LED_OUT, OUTPUT);

    pinMode(PANEL1_LED_OUT, OUTPUT);   // Panel LED start up seq power
    pinMode(PANEL2_LED_OUT, OUTPUT);
    pinMode(PANEL3_LED_OUT, OUTPUT);
    pinMode(PANEL4_LED_OUT, OUTPUT);

    pinMode(POWER1_LED_OUT, OUTPUT);   // Helmet LED power
    pinMode(POWER2_LED_OUT, OUTPUT);   // Other console LED power
    
    // Inputs
    // initialize the night light button pin as a input:
    pinMode(PROG_SW_IN, INPUT);
    pinMode(MOTION_IN, INPUT_PULLUP);

    /* Start audio */
    dfpSerial.begin (9600);
    dfpSerial.listen();
    dfpSetup(); /* Setup Busy imput pin is used */

    dfpSetVolume(dfpVolumeLev);
    dfpSetEq(DFP_EQ_CLASSIC);

#ifdef VIPER_DEBUG    
    Serial.print(F("Params Vol:"));
    Serial.println(dfpGetVolume(), HEX);
#endif

    // Begin start up sequence

    // Set Night Light
    analogWrite(NIGHT_LIGHT_LED_OUT, nightLightLev);

    // Set startup counter LED
    for (int l=0; l <= 10; l++) {
        multi_blink_led (CONSOLE_LED_OUT, 2, 100);
        delay(250);            
    }

    /* Setup display */
    Wire.begin();
    Wire.setClock(400000);
    // Deactivate internal pullups for twi.
    //digitalWrite(SDA, 0);
    //digitalWrite(SCL, 0);
    er_oled_begin();
    er_oled_clear(oled_buf);
    er_oled_display(oled_buf);

    // Light up helmet
    digitalWrite(POWER1_LED_OUT, HIGH); 

    delay (100);

    // Start seq audio track
    dfpPlayTrackMP3(2);

    /* display an image of bitmap matrix */
    viper_startup_seq(oled_buf);

#ifdef VIPER_DEBUG   
  //  int startTime = millis();

  //  int endTime = millis();

  //  Serial.print("Display startup seq time:"); // Debug
  //  Serial.println(endTime - startTime); // Debug
#endif

    dfpQueryVal = dfpGetStatus() & 0xFF;

    displayTimer = millis();           
    while (dfpQueryVal == 1) {
        dfpQueryVal = dfpGetStatus() & 0xFF;

        // scan display
        if (millis() - displayTimer > 2000) {
            /* show scanning display */
            viper_scanner_display(oled_buf, 1);
            displayTimer = millis();
        }
    }

    delay (1500);

    multi_blink_led (CONSOLE_LED_OUT, 2, 50);

    attachInterrupt(digitalPinToInterrupt(PROG_SW_IN),checkProgState, RISING); 
    attachInterrupt(digitalPinToInterrupt(MOTION_IN),motionDetect, RISING); 

    // Initialize timer counters
    wingTimer = millis();            
    flashTimer = millis();           
    displayTimer = millis();           

#ifdef VIPER_DEBUG    
    Serial.print("EIMSK Reg- PI: "); // Debug
    Serial.println(EIMSK, HEX);  // Debug

    Serial.print("TCCR1B Reg: "); // Debug
    Serial.println(TCCR1B, HEX);  // Debug

    Serial.print("TCCR2B Reg: "); // Debug
    Serial.println(TCCR2B, HEX);  // Debug
#endif

}

void loop() {
    if (motionState) {
      motionPhase++;
      if (motionPhase == 1) {
          // Start seq audio track
          dfpPlayTrackMP3(4);
          multi_blink_led (CONSOLE_LED_OUT, 4, 100);
    
          /* Display tracking Alert */
          viper_bogey_alert_display(oled_buf);
      }

      if (motionPhase == 2) {
          /* Bogey detected */
          viper_bogey_tracker_display(oled_buf);
      }

      if (motionPhase == 3) {
          /* Cylon ship in sites */
          viper_cylon_display(oled_buf);

          while ((dfpGetStatus() & 0xFF) == 1) {
              viperDelay(50);
          }

          viperGuns();
      }

      if (motionPhase == 4) {
          /* Cylon ship in sites */
          viper_cylon_display(oled_buf);

          viperGuns(); 
          viperDelay(800); /* About 700 ms delay needed, till next track can start */

          dfpPlayTrackMP3(5);
          /* Cylon ship explosion */
          viper_explosion_display(oled_buf);
      }

      if (motionPhase == 5) {
          motionState = false;
          motionPhase = 0;

          /* Back to scanning */
          viper_scanner_display(oled_buf, 0);
          viperDelay(2000);
          viper_scanner_display(oled_buf, 1);
          viperDelay(1000);

          // re-enable interrupts
          if (motionSenEn) {
              // Enable motion sensor
              EIMSK |= (1 << MOTION_INT);
          }
      }

      displayTimer = millis();   /* Keep display timer updated and inactive */   
      displayTimerCount = 0;
    } else {
        // Check program button
        checkProgButton ();
        // Wing tip LED, flash every second
        checkWingTimer ();
        // Electrical damage LED, random flashes 1-3 seconds
        checkFlashTimer ();
        // Display scanner sweep, 2 seconds
        checkDisplayTimer ();
    } /* end if-else motion detected */
}

/* Check timers */
void checkWingTimer () {
    // Wing tip LED, every second flashes
    if (millis() - wingTimer > 1000) {
        multi_blink_led (WING_LED_OUT, 2, 100);
        wingTimer = millis();            
    }
} /* end checkWingTimer */

void checkFlashTimer () {
    // Electrical damage LED, random flashes
    if (millis() - flashTimer > shortCircuitEnDelay) {
        flash_led (FLASH_LED_OUT, random(1, 6));
        shortCircuitEnDelay = random(1000, 3000);
        flashTimer = millis();           
    }
} /* end checkFlashTimer */

void checkDisplayTimer () {
    if (displayTimerCount > 10) {
        return;
    }

    // If nothing else happening scan display
    if (millis() - displayTimer > 2000) {
        if ((displayTimerCount == 10) || sleepEn) {
            /* Clear display and blank for now - sleep mode */
            er_oled_clear(oled_buf);
            er_oled_display(oled_buf);
            displayTimerCount = 10; /* Covers sleepEn case to put display to sleep */
        } else {
            /* show scanning display */
            viper_scanner_display(oled_buf, 1);
            displayTimer = millis();
        }

        displayTimerCount++;
    }
} /* end checkFlashTimer */

/* Custom delay to keep all timers active and execute timed activities */
void viperDelay(int delay) {
    unsigned long viperTimer = millis();

    while (millis() - viperTimer < delay) {
        // Check program button
        checkProgButton ();
        // Wing tip LED, every second flashes
        checkWingTimer ();
        // Electrical damage LED, random flashes
        checkFlashTimer ();

        displayTimer = millis();  /* don't trigger until in regular loop */         
    } /* end while */
} /* end viperDelay */

