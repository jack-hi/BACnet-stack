/***************************************************
 * 文件名：bacnet_lwip_if.c 
 * 功  能：
 **************************************************/
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "config.h"
#include "address.h"
#include "bacdef.h"
#include "handlers.h"
#include "client.h"
#include "bacdcode.h"
#include "npdu.h"
#include "apdu.h"
#include "iam.h"
#include "tsm.h"
#include "bacdevice.h"
#include "datalink.h"
#include "dcc.h"
#include "getevent.h"
#include "txbuf.h"
#include "version.h"
#include "bacdevice.h"
#include "bip.h"

#include "lwip/netifapi.h"
#include <lwip/sockets.h>
#include "bacnet_lwip_if.h"
#include "app.h"
#include "app_gwc_server.h"
#include "app_global.h"
#include "app_gwc_manage.h"


#define NUM_SOCKETS MEMP_NUM_NETCONN


bacnet_udp_conn_def bac_udp_conn;
rt_timer_t bacnet_polling_timer;
static rt_uint32_t board_mode;


/***************************************************
 * 函数名：Init_Service_Handlers
 * 功  能：注册服务的应用处理函数，在初始化时使用
 * 参  数：无
 * 返  回：无
 * 备  注：当需要加入命令时，加入对应的文件，然后在
 *         这里注册处理函数
 **************************************************/
static void Init_Service_Handlers(
    void)
{
    Device_Init(NULL);
    /* we need to handle who-is to support dynamic device binding */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);
    
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_HAS, handler_who_has);
    
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, handler_i_am_add);
    
    apdu_set_unrecognized_service_handler_handler(handler_unrecognized_service);
    
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY, handler_read_property);
    
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROP_MULTIPLE, 
                                handler_read_property_multiple);
    
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROPERTY, handler_write_property);
    
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROP_MULTIPLE,
                                handler_write_property_multiple);
    
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_REINITIALIZE_DEVICE, 
                                handler_reinitialize_device);
    
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL, 
                                handler_device_communication_control);
    
    apdu_set_confirmed_ack_handler(SERVICE_CONFIRMED_READ_PROPERTY, 
                                handler_read_property_ack);
    
    apdu_set_confirmed_ack_handler(SERVICE_CONFIRMED_READ_PROP_MULTIPLE, 
                                handler_read_property_multiple_ack);
}

/***************************************************
 * 函数名：BACnetInit
 * 功  能：BACnet模块初始化
 * 参  数：无
 * 返  回：无
 * 备  注：BACnet模块使用的是uip，但并不在这里对uip
 *         进行设置，而是把uip作为一个网络模块处理
 **************************************************/
void BACnetInit(void)
{
    rt_uint32_t ad;
    rt_uint32_t id;
    
    id = app_global_para_get(GLOBAL_PARA_BACNET_FLC_DEVICE);
    
    ad = LOCAL_IP_ADDR;
    /* allow the device ID to be set */
    Device_Set_Object_Instance_Number(id);
    bip_set_addr(ad);
    bip_set_broadcast_addr(ad | 0x000000FF);
    bip_set_port(BACNET_UDP_PORT);
    tsm_init();
    address_init();
    Init_Service_Handlers();
    Send_I_Am(&Handler_Transmit_Buffer[0]);
}

/***************************************************
 * 函数名：BACnetSendBytes
 * 功  能：bacnet协议栈发送数据的接口函数
 * 参  数：mtu 数据，mtu_len 数据长度，addr 目的地址
 * 返  回：rt_uint32_t 发送的长度
 * 备  注：如果本机是GWC，通过tcp端口发送出去
 *         如果本机时FLC，通过UDP端口发送出去
 **************************************************/
uint32_t BACnetSendBytes(uint8_t *mtu, uint32_t mtu_len, struct sockaddr_in addr)
{
    if(board_mode == BOARD_MODE_GWC)
    {   /* GWC发出的UDP数据直接通过TCP发送给Bivale即可 */
        gwc_tcp_send_frame(mtu, mtu_len, 0);
    }
    else
    {   /* FLC发出的UDP数据正常通过UDP发送出去 */ 
        if ((bac_udp_conn.sock < 0) || (bac_udp_conn.sock >= NUM_SOCKETS)) {
            return 0;
        }
        bacnet_udp_send_bytes(mtu, mtu_len, addr.sin_addr.s_addr, addr.sin_port);
    }
    
    return mtu_len;
}

