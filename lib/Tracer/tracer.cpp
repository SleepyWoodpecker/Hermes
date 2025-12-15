#include <string.h>

#include "tracer.h"

Tracer::Tracer(TraceLogger &logger, int queue_size) {
    if (!_task_message_queue) {
        _task_message_queue = xQueueCreate(queue_size, sizeof(TraceEntry_t));
        assert(_task_message_queue != NULL && "Task message queue could not be created");
    }

    if (!_logger) {
        _logger = &logger;
    }

    // register the panic handler
    set_arduino_panic_handler(static_panic_handler, NULL);
}

// TODO: log_raw will not work for this
void Tracer::trace_application_exception(arduino_panic_info_t *info, void *arg) {
    TraceEntry_t entry = {
        .trace_type = Event_t::PANIC,
        .core_id = info->core,
        .timestamp = millis(),
        .trace_id = _current_trace_id++
    }; 
    
    entry.panic_entry.faulting_pc = reinterpret_cast<uint32_t>(info->pc);
    strlcpy(entry.panic_entry.exception_reason, info->reason, MAX_EXCEPTION_STRING_LENGTH);

    // do a non-blocking send
    BaseType_t wake = false;
    xQueueSendFromISR(
        _task_message_queue,
        &entry,
        &wake
    );
}

void *Tracer::log_traces(void *args) {
    TraceEntry_t entry = {};
    for (;;) {
        if (xQueueReceive(_task_message_queue, &entry, portMAX_DELAY) == pdTRUE) {
            // _logger->log_raw(reinterpret_cast<uint8_t *>(&entry), sizeof(entry));
            _logger->log_pretty(&entry);
        }
    }
}