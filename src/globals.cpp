#include "globals.h"

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

volatile bool newData = false;
