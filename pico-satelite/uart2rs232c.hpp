#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pico/stdio.h>
#include "port_common.h"

#define BAUD_RATE 115200

#define RS232C_TX_IGNITION 0
#define RS232C_RX_IGNITION 1
#define RS232C_TX_MEASURE 6
#define RS232C_RX_MEASURE 7

/**
 * @brief 4byte同じデータにしてエンディアンを考慮しない設計
 *  */
const uint8_t onIgnition[4] = {0xFF, 0xFF, 0xFF, 0xFF};
const uint8_t offIgnition[4] = {0x00, 0x00, 0x00, 0x00};
const uint8_t statusIgnition[4] = {0xF0, 0xF0, 0xF0, 0xF0};

const uint8_t IG_CMD_LEN = sizeof(onIgnition)/sizeof(onIgnition[0]);

const uint8_t IG_CMD_SUCCESS = 0x01;
const uint8_t IG_CMD_FAILURE = 0x00;

/**
 * @brief Inirialize UART0: Ignition, UART1: Measure
 */
void initUartPins(){
    gpio_set_function(RS232C_TX_IGNITION, GPIO_FUNC_UART);
    gpio_set_function(RS232C_RX_IGNITION, GPIO_FUNC_UART);
    gpio_set_function(RS232C_TX_MEASURE, GPIO_FUNC_UART);
    gpio_set_function(RS232C_RX_MEASURE, GPIO_FUNC_UART);
    uart_init(uart0, BAUD_RATE);
    uart_init(uart1, BAUD_RATE);
    uart_set_hw_flow(uart0, false, false);
    uart_set_hw_flow(uart1, false, false);
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
    uart_set_format(uart1, 8, 1, UART_PARITY_NONE);
}

/**
 * @brief Send ON ignition command, and check the response
 * @return true: success, false: failure
 */
bool sendOnIgnition(){
    uart_write_blocking(uart0, onIgnition, IG_CMD_LEN);
    uint8_t rx[1];
    uart_read_blocking(uart0, rx, 1);
    if(rx[0] != IG_CMD_SUCCESS){
        return false;
    }
    return true;
}

/**
 * @brief Send OFF ignition command, and check the response
 * @return true: success, false: failure
 */
bool sendOffIgnition(){
    uart_write_blocking(uart0, offIgnition, IG_CMD_LEN);
    uint8_t rx[1];
    uart_read_blocking(uart0, rx, 1);
    if(rx[0] != IG_CMD_SUCCESS){
        return false;
    }
    return true;
}
