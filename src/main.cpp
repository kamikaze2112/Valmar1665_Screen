/*

This is the main program for the Valmar 1665 screen built by Trevor Kidd
for S5 Farms LTD. No warranty is implied and use is at your own risk. You may
freely modify, copy, and or distribute this source code as long as this header
remains intatct. All reference materials and design files can be found on
github:  https://github.com/kamikaze2112/Valmar1665_Screen

06/2025

*/

#include <Arduino.h>
#include "WAVPlayer.h"
#include "globals.h"
#include <WiFi.h>
#include <esp_now.h>
#include "ui.h"
#include <lvgl.h>            // Install "lvgl" with the Library Manager (last tested on v9.2.2)
#include "ui.h"
#include "comms.h"
#include "prefs.h"
#include "errorHandler.h"
#include "nonBlockingTimer.h"

NonBlockingTimer timer;
NonBlockingTimer timerHeartbeat;
int heartBeat = 0;

void heartbeat() {

  heartBeat++;

  if (heartBeat >= 5 && !errorRaised && incomingData.controllerBooted) {
    raiseError(4);
  } else if (heartBeat == 0) {
    clearError();
  }

}
void debugPrint(){
  
  //Serial.printf("incomingData.fwUpdateComplete: %d  fwUpdateStarted: %d\n", incomingData.fwUpdateComplete, fwUpdateStarted);
  Serial.printf("outgoingData.stallProtection: %d  outgoingData.stallDelay %d\n", outgoingData.stallProtection, outgoingData.stallDelay);
  Serial.printf("incomingData.heartbeat %d  heartBeat %d\n", incomingData.heartbeat, heartBeat);

}

unsigned long backlightStartTime = 0;
bool backlightFading = true;
const int backlightFadeDuration = 1500;  // 1 second
unsigned long lastPairingTime = 0;
float seedPerRev = 0.00f;
bool cooldownExpired = false;

void my_print(lv_log_level_t level, const char *buf);
uint32_t millis_cb(void);
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data);

void setupBacklight() {
  ledcSetup(BACKLIGHT_CH, BACKLIGHT_FREQ, BACKLIGHT_RES);
  ledcAttachPin(GFX_BL, BACKLIGHT_CH);
  ledcWrite(BACKLIGHT_CH, 0);  // Start at 0 brightness
  backlightStartTime = millis();
  backlightFading = true;

}

void setup() {
  Serial.begin(115200);
  int i = 0;

  while (i < 10) {
    i++; 
    delay(500);
      DBG_PRINTLN(i);
  }

  timer.set(debugPrint, 1000);
  timerHeartbeat.set(heartbeat, 1000);
  incomingData.controllerBooted = false;

  //init littleFS

  if (!LittleFS.begin(true)) {
      Serial.println("LittleFS Mount Failed");
      return;
  }
  
  Serial.printf("Total storage: %u KB\n", LittleFS.totalBytes() / 1024);
  Serial.printf("Available: %u KB\n", (LittleFS.totalBytes() - LittleFS.usedBytes()) / 1024);
  
  loadComms();

  setupComms();
  
  DBG_PRINTLN("Arduino_GFX LVGL_Arduino_v9 example ");
  String LVGL_Arduino = String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  DBG_PRINTLN(LVGL_Arduino);

  lv_init();

// Init Display
if (!gfx->begin())
{
  DBG_PRINTLN("gfx->begin() failed!");
  while (true)
  {
    /* no need to continue */
  }
}

// ✨ Make sure screen is blanked before backlight is on
gfx->fillScreen(RGB565_BLACK);
gfx->flush(); // Force immediate update (if needed)

// THEN turn on backlight

setupBacklight();

  // Init touch device
  touchController.begin();
  touchController.setRotation(ROTATION_INVERTED); // Change as needed

  // Set a tick source so that LVGL will know how much time elapsed
  lv_tick_set_cb(millis_cb);

  // register print function for debugging
#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif

  screenWidth = gfx->width();
  screenHeight = gfx->height();
  bufSize = screenWidth * 40;

  disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  

  if (!disp_draw_buf)
  {
    // remove MALLOC_CAP_INTERNAL flag try again
    disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_8BIT);
  }
  if (!disp_draw_buf)
  {
    DBG_PRINTLN("LVGL disp_draw_buf allocate failed!");
    while (true)
    {
      /* no need to continue */
    }
  }
  else
  {
    disp = lv_display_create(480, 272);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, disp_draw_buf, NULL, bufSize * 2, LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Create input device (touchpad of the JC4827W543)
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touchpad_read);

  // init UI
  ui_init();

  loadPrefs();

  
  }

  lv_label_set_text(ui_lblVersion1, APP_VERSION);

  setupAudio();

  player.play("/startup.wav");

  outgoingData.stallProtection = true;
  outgoingData.stallDelay = 200;

  DBG_PRINTLN("Setup done");
}



