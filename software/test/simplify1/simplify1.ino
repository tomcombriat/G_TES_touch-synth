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



#include "MozziConfigValues.h"  // for named option values
#define MOZZI_AUDIO_MODE MOZZI_OUTPUT_I2S_DAC
#define MOZZI_I2S_FORMAT MOZZI_I2S_FORMAT_LSBJ
#define MOZZI_AUDIO_CHANNELS 1
#define MOZZI_CONTROL_RATE 256  // Hz, powers of 2 are most reliable

#include <Mozzi.h>
#include <Oscil.h>                // oscillator templateu
#include <tables/saw2048_int8.h>  // sine table for oscillator
#include <tables/sin2048_int8.h>  // sine table for oscillator
#include <ResonantFilter.h>

/******************
          SCREEN
        */
//#include <Adafruit_SPITFT.h>
#include <Adafruit_GFX.h>  // Core graphics library
#include <SPI.h>
#include <Wire.h>  // this is needed even tho we aren't using it
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <MIDI.h>
#include <SD.h>

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 422
#define TS_MINY 295
#define TS_MAXX 3844
#define TS_MAXY 3810

// The STMPE610 uses hardware SPI on the shield, and #8
#define STMPE_CS 255
//Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);
XPT2046_Touchscreen ts(STMPE_CS, 18);
//XPT2046_Touchscreen ts()

void readAndAlignData(XPT2046_Touchscreen* ts, int16_t* xnew, int16_t* ynew, uint8_t* znew) {
  uint16_t x, y;
  uint8_t z;
  ts->readData(&x, &y, &z);
  *xnew = map(x, TS_MINX, TS_MAXX, 0, 320);
  *ynew = map(y, TS_MINY, TS_MAXY, 0, 240);
  *znew = z;
}

#define SCREEN_REFRESH_TIME 20
#define TFT_CS 15
#define TFT_DC 14
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// ROTARY
#define PIN0 6
#define PIN1 7
#include <RotaryEncoder.h>
//RotaryEncoder *encoder = nullptr;
RotaryEncoder encoder(PIN1, PIN0, RotaryEncoder::LatchMode::FOUR3);


void checkPosition() {
  encoder.tick();  // just call tick() to check the state.
}








#include <vPotentiometer.h>

const byte N_VPOT = 3;
ClassicPot pot1(&tft);
ClassicPot pot2(&tft);
ClassicPot pot3(&tft);
ClassicPot* const pots[N_VPOT] = { &pot1, &pot2, &pot3 };

#include <GT_Input.h>
const byte N_INPUT = 4;
GT_AnalogInput bluePot("blue", tft.color565(0, 0, 255), 26, 12, 2, true);
GT_AnalogInput redPot("red", tft.color565(255, 0, 0), 27, 12, 2, true);
GT_RotaryEncoder enc("Rot", tft.color565(0, 255, 255), &encoder, 20, true);

GT_PhysicalInput* const allInputs[N_INPUT] = { nullptr, &bluePot, &redPot, &enc };

#include <GT_Parameter.h>

const byte N_PARAM = 3;
GT_Parameter resonance("resonance", false, 16, allInputs, N_INPUT);
GT_Parameter cutoff("cutoff", false, 16, allInputs, N_INPUT);
GT_Parameter freq("freq", false, 10, allInputs, N_INPUT);

GT_Parameter* const allParams[N_PARAM] = {&resonance, &cutoff, &freq};


/*******************
        SD
        */
File myFile;




/*************************
          MOZZI
        */
// use: Oscil <table_size, update_rate> oscilName (wavetable), look in .h file of table #included above
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> aSaw(SAW2048_DATA);
//Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> aSaw2(SAW2048_DATA);
LowPassFilter16 lpf;


/*
int freq1 = 440;
volatile int freq2 = 221;

volatile int rotary = 0;
int rotary_prev = 0;

*/

void callback(byte channel, byte number, byte value) {
  /*Serial.print(channel);
Serial.print(" ");
Serial.println(number);*/
  resonance.notifyMIDI(channel, number, value);
}

/*************
MIDI
        */
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);






void setup() {
  
  startMozzi(MOZZI_CONTROL_RATE);
}


