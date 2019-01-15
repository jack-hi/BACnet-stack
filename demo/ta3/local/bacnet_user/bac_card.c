/*
 * File      : card.c
 * function  : bacnet card  写入卡，删除卡，修改卡操作
 * Change Logs:
 * Date           Author  Notes
 * 2018-06-28     limian  create object
 * 2018-10-10     limian  调试完成
 * 2018-12-04  limian  卡号改成64位
 *
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <rtthread.h>

#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "bactext.h"
#include "config.h"     /* the custom stuff */
#include "bacdevice.h"
#include "handlers.h"
#include "timestamp.h"
#include "bac_card.h"
#include "app_card_message.h"

/*
PROP_CARDID = 69682,		
PROP_ACTION = 2,

PROP_ACTION取值 ：
0x00 新增卡，
0x01 删除卡
0x02 修改卡
*/
typedef struct
{
    uint32_t no;
    BACNET_DATE valid_date;
    BACNET_DATE expier_date;
}card_pass_pattern_def;



typedef struct 
{
    uint32_t index;
    uint32_t id;                            /* 0x10 */
    uint32_t id_h;                          
    uint32_t pass_code;                     /* 0x11 */
    uint32_t type;                          /* 0x12 */
    int state;                              /* 0x13 */
    BACNET_DATE vilid_data;                 /* 0x14 */
    BACNET_DATE expier_date;                /* 0x15 */
    uint32_t pass_code_period;              /* 0x16 */
    BACNET_DATE pass_code_expierdate;       /* 0x17 */
    uint8_t route_privilege;                /* 0x18 */
    card_pass_pattern_def pass_pattern[MAX_PASS_PATTERN_PER_CARD];  /* 0x20  0x21 0x22*/
}card_bivale_def;

uint32_t card_action = 0xFFFFFFFF;
card_bivale_def card_bivale_current;

#define MAX_PROP_VALUE_COUNT        25      /* 结构体内最多24个value值 */

#ifndef MAX_FLC_CARD
#define MAX_FLC_CARD 1
#endif

#define CARD_ACTION_ADD_NEW         0x00    /* 新增卡 */
#define CARD_ACTION_DELETE          0x01    /* 删除卡 */
#define CARD_ACTION_CHANGE          0x02    /* 修改卡 */


bool Flc_Card_Add_New(card_bivale_def card_b);


/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
	
    PROP_ACTION,
    PROP_CARDDATA,
    
    -1
};

static const int Properties_Optional[] = {
	PROP_DESCRIPTION,
    -1
};

static const int Properties_Proprietary[] = {
    -1
};


void Flc_Card_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    if (pRequired)
        *pRequired = Properties_Required;
    if (pOptional)
        *pOptional = Properties_Optional;
    if (pProprietary)
        *pProprietary = Properties_Proprietary;

    return;
}


void Flc_Card_Init(void)
{
    unsigned i;

    for (i = 0; i < MAX_FLC_CARD; i++) {
	/* 变量初始化 */
	
    }
}


