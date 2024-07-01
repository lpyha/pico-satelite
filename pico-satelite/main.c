/**
 * @file main.c
 * @brief socket通信でvalveを制御するプログラム
 * @author Murakami Kantaro
 * @date 2024-07-01
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
/* Port */
#define PORT 5000

#define HIGH 1
#define LOW 0

const uint8_t SOCKET_NUM = 0;

#define INDICATOR_O2_VALVE 11
#define INDICATOR_N2O_FILL_VALVE 12
#define INDICATOR_N2O_DUMP_VALVE 13

/**
 * @brief ネットワーク情報
 * @param mac MACアドレス
 * @param ip IPアドレス
 * @param sn サブネットマスク
 * @param gw ゲートウェイ
 * @param dns DNSサーバ
 * @param dhcp DHCPの有効/無効
 */
static wiz_NetInfo g_net_info ={
    .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56},
    .ip = {192, 168, 100, 100},
    .sn = {255, 255, 255, 0},
    .gw = {192, 168, 100, 1},
    .dns = {8, 8, 8, 8},
    .dhcp = NETINFO_STATIC
};

/**
 * @brief 受信バッファ
 */
static uint8_t g_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};

/**
 * @brief システムクロックの周波数を設定する
 */
static void set_clock_khz(void){
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

/**
 * @brief 4Byteのデータを32bitの符号なし整数(uint32_t)に変換する
 * @param[in] buf* uint8_t型の配列,サイズは4Byte
 * @return uint32_t型の値
 */
uint32_t convert2Uint32(const uint8_t* buf) {
    return  ((uint32_t)buf[0] << 24) |
            ((uint32_t)buf[1] << 16) |
            ((uint32_t)buf[2] << 8)  |
            (uint32_t)buf[3];
}

/**
 * @brief 2つのuint32_t型の値をuint8_t型の配列に変換する
 *        socket通信で送信するためのデータ(header, cmd)を作成している
 * @param[in] header uint32_t型のheader
 * @param[in] command uint32_t型のcommand
 * @param[out] array uint8_t型の配列,サイズは8Byte
 */
void convert2Uint8Array(uint32_t header, uint32_t command, uint8_t* array) {
    array[0] = (uint8_t)(header >> 24); // 最上位バイト
    array[1] = (uint8_t)(header >> 16);
    array[2] = (uint8_t)(header >> 8);
    array[3] = (uint8_t)(header);

    array[4] = (uint8_t)(command >> 24);
    array[5] = (uint8_t)(command >> 16);
    array[6] = (uint8_t)(command >> 8);
    array[7] = (uint8_t)(command); // 最下位バイト
}

/**
 * @brief valveを制御する関数
 * @param[in] header uint32_t型のheader
 * @param[in] command uint32_t型のcommand
 * @return 0:正常終了, -1:エラー
 */
int actionActuator(uint32_t header, uint32_t command){
    uint32_t ret;
    uint8_t sendBuf[8]; 
    switch (header){
        case HEADER_FILL:
            if (command == COMMAND_OPEN){
                ret = onFillSequence();
                convert2Uint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else if (command == COMMAND_STATUS){
                ret = getN2OFillValveStatus();
                convert2Uint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else if (command == COMMAND_CLOSE){
                ret = offFillSequence();
                convert2Uint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else {
                ret = COMMAND_ERORR;
                convert2Uint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
                return -1;
            }
            break;
        case HEADER_DUMP:
            if (command == COMMAND_OPEN){
                ret = onDumpSequence();
                convert2Uint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else if (command == COMMAND_STATUS){
                ret = getN2ODumpValveStatus();
                convert2Uint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else if (command == COMMAND_CLOSE){
                ret = offDumpSequence();
                convert2Uint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else {
                ret = COMMAND_ERORR;
                convert2Uint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
                return -1;
            }
            break;
        case HEADER_PURGE:
            if (command == COMMAND_OPEN){
                ret = onPurgeSequence();
                convert2Uint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else if (command == COMMAND_STATUS){
                ret = getN2ODumpValveStatus();
                convert2Uint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else if (command == COMMAND_CLOSE){
                ret = offPurgeSequence();
                convert2Uint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else {
                ret = COMMAND_ERORR;
                convert2Uint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
                return -1;
            }
            break;
        case HEADER_IGNITION:
            if (command == COMMAND_OPEN){
                ret = onIgnitionSequence();
                convert2Uint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else if (command == COMMAND_STATUS){
                ret = getO2ValveStatus();
                convert2Uint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else if (command == COMMAND_CLOSE){
                ret = offIgnitionSequence();
                convert2Uint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
            } else {
                ret = COMMAND_ERORR;
                convert2Uint8Array(header, ret, sendBuf);
                send(SOCKET_NUM, (uint8_t*)&sendBuf, 8);
                return -1;
            }
            break;
        default:
            ret = COMMAND_ERORR;
            convert2Uint8Array(header, ret, sendBuf);
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
            // socket close->GSEとの通信が遮断されたときすべてのValveを閉じる
            emergencyShutdown();
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
            if ((ret = socket(SOCKET_NUM, Sn_MR_TCP, PORT, 0)) == 0)
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
