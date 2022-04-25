/*   Root Commander - Jupertronic MIDI Note Commander
      by Janis Wilson Hughes aka Jupertronic aka vitalWho aka J Dub aka Evolution Stoneware on Youtube
      MIDI output to control root note of arpeggiator box or synth box
      Control of scale mode
      Control of octave
      OLED screen
      Set for Leonardo/Pro Micro/Micro
*/

#include <MIDI.h>
#include <ss_oled.h>

// if your system doesn't have enough RAM for a back buffer, comment out
// this line (e.g. ATtiny85)
#define USE_BACKBUFFER

#ifdef USE_BACKBUFFER
static uint8_t ucBackBuffer[1024];
#else
static uint8_t *ucBackBuffer = NULL;
#endif

// Use -1 for the Wire library default pins
// or specify the pin numbers to use with the Wire library or bit banging on any GPIO pins
// These are the pin numbers for the M5Stack Atom default I2C
#define SDA_PIN 2
#define SCL_PIN 3
// Set this to -1 to disable or the GPIO pin number connected to the reset
// line of your display if it requires an external reset
#define RESET_PIN -1
// let ss_oled figure out the display address
#define OLED_ADDR -1
// don't rotate the display
#define FLIP180 0
// don't invert the display
#define INVERT 0
// Bit-Bang the I2C bus
#define USE_HW_I2C 0

// Change these if you're using a different OLED display
#define MY_OLED OLED_128x64
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
//#define MY_OLED OLED_64x32
//#define OLED_WIDTH 64
//#define OLED_HEIGHT 32

SSOLED ssoled;

MIDI_CREATE_DEFAULT_INSTANCE();

//Buttons
const int nButtons = 7;
const int buttonPins[nButtons] = {4, 5, 6, 7, 8, 9, 10}; // Array of digital input pins for buttons
const int I = 4;    // Button 1
const int II = 5;   // Button 2
const int III = 6;  // Button 3
const int iV = 7;   // Button 4
const int V = 8;    // Button 5
const int VI = 9;   // Button 6
const int VII = 10;  // Button 7
//const int up = 11;  // Button Up
//const int down = 12;// Button Down
//const int funct = 13;   // Function Button
int buttonCState[nButtons] = {0};        // stores the button current value
int buttonPState[nButtons] = {0};        // stores the button previous value
int octave = 0;

// debounce
unsigned long lastDebounceTime[nButtons] = {0};  // the last time the output pin was toggled
unsigned long debounceDelay = 5;    //* the debounce time; increase if the output flickers

// velocity
byte velocity[nButtons] = {127};
byte toggleVelocity[nButtons] = {127};


//Pots
const int nPots = 3;
const int potPins[nPots] = {A0, A1, A2}; // Array of analog input pins for pots
const int scalePot = 0;
const int octavePot = 1;
const int keyPot = 2;

//Scale and Note Controls
//const int octave = 12;  // Number of steps between octaves to calculate MIDI note
const int nRoots = 7;   // Number of root notes and number of notes in scale
//#define nChords = 7;
//const int key[nRoots] = {57, 59, 60, 62, 64, 65, 67}; //Array of MIDI notes whole notes from A2 to G3
const int key[12] = {57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68}; //Array of MIDI notes from A2 to Ab3 including flats
const int nScales = 4; // Number of modes/scales
const int scales[nScales][nRoots] = { //Array of scale arrays containing the steps between each note
  {0, 2, 4, 5, 7, 9, 11}, // Ionian, Major
  {0, 2, 3, 5, 7, 8, 10}, // Natural Minor
  {0, 2, 4, 5, 7, 9, 11}, // Dorian
  {0, 2, 4, 5, 7, 9, 10}, // Mixolydian
};
int scaleSelect;
int keySelect;
int octaveSelect;
char scaleName;
char keyName;
char octaveName;
char *stepName[7] = {"   I    ", "  II   ", " III", "  IV  ", "   V   ", "  VI  ", " VII "};


