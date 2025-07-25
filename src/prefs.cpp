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

}
void loadPrefs() {

    prefs.begin("valmar_screen", false);

    prefsValid = prefs.getBool("prefsValid", false);

    if (prefsValid) {

    }

    prefs.end();
}

void savePrefs() {

    prefs.begin("valmar_screen", false);

    prefs.putFloat("targetRate", seedingRate);
    prefs.putFloat("workingWidth", workingWidth);
    prefs.putInt("runs", numberOfRuns);
    prefs.putInt("brightness", lv_slider_get_value(ui_sldBrightness));
}

void clearPrefs() {

}