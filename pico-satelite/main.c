/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pico/stdio.h>
#include "port_common.h"

#include "wizchip_conf.h"
#include "w5x00_spi.h"

#include "loopback.h"
#include "socket.h"

#include "sequence.hpp"


/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 2)

/* Socket */
#define SOCKET_LOOPBACK 0

/* Port */
#define PORT_LOOPBACK 5000

#define HIGH 1
#define LOW 0

const uint8_t SOCKET_NUM = 0;

/* Pins */

#define INDICATOR_O2_VALVE 11
#define INDICATOR_N2O_FILL_VALVE 12
#define INDICATOR_N2O_DUMP_VALVE 13

static wiz_NetInfo g_net_info ={
    .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
    .ip = {192, 168, 100, 100},                     // IP address
    .sn = {255, 255, 255, 0},                    // Subnet Mask
    .gw = {192, 168, 100, 1},                     // Gateway
    .dns = {8, 8, 8, 8},                         // DNS server
    .dhcp = NETINFO_STATIC                       // DHCP enable/disable
};

static uint8_t g_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};


/* Clock */
static void set_clock_khz(void);

uint32_t convert2Uint32(const uint8_t* buf) {
    return  ((uint32_t)buf[0] << 24) |
            ((uint32_t)buf[1] << 16) |
            ((uint32_t)buf[2] << 8)  |
            (uint32_t)buf[3];
}

void convertToUint8Array(uint32_t header, uint32_t command, uint8_t* array) {
    array[0] = (uint8_t)(header >> 24); // 最上位バイト
    array[1] = (uint8_t)(header >> 16);
    array[2] = (uint8_t)(header >> 8);
    array[3] = (uint8_t)(header);

    array[4] = (uint8_t)(command >> 24);
    array[5] = (uint8_t)(command >> 16);
    array[6] = (uint8_t)(command >> 8);
    array[7] = (uint8_t)(command); // 最下位バイト
}

