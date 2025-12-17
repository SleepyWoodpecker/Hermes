#include <string.h>

#include "tracer.h"

Tracer::Tracer(TraceLogger &logger, int queue_size) 
    :_logger(&logger)
{
    configASSERT(_instance == nullptr && "Should only have one tracer!");

    if (!_task_message_queue) {
        _task_message_queue = xQueueCreate(queue_size, sizeof(TraceEntry_t));
        configASSERT(_task_message_queue != NULL && "Task message queue could not be created");
    }

    _instance = this;
}

void Tracer::trace_application_exception(arduino_panic_info_t *info, void *arg) {
    TraceEntry_t entry = {};
    entry.trace_type = Event_t::PANIC;
    entry.core_id = info->core;
    entry.timestamp = millis();
    entry.trace_id = _current_trace_id++; 
    
    entry.panic_entry.faulting_pc = reinterpret_cast<uint32_t>(info->pc);
    strlcpy(entry.panic_entry.exception_reason, info->reason, MAX_EXCEPTION_STRING_LENGTH);

    _logger->log_raw(reinterpret_cast<uint8_t *>(&entry), sizeof(entry));
}

void *Tracer::log_traces(void *args) {
    TraceEntry_t entry = {};
    for (;;) {
        if (xQueueReceive(_task_message_queue, &entry, portMAX_DELAY) == pdTRUE) {
            _logger->log_raw(reinterpret_cast<uint8_t *>(&entry), sizeof(entry));
            // _logger->log_pretty(entry);
        }
    }
}