void loop() {

  timer.update();
  timerHeartbeat.update();

  handleAudio();  // Handles tone playing without blocking the loop
  handleErrorTimer();  // Handles repeating errors if they've been ack'd but still persist

  if (backlightFading) {

  unsigned long elapsed = millis() - backlightStartTime;

  if (elapsed >= backlightFadeDuration) {
    ledcWrite(BACKLIGHT_CH, 255);  // Final brightness
    backlightFading = false;

  } else {
    int brightness = map(elapsed, 0, backlightFadeDuration, 0, 255);
    ledcWrite(BACKLIGHT_CH, brightness);
  }
}

  // Format gpsSpeed and shaftRPM to 1 decimal place
  char speedBuf[10];
  char rpmBuf[10];
  char rateBuf[10];
  
  snprintf(rateBuf, sizeof(rateBuf), "%.1f", incomingData.actualRate);
  snprintf(speedBuf, sizeof(speedBuf), "%.1f", incomingData.gpsSpeed);
  snprintf(rpmBuf, sizeof(rpmBuf), "%.1f", incomingData.shaftRPM);

  // Update the LVGL labels
  lv_label_set_text(ui_lblSpeed, speedBuf);
  lv_label_set_text(ui_lblShaftRPM, rpmBuf);
  lv_label_set_text(ui_lblRate, rateBuf);
  lv_label_set_text(ui_lblRPM, rpmBuf);

  // If in speed test mode, update the MPH label

  if (speedTestSwitch) {

  }
  
  lv_task_handler(); /* let the GUI do its work */

  #ifdef DIRECT_MODE
  #if defined(CANVAS) || defined(RGB_PANEL) || defined(DSI_PANEL)
    gfx->flush();
  #else  // !(defined(CANVAS) || defined(RGB_PANEL) || defined(DSI_PANEL))
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, screenWidth, screenHeight);
  #endif // !(defined(CANVAS) || defined(RGB_PANEL) || defined(DSI_PANEL))
  #else  // !DIRECT_MODE
  #ifdef CANVAS
    gfx->flush();
  #endif
  #endif // !DIRECT_MODE

  delay(5);

if (newData) {
    newData = false;

    heartBeat = incomingData.heartbeat;

  // Handle incoming error messages
  if (incomingData.errorRaised && incomingData.errorCode != 0) {
    if (!errorRaised || incomingData.errorCode != errorCode) {
      raiseError(incomingData.errorCode);
    }
  } else if (errorRaised) {  // Clear if we had an error but now don't
    clearError();
  }

  if (!fwUpdateStarted) {
      if (!incomingData.fwUpdateComplete) {
        fwUpdateStarted = true;
        lv_label_set_text(ui_lblCancelButton, "CLOSE");
      }
  }

  if (fwUpdateStarted) {
      if (incomingData.fwUpdateComplete) {
          fwUpdateStarted = false;
          lv_obj_set_state(ui_btnFirmwareCancel, LV_STATE_DISABLED, false);
      }
  }

  // process controller version and update label
  lv_label_set_text(ui_lblVersion, incomingData.controllerVersion);

  // Process time and set ui label
  if (incomingData.fixStatus != 0) { // no point in updating the clock if there's no fix to get the time from
    char timeStr[9];  // "HH:MM:SS" + null terminator
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", incomingData.gpsHour, incomingData.gpsMinute, incomingData.gpsSecond);
    lv_label_set_text(ui_lblTime, timeStr);
  }
  // Process fix stats and number of satellites and update labels
  if (incomingData.fixStatus != 0) {

    lv_obj_clear_flag(ui_imgGPSFix, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_lblNumSats, LV_OBJ_FLAG_HIDDEN);

    char satsBuf[10];  // Make sure this buffer stays in scope as long as you need numSats
    snprintf(satsBuf, sizeof(satsBuf), "%d", incomingData.numSats);
    const char* numSats = satsBuf;
    lv_label_set_text(ui_lblNumSats, numSats);
  } else {

    lv_obj_add_flag(ui_imgGPSFix, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_lblNumSats, LV_OBJ_FLAG_HIDDEN);
  }
  
  // Process work switch state and update "LED" on screen
  if (incomingData.workSwitch) {
    lv_obj_add_state(ui_workLED, LV_STATE_USER_1);
  } else {
    lv_obj_clear_state(ui_workLED, LV_STATE_USER_1);
  }

  // Handle the paring mode, should probably end up in comms.cpp
  if (pairingMode) {
      unsigned long now = millis();
      if (now - lastPairingTime >= 500) {
          lastPairingTime = now;
          sendPairingRequest();  // broadcast pairing request
      }
  }

  // put the incoming seedPerRev value in to the label on Cal2
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%.2f", incomingData.seedPerRev);  // format with 2 decimal places
  lv_label_set_text(ui_lblSeedPerRev, buffer);

  // change the rate label to yellow if the rate is out of bounds +/- 25%
  if (incomingData.rateOutOfBounds) {
    lv_obj_set_style_text_color(ui_lblRate, lv_color_hex(0xffb81d), LV_PART_MAIN);
  } else {
    lv_obj_set_style_text_color(ui_lblRate, lv_color_hex(0xffffff), LV_PART_MAIN);
  }

  } // end newData
} // end loop

