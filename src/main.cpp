#include <Arduino.h>
#include <Bounce.h>
#include <MIDI.h>
#include <Encoder.h>
//#include <MIDIcontroller.h>
#include <LedControl.h>
// #include <usb_midi.h>

/* pin descriptions
0/Rx1     MIDI_In
5/Tx1     MIDI_Out
7         SPI_MOSI
14        SPI_SCK
31        MAX_CS
23        R1
6         C1
8         C2
12        C3
26        C4
27        C5
28        C6
30/A20    FSR
 */

/* Define Keyboard Matrix */
#define R1 23
#define C1 6
#define C2 8
#define C3 12
#define C4 26
#define C5 27
#define C6 28

/* Define Debounce for KB */
#define KEY_HELD_DELAY 10 // bounce counts
#define BOUNCE_TIME 5 // milliseconds

/* INITIALIZE HW MIDI */
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
const int MIDI_NOTES[4][6] = {{0, 0, 60, 62, 64, 65},
                              {0, 0, 67, 69, 71, 72},
                              {0, 0, 74, 76, 77, 79},
                              {0, 0, 81, 83, 84, 86}}; // first 2 keys are modifiers
//const int MIDI_CCS[2][6] = {{0, 0, 60, 62, 64, 65}, {0, 0, 67, 69, 71, 72}};
const int MODIFIER_VALS[6] = {1,2,0,0,0,0};
int mod = 0;

/* INITIALIZE KEYPAD */
const byte ROWS = 1; //1 rows
const byte COLS = 6; //6 columns
const char KEYS[ROWS][COLS] = {{'1', '2', '3', '4', '5', '6'}};
const byte ROW_PINS[ROWS] = {R1};                   //connect to the row pinouts of the keypad
const byte COL_PINS[COLS] = {C1,C2,C3,C4,C5,C6}; //connect to the column pinouts of the keypad
bool keyPressed[6] = {false};
int keyPrev[6] = {0};

/* INTIALIZE ARDUINO LED */
static const unsigned LED_PIN = 13; // LED pin on Arduino Uno

/* INITIALIZE MAX7221 LED CONTROLLER (DIN_pin, CLK_pin, LOAD_pin, number_of_chips) */
LedControl backlight = LedControl(7, 14, 31, 1);
const int LEDS[6] = {0, 4, 2, 3, 5, 1}; // LED Order 0,4,2,3,5,1

void keyScan() /* Scan for pressed KEYS */
{
        for (int i = 0; i < COLS; i++)
        {
                digitalWrite(COL_PINS[i], LOW); // set all COL_PINS low
        }

        for (int i = 0; i < COLS; i++) //KEYScan
        {
                digitalWrite(COL_PINS[i], HIGH); // Set colPin we are looking for high
                if (i > 0) digitalWrite(COL_PINS[i - 1], LOW); // Set previous colPin back low
                delay(BOUNCE_TIME); // Debounce: wait for electrons to settle
                if (digitalRead(R1))    keyPressed[i] = true; // if R1, current col pressed
                else keyPressed[i] = false;
        }
}

void ledTest()  /* Fancy LED tricks */
{
        for(int i = 0; i < 6; i++){
                backlight.setLed(0,LEDS[i],1,true);
                delay(25);
        }
        for (int i = 1; i < 16; i++)
        {
                backlight.setIntensity(0, i);
                delay(25);
        }
        for (int i = 1; i < 16; i++)
        {
                backlight.setIntensity(0, 16 - i);
                delay(25);
        }
        for(int i = 0; i < 6; i++){
                backlight.setLed(0,LEDS[5-i],1,false);
                delay(25);
        }
}

void setup()
{
        /* MIDI Setup */
        Serial1.setTX(5);
        MIDI.begin();
        usbMIDI.begin();

        /* MAX7221 Setup */
        backlight.shutdown(0, false); // take Max7221 out of shutdown mode
        backlight.setIntensity(0, 16); // set Max7221 to max brightness
        pinMode(LED_PIN, OUTPUT);
        Serial.begin(9600);

        for (int i = 0; i < 6; i++)
        {
                pinMode(COL_PINS[i], OUTPUT);
        }
        pinMode(R1, INPUT_PULLDOWN);
        ledTest();
}

void loop()
{       
        mod = 0;
        keyScan();
        for(int i = 0; i < COLS; i++){ // figure out which modifiers are active
               if(keyPressed[i]) mod += MODIFIER_VALS[i];
        }
        Serial.println(mod);

        for (int i = 2; i < COLS; i++)
        {
                if (keyPressed[i]) { // if key was pressed
                       backlight.setLed(0, LEDS[i], 1, true);
                        if(!keyPrev[i]) { // if key pressed for first time send note on
                                usbMIDI.sendNoteOn(MIDI_NOTES[mod][i], 99, 1);  // 61 = C#4
                                MIDI.sendNoteOn(MIDI_NOTES[mod][i], 99, 1);  // 61 = C#4
                                // if(!keyPrev[i] | (keyPrev[i] > KEY_HELD_DELAY)) Keyboard.print(KEYS[0][i]);
                        }
                        keyPrev[i]++;
                }
                else { // if key not pressed
                        if(keyPrev[i]) { // if previously pressed, sent note off for all notes on key
                                for(int j = 0; j < 3; j++){
                                        usbMIDI.sendNoteOff(MIDI_NOTES[j][i], 0, 1);
                                        MIDI.sendNoteOff(MIDI_NOTES[j][i], 0, 1);
                                }
                                keyPrev[i] = 0;
                        }
                        backlight.setLed(0, LEDS[i], 1, false);
                }
        } 
} // End loop  