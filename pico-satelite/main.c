/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <pico/stdio.h>
#include "port_common.h"

#include "wizchip_conf.h"
#include "w5x00_spi.h"

#include "loopback.h"
#include "socket.h"
/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 2)

/* Socket */
#define SOCKET_LOOPBACK 0

/* Port */
#define PORT_LOOPBACK 5000

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
/* Network */
static wiz_NetInfo g_net_info =
    {
        .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
        .ip = {192, 168, 11, 2},                     // IP address
        .sn = {255, 255, 255, 0},                    // Subnet Mask
        .gw = {192, 168, 11, 1},                     // Gateway
        .dns = {8, 8, 8, 8},                         // DNS server
        .dhcp = NETINFO_STATIC                       // DHCP enable/disable
};

/* Loopback */
static uint8_t g_loopback_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void);

/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */
int main()
{
    /* Initialize */
    int retval = 0;

    set_clock_khz();

    stdio_init_all();

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
        uint8_t socketNum = 0;

        switch (getSn_SR(socketNum))    // Get Sn_SR register
        {
        case SOCK_ESTABLISHED:
            if (getSn_IR(socketNum) & Sn_IR_CON)
            {
                // 接続先のIPとポート番号を取得
                getSn_DIPR(socketNum, destip);
                destport = getSn_DPORT(socketNum);
                printf(
                    "%d:Connected - %d.%d.%d.%d : %d\r\n",
                    socketNum,
                    destip[0], destip[1], destip[2], destip[3],
                    destport
                );
                // Set Sn_IR register
                setSn_IR(socketNum,Sn_IR_CON);
            }
            // 受信バッファにデータがあるか確認
            if((size = getSn_RX_RSR(socketNum)) > 0){
                if (size > DATA_BUF_SIZE) size = DATA_BUF_SIZE;
                ret = recv(socketNum, g_loopback_buf, size);
                size = (uint16_t) ret;
                sentsize = 0;
                printf("Received data size: %d\r\n", size);
                // 受信データを出力
                printf("Received data: ");
                for (int i = 0; i < size; i++)
                {
                    printf("%c", g_loopback_buf[i]);
                }
                printf("\r\n");
                /* loopback処理
                while(size != sentsize)
                {
                    
                    ret = send(socketNum, g_loopback_buf+sentsize, size-sentsize); // Data send process (User's buffer -> Destination through H/W Tx socket buffer)
                    if(ret < 0) // Send Error occurred (sent data length < 0)
                    {
                        close(socketNum); // socket close
                        return ret;
                    }
                    sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
                    
                }
                //*/
            }

            break;
        case SOCK_CLOSE_WAIT:
            if ((ret = disconnect(socketNum)) != SOCK_OK)
            {
                break;
                //return ret;
            }
            printf("%d:Socket Closed\r\n", socketNum);
            break;
        case SOCK_INIT:
            if ((ret = listen(socketNum)) != SOCK_OK)
            {
                break;
                //return ret;
            }
            break;
        case SOCK_CLOSED:
            if ((ret = socket(socketNum, Sn_MR_TCP, PORT_LOOPBACK, 0)) == 0)
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

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
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