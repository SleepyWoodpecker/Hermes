#include <string.h>

#include "tracer.h"

Tracer::Tracer(TraceLogger &logger, NVStorage &storage, int queue_size) 
    :_logger(&logger), _storage(&storage)
{
    if (!_task_message_queue) {
        _task_message_queue = xQueueCreate(queue_size, sizeof(TraceEntry_t));
        configASSERT(_task_message_queue != NULL && "Task message queue could not be created");
    }
}

void Tracer::trace_application_exception(arduino_panic_info_t *info, void *arg) {
    NVTraceEntry_t entry = { 0 };
    entry.panic_entry.trace_type = Event_t::PANIC;
    entry.panic_entry.core_id = info->core;
    entry.panic_entry.timestamp = millis();
    entry.panic_entry.trace_id = _current_trace_id++; 
    
    entry.panic_entry.panic_entry.faulting_pc = reinterpret_cast<uint32_t>(info->pc);
    strlcpy(entry.panic_entry.panic_entry.exception_reason, info->reason, MAX_EXCEPTION_STRING_LENGTH);

    entry.has_unread_panic_entry = true;
    _logger->log_pretty(entry.panic_entry);
}

void *Tracer::log_traces(void *args) {
    TraceEntry_t entry = {};
    for (;;) {
        if (xQueueReceive(_task_message_queue, &entry, portMAX_DELAY) == pdTRUE) {
            // _logger->log_raw(reinterpret_cast<uint8_t *>(&entry), sizeof(entry));
            _logger->log_pretty(entry);
        }
    }
}

void Tracer::check_and_log_previous_panic() {
    NVTraceEntry_t entry = {};
    if (!_storage->read(entry) || !entry.has_unread_panic_entry) {
        Serial.println("Found no previous panic entry");
        return;
    }

    _logger->log_pretty(entry.panic_entry);

    entry.has_unread_panic_entry = false;
    _storage->write(entry);
}