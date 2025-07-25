#pragma once

#include <lvgl.h>
#include <TAMC_GT911.h> // Install "TAMC_GT911" with the Library Manager (last tested on v1.0.2)
#include "WAVPlayer.h"
#include "PINS_JC4827W543.h"

// Touch Controller configuration
/* #define TOUCH_SDA    8
#define TOUCH_SCL    4
#define TOUCH_INT    3 */
#define TOUCH_RST    38
#define TOUCH_WIDTH  480
#define TOUCH_HEIGHT 272

extern TAMC_GT911 touchController; 
extern WAVPlayer player;


// Display globals
extern uint32_t screenWidth;
extern uint32_t screenHeight;
extern uint32_t bufSize;
extern lv_display_t *disp;
extern lv_color_t *disp_draw_buf;

extern uint8_t controllerAddress[6];
extern uint8_t broadcastAddress[6];

extern bool calibrationMode;
extern float seedingRate;
extern float calibrationWeight;
extern bool motorTestSwitch;
extern int motorTestPWM;
extern bool speedTestSwitch;
extern float speedTestSpeed;
extern float actualRate;
extern bool pairingMode;
extern bool controllerPaired;
extern float seedPerRev;
extern float newSeedPerRev;
extern bool seedPerRevUpdated;
extern int numberOfRuns;
extern float workingWidth;
extern bool manualSeedUpdate;
extern bool errorRaised;
extern int errorCode;
extern bool errorAck;
extern bool warningTone;
extern bool warningAck;
extern unsigned long warningCooldownStart;
extern const unsigned long warningCooldownDuration;


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

// crap from the pins header


