#include "WAVPlayer.h"

WAVPlayer::WAVPlayer(int ws_pin, int bck_pin, int do_pin, i2s_port_t port) {
    i2s_ws_pin = ws_pin;
    i2s_bck_pin = bck_pin;
    i2s_do_pin = do_pin;
    i2s_port = port;
}

void WAVPlayer::audioTaskWrapper(void* parameter) {
    ((WAVPlayer*)parameter)->audioTask();
}

void WAVPlayer::audioTask() {
    const size_t bufferSize = 1024;
    uint8_t* buffer = (uint8_t*)malloc(bufferSize);
    size_t bytesWritten;
    
    if (!buffer) {
        Serial.println("Failed to allocate audio buffer");
        isPlaying = false;
        vTaskDelete(NULL);
        return;
    }
    
    Serial.println("Audio playback started");
    
    while (audioFile.available() && !shouldStop) {
        size_t bytesRead = audioFile.read(buffer, bufferSize);
        if (bytesRead > 0) {
            // Convert 8-bit to 16-bit if needed
            if (header.bitsPerSample == 8) {
                convertTo16Bit(buffer, bytesRead);
            }
            
            esp_err_t result = i2s_write(i2s_port, buffer, bytesRead, &bytesWritten, portMAX_DELAY);
            if (result != ESP_OK) {
                Serial.printf("I2S write error: %d\n", result);
                break;
            }
        }
        // Small delay to prevent watchdog issues
        vTaskDelay(1);
    }
    
    audioFile.close();
    free(buffer);
    isPlaying = false;
    shouldStop = false;
    
    Serial.println("Audio playback finished");
    vTaskDelete(NULL);
}

void WAVPlayer::convertTo16Bit(uint8_t* buffer, size_t length) {
    // Convert 8-bit unsigned to 16-bit signed
    for (int i = length - 1; i >= 0; i--) {
        int16_t sample = (buffer[i] - 128) * 256;
        ((int16_t*)buffer)[i] = sample;
    }
}

bool WAVPlayer::parseWAVHeader(File& file) {
    if (file.read((uint8_t*)&header, sizeof(WAVHeader)) != sizeof(WAVHeader)) {
        return false;
    }
    
    // Basic validation
    if (strncmp(header.riff, "RIFF", 4) != 0 || 
        strncmp(header.wave, "WAVE", 4) != 0 ||
        strncmp(header.data, "data", 4) != 0) {
        return false;
    }
    
    Serial.printf("WAV Info: %dHz, %d channels, %d bits\n", 
                 header.sampleRate, header.numChannels, header.bitsPerSample);
    
    return true;
}

bool WAVPlayer::begin() {
    // I2S configuration
    i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,  // Will be updated per file
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    // I2S pin configuration  
    i2s_pin_config_t pin_config = {
        .bck_io_num = i2s_bck_pin,
        .ws_io_num = i2s_ws_pin,
        .data_out_num = i2s_do_pin,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    
    esp_err_t result = i2s_driver_install(i2s_port, &i2s_config, 0, NULL);
    if (result != ESP_OK) {
        Serial.printf("Failed to install I2S driver: %d\n", result);
        return false;
    }
    
    result = i2s_set_pin(i2s_port, &pin_config);
    if (result != ESP_OK) {
        Serial.printf("Failed to set I2S pins: %d\n", result);
        return false;
    }
    
    Serial.println("I2S Audio initialized");
    return true;
}

bool WAVPlayer::play(const char* filename) {
    if (isPlaying) {
        Serial.println("Already playing audio");
        return false;
    }
    
    audioFile = LittleFS.open(filename, "r");
    if (!audioFile) {
        Serial.printf("Failed to open audio file: %s\n", filename);
        return false;
    }
    
    if (!parseWAVHeader(audioFile)) {
        Serial.println("Invalid WAV file format");
        audioFile.close();
        return false;
    }
    
    // Update I2S sample rate to match file
    i2s_set_sample_rates(i2s_port, header.sampleRate);
    
    isPlaying = true;
    shouldStop = false;
    
    // Create audio playback task
    BaseType_t result = xTaskCreatePinnedToCore(
        audioTaskWrapper,
        "AudioTask",
        8192,           // Stack size
        this,           // Parameter
        5,              // Priority  
        &playTask,      // Task handle
        1               // Core (use core 1, leaving core 0 for main tasks)
    );
    
    if (result != pdPASS) {
        Serial.println("Failed to create audio task");
        audioFile.close();
        isPlaying = false;
        return false;
    }
    
    return true;
}

void WAVPlayer::stop() {
    if (isPlaying) {
        shouldStop = true;
        // Wait for task to finish
        while (isPlaying) {
            vTaskDelay(10);
        }
    }
}

bool WAVPlayer::playing() {
    return isPlaying;
}

void WAVPlayer::setVolume(float volume) {
    // Volume control would need to be done in software
    // by scaling the audio samples, or with external hardware
    // The NS4168 doesn't have built-in volume control
}