#include <Arduino.h>
#include <Preferences.h>
#include "prefs.h"
#include "globals.h"
#include "comms.h"

Preferences prefs;

bool prefsValid = false;

void loadPrefs() {

    prefs.begin("valmar_screen", false);

    prefsValid = prefs.getBool("prefsValid", false);

    if (prefsValid) {

    }

    prefs.end();
}

void savePrefs() {

}

void clearPrefs() {

}