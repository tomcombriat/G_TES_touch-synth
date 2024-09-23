#include "config.h"


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
#include <RotaryEncoder.h>
#include <vPotentiometer.h>
#include <GT_Menu.h>
#include <GT_Parameter.h>

#include <GT_Input.h>
#include <GT_TouchScreen.h>
#include <GT_Button.h>




// The STMPE610 uses hardware SPI on the shield, and #8

XPT2046_Touchscreen ts(STMPE_CS, 18);

GT_Touchscreen touch(&ts, TFT_SIZE_X, TFT_SIZE_Y, TOUCH_RESPONSE_TIME);

/*
void readAndAlignData(XPT2046_Touchscreen* ts, int16_t* xnew, int16_t* ynew, uint8_t* znew) {
  uint16_t x, y;
  uint8_t z;
  ts->readData(&x, &y, &z);
  *xnew = map(x, TS_MINX, TS_MAXX, 0, 320);
  *ynew = map(y, TS_MINY, TS_MAXY, 0, 240);
  *znew = z;
}*/


Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// ROTARY

RotaryEncoder encoder(PINROT1, PINROT0, RotaryEncoder::LatchMode::FOUR3);


void checkPosition() {
  encoder.tick();  // just call tick() to check the state.
}


/********
 Visual pot
 */
const byte N_VPOT = 6;
ClassicPot pot1(&tft);
ClassicPot pot2(&tft);
ClassicPot pot3(&tft);
ClassicPot pot4(&tft);
ClassicPot pot5(&tft);
ClassicPot pot6(&tft);
ClassicPot* const pots[N_VPOT] = { &pot1, &pot2, &pot3, &pot4, &pot5, &pot6 };



/**********
PHYSICAL INPUTS
*/
const byte N_INPUT = 4;
GT_AnalogInput bluePot("blue", tft.color565(0, 0, 255), 26, 12, 2, true);
GT_AnalogInput redPot("red", tft.color565(255, 0, 0), 27, 12, 2, true);
GT_RotaryEncoder enc("Rot", tft.color565(0, 255, 255), &encoder, 20, true);

GT_PhysicalInput* const allInputs[N_INPUT] = { nullptr, &enc, &bluePot, &redPot };


/**
MENU
*/
GT_Menu test_menu(&tft, &enc);
GT_MenuParameter test_menu_para(&tft, &enc);

/**********
SYNTHESIS PARAMETERS
*/
const byte N_PARAM = 6;
GT_Parameter resonance("resonance", false, 16, allInputs, N_INPUT);
GT_Parameter cutoff("cutoff", false, 16, allInputs, N_INPUT);
GT_Parameter freq("freq", false, 10, allInputs, N_INPUT);
GT_Parameter LFOfreq("LFOfreq", false, 8, allInputs, N_INPUT);
GT_Parameter bassLevel("BassL", false, 8, allInputs, N_INPUT);
GT_Parameter test("test", false, 8, allInputs, N_INPUT);

GT_Parameter* const allParams[N_PARAM] = { &freq, &cutoff, &resonance, &LFOfreq, &bassLevel, &test };


/*******
BUTTONS
*/

const byte N_BUTTON = 2;
GT_BasicButton resetInputs(&tft);
GT_BasicButton randomize(&tft);

GT_BasicButton* const allButtons[N_BUTTON] = { &resetInputs, &randomize };



/*******************
        SD
        */
File myFile;




/*************************
          MOZZI
        */
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> aSaw(SAW2048_DATA);
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> aSawBass(SAW2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aLFO(SIN2048_DATA);
LowPassFilter16 lpf;



void callback(byte channel, byte number, byte value) {
  resonance.notifyMIDI(channel, number, value);
}

/*************
MIDI
        */
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);




#include <GT_Actions.h>

void setup() {

  startMozzi(MOZZI_CONTROL_RATE);
}


