/*
 * File      : bacnet_lwip_if.h
 * function  : bacnet 与lwip对接，完成UDP数据的接收和发送
 * Change Logs:
 * Date           Author       Notes
 * 
 */
#ifndef __BACNET_LWIP_IF_H__
#define __BACNET_LWIP_IF_H__

#include "bip.h"
#include <rtthread.h>



#define BACNET_UDP_PORT                 47808
#define BACNET_UDP_RCV_BUF_NUM_MAX      4       /* bacnet接收缓冲区数量 */


typedef struct
{
    uint8_t Buf[MAX_MPDU];
    uint32_t Len;
    uint32_t Addr;
    uint16_t Port;
}BACnetLowRxBufDef;
typedef struct
{
    rt_uint8_t *buf;
    rt_uint32_t len;
    struct sockaddr_in saddr;       /* source addr源地址 */
}bacnet_udp_rcv_buf_def;

typedef struct
{
    int sock;  
    bacnet_udp_rcv_buf_def rcv_buf[BACNET_UDP_RCV_BUF_NUM_MAX];
    rt_uint8_t rcv_buf_index;       /* 接收缓冲区轮询计数 */
    rt_mailbox_t mb;                /* 数据接收邮箱 */
}bacnet_udp_conn_def;


void BACnetInit(void);
void BACnetTask(void);
uint32_t BACnetSendBytes(uint8_t *mtu, uint32_t mtu_len, struct sockaddr_in addr);
rt_uint32_t bacnet_udp_send_bytes(uint8_t *mtu, uint32_t mtu_len, 
                                uint32_t ip, uint16_t port);
#endif

