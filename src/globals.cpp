#include "globals.h"
#include "WAVPlayer.h"
#include "PINS_JC4827W543.h"
#include <Arduino_GFX_Library.h>

//APP VERSION
const char* APP_VERSION = "20250730192419";

// Actual definitions
Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    45 /* cs */, 47 /* sck */, 21 /* d0 */, 48 /* d1 */, 40 /* d2 */, 39 /* d3 */);

Arduino_GFX *gfx = new Arduino_NV3041A(bus, GFX_NOT_DEFINED /* RST */, 0 /* rotation */, true /* IPS */);



// Define the actual variables here
TAMC_GT911 touchController(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, TOUCH_WIDTH, TOUCH_HEIGHT);

//Define the audio player
WAVPlayer player(I2S_LRCK, I2S_BCLK, I2S_DOUT);

uint32_t screenWidth = 0;
uint32_t screenHeight = 0;
uint32_t bufSize = 0;
lv_display_t *disp = nullptr;
lv_color_t *disp_draw_buf = nullptr;

bool calibrationMode = false;
float seedingRate = 0.0f;
float calibrationWeight = 0.0f;
bool motorTestSwitch = false;
int motorTestPWM = 10;
bool speedTestSwitch = false;
float speedTestSpeed = 0.0f;
float actualRate = 0.0f;
float workingWidth = 60.0f;
int numberOfRuns = 8;
volatile bool newData = false;
float newSeedPerRev = 0.0f;
bool manualSeedUpdate = false;
bool errorRaised = false;
int errorCode = 0;
bool errorAck = false;
bool fwUpdateStarted = false;

bool workSwitchState = false;
int savedBrightness = 200;