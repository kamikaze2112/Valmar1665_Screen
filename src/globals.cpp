#include "globals.h"

// APP VERSION

const char* APP_VERSION = "1.6.0";

// Define the actual variables here
TAMC_GT911 touchController(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, TOUCH_WIDTH, TOUCH_HEIGHT);

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
