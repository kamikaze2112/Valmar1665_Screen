#pragma once

#include <lvgl.h>
#include <TAMC_GT911.h> // Install "TAMC_GT911" with the Library Manager (last tested on v1.0.2)

// Touch Controller configuration
#define TOUCH_SDA    8
#define TOUCH_SCL    4
#define TOUCH_INT    3
#define TOUCH_RST    38
#define TOUCH_WIDTH  480
#define TOUCH_HEIGHT 272

extern TAMC_GT911 touchController; 

// Display globals
extern uint32_t screenWidth;
extern uint32_t screenHeight;
extern uint32_t bufSize;
extern lv_display_t *disp;
extern lv_color_t *disp_draw_buf;


extern bool calibrationMode;
extern float seedingRate;
extern float calibrationWeight;

#define DEBUG_MODE 1 // toggle 1 for on, and 0 for off

#if DEBUG_MODE
  #define DBG_PRINT(x)     Serial.print(x)
  #define DBG_PRINTLN(x)   Serial.println(x)
  #define DBG_WRITE(x)     Serial.write(x)

#else
  #define DBG_PRINT(x)     ((void)0)
  #define DBG_PRINTLN(x)   ((void)0)
  #define DBG_WRITE(x)     ((void)0)
#endif

extern volatile bool newData;
extern const char* APP_VERSION;

