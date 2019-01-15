/*
 * File      : calendar.c
 * function  : bacnet calendar 日历
 * Change Logs:
 * Date           Author  Notes
 * 2018-06-27     limian  create object
 * 2018-10-11     limian  调试完成，可以正常写入日历数据，每次写入一年
 *                        12*32个数据
 *
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "bactext.h"
#include "config.h"     /* the custom stuff */
#include "bacdevice.h"
#include "handlers.h"
#include "timestamp.h"
#include "bac_calendar.h"
#include "app_calendar.h"


#ifndef MAX_FLC_CALENDAR
#define MAX_FLC_CALENDAR MAX_CALENDAR_COUNT
#endif

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
	
    PROP_CALENDARDATA,
    -1
};

static const int Properties_Optional[] = {
	PROP_DESCRIPTION,
    -1
};

static const int Properties_Proprietary[] = {
    -1
};


void Flc_Calendar_Property_Lists(
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


void Flc_Calendar_Init(void)
{
    unsigned i;

    for (i = 0; i < MAX_FLC_CALENDAR; i++) {
	/* 变量初始化 */
	
    }
}


bool Flc_Calendar_Valid_Instance(
    uint32_t object_instance)
{
    unsigned int index;

    index = Flc_Calendar_Instance_To_Index(object_instance);
    if (index < MAX_FLC_CALENDAR)
        return true;

    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Flc_Calendar_Count(
    void)
{
    return MAX_FLC_CALENDAR;
}


uint32_t Flc_Calendar_Index_To_Instance(
    unsigned index)
{
    return index + 1;
}

unsigned Flc_Calendar_Instance_To_Index(uint32_t object_instance)
{
    unsigned index = MAX_FLC_CALENDAR;

    if (object_instance <= MAX_FLC_CALENDAR)
        index = object_instance - 1;

    return index;
}

bool Flc_Calendar_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[32] = "";   /* okay for single thread */
    unsigned int index;
    bool status = false;

    index = Flc_Calendar_Instance_To_Index(object_instance);
    if (index < MAX_FLC_CALENDAR) {
        sprintf(text_string, "FLC CALENDAR");
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}


/* return apdu length, or BACNET_STATUS_ERROR on error */
/* assumption - object has already exists */
int Flc_Calendar_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    /* bacnet不读取日历 */
    int apdu_len = 0;   /* return value */
    BACNET_CHARACTER_STRING char_string;
    unsigned object_index = 0;
    uint8_t *apdu = NULL;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }

    object_index = Flc_Calendar_Instance_To_Index(rpdata->object_instance);
    if (object_index >= MAX_FLC_CALENDAR)
        return BACNET_STATUS_ERROR;

    apdu = rpdata->application_data;
    switch ((int) rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len = encode_application_object_id(&apdu[0], OBJECT_CALENDAR,
								rpdata->object_instance + 1);
            break;
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            Flc_Calendar_Object_Name(rpdata->object_instance, &char_string);
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len = encode_application_enumerated(&apdu[0], OBJECT_CALENDAR);
            break;

        
        case PROP_CALENDARDATA:
            apdu_len = 0;
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
bool Flc_Calendar_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
	bool status = false;        /* return value */
	unsigned int object_index = 0;
	int len = 0;
	BACNET_APPLICATION_DATA_VALUE *value;    
    uint32_t value_len = 1;
    uint32_t left;
    uint8_t temp[CALENDAR_MEM_SIZE];
    uint32_t cnt;
    
    value = (BACNET_APPLICATION_DATA_VALUE *)rt_malloc(
                sizeof(BACNET_APPLICATION_DATA_VALUE)*CALENDAR_MEM_SIZE);
    if(value == RT_NULL)
        return false;
	/* 应用数据解码 */
	len = bacapp_decode_application_data_complex(wp_data->application_data,
	   wp_data->application_data_len, value, CALENDAR_MEM_SIZE, &left);          
	value_len = len;
	if (len < 0) {
        /* error while decoding - a value larger than we can handle */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        rt_free(value);
        return false;
	}

	object_index = Flc_Calendar_Instance_To_Index(wp_data->object_instance);
	if (object_index >= MAX_FLC_CALENDAR) {
        rt_free(value);
		return false;
	}

	switch ((int) wp_data->object_property) {
        case PROP_CALENDARDATA:
            if(value_len != CALENDAR_MEM_SIZE)
            {
                rt_free(value);
                return false;
            }
            for(cnt = 0; cnt < value_len; cnt++)
            {
                status = WPValidateArgType(&value[cnt], BACNET_APPLICATION_TAG_UNSIGNED_INT,
								&wp_data->error_class, &wp_data->error_code);
                if (status) 
                {
                    temp[cnt] = (uint8_t)value[cnt].type.Unsigned_Int;
                }
                else
                {
                    rt_free(value);
                    return false;
                }
            }
            app_calendar_write(temp, wp_data->object_instance);
            
//            rt_kprintf("calendar check start*******************\r\n");
//            rt_kprintf("ID:  %d\r\n", wp_data->object_instance);
//            for(cnt = 0; cnt < value_len; cnt++)
//            {
//                if(temp[cnt] != 0)
//                {
//                    rt_kprintf("%d---%d\r\n", temp[cnt], cnt);
//                }
//            }
//            rt_kprintf("calendar check end*******************\r\n\r\n");
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