bool Flc_Card_Valid_Instance(
    uint32_t object_instance)
{
    unsigned int index;

    index = Flc_Card_Instance_To_Index(object_instance);
    if (index < MAX_FLC_CARD)
        return true;

    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Flc_Card_Count(
    void)
{
    return MAX_FLC_CARD;
}


uint32_t Flc_Card_Index_To_Instance(
    unsigned index)
{
    return index;
}

unsigned Flc_Card_Instance_To_Index(uint32_t object_instance)
{
    unsigned index = MAX_FLC_CARD;

    if (object_instance < MAX_FLC_CARD)
        index = object_instance;

    return index;
}

bool Flc_Card_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[32] = "";   /* okay for single thread */
    unsigned int index;
    bool status = false;

    index = Flc_Card_Instance_To_Index(object_instance);
    if (index < MAX_FLC_CARD) {
        sprintf(text_string, "FLC CARD");
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}


/* return apdu length, or BACNET_STATUS_ERROR on error */
/* assumption - object has already exists */
int Flc_Card_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    BACNET_CHARACTER_STRING char_string;
    unsigned object_index = 0;
    uint8_t *apdu = NULL;
	uint32_t i, j;
    BACNET_DATE date = {2018, 6, 28, 4};

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }

    object_index = Flc_Card_Instance_To_Index(rpdata->object_instance);
    if (object_index >= MAX_FLC_CARD)
        return BACNET_STATUS_ERROR;

    apdu = rpdata->application_data;
    switch ((int) rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len = encode_application_object_id(&apdu[0], OBJECT_CARD,
								rpdata->object_instance + 1);
            break;
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            Flc_Card_Object_Name(rpdata->object_instance, &char_string);
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len = encode_application_enumerated(&apdu[0], OBJECT_CARD);
            break;

        
        case PROP_PASSCODE:
            apdu_len = encode_application_unsigned(&apdu[0], 999);
            break;
        case PROP_TYPE:
            apdu_len = encode_application_unsigned(&apdu[0], 333);
            break;
        case PROP_STATE:
            apdu_len = encode_application_signed(&apdu[0], 11);
            break;
        case PROP_VALIDDATE:
            apdu_len = encode_application_date(&apdu[0], &date);
            break;
        case PROP_EXPIERDATE:
            apdu_len = encode_application_date(&apdu[0], &date);
            break;
        case PROP_PASSCODEPERIOD:
            apdu_len = encode_application_unsigned(&apdu[0], 44);
            break;
        case PROP_PASSCODEEXPIERDATE:
            apdu_len = encode_application_date(&apdu[0], &date);
            break;
        case PROP_ROUTEPRIVILEGE:
            apdu_len = encode_application_boolean(&apdu[0], 1);
            break;
        case PROP_PASSPATTERN:
            for(i = 0; i < 5; i++)
            {
                apdu_len += encode_opening_tag(&apdu[apdu_len], TAG_PATTERNS);
                apdu_len += encode_application_unsigned(&apdu[apdu_len], j+1);
                apdu_len += encode_application_date(&apdu[apdu_len], &date);
                apdu_len += encode_application_date(&apdu[apdu_len], &date);
                apdu_len += encode_closing_tag(&apdu[apdu_len], TAG_PATTERNS);
			}                
            break;
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }
    return apdu_len;
}

