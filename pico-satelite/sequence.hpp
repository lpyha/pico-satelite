/**
 * @file sequence.hpp
 * @brief valveのON/OFFシーケンスを定義したファイル
 * @author Murakami Kantaro
 * @date 2024-07-01 
 */
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

const uint32_t HEADER_FILL      = 0xFFFFFFF0;
const uint32_t HEADER_DUMP      = 0xFFFFFFF1;
const uint32_t HEADER_PURGE     = 0xFFFFFFF2;
const uint32_t HEADER_IGNITION  = 0xFFFFFFF3;
const uint32_t COMMAND_CLOSE    = 0x00000000;
const uint32_t COMMAND_STATUS   = 0x00000001;
const uint32_t COMMAND_OPEN     = 0x00000002;
const uint32_t COMMAND_ERORR    = 0x99999999;

/**
 * @brief Fill操作、N2O main Valve (Fill Valve)をOPENにする
 * @return COMMAND_OPEN
 */
uint32_t onFillSequence(){
    gpio_put(N2O_FILL_VALVE, HIGH);
    gpio_put(INDICATOR_N2O_FILL_VALVE, HIGH);
    return COMMAND_OPEN;
}

/**
 * @brief Fill valveをCLOSEにする
 * @return COMMAND_CLOSE
 */
uint32_t offFillSequence(){
    gpio_put(N2O_FILL_VALVE, LOW);
    gpio_put(INDICATOR_N2O_FILL_VALVE, LOW);
    return COMMAND_CLOSE;
}

/**
 * @brief dump操作、N2O dump ValveをOPENにした後にFill ValveをCLOSEにする
 * @return COMMAND_OPEN
 */
uint32_t onDumpSequence(){
    gpio_put(N2O_DUMP_VALVE, HIGH);
    gpio_put(INDICATOR_N2O_DUMP_VALVE, HIGH);
    gpio_put(N2O_FILL_VALVE, LOW);
    gpio_put(INDICATOR_N2O_FILL_VALVE, LOW);
    return COMMAND_OPEN;
}

/**
 * @brief N2O dump ValveをCLOSEした後にFill ValveをCLOSEにする
 * @return COMMAND_CLOSE
 */
uint32_t offDumpSequence(){
    gpio_put(N2O_DUMP_VALVE, LOW);
    gpio_put(INDICATOR_N2O_DUMP_VALVE, LOW);
    gpio_put(N2O_FILL_VALVE, LOW);
    gpio_put(INDICATOR_N2O_FILL_VALVE, LOW);
    return COMMAND_CLOSE;
}

/**
 * @brief Purge操作、N2O dump ValveをOPENしてN2Oを排出する
 * @return COMMAND_OPEN
 */
uint32_t onPurgeSequence(){
    gpio_put(N2O_DUMP_VALVE, HIGH);
    gpio_put(INDICATOR_N2O_DUMP_VALVE, HIGH);
    return COMMAND_OPEN;
}

/**
 * @brief Purge操作の終了、N2O dump ValveをCLOSEする
 * @return COMMAND_CLOSE
 */
uint32_t offPurgeSequence(){
    gpio_put(N2O_DUMP_VALVE, LOW);
    gpio_put(INDICATOR_N2O_DUMP_VALVE, LOW);
    return COMMAND_CLOSE;
}

/**
 * @brief Ignition操作、O2 ValveをOPENにして点火コマンドをIginition Controllerに送信する
 * @return COMMAND_OPEN
 */
uint32_t onIgnitionSequence(){
    gpio_put(O2_VALVE, HIGH);
    gpio_put(INDICATOR_O2_VALVE, HIGH);
    return COMMAND_OPEN;
}

/**
 * @brief Ignition操作の終了、O2 ValveをCLOSEにしてキャンセルコマンドをIginition Controllerに送信する
 * @return COMMAND_CLOSE
 */
uint32_t offIgnitionSequence(){
    gpio_put(O2_VALVE, LOW);
    gpio_put(INDICATOR_O2_VALVE, LOW);
    return COMMAND_CLOSE;
}

/**
 * @brief Fill Valveのステータスを取得する
 * @return COMMAND_OPEN or COMMAND_CLOSE
 */
uint32_t getN2OFillValveStatus(){
    if (gpio_get(N2O_FILL_VALVE) == HIGH){
        return COMMAND_OPEN;
    }
    return COMMAND_CLOSE;
}

/**
 * @brief Dump Valveのステータスを取得する
 * @return COMMAND_OPEN or COMMAND_CLOSE
 */
uint32_t getN2ODumpValveStatus(){
    if (gpio_get(N2O_DUMP_VALVE) == HIGH){
        return COMMAND_OPEN;
    }
    return COMMAND_CLOSE;
}

/**
 * @brief O2 Valveのステータスを取得する
 * @return COMMAND_OPEN or COMMAND_CLOSE
 */
uint32_t getO2ValveStatus(){
    if (gpio_get(O2_VALVE) == HIGH){
        return COMMAND_OPEN;
    }
    return COMMAND_CLOSE;
}

/**
 * @brief Emergency Shutdown、全てのValveをCLOSEにする
 */
void emergencyShutdown(){
    gpio_put(O2_VALVE, LOW);
    gpio_put(INDICATOR_O2_VALVE, LOW);
    gpio_put(N2O_FILL_VALVE, LOW);
    gpio_put(INDICATOR_N2O_FILL_VALVE, LOW);
    gpio_put(N2O_DUMP_VALVE, LOW);
    gpio_put(INDICATOR_N2O_DUMP_VALVE, LOW);
}