int actionActuator(uint32_t header, uint32_t command){
    uint32_t ret;
    uint8_t sendBuf[8]; 
    switch (header){
        case HEADER_FILL:
            if (command == COMMAND_OPEN){
                ret = onFillSequence();
                convertToUint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else if (command == COMMAND_STATUS){
                ret = getN2OFillValveStatus();
                convertToUint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else if (command == COMMAND_CLOSE){
                ret = offFillSequence();
                convertToUint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else {
                ret = COMMAND_ERORR;
                convertToUint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
                return -1;
            }
            break;
        case HEADER_DUMP:
            if (command == COMMAND_OPEN){
                ret = onDumpSequence();
                convertToUint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else if (command == COMMAND_STATUS){
                ret = getN2ODumpValveStatus();
                convertToUint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else if (command == COMMAND_CLOSE){
                ret = offDumpSequence();
                convertToUint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else {
                ret = COMMAND_ERORR;
                convertToUint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
                return -1;
            }
            break;
        case HEADER_PURGE:
            if (command == COMMAND_OPEN){
                ret = onPurgeSequence();
                convertToUint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else if (command == COMMAND_STATUS){
                ret = getN2ODumpValveStatus();
                convertToUint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else if (command == COMMAND_CLOSE){
                ret = offPurgeSequence();
                convertToUint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else {
                ret = COMMAND_ERORR;
                convertToUint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
                return -1;
            }
            break;
        case HEADER_IGNITION:
            if (command == COMMAND_OPEN){
                ret = onIgnitionSequence();
                convertToUint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else if (command == COMMAND_STATUS){
                ret = getO2ValveStatus();
                convertToUint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else if (command == COMMAND_CLOSE){
                ret = offIgnitionSequence();
                convertToUint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else {
                ret = COMMAND_ERORR;
                convertToUint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
                return -1;
            }
            break;
        default:
            ret = COMMAND_ERORR;
            convertToUint8Array(header, ret, sendBuf);
            send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            return -1;
            break;
    }
    return 0;
}

int main()
{
    /* Initialize */
    int retval = 0;

    set_clock_khz();

    stdio_init_all();
    // -------------initialize GPIO------------------
    gpio_init(O2_VALVE);
    gpio_init(N2O_FILL_VALVE);
    gpio_init(N2O_DUMP_VALVE);
    gpio_init(INDICATOR_O2_VALVE);
    gpio_init(INDICATOR_N2O_FILL_VALVE);
    gpio_init(INDICATOR_N2O_DUMP_VALVE);
    gpio_set_dir(O2_VALVE, GPIO_OUT);
    gpio_set_dir(N2O_FILL_VALVE, GPIO_OUT);
    gpio_set_dir(N2O_DUMP_VALVE, GPIO_OUT);
    gpio_set_dir(INDICATOR_O2_VALVE, GPIO_OUT);
    gpio_set_dir(INDICATOR_N2O_FILL_VALVE, GPIO_OUT);
    gpio_set_dir(INDICATOR_N2O_DUMP_VALVE, GPIO_OUT);
    // ----------------------------------------------
    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    network_initialize(g_net_info);

    /* Get network information */
    print_network_information(g_net_info);

    /* Infinite loop */
    while (1)
    {
        int32_t ret;
        uint16_t size = 0, sentsize = 0;
        uint8_t destip[4];
        uint16_t destport;

        switch (getSn_SR(SOCKET_NUM))    // Get Sn_SR register
        {
        case SOCK_ESTABLISHED:
            if (getSn_IR(SOCKET_NUM) & Sn_IR_CON)
            {
                // 接続先のIPとポート番号を取得
                getSn_DIPR(SOCKET_NUM, destip);
                destport = getSn_DPORT(SOCKET_NUM);
                printf(
                    "%d:Connected - %d.%d.%d.%d : %d\r\n",
                    SOCKET_NUM,
                    destip[0], destip[1], destip[2], destip[3],
                    destport
                );
                // Set Sn_IR register
                setSn_IR(SOCKET_NUM,Sn_IR_CON);
            }
            // 受信バッファにデータがあるか確認
            if((size = getSn_RX_RSR(SOCKET_NUM)) > 0){
                if (size > DATA_BUF_SIZE) size = DATA_BUF_SIZE;
                ret = recv(SOCKET_NUM, g_buf, size);
                size = (uint16_t) ret;
                sentsize = 0;
                printf("Received data size: %d\r\n", size);
                { // 受信データを出力
                    printf("Received data: ");
                    for (int i = 0; i < size; i++){
                        printf("%x", g_buf[i]);
                    }
                    printf("\r\n");
                }
                uint32_t header = convert2Uint32(g_buf);
                uint32_t command = convert2Uint32(g_buf + 4);
                actionActuator(header, command);
                /* loopback処理
                while(size != sentsize)
                {
                    
                    ret = send(SOCKET_NUM, g_buf+sentsize, size-sentsize); // Data send process (User's buffer -> Destination through H/W Tx socket buffer)
                    if(ret < 0) // Send Error occurred (sent data length < 0)
                    {
                        close(SOCKET_NUM); // socket close
                        return ret;
                    }
                    sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
                    
                }
                //*/
            }

            break;
        case SOCK_CLOSE_WAIT:
            if ((ret = disconnect(SOCKET_NUM)) != SOCK_OK)
            {
                break;
                //return ret;
            }
            printf("%d:Socket Closed\r\n", SOCKET_NUM);
            break;
        case SOCK_INIT:
            if ((ret = listen(SOCKET_NUM)) != SOCK_OK)
            {
                break;
                //return ret;
            }
            break;
        case SOCK_CLOSED:
            if ((ret = socket(SOCKET_NUM, Sn_MR_TCP, PORT_LOOPBACK, 0)) == 0)
            {
                break;
                //return ret;
            }
            break;
        default:
            break;
        };
    }
}

/* Clock */
static void set_clock_khz(void)
{
    // set a system clock frequency in khz
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    // configure the specified clock
    clock_configure(
        clk_peri,
        0,                                                // No glitchless mux
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
        PLL_SYS_KHZ * 1000,                               // Input frequency
        PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    );
}
