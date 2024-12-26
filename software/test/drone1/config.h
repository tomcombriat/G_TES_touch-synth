/*********
 MOZZI
 */
#include "MozziConfigValues.h"  // for named option values
#define MOZZI_AUDIO_MODE MOZZI_OUTPUT_I2S_DAC
#define MOZZI_I2S_FORMAT MOZZI_I2S_FORMAT_LSBJ
#define MOZZI_AUDIO_CHANNELS 1
#define MOZZI_CONTROL_RATE 256  // Hz, powers of 2 are most reliable


/**********
TOUCHSCREEN
*/
#define STMPE_CS 255
#define TOUCH_RESPONSE_TIME 50

// This is default calibration data for the raw touch data to the screen coordinates
#define TS_MINX 930
#define TS_MINY 860
#define TS_MAXX 3300
#define TS_MAXY 2800



/*************
LCD
*/
#define SCREEN_REFRESH_TIME 20
#define TFT_CS 15
#define TFT_DC 14
#define TFT_SIZE_X 320
#define TFT_SIZE_Y 240


/******
ROTARY ENCODER
*/
#define PINROT0 6
#define PINROT1 7