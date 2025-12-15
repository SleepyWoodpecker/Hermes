#include "logger.h"

SerialLogger::SerialLogger(const char *control_sequence)
    :_control_sequence(control_sequence), _control_sequence_len(strlen(control_sequence))
{
}

bool SerialLogger::begin() {
    return Serial;
}

void SerialLogger::log_raw(uint8_t *data, size_t length) {
    Serial.write(data, length);
    Serial.write(_control_sequence, _control_sequence_len);
}

void SerialLogger::log_pretty(TraceEntry_t *data) {
    TraceEntry_t *entry = (TraceEntry_t *)data;

    // 1. Print Common Header (Timestamp, Core, ID)
    // Using simple format: [Time] Core:ID | Type
    Serial.printf("[%" PRIu32 "] Core:%ld ID:%" PRIu32 " | ", 
        entry->timestamp, 
        (long)entry->core_id, 
        entry->trace_id
    );

    switch (entry->trace_type) {
        case ENTER: {
            const TraceFunctionEntry_t& fn = entry->function_entry;
            Serial.printf("ENTER: %s(", fn.func_name);

            // Loop through arguments
            for (int i = 0; i < fn.arg_count; i++) {
                // Note: We are printing raw hex because we don't know 
                // how to decode 'val_types' yet.
                Serial.printf("0x%08" PRIx32, fn.func_arguments[i]);
                
                if (i < fn.arg_count - 1) {
                    Serial.printf(", ");
                }
            }
            Serial.printf(")\n");
            break;
        }

        case EXIT: {
            const TraceFunctionEntry_t& fn = entry->function_entry;
            Serial.printf("EXIT : %s | Return: 0x%08" PRIx32, 
                   fn.func_name, 
                   fn.return_val);

            #ifdef TRACER_TRACE_STACK
            Serial.printf(" | Stack Left: %ld bytes", (long)fn.stack_space_left);
            #endif
            
            Serial.printf("\n");
            break;
        }

        case PANIC: {
            const TracePanicEntry_t& panic = entry->panic_entry;
            Serial.printf("PANIC: PC:0x%08" PRIx32 " Reason: %s\n", 
                   panic.faulting_pc, 
                   panic.exception_reason);
            break;
        }

        default:
            Serial.printf("UNKNOWN EVENT TYPE\n");
            break;
    }
}