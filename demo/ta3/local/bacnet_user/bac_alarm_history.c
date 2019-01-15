/*
 * File    : alarm_history.c
 * function: 警报记录读取
 * Change Logs:
 * Date           Author  Notes
 * 2018-08-11     limian  first version
 * 2018-10-10     limian  调试完成
 * 2018-12-04  limian  增加64位卡号的上传处理
 *
 * note：   
 */
 
#include "bac_alarm_history.h"
#include <dfs_posix.h> 

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
#include "app_flc_alarm.h"

#define ALARM_HISTORY_READ_ONCE_MAX      10


/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
	
    PROP_FLCHISTORYDATA,
    -1
};

static const int Properties_Optional[] = {
	PROP_DESCRIPTION,
    -1
};

static const int Properties_Proprietary[] = {
    -1
};


void Flc_Alarm_History_Property_Lists(
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


void Flc_Alarm_History_Init(void)
{

}


bool Flc_Alarm_History_Valid_Instance(
    uint32_t object_instance)
{
	(void)object_instance;
    return true;

}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Flc_Alarm_History_Count(
    void)
{
    return 1;
}


uint32_t Flc_Alarm_History_Index_To_Instance(
    unsigned index)
{
    return index;
}

unsigned Flc_Alarm_History_Instance_To_Index(uint32_t object_instance)
{
    return object_instance;
}

bool Flc_Alarm_History_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[32] = "";   /* okay for single thread */
    bool status;

	sprintf(text_string, "FLC ALARM History");
	status = characterstring_init_ansi(object_name, text_string);

    return status;
}


/* return apdu length, or BACNET_STATUS_ERROR on error */
/* assumption - object has already exists */
int Flc_Alarm_History_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    BACNET_CHARACTER_STRING char_string;
    uint8_t *apdu = NULL;
	uint32_t i;
    uint32_t start_no;
    struct str_card_history history[ALARM_HISTORY_READ_ONCE_MAX];
    uint32_t count = 1;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }

    start_no = rpdata->object_instance;

    apdu = rpdata->application_data;
    switch ((int) rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len = encode_application_object_id(&apdu[0], OBJECT_FLCALARMHISTORY,
								rpdata->object_instance + 1);
            break;
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            Flc_Alarm_History_Object_Name(rpdata->object_instance, &char_string);
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len = encode_application_enumerated(&apdu[0], OBJECT_FLCALARMHISTORY);
            break;
            
        case PROP_FLCHISTORYDATA:
            
            /* 读取20条记录 */
            count = read_flc_alarm_record(history, start_no, ALARM_HISTORY_READ_ONCE_MAX);
            if(count == 0)      /* 已经读完 */ 
            {
                break;
            }
            apdu_len += encode_opening_tag(&apdu[apdu_len], 0);
            for(i = 0; i < count; i++)
            {
                apdu_len += encode_opening_tag(&apdu[apdu_len], 1);
                apdu_len += encode_application_unsigned(&apdu[apdu_len], history[i].id);
                apdu_len += encode_application_date(&apdu[apdu_len], &history[i].data_time.date);
                apdu_len += encode_application_time(&apdu[apdu_len], &history[i].data_time.time);
                apdu_len += encode_application_unsigned(&apdu[apdu_len], history[i].device_id);
                apdu_len += encode_application_unsigned(&apdu[apdu_len], history[i].type);
                apdu_len += encode_application_unsigned(&apdu[apdu_len], history[i].door_no);
                apdu_len += encode_application_unsigned(&apdu[apdu_len], history[i].cr_no);
                apdu_len += encode_application_unsigned(&apdu[apdu_len], history[i].card_id);
                apdu_len += encode_application_unsigned(&apdu[apdu_len], history[i].card_id_h);
                apdu_len += encode_application_unsigned(&apdu[apdu_len], history[i].message_no);
                apdu_len += encode_application_unsigned(&apdu[apdu_len], history[i].explanation_no);
                apdu_len += encode_application_unsigned(&apdu[apdu_len], history[i].sequence_no);
                apdu_len += encode_closing_tag(&apdu[apdu_len], 1);
            }
            apdu_len += encode_closing_tag(&apdu[apdu_len], 0);
            break;
        case PROP_ISLAST:
            apdu_len += encode_application_boolean(&apdu[apdu_len], 1);
            break;
        case PROP_RECENTID:
            apdu_len += encode_application_unsigned(&apdu[apdu_len], start_no);
            break;
        default:
            return 0;
    }
    return apdu_len;
}

/* returns true if successful */
bool Flc_Alarm_History_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    return true;
}



