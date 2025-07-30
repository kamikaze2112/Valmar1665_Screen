#ifndef WAVPLAYER_H
#define WAVPLAYER_H

#include "FS.h"
#include "LittleFS.h"
#include "driver/i2s.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void setupAudio();
void handleAudio();

// WAV file header structure
struct WAVHeader {
    char riff[4];
    uint32_t fileSize;
    char wave[4];
    char fmt[4];
    uint32_t fmtSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char data[4];
    uint32_t dataSize;
};

class WAVPlayer {
private:
    TaskHandle_t playTask = NULL;
    File audioFile;
    WAVHeader header;
    bool isPlaying = false;
    bool shouldStop = false;
    
    // I2S pins - set these in the constructor or begin()
    int i2s_ws_pin;
    int i2s_bck_pin;
    int i2s_do_pin;
    i2s_port_t i2s_port;
    
    static void audioTaskWrapper(void* parameter);
    void audioTask();
    void convertTo16Bit(uint8_t* buffer, size_t length);
    bool parseWAVHeader(File& file);
    
public:
    WAVPlayer(int ws_pin, int bck_pin, int do_pin, i2s_port_t port = I2S_NUM_0);
    
    bool begin();
    bool play(const char* filename);
    void stop();
    bool playing();
    void setVolume(float volume);
};

#endif