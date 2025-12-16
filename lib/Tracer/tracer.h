#ifndef TRACER_H_
#define TRACER_H_

#include <stdint.h>
#include <type_traits>
#include <cstdarg>

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "esp_rom_sys.h" // For ets_printf

#include "tracer_types.h"
#include "logger.h"
#include "nv_storage.h"

class Tracer
{
public:
    /**
     * @brief: initialize tracer with queue sizem, timer interrupt interval as well as method of transporting logs out of system
     */
    Tracer(TraceLogger &logger, NVStorage &storage, int queue_size = 100);

    /**
     * @brief: convert any generic argument into a uint32_t so that it can be stored in TraceEntry
     */
    template <typename T>
    void convert_argument(T arg, ConversionResult& conversion_result);

    /**
     * @brief: accept a variadic number of arguments and fill out conversion_result with input
     */
    template <typename... Args>
    void fill_args(TraceFunctionEntry_t &entry, Args... args);

    /**
     * @brief: trace entrypoint into a function
     */
    template <typename... Args>
    void trace_function_entry(const char* func_name, Args... args);

    /**
     * @brief: trace function exit
     */
    template <typename T>
    void trace_function_exit(const char* func_name, T return_val);

    /**
     * @brief: trace application exception, flush right after
     */
    void trace_application_exception(arduino_panic_info_t *info, void *arg);

    /**
     * @brief: trampoline function to register as the panic handler
     */
    static void static_panic_handler(arduino_panic_info_t *info, void *arg) {
        if (arg == nullptr) return;

        Tracer *instance = static_cast<Tracer*>(arg);

        instance->trace_application_exception(info, NULL);
    }

    /**
     * @brief: consumer task for tracer
     */
    void *log_traces(void *args);

    static void static_log_traces(void *args) {
        Tracer *instance = static_cast<Tracer *>(args);
        instance->log_traces(NULL);
    }

    void check_and_log_previous_panic();

private:
    QueueHandle_t _task_message_queue = NULL;
    uint32_t _current_trace_id = {0};

    TraceLogger *_logger = nullptr;
    NVStorage *_storage = nullptr;
};

template <typename T>
void Tracer::convert_argument(T arg, ConversionResult &converstion_result) {
    T actual_val = arg;
    
    if constexpr (std::is_pointer<T>()) {
        actual_val = *arg;
    }

    if constexpr (std::is_floating_point<T>()) {
        converstion_result.is_float = true;
        if (sizeof(T) > sizeof(float)) {
            actual_val = static_cast<float>(actual_val);
        }
    } else if constexpr (std::is_signed<T>()) {
        converstion_result.is_unsigned = true;
    }

    memcpy(&converstion_result.result, &arg, sizeof(uint32_t));    
}

template <typename... Args>
void Tracer::fill_args(TraceFunctionEntry_t& entry, Args... args) {
    int no_of_args_to_trace = MAX_NO_ARGUMENTS_TRACKED < (sizeof...(args)) ? MAX_NO_ARGUMENTS_TRACKED : (sizeof...(args));
    int i = 0;

    // makes use of a fold expression to extract up to 4 arguments
    ([&]
    {
        if (i >= no_of_args_to_trace) return;
        ConversionResult res;
        convert_argument(args, res);
        
        entry.func_arguments[i] = res.result;
        entry.val_types |= (res.is_unsigned << (i * 2 + 1)) | (res.is_float << (i * 2));
        ++i;
    }
    (), ...);
    
    entry.arg_count = no_of_args_to_trace;
}

template <typename... Args>
void Tracer::trace_function_entry(const char* func_name, Args... args) {
    TraceEntry_t entry = {
        .trace_type = Event_t::ENTER,
        .core_id = xTaskGetCoreID(NULL),
        .timestamp = millis(),
        .trace_id = _current_trace_id++
    };
    
    strlcpy(entry.function_entry.func_name, func_name, FUNC_NAME_MAX_SIZE);

    fill_args(entry.function_entry, args...);
    
    // do a non-blocking send
    BaseType_t wake = false;
    xQueueSendFromISR(
        _task_message_queue,
        &entry,
        &wake
    );
}

template <typename T>
void Tracer::trace_function_exit(const char* func_name, T return_val) {
    TraceEntry_t entry = {
        .trace_type = Event_t::EXIT,
        .core_id = xTaskGetCoreID(NULL),
        .timestamp = millis(),
        .trace_id = _current_trace_id++
    };    
    strlcpy(entry.function_entry.func_name, func_name, FUNC_NAME_MAX_SIZE);

    ConversionResult r;
    convert_argument(return_val, r);

    entry.function_entry.return_val = r.result;
    entry.function_entry.val_types = (r.is_float) | (r.is_unsigned << 1);

    // do a non-blocking send
    BaseType_t wake = false;
    xQueueSendFromISR(
        _task_message_queue,
        &entry,
        &wake
    );
}


#endif