#include <Arduino.h>
#include <Bounce.h>
#include <MIDI.h>
#include <Encoder.h>
// #include <MIDIcontroller.h>
#include <Keypad.h>
#include <LedControl.h>

#define R1 23
#define C1 6
#define C2 8
#define C3 12
#define C4 26
#define C5 27
#define C6 28

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

/* INITIALIZE MIDI */
MIDI_CREATE_DEFAULT_INSTANCE();

/* INITIALIZE KEYPAD */
const byte ROWS = 1; //1 rows
const byte COLS = 6; //6 columns
char keys[ROWS][COLS] = {{'1', '2', '3', '4', '5', '6'}};
//char forceCurrent[ROWS][COLS] = {{0,0,0,0,0,0}};
//char forcePrevious[ROWS][COLS] = {{0,0,0,0,0,0}};
byte rowPins[ROWS] = {23};                   //connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 8, 12, 26, 27, 28}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

bool keyPressed[6] = {false};
// bool keySwitchedOff[6] = {false};
// bool keyHeld[6] = {false};
// bool keySwitchedOn[6] = {false};

/* INTIALIZE FLASHING LED */
static const unsigned ledPin = 13; // LED pin on Arduino Uno

/* INITIALIZE MAX7221 LED CONTROLLER (DIN_pin, CLK_pin, LOAD_pin, number_of_chips)
LED1 (0,0,1)
LED2 (0,4,1)
LED3 (0,2,1)
LED4 (0,3,1)
LED5 (0,5,1)
LED6 (0,1,1)
*/
// LED Order 0,4,2,3,5,1
LedControl backlight = LedControl(7, 14, 31, 1);
int leds[6] = {0, 4, 2, 3, 5, 1};

void setup()
{
        // put your setup code here, to run once:
        MIDI.begin(4);
        backlight.shutdown(0, false); // take Max7221 out of shutdown mode
        backlight.setIntensity(0, 16);
        pinMode(ledPin, OUTPUT);
        Serial.begin(9600);

        for (int i = 0; i < 6; i++)
        {
                pinMode(colPins[i], OUTPUT);
        }
        pinMode(R1, INPUT_PULLDOWN);
}

void keyScan()
{
        for (int i = 0; i < 6; i++)
        {
                digitalWrite(colPins[i], LOW);
        }

        for (int i = 0; i < 6; i++) //keyscan
        {
                digitalWrite(colPins[i], HIGH);
                if (i > 0)
                        digitalWrite(colPins[i - 1], LOW);
                delay(10); //debounce
                if (digitalRead(R1))
                {

                        keyPressed[i] = true;
                        // for (int i = 0; i < 6; i++)
                        // {
                        //         backlight.setLed(0, leds[i], 1, false);
                        // }
                        // backlight.setLed(0, leds[i], 1, true);
                        // delay(100);
                        // backlight.setLed(0, leds[i], 1, false);
                }
                else
                        keyPressed[i] = false;
        }
}

void ledFade(){
               for (int i = 1; i < 16; i++)
        {
                backlight.setIntensity(0, 16 - i);
                delay(25);
        }
}
void ledTest()
{
        backlight.setLed(0, 1, 1, true);
        delay(25);
        backlight.setLed(0, 5, 1, true);
        delay(25);
        backlight.setLed(0, 3, 1, true);
        delay(25);
        backlight.setLed(0, 2, 1, true);
        delay(25);
        backlight.setLed(0, 4, 1, true);
        delay(25);
        backlight.setLed(0, 0, 1, true);
        delay(25);
        for (int i = 1; i < 16; i++)
        {
                backlight.setIntensity(0, i);
                delay(75);
        }
        for (int i = 1; i < 16; i++)
        {
                backlight.setIntensity(0, 16 - i);
                delay(50);
        }

        backlight.setLed(0, 0, 1, false);
        delay(25);
        backlight.setLed(0, 4, 1, false);
        delay(25);
        backlight.setLed(0, 2, 1, false);
        delay(25);
        backlight.setLed(0, 3, 1, false);
        delay(25);
        backlight.setLed(0, 5, 1, false);
        delay(25);
        backlight.setLed(0, 1, 1, false);
        delay(25);
}

void loop()
{       //ledTest();
        keyScan();
        for (int i = 0; i < 6; i++)
        {
                if (keyPressed[i])
                        backlight.setLed(0, leds[i], 1, true);
                else
                        backlight.setLed(0, leds[i], 1, false);
        }

        //    if (MIDI.read())                    // If we have received a message
        //   {
        //backlight.setColumn(0,1,B10110000);

        // digitalWrite(ledPin, HIGH);
        // MIDI.sendNoteOn(42, 127, 1); // Send a Note (pitch 42, velo 127 on channel 1)
        // delay(1000);                 // Wait for a second
        // MIDI.sendNoteOff(42, 0, 1);  // Stop the note
        // delay(250);

        // digitalWrite(ledPin, LOW);
        // delay(250);
        //    }

        // if (keypad.getKeys())
        //     {
        //         for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.
        //         {
        //             if ( keypad.key[i].stateChanged )   // Only find keys that have changed state.
        //             {
        //                 switch (keypad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
        //                     case PRESSED:
        //                     // send note
        //                 break;
        //                     case HOLD:
        //                     // do aftertouch stuff
        //                 break;
        //                     case RELEASED:
        //                     // send note off
        //                 break;
        //                     case IDLE:
        //                     // do nothing
        //                 }
        //                 Serial.print("Key ");
        //                 Serial.print(keypad.key[i].kchar);
        //             }
        //         }
        //     }
} // End loop  