/* returns true if successful */
bool Flc_Card_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
	bool status = false;        /* return value */
	int len = 0;
	BACNET_APPLICATION_DATA_VALUE *value;    /* 最多需读取数据 */
    uint32_t i, j;
    uint32_t left;
    uint32_t new_left;
    uint8_t *data;
    
    value = rt_malloc(sizeof(BACNET_APPLICATION_DATA_VALUE) * MAX_PROP_VALUE_COUNT);
    if(value == RT_NULL)
        return false;
    

	switch ((int) wp_data->object_property) {
        case PROP_ACTION:
            	/* 应用数据解码 */
            len = bacapp_decode_application_data_complex(wp_data->application_data,
               wp_data->application_data_len, value, 1, &left);          

            if (len < 0) {
               /* error while decoding - a value larger than we can handle */
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                rt_free(value);
                return false;
            }
            
            card_action = value[0].type.Unsigned_Int;
            status = true;
            break;
        case PROP_CARDDATA: 
            new_left = wp_data->application_data_len;
            left = new_left;
            data = wp_data->application_data;
            while(new_left)
            {
                /* 应用数据解码 */
                data += (left - new_left);
                left = new_left;
                len = bacapp_decode_application_data_complex(data,
                   left, value, MAX_PROP_VALUE_COUNT, &new_left);          

                if (len < 0) {
                   /* error while decoding - a value larger than we can handle */
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                    rt_free(value);
                    return false;
                }
                
                if(card_action == CARD_ACTION_DELETE)
                {
                    card_bivale_current.index = value[0].type.Unsigned_Int;
                    card_bivale_current.id = value[1].type.Unsigned_Int;
                    card_bivale_current.id_h = value[2].type.Unsigned_Int;
                    delete_card_message(card_bivale_current.id, 
                                        card_bivale_current.id_h, 1);
                    status = true;
                    continue;
                }
                if(len < 10)
                {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                    rt_free(value);
                    return false;               
                }
                i = 10;
                while(i <= MAX_PROP_VALUE_COUNT)
                {
                    if(len == i)
                        break;
                    i += 3;
                }
                if(i > MAX_PROP_VALUE_COUNT)
                {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                    rt_free(value);
                    return false;    
                }
                card_bivale_current.index = value[0].type.Unsigned_Int;
                card_bivale_current.id = value[1].type.Unsigned_Int;
                card_bivale_current.id_h = value[2].type.Unsigned_Int;
                card_bivale_current.pass_code = value[3].type.Unsigned_Int;
                card_bivale_current.type = value[4].type.Unsigned_Int;
                card_bivale_current.state = value[5].type.Signed_Int;
                card_bivale_current.vilid_data = value[6].type.Date;
                card_bivale_current.expier_date = value[7].type.Date;
                card_bivale_current.pass_code_period = value[8].type.Unsigned_Int;
                card_bivale_current.pass_code_expierdate = value[9].type.Date;
                card_bivale_current.route_privilege = value[10].type.Boolean;
                for(i = 11, j = 0; i < len; i+=3, j++)
                {
                    card_bivale_current.pass_pattern[j].no = value[i].type.Unsigned_Int;
                    card_bivale_current.pass_pattern[j].valid_date = value[i+1].type.Date;
                    card_bivale_current.pass_pattern[j].expier_date = value[i+2].type.Date;                
                }
                if((card_action == CARD_ACTION_ADD_NEW) ||
                    (card_action == CARD_ACTION_CHANGE))
                    Flc_Card_Add_New(card_bivale_current);
            }
            card_action = 0xFFFFFFFF;
            status = true;
            break;

		case PROP_OBJECT_IDENTIFIER:
		case PROP_OBJECT_NAME:
		case PROP_DESCRIPTION:
		case PROP_OBJECT_TYPE:
		   wp_data->error_class = ERROR_CLASS_PROPERTY;
		   wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
		   break;
		default:
		   wp_data->error_class = ERROR_CLASS_PROPERTY;
		   wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
		   break;
	}
    rt_free(value);
	return status;
}



bool Flc_Card_Add_New(card_bivale_def card_b)
{
    struct str_card_message *card;
    bool status;
    
    card = rt_malloc(sizeof(struct str_card_message));
    if(card == RT_NULL)
        return false;
    memset((rt_uint8_t *)card, 0, sizeof(struct str_card_message));
    
    
    card->index = card_b.index;         
    card->id = card_b.id;	
    card->id_64_h = card_b.id_h;                          /* 64位ID号使用 */
	get_date_now(&card->update_date);
    
	card->valid_date = card_b.vilid_data;	    /* 激活日期，认证信息可用的第一天*/
	card->expier_date = card_b.expier_date;		/* 到期日期，认证信息可用的最后一天 4 20 */
	card->num_code = card_b.pass_code;			/* 4位数字密码  */
    
    card->code_validity = card_b.pass_code_period;
    if(card->code_validity)
        card->code_validity_enable = true;
    else
        card->code_validity_enable = false;
	card->card_state = card_b.state;			/* 卡状态 1 28 */
	card->cart_type = card_b.type;			    /* 卡类型 1 29 */
    for(rt_uint32_t i = 0; i < MAX_PASS_PATTERN_PER_CARD; i++)
    {
        card->pass_pattern[i].no = card_b.pass_pattern[i].no;
        card->pass_pattern[i].start_date = card_b.pass_pattern[i].valid_date;
        card->pass_pattern[i].end_date = card_b.pass_pattern[i].expier_date;
    }
	card->screen_id = card_b.id;			/* 要显示在屏幕上以识别卡的号码 4 88 */ 
    if(add_card_message(card, 1) == 1)
        status = true;
    else
        status = false;
    rt_free(card);
    return status;
}