void setup1(void) {
  digitalWrite(LED_BUILTIN,HIGH);
  //encoder.getMillisBetweenRotations()

  Serial.begin(115200);

  /* SPI AND SCREEN INIT */
  SPI.setRX(4);
  SPI.setTX(3);
  SPI.setSCK(2);


  SPI1.setRX(12);
  SPI1.setTX(11);
  SPI1.setSCK(10);
  SPI1.setCS(13);

  tft.begin();
  tft.setRotation(3);

  if (!ts.begin(SPI1)) {
    Serial.println("Couldn't start touchscreen controller");
    while (1)
      ;
  }

  ts.setRotation(3);
  Serial.println("Touchscreen started");
  tft.fillScreen(ILI9341_BLACK);


  /* ROTARY ENCODER INIT */
  pinMode(PIN0, INPUT_PULLUP);
  pinMode(PIN1, INPUT_PULLUP);


  attachInterrupt(digitalPinToInterrupt(PIN0), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN1), checkPosition, CHANGE);



  /* MIDI */
  Serial1.setRX(1);
  Serial1.setTX(0);

  MIDI.setHandleControlChange(callback);
  MIDI.begin(MIDI_CHANNEL_OMNI);

  /* SD */
  SD.begin(19);
  /*myFile = SD.open("test.txt", FILE_WRITE);
  myFile.println("testing 1, 2, 3.");
  myFile.close();*/


  resonance.setMidiChannel(2);
  resonance.setMidiControl1(71);
  //test.setMidiControl2(65);

  /* pots[0]->attachParameter(&resonance);
  pots[0]->setPosition(20, 20);
  pots[0]->setColor(10000);
  pots[0]->setSize(20);*/
  for (byte i = 0; i < N_VPOT; i++) {
    pots[i]->attachParameter(allParams[i]);
    pots[i]->setColor(10000);
    pots[i]->setPosition(40 + i * 60, 30);
    pots[i]->setSize(25);
  }

  //bluePot.setTarget(&test);
  //bluePot.setInverted(true);
  //enc.setTarget(&test);
  /*resonance.setInput(&bluePot);
  cutoff.setInput(&redPot);
  freq.setInput(&enc);*/
  resonance.setInput(1);
  cutoff.setInput(2);
  freq.setInput(3);

  freq.incrementProspectiveInput(3);
  resonance.incrementProspectiveInput(1);
  cutoff.incrementProspectiveInput(1);
  freq.disconnectInput();

}



/******
        * MOZZI
        */
void loop() {
  audioHook();
}

void updateControl() {
  while (MIDI.read()) {}  // move to other loop?
  //aSaw.setFreq(freq1);
  //aSaw.setFreq((int)test.getValue());
  //aSaw2.setFreq(freq2);
  aSaw.setFreq((int)freq.getValue());
  lpf.setCutoffFreqAndResonance(cutoff.getValue(), resonance.getValue());
}

AudioOutput updateAudio() {
  //return MonoOutput::fromNBit(9, aSaw2.next() + aSaw.next());  // return an int signal centred around 0
  //return MonoOutput::fromSFix(SFix<7,0>(SFix<7,0>(aSaw.next())));
  return MonoOutput::fromNBit(10, lpf.next(aSaw.next())).clip();
  //return MonoOutput::fromSFix(SFix<7,0>(aSaw.next()));
}


/**********
        * OTHER THREAD, MANAGES PARAMETERS AND ALL
        */

unsigned long tim = millis();
void loop1() {
  for (byte i = 0; i < N_VPOT; i++) pots[i]->update();  // GUI update
  for (byte i = 0; i < N_INPUT; i++) {
    if (allInputs[i] != nullptr) allInputs[i]->update();  // physical input update
  }
  for (byte i=0; i<N_PARAM; i++) allParams[i]->update();
  if (millis() - tim > 50) {
    if (ts.touched()) {
      int16_t x, y;
      uint8_t z;
      //ts.readData(&x,&y,&z);
      readAndAlignData(&ts, &x, &y, &z);
      Serial.print(x);
      Serial.print(" ");
      Serial.println(y);
      for (byte i = 0; i < N_VPOT; i++) {
        if (pots[i]->isInHitBox(x, y)) {
          Serial.print("HIT! ");
          Serial.println(pots[i]->getAttachedParameter()->getName());
        }
      }
    }
    tim = millis();
  }
}
