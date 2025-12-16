#include "nv_storage.h"

NVStorage::NVStorage() {}

bool NVStorage::begin() {
    while (!LittleFS.begin(true)) {
        delay(100);
        Serial.println("Could not start littleFS...");
    }

    File file = LittleFS.open(_nvs_filepath, "r", false);
    
    if (!file) {
        File file = LittleFS.open(_nvs_filepath, "w", true);
    }

    return true;
}

bool NVStorage::write(NVTraceEntry_t &entry) {
    File file = LittleFS.open(_nvs_filepath, "w", false);

    if (!file) {
        return false;
    }

    file.write(reinterpret_cast<uint8_t *>(&entry), sizeof(entry));
    return true;
}

bool NVStorage::read(NVTraceEntry_t &entry) {
    File file = LittleFS.open(_nvs_filepath, "r", false);
    
    if (!file) {
        return false;
    }

    file.readBytes((char *)&entry, NV_TRACE_SIZE);
    return true;
}