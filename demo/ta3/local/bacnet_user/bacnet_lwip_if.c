/***************************************************
 * �ļ�����bacnet_lwip_if.c 
 * ��  �ܣ�
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
 * ��������Init_Service_Handlers
 * ��  �ܣ�ע������Ӧ�ô��������ڳ�ʼ��ʱʹ��
 * ��  ������
 * ��  �أ���
 * ��  ע������Ҫ��������ʱ�������Ӧ���ļ���Ȼ����
 *         ����ע�ᴦ����
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
 * ��������BACnetInit
 * ��  �ܣ�BACnetģ���ʼ��
 * ��  ������
 * ��  �أ���
 * ��  ע��BACnetģ��ʹ�õ���uip���������������uip
 *         �������ã����ǰ�uip��Ϊһ������ģ�鴦��
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
 * ��������BACnetSendBytes
 * ��  �ܣ�bacnetЭ��ջ�������ݵĽӿں���
 * ��  ����mtu ���ݣ�mtu_len ���ݳ��ȣ�addr Ŀ�ĵ�ַ
 * ��  �أ�rt_uint32_t ���͵ĳ���
 * ��  ע�����������GWC��ͨ��tcp�˿ڷ��ͳ�ȥ
 *         �������ʱFLC��ͨ��UDP�˿ڷ��ͳ�ȥ
 **************************************************/
uint32_t BACnetSendBytes(uint8_t *mtu, uint32_t mtu_len, struct sockaddr_in addr)
{
    if(board_mode == BOARD_MODE_GWC)
    {   /* GWC������UDP����ֱ��ͨ��TCP���͸�Bivale���� */
        gwc_tcp_send_frame(mtu, mtu_len, 0);
    }
    else
    {   /* FLC������UDP��������ͨ��UDP���ͳ�ȥ */ 
        if ((bac_udp_conn.sock < 0) || (bac_udp_conn.sock >= NUM_SOCKETS)) {
            return 0;
        }
        bacnet_udp_send_bytes(mtu, mtu_len, addr.sin_addr.s_addr, addr.sin_port);
    }
    
    return mtu_len;
}

/***************************************************
 * ��������bacnet_udp_send_bytes
 * ��  �ܣ�bacnet�˿ڷ���udp����
 * ��  ����mtu ���ݣ�mtu_len ���ݳ��ȣ�ip Ŀ��ip��ַ��
 *         port Ŀ�Ķ˿ں�
 * ��  �أ�rt_uint32_t ���͵ĳ���
 * ��  ע��
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
 * ��������BACnetTask
 * ��  �ܣ�BACnet��������ѭ��ʵʱ����
 * ��  ������
 * ��  �أ���
 * ��  ע�����ļ�������npdu�Ļ�������uip��������ʱ��
 *         ������ֱ�ӷŵ��������������������д���
 *         �����������ȡϵͳ��10msʱ�ӣ�������ʱ��
 *         ����Ķ�ʱ
 **************************************************/
void rt_app_bacnet_udp_rcv_entry(void* para)
{
       
    int bytes_read;
    rt_uint8_t *recv_data;
    rt_uint32_t addr_len;
    struct sockaddr_in server_addr, client_addr;
    rt_uint32_t i;
    rt_uint32_t local_ad;
            
    
    /* ��������õ����ݻ��� */
    for(i = 0; i < BACNET_UDP_RCV_BUF_NUM_MAX; i++)
    {
        bac_udp_conn.rcv_buf[i].buf = rt_malloc(MAX_MPDU);
        if (bac_udp_conn.rcv_buf[i].buf == RT_NULL)
        {
            rt_kprintf("bacnet no memory\n");   /* �����ڴ�ʧ�ܣ����� */
            return;
        }
    }
    recv_data = rt_malloc(MAX_MPDU);
    if (recv_data == RT_NULL)
    {
        rt_kprintf("bacnet no memory\n");   /* �����ڴ�ʧ�ܣ����� */
        return;
    }
    
    /* ����һ��socket��������SOCK_DGRAM��UDP���� */
    if ((bac_udp_conn.sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        rt_kprintf("Socket error\n");           /* �ͷŽ����õ����ݻ��� */
        rt_free(recv_data);
        return;
    }
    /* ��ʼ������˵�ַ */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(BACNET_UDP_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    
    if (bind(bac_udp_conn.sock, (struct sockaddr *) &server_addr, 
                sizeof(struct sockaddr)) == -1)   /* ��socket������˵�ַ */
    {
        rt_kprintf("Bind error\n");     /* �󶨵�ַʧ�� */
        rt_free(recv_data);             /* �ͷŽ����õ����ݻ��� */
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
                /* GWC�յ���Դ��ַ���Ǳ�����֡��ͨ��TCPֱ�ӷ��͸�Bivale���ļ��� */
                /* �㲥�����п�������������û��Ҫ���� */
                local_ad = LOCAL_IP_ADDR;
                if(client_addr.sin_addr.s_addr != htonl(local_ad))
                    gwc_tcp_send_frame(recv_data, bytes_read, ntohl(client_addr.sin_addr.s_addr));
            }
            else
            {   
                /* FLC��UDP���յ������ݣ�����bacnet stack�д��� */
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
 * ��������rt_app_bacnet_task_entry
 * ��  �ܣ�bacnet������
 * ��  ����
 * ��  �أ�
 * ��  ע��ͨ������ȴ�����
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
        /* �ȴ�������Ϣ��ÿ�εȴ�1s */
        if (rt_mb_recv(bac_udp_conn.mb, (rt_uint32_t*)&rcv_buf, 1000) == RT_EOK)
        {
            /* ����ȡ������client_addrΪ����˳��ĵ�ַ������ת��Ϊ����˳�� */
            client_addr.sin_addr.s_addr = htonl(rcv_buf->saddr.sin_addr.s_addr);
            client_addr.sin_port = htons(rcv_buf->saddr.sin_port);
            bytes_read = datalink_receive(&src, rcv_buf->buf, MAX_MPDU, 0, 
                            client_addr, rcv_buf->len);
            if (bytes_read) 
            {   /* NPDU ���ݴ��� */
                npdu_handler(&src, rcv_buf->buf, bytes_read);
            }
            rcv_buf->len = 0;           /* ������ɣ��������� */
        }
    }
}

/***************************************************
 * ��������bacnet_polling_timerout
 * ��  �ܣ�bacnetЭ��ջ��ʱ������
 * ��  ����
 * ��  �أ�
 * ��  ע��1s����һ�Σ�ͨ��rt_timer�ص�
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
 * ��������rt_app_bacnet_init
 * ��  �ܣ�bacnet ģ���ʼ��
 * ��  ����
 * ��  �أ�
 * ��  ע��
 **************************************************/
int rt_app_bacnet_init(void)
{
    rt_thread_t tid;
    
    board_mode = app_global_para_get(GLOBAL_PARA_BOARD_MODE);
    if((board_mode != BOARD_MODE_GWC) && (board_mode != BOARD_MODE_FLC))
        return 0;
    
    /* ����bacnet��1s��ѯ���� */
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
    
    /* �������߳� */ 
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