/***************************************************
 * 函数名：bacnet_udp_send_bytes
 * 功  能：bacnet端口发送udp数据
 * 参  数：mtu 数据，mtu_len 数据长度，ip 目的ip地址，
 *         port 目的端口号
 * 返  回：rt_uint32_t 发送的长度
 * 备  注：
 **************************************************/
rt_uint32_t bacnet_udp_send_bytes(uint8_t *mtu, uint32_t mtu_len, 
                                uint32_t ip, uint16_t port)
{
    struct sockaddr_in addr;
        
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(ip);
    rt_memset(&(addr.sin_zero), 0, sizeof(addr.sin_zero));
    
    sendto(bac_udp_conn.sock, mtu, mtu_len, 0,
            (struct sockaddr *) &addr, sizeof(struct sockaddr)); 
    return mtu_len;
}

/***************************************************
 * 函数名：BACnetTask
 * 功  能：BACnet任务，在主循环实时调用
 * 参  数：无
 * 返  回：无
 * 备  注：本文件开辟了npdu的缓冲区，uip接收数据时，
 *         把数据直接放到这个缓冲区，在这里进行处理
 *         本函数还会获取系统的10ms时钟，用来做时间
 *         任务的定时
 **************************************************/
void rt_app_bacnet_udp_rcv_entry(void* para)
{
       
    int bytes_read;
    rt_uint8_t *recv_data;
    rt_uint32_t addr_len;
    struct sockaddr_in server_addr, client_addr;
    rt_uint32_t i;
    rt_uint32_t local_ad;
            
    
    /* 分配接收用的数据缓冲 */
    for(i = 0; i < BACNET_UDP_RCV_BUF_NUM_MAX; i++)
    {
        bac_udp_conn.rcv_buf[i].buf = rt_malloc(MAX_MPDU);
        if (bac_udp_conn.rcv_buf[i].buf == RT_NULL)
        {
            rt_kprintf("bacnet no memory\n");   /* 分配内存失败，返回 */
            return;
        }
    }
    recv_data = rt_malloc(MAX_MPDU);
    if (recv_data == RT_NULL)
    {
        rt_kprintf("bacnet no memory\n");   /* 分配内存失败，返回 */
        return;
    }
    
    /* 创建一个socket，类型是SOCK_DGRAM，UDP类型 */
    if ((bac_udp_conn.sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        rt_kprintf("Socket error\n");           /* 释放接收用的数据缓冲 */
        rt_free(recv_data);
        return;
    }
    /* 初始化服务端地址 */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(BACNET_UDP_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    
    if (bind(bac_udp_conn.sock, (struct sockaddr *) &server_addr, 
                sizeof(struct sockaddr)) == -1)   /* 绑定socket到服务端地址 */
    {
        rt_kprintf("Bind error\n");     /* 绑定地址失败 */
        rt_free(recv_data);             /* 释放接收用的数据缓冲 */
        return;
    }
    addr_len = sizeof(struct sockaddr);
    rt_kprintf("bacnet open port %d\n", BACNET_UDP_PORT);
    
    while (1)
    {
        bytes_read = recvfrom(bac_udp_conn.sock, recv_data, MAX_MPDU, 0,
                    (struct sockaddr *)&client_addr, (socklen_t *)&addr_len);
        if(bytes_read)
        {
            app_led_blink(LED_NET);
            if((recv_data[0] == 0x55)&& (recv_data[1] == 0xAA))
            {
                app_gwc_manage_receive_frame_handler(recv_data, bytes_read, ntohl(client_addr.sin_addr.s_addr));
                continue;
            }
            if(board_mode == BOARD_MODE_GWC)
            {   
                /* GWC收到的源地址不是本机的帧，通过TCP直接发送给Bivale中心即可 */
                /* 广播数据有可能是自身发出，没必要处理 */
                local_ad = LOCAL_IP_ADDR;
                if(client_addr.sin_addr.s_addr != htonl(local_ad))
                    gwc_tcp_send_frame(recv_data, bytes_read, ntohl(client_addr.sin_addr.s_addr));
            }
            else
            {   
                /* FLC的UDP接收到的数据，传入bacnet stack中处理 */
                for(i = 0; i < BACNET_UDP_RCV_BUF_NUM_MAX; i++)
                {
                    if(++bac_udp_conn.rcv_buf_index > BACNET_UDP_RCV_BUF_NUM_MAX)
                        bac_udp_conn.rcv_buf_index = 0;
                    if(bac_udp_conn.rcv_buf[bac_udp_conn.rcv_buf_index].len == 0)
                        break;
                }
                if(i >= BACNET_UDP_RCV_BUF_NUM_MAX)
                    continue;
                memcpy(bac_udp_conn.rcv_buf[i].buf, recv_data, bytes_read);
                bac_udp_conn.rcv_buf[i].len = bytes_read;
                bac_udp_conn.rcv_buf[i].saddr = client_addr;
                rt_mb_send(bac_udp_conn.mb, (rt_uint32_t)&(bac_udp_conn.rcv_buf[i]));            
            }

        }
    }
}

/***************************************************
 * 函数名：rt_app_bacnet_task_entry
 * 功  能：bacnet任务处理
 * 参  数：
 * 返  回：
 * 备  注：通过邮箱等待接收
 **************************************************/
void rt_app_bacnet_task_entry(void* para)
{
    
    struct sockaddr_in client_addr;
    rt_uint32_t bytes_read;
    BACNET_ADDRESS src = {0};           /* address where message came from */ 
    bacnet_udp_rcv_buf_def *rcv_buf;
    
    rt_thread_delay(500);
    BACnetInit();
    rt_kprintf("bacnet stack init suc.\n");
    while(1)
    {
        /* 等待邮箱信息，每次等待1s */
        if (rt_mb_recv(bac_udp_conn.mb, (rt_uint32_t*)&rcv_buf, 1000) == RT_EOK)
        {
            /* 以上取出来的client_addr为网络顺序的地址，以下转换为正常顺序 */
            client_addr.sin_addr.s_addr = htonl(rcv_buf->saddr.sin_addr.s_addr);
            client_addr.sin_port = htons(rcv_buf->saddr.sin_port);
            bytes_read = datalink_receive(&src, rcv_buf->buf, MAX_MPDU, 0, 
                            client_addr, rcv_buf->len);
            if (bytes_read) 
            {   /* NPDU 数据处理 */
                npdu_handler(&src, rcv_buf->buf, bytes_read);
            }
            rcv_buf->len = 0;           /* 处理完成，长度清零 */
        }
    }
}

/***************************************************
 * 函数名：bacnet_polling_timerout
 * 功  能：bacnet协议栈定时任务处理
 * 参  数：
 * 返  回：
 * 备  注：1s调用一次，通过rt_timer回调
 **************************************************/
void bacnet_polling_timerout(void *para)
{
    static uint32_t address_binding_tmr = 0;

    dcc_timer_seconds(1);
#if defined(BACDL_BIP) && BBMD_ENABLED
    bvlc_maintenance_timer(1);
#endif
    tsm_timer_milliseconds(1 * 1000);
    address_binding_tmr++;              /* scan cache address */
    if (address_binding_tmr >= 60)      /* 60s */
    {
        address_cache_timer(address_binding_tmr);
        address_binding_tmr = 0;
    }
}

/***************************************************
 * 函数名：rt_app_bacnet_init
 * 功  能：bacnet 模块初始化
 * 参  数：
 * 返  回：
 * 备  注：
 **************************************************/
int rt_app_bacnet_init(void)
{
    rt_thread_t tid;
    
    board_mode = app_global_para_get(GLOBAL_PARA_BOARD_MODE);
    if((board_mode != BOARD_MODE_GWC) && (board_mode != BOARD_MODE_FLC))
        return 0;
    
    /* 建立bacnet的1s轮询任务 */
    bacnet_polling_timer = rt_timer_create("bacpoll", bacnet_polling_timerout, 
                    NULL, 1000, RT_TIMER_FLAG_PERIODIC);
    if(bacnet_polling_timer == RT_NULL)
    {
        rt_kprintf("bacnet polling timer create fail.\r\n");
        return 0;
    }
    
    bac_udp_conn.mb = rt_mb_create("bac_mb", BACNET_UDP_RCV_BUF_NUM_MAX, 
                        RT_IPC_FLAG_FIFO);
    if(bac_udp_conn.mb == RT_NULL)
    {
        rt_kprintf("bacnet rcv mb create fail.\r\n");
        return 0;
    }
    
    /* 建立主线程 */ 
    tid = rt_thread_create("bac_udp",
                           rt_app_bacnet_udp_rcv_entry, RT_NULL,
                           32 * 1024, RT_THREAD_PRIORITY_MAX / 3, 25);
    if (tid != RT_NULL) 
        rt_thread_startup(tid);
    
    tid = rt_thread_create("bac_task",
                           rt_app_bacnet_task_entry, RT_NULL,
                           32*1024, RT_THREAD_PRIORITY_MAX / 3, 25);
    if (tid != RT_NULL) 
        rt_thread_startup(tid);
    


    return 0;
}
INIT_APP_EXPORT(rt_app_bacnet_init);