void setup() {
  //Intialize button inputs utilizing internal pullup resistors
  for (int i = 0; i < nButtons; i++)
  {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  //Initialize pots
  for (int i = 0; i < nPots; i++)
  {
    pinMode(potPins[i], INPUT);
    //      analogRead(potPins[i]);
  }

  //Set default start up values for scale, key, and octave
  keySelect = 2;    // C


  //Initialize MIDI input
  MIDI.begin(MIDI_CHANNEL_OMNI);
  //    MIDI.setHandleNoteOn(NoteOnMidi);
  //    MIDI.setHandleNoteOff(NoteOffMidi);

  //Initialize OLED display
  int rc;
  // The I2C SDA/SCL pins set to -1 means to use the default Wire library
  // If pins were specified, they would be bit-banged in software
  // This isn't inferior to hw I2C and in fact allows you to go faster on certain CPUs
  // The reset pin is optional and I've only seen it needed on larger OLEDs (2.4")
  //    that can be configured as either SPI or I2C
  //
  // oledInit(SSOLED *, type, oled_addr, rotate180, invert, bWire, SDA_PIN, SCL_PIN, RESET_PIN, speed)

  rc = oledInit(&ssoled, MY_OLED, OLED_ADDR, FLIP180, INVERT, USE_HW_I2C, SDA_PIN, SCL_PIN, RESET_PIN, 400000L); // use standard I2C bus at 400Khz


  oledFill(&ssoled, 1, 1);
//  oledDumpBuffer(&ssoled, NULL);
//  //    oledWriteString(&ssoled, 0, 5, 0,(char *)"SKEETER ARP BOX", FONT_NORMAL, 0, 1);
//  oledWriteString(&ssoled, 0, 0, 1, (char *)"   Jupertronic       ", FONT_NORMAL, 1, 1);
//  oledWriteString(&ssoled, 0, 20, 3, (char *)"  Root", FONT_NORMAL, 0, 1);
//  oledWriteString(&ssoled, 0, 0, 6, (char *)"Commander", FONT_NORMAL, 0, 1);
//
//  delay(1000);

  oledFill(&ssoled, 0, 1);
  oledDumpBuffer(&ssoled, NULL);
}

void loop() {


  // Update Pots
  int scalePot = analogRead(A0);
  switch (scalePot)
  {
    case 0 ... 222:
      //        Serial.println("track 1");
      scaleSelect = 0;
      oledWriteString(&ssoled, 0, 5, 0, (char *)"     MAJOR     ", FONT_NORMAL, 0, 1);
      break;
    case 250 ... 472:
      //        Serial.println("track 2");
      scaleSelect = 1;
      oledWriteString(&ssoled, 0, 5, 0, (char *)"     MINOR     ", FONT_NORMAL, 0, 1);
      break;
    case 500 ... 722:
      //        Serial.println("track 1");
      scaleSelect = 2;
      oledWriteString(&ssoled, 0, 5, 0, (char *)"     DORIAN    ", FONT_NORMAL, 0, 1);
      break;
    case 750 ... 1027:
      //        Serial.println("track 1");
      scaleSelect = 3;
      oledWriteString(&ssoled, 0, 5, 0, (char *)"   MIXOLYDIAN ", FONT_NORMAL, 0, 1);
      break;
  }

  int octavePot = analogRead(A1);
  switch (octavePot)
  {
    case 0 ... 230:
      //        Serial.println("track 1");
      octaveSelect = -24;
      oledWriteString(&ssoled, 0, 20, 1, (char *)"  -2", FONT_STRETCHED, 0, 1);
      break;
    case 255 ... 490:
      //        Serial.println("track 1");
      octaveSelect = -12;
      oledWriteString(&ssoled, 0, 20, 1, (char *)"  -1", FONT_STRETCHED, 0, 1);
      break;
    case 515 ... 750:
      //        Serial.println("track 1");
      octaveSelect = 0;
      oledWriteString(&ssoled, 0, 20, 1, (char *)"  0  ", FONT_STRETCHED, 0, 1);
      oledSetBackBuffer(&ssoled, ucBackBuffer);
      break;
    case 775 ... 1027:
      //        Serial.println("track 1");
      octaveSelect = 12;
      oledWriteString(&ssoled, 0, 20, 1, (char *)"  +1", FONT_STRETCHED, 0, 1);
      break;
  }

  int keyPot = analogRead(A2);
  switch (keyPot)
  {
    case 0 ... 60:
      //        Serial.println("track 1");
      keySelect = key[0];
      oledWriteString(&ssoled, 0, 20, 3, (char *)"Key A  ", FONT_STRETCHED, 0, 1);
      break;
    case 85 ... 145:
      //        Serial.println("track 1");
      keySelect = key[1];
      oledWriteString(&ssoled, 0, 20, 3, (char *)"Key Bb", FONT_STRETCHED, 0, 1);
      break;
    case 170 ... 230:
      //        Serial.println("track 1");
      keySelect = key[2];
      oledWriteString(&ssoled, 0, 20, 3, (char *)"Key B  ", FONT_STRETCHED, 0, 1);
      break;
    case 255 ... 315:
      //        Serial.println("track 1");
      keySelect = key[3];
      oledWriteString(&ssoled, 0, 20, 3, (char *)"Key C  ", FONT_STRETCHED, 0, 1);
      break;
    case 340 ... 400:
      //        Serial.println("track 1");
      keySelect = key[4];
      oledWriteString(&ssoled, 0, 20, 3, (char *)"Key Db", FONT_STRETCHED, 0, 1);
      break;
    case 425 ... 485:
      //        Serial.println("track 1");
      keySelect = key[5];
      oledWriteString(&ssoled, 0, 20, 3, (char *)"Key D  ", FONT_STRETCHED, 0, 1);
      break;
    case 510 ... 570:
      //        Serial.println("track 1");
      keySelect = key[6];
      oledWriteString(&ssoled, 0, 20, 3, (char *)"Key Eb", FONT_STRETCHED, 0, 1);
      break;
    case 595 ... 655:
      //        Serial.println("track 1");
      keySelect = key[7];
      oledWriteString(&ssoled, 0, 20, 3, (char *)"Key E  ", FONT_STRETCHED, 0, 1);
      break;
    case 680 ... 740:
      //        Serial.println("track 1");
      keySelect = key[8];
      oledWriteString(&ssoled, 0, 20, 3, (char *)"Key F  ", FONT_STRETCHED, 0, 1);
      break;
    case 765 ... 825:
      //        Serial.println("track 1");
      keySelect = key[9];
      oledWriteString(&ssoled, 0, 20, 3, (char *)"Key Gb", FONT_STRETCHED, 0, 1);
      break;
    case 850 ... 910:
      //        Serial.println("track 1");
      keySelect = key[10];
      oledWriteString(&ssoled, 0, 20, 3, (char *)"Key G  ", FONT_STRETCHED, 0, 1);
      break;
    case 935 ... 1027:
      //        Serial.println("track 1");
      keySelect = key[11];
      oledWriteString(&ssoled, 0, 20, 3, (char *)"Key Ab", FONT_STRETCHED, 0, 1);
      break;
  }

  // Update buttons
  for (int i = 0; i < nRoots; i++) {
    buttonCState[i] = digitalRead(buttonPins[i]);
    if ((millis() - lastDebounceTime[i]) > debounceDelay) {

      if (buttonPState[i] != buttonCState[i]) {
        lastDebounceTime[i] = millis();

        if (buttonCState[i] == LOW) { // if button is pressed

          velocity[i] = 127; // if button is pressed velocity is 127
          int note = (scales[scaleSelect][i] + keySelect + octaveSelect);
        }
        else {

          velocity[i] = 0; // if button is released velocity is 0
        }
        if (buttonCState[i] == LOW) { // only sends note on when button is pressed, nothing when released
          MIDI.sendNoteOn((scales[scaleSelect][i] + keySelect + octaveSelect), 127, 1);
          //          Serial.print("BUTTON ");
          //          Serial.print(i);
          //          Serial.print("MIDI note ");
          //          Serial.println((scales[scaleSelect][i] + keySelect + octaveSelect));
          //          break;
          oledWriteString(&ssoled, 0, 20, 6, (char *)stepName[i], FONT_STRETCHED, 0, 1);
        }
        buttonPState[i] = buttonCState[i];
      }
    }
  }
}
