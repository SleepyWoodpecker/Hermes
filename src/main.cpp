#include <Arduino.h>

#include "tracer.h"

SerialLogger logger("\r\n");
Tracer tracer(logger, 100);

int try_add(int a, int b) {
    return TRACE_BLOCK({
        int ret_value = a + b;

        return ret_value;
    }, a, b);
}

void setup() {
    Serial.begin(115200);

    if (!logger.begin()) {
        Serial.printf("Welp!");
    }

    delay(2.5);

    // register the panic handler
    set_arduino_panic_handler(Tracer::static_panic_handler, (void *)&tracer);

    xTaskCreatePinnedToCore(
        Tracer::static_log_traces,
        "logging task",
        2048,
        &tracer,
        1,
        NULL,
        0
    );
}

void loop() {
    int res = try_add(3, 4);
    res = try_add(4, 5);

    delay(1000);

    // int *p = NULL;
    // *p = 42;
}