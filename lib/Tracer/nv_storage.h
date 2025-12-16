#ifndef NV_STORAGE_H_
#define NV_STORAGE_H_

#include "FS.h"
#include "LittleFS.h"

#include "tracer_types.h"

class NVStorage {
    public:
        NVStorage();
        bool read(NVTraceEntry_t &entry);
        bool write(NVTraceEntry_t &entry);
        bool begin();
    private:
        const char *_nvs_filepath = "/crash_data.bin";
};

#endif