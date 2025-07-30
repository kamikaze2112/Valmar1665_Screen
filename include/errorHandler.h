#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

void raiseError(int code);
void clearError();
void displayWarning();
void acknowledgeWarning();
void handleErrorTimer();

extern bool errorTimer;

#endif