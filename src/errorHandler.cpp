#include <Arduino.h>
#include "errorHandler.h"
#include "globals.h"
#include "ui.h"
#include "comms.h"

unsigned long lastTime = 0;
const unsigned long stopInterval = 5000; // 5 seconds
const unsigned long warningInterval = 10000; // 10 seconds;
bool errorTimer = false;

void raiseError(int code){

    errorRaised = true;
    errorCode = code;
    displayWarning();

DBG_PRINTLN("raiseError");
DBG_PRINT("errorRaised: ");
DBG_PRINT(incomingData.errorRaised);
DBG_PRINT(" Code: ");
DBG_PRINT(incomingData.errorCode);
DBG_PRINT(" errorAck: ");
DBG_PRINTLN(outgoingData.errorAck);

}

void clearError() {

    errorRaised = false;
    errorCode = 0;
    errorTimer = false;

DBG_PRINTLN("clearError");
DBG_PRINT("errorRaised: ");
DBG_PRINT(incomingData.errorRaised);
DBG_PRINT(" Code: ");
DBG_PRINT(incomingData.errorCode);
DBG_PRINT(" errorAck: ");
DBG_PRINTLN(outgoingData.errorAck);
}

void displayWarning() {

    if (errorCode == 1) {
        lv_obj_clear_flag(ui_panelWarning, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(ui_lblWarningMessage, "Minimum PWM\nthreshold.\nRate can not\nbe maintained.");
        player.play("/warning.wav");
    } else if (errorCode == 2) {
        lv_obj_clear_flag(ui_panelWarning, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(ui_lblWarningMessage, "Maximum PWM\nthreshold.\nRate can not\nbe maintained.");
        player.play("/warning.wav");
    } else if (errorCode == 3) {
        lv_obj_add_flag(ui_panelWarning, LV_OBJ_FLAG_HIDDEN);
        player.stop();
        errorTimer = false;
        lv_obj_clear_flag(ui_panelStop, LV_OBJ_FLAG_HIDDEN);
        player.play("/stop.wav");
    }
}

void acknowledgeWarning() {

    if (errorCode == 0) {
        lv_obj_add_flag(ui_panelWarning, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_panelStop, LV_OBJ_FLAG_HIDDEN);
        player.stop();
        errorTimer = false;
    } else if (errorCode == 1 || errorCode == 2) {
        lv_obj_add_flag(ui_panelWarning, LV_OBJ_FLAG_HIDDEN);
        lastTime = millis();
        errorTimer = true;
    } else if (errorCode == 3 && !incomingData.workSwitch) {
        lv_obj_add_flag(ui_panelStop, LV_OBJ_FLAG_HIDDEN);
        player.stop();
        outgoingData.errorAck = true;

        for (int i = 0; i < 3; i++) {
        outgoingData.type = PACKET_TYPE_DATA;
        esp_now_send(controllerAddress, (uint8_t *)&outgoingData, sizeof(outgoingData));                
        delay(5);
        }

        outgoingData.errorAck = false;

    }
}

void handleErrorTimer() {

    if (errorCode == 1 || errorCode == 2) {
    
        if (errorRaised && errorTimer) {
            unsigned long currentTime = millis();
        
            if (currentTime - lastTime >= warningInterval) {
                lv_obj_clear_flag(ui_panelWarning, LV_OBJ_FLAG_HIDDEN);
                player.play("/warning.wav");
                lastTime = currentTime; // Reset timer
                errorTimer = false;
            }
        }
    } else if (errorCode == 3) {
        unsigned long currentTime = millis();
        
        if (currentTime - lastTime >= stopInterval) {
            player.play("/stop.wav");
            lastTime = currentTime; // Reset timer
        }
    }
}