void setup1(void) {
  digitalWrite(LED_BUILTIN, HIGH);
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

  touch.calib(TS_MINX, TS_MINY, TS_MAXX, TS_MAXY);

  ts.setRotation(3);
  //Serial.println("Touchscreen started");
  tft.fillScreen(ILI9341_BLACK);


  /* ROTARY ENCODER INIT */
  pinMode(PINROT0, INPUT_PULLUP);
  pinMode(PINROT1, INPUT_PULLUP);


  attachInterrupt(digitalPinToInterrupt(PINROT0), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PINROT1), checkPosition, CHANGE);



  /* MIDI */
  Serial1.setRX(1);
  Serial1.setTX(0);

  MIDI.setHandleControlChange(callback);
  MIDI.begin(MIDI_CHANNEL_OMNI);

  /* SD */
  SD.begin(19);



  resonance.setMidiChannel(2);
  resonance.setMidiControl1(71);


  for (byte i = 0; i < N_VPOT; i++) {
    pots[i]->attachParameter(allParams[i]);
    pots[i]->setColor(10000);
    pots[i]->setPosition(40 + (i % 5) * 60, 30 + (70 * (i / 5)));
    pots[i]->setSize(25);
  }

  resonance.setInput(2 /*, true*/);
  // cutoff.setInput(2);
  // freq.setInput(3);


  resetInputs.setPositionAndSize(0, 200, 100, 40);
  resetInputs.setColor(tft.color565(120, 120, 120));
  resetInputs.setTextColor(0);
  resetInputs.setText("ResetIn");
  resetInputs.setAction(&disconnectAllParam);


  randomize.setPositionAndSize(100, 200, 100, 40);
  randomize.setColor(tft.color565(0, 255, 0));
  randomize.setTextColor(0);
  randomize.setText("Randomize");
  randomize.setAction(&randomizeAllParam);
}



/******
        * MOZZI
        */
void loop() {
  audioHook();
}

void updateControl() {
  while (MIDI.read()) {}  // move to other loop?
  aSaw.setFreq((int)freq.getValue());
  aSawBass.setFreq((int)freq.getValue() >> 1);
  aLFO.setFreq(UFix<4, 4>(LFOfreq.getValue(), true));
  lpf.setCutoffFreqAndResonance(cutoff.getValue(), resonance.getValue());
}

AudioOutput updateAudio() {
  //return MonoOutput::fromNBit(9, aSaw2.next() + aSaw.next());  // return an int signal centred around 0
  //return MonoOutput::fromSFix(SFix<7,0>(SFix<7,0>(aSaw.next())));
  //return MonoOutput::fromNBit(11, aSawBass.next() + lpf.next(aSaw.next())).clip();
  auto LF = toSFraction(aSawBass.next()) * UFix<0, 8>(bassLevel.getValue(), true) * (toSFraction(aLFO.next()) + SFix<0, 4>(0.5));
  auto sample = SFix<9, 0>(lpf.next(aSaw.next())) + LF.sL<8>();
  return MonoOutput::fromSFix(sample).clip();
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

  for (byte i = 0; i < N_BUTTON; i++) allButtons[i]->update();

  for (byte i = 0; i < N_PARAM; i++) allParams[i]->update();

  touch.update();
  test_menu_para.update();

  if (touch.hasBeenReleased()) {

    int16_t x, y;
    touch.data(&x, &y);

    for (byte i = 0; i < N_VPOT; i++)
      if (pots[i]->isInHitBox(x, y)) pots[i]->getAttachedParameter()->incrementProspectiveInput();
    for (byte i = 0; i < N_BUTTON; i++) if (allButtons[i]->isInHitBox(x, y)) allButtons[i]->trigAction();
  }
  if (touch.hasBeenReleasedAfterLongPress()) {
    /* int16_t x, y;
    touch.data(&x, &y);
    for (byte i = 0; i < N_VPOT; i++) if (pots[i]->isInHitBox(x, y)) pots[i]->getAttachedParameter()->disconnectInput();    
*/
    test_menu_para.start(allParams[0]);
  }
}
