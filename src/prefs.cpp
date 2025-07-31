#include <Arduino.h>
#include <Preferences.h>
#include "prefs.h"
#include "globals.h"
#include "comms.h"
#include "ui.h"

Preferences prefs;

bool prefsValid = false;
bool commsValid = false;

void loadComms() {

    prefs.begin("cntlr_comms", true);

    commsValid = prefs.getBool("commsValid", false);

    if (commsValid) {

        controllerPaired = prefs.getBool("cntlrPaired", false);
        prefs.getBytes("cntlrAddress", controllerAddress, 6);
        DBG_PRINTLN("Loaded comms values from NVS");

    } else {

        DBG_PRINTLN("No stored comms values in NVS.  Using broadcast as defualt.");
    }

    prefs.end();
}

void saveComms() {

    prefs.begin("cntlr_comms", false);

    if (controllerPaired) {

        prefs.putBool("cntlrPaired", controllerPaired);
        prefs.putBytes("cntlrAddress", controllerAddress, 6);
        prefs.putBool("commsValid", true);
        DBG_PRINTLN("Comms settings saved to NVS.");
    } else {
        DBG_PRINTLN("No controller paired.  Can't save.");
    }

    prefs.end();

}

void clearComms() {

    prefs.begin("cntlr_comms", false);

    prefs.clear();

    prefs.end();

    DBG_PRINTLN("Comms cleared.");

}
void loadPrefs() {

    prefs.begin("valmar_screen", false);

    prefsValid = prefs.getBool("prefsValid", false);

    if (prefsValid) {

        DBG_PRINTLN("Valid prefs found.  loading...");
        lv_spinbox_set_value(ui_spinboxRate, prefs.getInt("targetRate", 0));
        lv_spinbox_set_value(ui_spnWidth, prefs.getInt("workingWidth", 600));
        lv_spinbox_set_value(ui_spnRuns, prefs.getInt("runs", 8));
        lv_slider_set_value(ui_sldBrightness, prefs.getInt("brightness", 200), LV_ANIM_OFF);

        lv_obj_set_style_border_color(ui_spinboxRate, lv_color_hex(0x7B1008), LV_PART_CURSOR | LV_STATE_DEFAULT);

        outgoingData.seedingRate = prefs.getInt("targetRate", 0) / 10;
        outgoingData.workingWidth = prefs.getInt("workingWidth", 0) / 10;
        outgoingData.numberOfRuns = prefs.getInt("runs", 0);

        savedBrightness = prefs.getInt("brightness", 200);

        ledcWrite(BACKLIGHT_CH, savedBrightness);

    } else {
        DBG_PRINTLN("Valid prefs not found.");
    }

    prefs.end();
}

void savePrefs() {

    prefs.begin("valmar_screen", false);

    prefs.putInt("targetRate", lv_spinbox_get_value(ui_spinboxRate));
    prefs.putInt("workingWidth", lv_spinbox_get_value(ui_spnWidth));
    prefs.putInt("runs", lv_spinbox_get_value(ui_spnRuns));
    prefs.putInt("brightness", lv_slider_get_value(ui_sldBrightness));

    prefs.putBool("prefsValid", true);

    prefs.end();
    
    DBG_PRINTLN("Prefs saved.");
}

void saveRate() {

    prefs.begin("valmar_screen", false);

    if (prefs.getInt("targetRate", 0) != lv_spinbox_get_value(ui_spinboxRate)) {
        prefs.putInt("targetRate", lv_spinbox_get_value(ui_spinboxRate));
        DBG_PRINTLN("Rate changed from stored value.  Storing new value.");
    } else {
        DBG_PRINTLN("Rate unchanged from saved value.  Not saving.");
    }

    lv_obj_set_style_border_color(ui_spinboxRate, lv_color_hex(0x7B1008), LV_PART_CURSOR | LV_STATE_DEFAULT);
    savePending = false;

}

void clearPrefs() {

    prefs.begin("valmar_screen", true);

    prefs.clear();

    prefs.end();

    DBG_PRINTLN("Prefs cleared.");
}