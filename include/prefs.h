#ifndef PREFS_H
#define PREFS_H

extern bool prefsValid;
extern bool commsValid;

void loadComms();
void saveComms();
void clearComms();

void loadPrefs();
void savePrefs();
void clearPrefs();

#endif