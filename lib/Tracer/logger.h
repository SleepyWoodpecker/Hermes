#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdint.h>
#include <string.h>

#include <Arduino.h>

#include "tracer_types.h"

class TraceLogger {
    public:
        virtual void log_raw(uint8_t *data, size_t length) = 0;
        virtual void log_pretty(TraceEntry_t *data) = 0;
};

class SerialLogger : public TraceLogger {
    public:
        SerialLogger(const char *control_sequence);

        bool begin();
        void log_raw(uint8_t *data, size_t length);
        void log_pretty(TraceEntry_t *data);

    private:
        // require a control sequence to ensure proper syncing with serial port
        const char *_control_sequence;
        size_t _control_sequence_len;

};

#endif