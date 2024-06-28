#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pico/stdio.h>
#include "port_common.h"

#define HIGH 1
#define LOW 0

#define O2_VALVE 9
#define N2O_FILL_VALVE 10
#define N2O_DUMP_VALVE 11

#define INDICATOR_O2_VALVE 11
#define INDICATOR_N2O_FILL_VALVE 12
#define INDICATOR_N2O_DUMP_VALVE 13


bool onFillSequence(){
    gpio_put(N2O_FILL_VALVE, HIGH);
    gpio_put(INDICATOR_N2O_FILL_VALVE, HIGH);
    return true;
}

bool offFillSequence(){
    gpio_put(N2O_FILL_VALVE, LOW);
    gpio_put(INDICATOR_N2O_FILL_VALVE, LOW);
    return true;
}

bool onDumpSequence(){
    gpio_put(N2O_DUMP_VALVE, HIGH);
    gpio_put(INDICATOR_N2O_DUMP_VALVE, HIGH);
    gpio_put(N2O_FILL_VALVE, LOW);
    gpio_put(INDICATOR_N2O_FILL_VALVE, LOW);
    return true;
}

bool offDumpSequence(){
    gpio_put(N2O_DUMP_VALVE, LOW);
    gpio_put(INDICATOR_N2O_DUMP_VALVE, LOW);
    gpio_put(N2O_FILL_VALVE, LOW);
    gpio_put(INDICATOR_N2O_FILL_VALVE, LOW);
    return true;
}

bool onPurgeSequence(){
    gpio_put(N2O_DUMP_VALVE, HIGH);
    gpio_put(INDICATOR_N2O_DUMP_VALVE, HIGH);
    return true;
}

bool offPurgeSequence(){
    gpio_put(N2O_DUMP_VALVE, LOW);
    gpio_put(INDICATOR_N2O_DUMP_VALVE, LOW);
    return true;
}

bool onIgnitionSequence(){
    gpio_put(O2_VALVE, HIGH);
    gpio_put(INDICATOR_O2_VALVE, HIGH);
    return true;
}

bool offIgnitionSequence(){
    gpio_put(O2_VALVE, LOW);
    gpio_put(INDICATOR_O2_VALVE, LOW);
    return true;
}
