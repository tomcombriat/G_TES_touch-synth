/***************************************************
  This is our touchscreen painting example for the Adafruit ILI9341 Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/



/******************
   SCREEN
*/

#include <Adafruit_GFX.h>  // Core graphics library
#include <SPI.h>
#include <Wire.h>  // this is needed even tho we aren't using it
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <MIDI.h>

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 3900
#define TS_MINY 300
#define TS_MAXX 200
#define TS_MAXY 3800

// The STMPE610 uses hardware SPI on the shield, and #8
#define STMPE_CS 5
//Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);
XPT2046_Touchscreen ts(STMPE_CS);

#define TFT_CS 15
#define TFT_DC 14
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);


// ROTARY
#define PIN0 6
#define PIN1 7
#include <RotaryEncoder.h>
RotaryEncoder *encoder = nullptr;

void checkPosition() {
  encoder->tick();  // just call tick() to check the state.
}


/**************
   GT touch
*/
#include <vPotentiometer.h>
#include <Parameter.h>
#include <Input.h>

ClassicPot pot(&tft);
Parameter<uint8_t> testParameter("coucou blac blac");

AnalogInput<> AI[3] = { AnalogInput("A0", 26), AnalogInput("A1", 27), AnalogInput("A2", 28) };
AnalogInput<> AII[2] = { AnalogInput("A0", 26), AnalogInput("A1", 27) };
MidiInput MI("testMidi", 1,25);
MidiInputHQ MIHQ("testMidiHQ", 1,26,27);
GTInput *in[5] = { &AII[0], &AII[1], &AI[0], &AI[1], &AI[2] };

#define SCREEN_REFRESH_TIME 20
unsigned long last_screen_refresh;







/*************************
   MOZZI
*/

#include <Oscil.h>                // oscillator templateu
#include <tables/saw2048_int8.h>  // sine table for oscillator
#include <tables/sin2048_int8.h>  // sine table for oscillator

// use: Oscil <table_size, update_rate> oscilName (wavetable), look in .h file of table #included above
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> aSaw(SAW2048_DATA);
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> aSaw2(SAW2048_DATA);

int freq1 = 440;
volatile int freq2 = 221;

volatile int rotary = 0;
int rotary_prev = 0;




/*************
* MIDI
*/
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);




void setup() {
  startMozzi(CONTROL_RATE);
  //testParameter.setValue((uint8_t)15);
}


void setup1(void) {

 Serial.begin(115200);


  /* SPI AND SCREEN INIT */
  SPI.setRX(4);
  SPI.setTX(3);
  SPI.setSCK(2);

  tft.begin();
  tft.setRotation(3);

  if (!ts.begin()) {
    Serial.println("Couldn't start touchscreen controller");
    while (1)
      ;
  }
  Serial.println("Touchscreen started");
  tft.fillScreen(ILI9341_BLACK);


  /* ROTARY ENCODER INIT */
  pinMode(PIN0, INPUT_PULLUP);
  pinMode(PIN1, INPUT_PULLUP);
  encoder = new RotaryEncoder(PIN1, PIN0, RotaryEncoder::LatchMode::FOUR3);

  attachInterrupt(digitalPinToInterrupt(PIN0), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN1), checkPosition, CHANGE);





  // attachInterrupt(digitalPinToInterrupt(PIN0), rotary_encoder_irq, FALLING);

  /* VISUAL POTENTIOMETER INIT */
  pot.attachParameter(&testParameter);
  pot.setPosition(40, 40);
  pot.setSize(10);
  pot.setColor(60000);
  pot.setBackgroundColor(0);
  pot.update();

  AI[1].setInvert(true);


  /* MIDI */
  Serial1.setRX(1);
  Serial1.setTX(0);
  MIDI.begin(MIDI_CHANNEL_OMNI);
}



/******
* MOZZI
*/
void loop() {
  audioHook();
}

void updateControl() {
  aSaw.setFreq(freq1);
  aSaw2.setFreq(freq2);
}

AudioOutput_t updateAudio() {
  return MonoOutput::fromNBit(9, aSaw2.next() + aSaw.next());  // return an int signal centred around 0
}


/**********
* OTHER THREAD, MANAGES PARAMETERS AND ALL
*/
void loop1() {
  static int pos = 0;

  // encoder->tick(); // just call tick() to check the state.

  int newPos = encoder->getPosition();
  if (pos != newPos) {
    Serial.print("pos:");
    Serial.print(newPos);
    Serial.print(" dir:");
    Serial.println((int)(encoder->getDirection()));
    pos = newPos;
    pot.setColor(newPos << 4);
    //pot.setText(String(newPos));
    pot.setSize(newPos + 20);
    pot.setPosition(newPos + 60, newPos + 60);
  }  // if




  if (MIDI.read())
  {
Serial.print("MIDI IN ");
Serial.println(MIDI.getType());
  }

  in[3]->update();

  testParameter.setRawValue((in[3]->getValue()));
  pot.update();






  // Retrieve a point
  boolean istouched = ts.touched();
  if (istouched) {
    TS_Point p = ts.getPoint();
    /* Serial.print("X = "); Serial.print(p.x);
      Serial.print("\tY = "); Serial.print(p.y);
      Serial.print("\tPressure = "); Serial.println(p.z);*/
    // Scale from ~0->4000 to tft.width using the calibration #'s
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
    freq1 = p.x;
    freq2 = p.y << 1;
  }
}