// LVGL calls this function to print log information
void my_print(lv_log_level_t level, const char *buf)
{
  LV_UNUSED(level);
  DBG_PRINTLN(buf);
  Serial.flush();
}

// LVGL calls this function to retrieve elapsed time
uint32_t millis_cb(void)
{
  return millis();
}

// LVGL calls this function when a rendered image needs to copied to the display
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);

  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);

  lv_disp_flush_ready(disp);
}

// LVGL calls this function to read the touchpad
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  // Update the touch data from the GT911 touch controller
  touchController.read();

  // If a touch is detected, update the LVGL data structure with the first point's coordinates.
  if (touchController.isTouched && touchController.touches > 0)
  {
    data->point.x = touchController.points[0].x;
    data->point.y = touchController.points[0].y;
    data->state = LV_INDEV_STATE_PRESSED; // Touch is pressed
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED; // No touch detected
  }
}

/* if (incomingData.errorRaised) {


  if (incomingData.errorCode != errorCode) {
    warningAck = false;
    DBG_PRINTLN(warningAck);
  }

  errorCode = incomingData.errorCode;
  DBG_PRINT(" errorCode: ");
  DBG_PRINTLN(errorCode);

  cooldownExpired = (millis() - warningCooldownStart >= warningCooldownDuration);

  if (!warningAck && cooldownExpired) {
    if (errorCode == 1) {
      lv_obj_remove_flag(ui_panelWarning, LV_OBJ_FLAG_HIDDEN);
      lv_label_set_text(ui_lblWarningMessage, "Minimum PWM\nthreshold.\nRate can not\nbe maintained.");
      player.play("/warning.wav");
    } else if (errorCode == 2) {
      lv_obj_remove_flag(ui_panelWarning, LV_OBJ_FLAG_HIDDEN);
      lv_label_set_text(ui_lblWarningMessage, "Maximum PWM\nthreshold.\nRate can not\nbe maintained.");
      player.play("/warning.wav");
    } else if (errorCode == 3) {
      lv_obj_remove_flag(ui_panelStop, LV_OBJ_FLAG_HIDDEN);
      player.play("/stop.wav");
    }

    warningAck = true;
  } 
} else {
  //errorRaised = false;
  warningAck = false;
  errorAck = false;
  outgoingData.errorAck = false;
  cooldownExpired = false;
  
  */


/*  DBG_PRINTLN("=== Received Data ===");
    DBG_PRINT("Fix: "); DBG_PRINTLN(incomingData.fixStatus);
    DBG_PRINT("Sats: "); DBG_PRINTLN(incomingData.numSats);
    DBG_PRINT("Speed: "); DBG_PRINTLN(incomingData.gpsSpeed);
    Serial.printf("Time: %02d:%02d:%02d\n", incomingData.gpsHour, incomingData.gpsMinute, incomingData.gpsSecond);
    DBG_PRINT("Revs: "); DBG_PRINTLN(incomingData.calibrationRevs);
    DBG_PRINT("WorkSwitch: "); DBG_PRINTLN(incomingData.workSwitch);
    DBG_PRINT("Motor: "); DBG_PRINTLN(incomingData.motorActive);
    DBG_PRINT("RPM: "); DBG_PRINTLN(incomingData.shaftRPM);
    DBG_PRINT("Error: "); DBG_PRINTLN(incomingData.errorCode); */