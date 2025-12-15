#ifndef TRACER_TYPES_H_
#define TRACER_TYPES_H_

#include <stdint.h>
#include <freertos/FreeRTOS.h>

#define FUNC_NAME_MAX_SIZE          16
#define MAX_NO_ARGUMENTS_TRACKED    4
#define MAX_EXCEPTION_STRING_LENGTH 64

#define FIRST_ARG_IS_FLOAT          (1UL << 0)
#define FIRST_ARG_IS_UNSIGNED       (1UL << 1)

#define SECOND_ARG_IS_FLOAT         (1UL << 2)
#define SECOND_ARG_IS_UNSIGNED      (1UL << 3)

#define THIRD_ARG_IS_FLOAT          (1UL << 4)
#define THIRD_ARG_IS_UNSIGNED       (1UL << 5)

#define FOURTH_ARG_IS_FLOAT         (1UL << 6)
#define FOURTH_ARG_IS_UNSIGNED      (1UL << 7)

#define RETURN_VAL_IS_FLOAT         (1UL << 0)
#define RETURN_VAL_IS_UNSIGNED      (1UL << 1)

// tracer types
enum Event_t {
    ENTER,
    EXIT,
    PANIC
};

struct TraceFunctionEntry_t {
    // flag to mark the type information for return_val OR func_arguments
    uint8_t val_types = {0};
    uint8_t arg_count;
    #ifdef TRACER_TRACE_STACK
    BaseType_t stack_space_left
    #endif
    union {
        uint32_t return_val;
        uint32_t func_arguments[MAX_NO_ARGUMENTS_TRACKED];
    };
    char func_name[FUNC_NAME_MAX_SIZE];
};

struct TracePanicEntry_t {
    uint32_t faulting_pc;
    char exception_reason[MAX_EXCEPTION_STRING_LENGTH];
};

struct TraceEntry_t {
    Event_t trace_type;
    BaseType_t core_id;
    uint32_t timestamp;
    uint32_t trace_id;
    union {
        TraceFunctionEntry_t function_entry;
        TracePanicEntry_t panic_entry;
    };
};

struct ConversionResult {
    uint32_t result;
    bool is_float = {false};
    bool is_unsigned = {false};
};

#endif