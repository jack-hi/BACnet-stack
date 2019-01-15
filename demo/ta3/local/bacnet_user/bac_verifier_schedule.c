/*
 * File      : bac_verifier_schedule.c
 * function  : bacnet verifier_schedule 读卡器时间表
 * Change Logs:
 * Date           Author       Notes
 * 2018-06-28     limian      create object
 * 2018-10-15     limian 
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
#include "bac_verifier_schedule.h"
#include "app_schedule.h"



#ifndef MAX_FLC_VERIFIER_SCHEDULE
#define MAX_FLC_VERIFIER_SCHEDULE MAX_VERIFIER_SCHEDULE_COUNT
#endif

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
	
    PROP_SCHEDULE,
    -1
};

static const int Properties_Optional[] = {
	PROP_DESCRIPTION,
    -1
};

static const int Properties_Proprietary[] = {
    -1
};


void Flc_Verifier_Schedule_Property_Lists(
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


void Flc_Verifier_Schedule_Init(void)
{
    unsigned i;

    for (i = 0; i < MAX_FLC_VERIFIER_SCHEDULE; i++) {
	/* 变量初始化 */
	
    }
}


bool Flc_Verifier_Schedule_Valid_Instance(
    uint32_t object_instance)
{
    unsigned int index;

    index = Flc_Verifier_Schedule_Instance_To_Index(object_instance);
    if (index < MAX_FLC_VERIFIER_SCHEDULE)
        return true;

    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Flc_Verifier_Schedule_Count(
    void)
{
    return MAX_FLC_VERIFIER_SCHEDULE;
}


uint32_t Flc_Verifier_Schedule_Index_To_Instance(
    unsigned index)
{
    if(index < MAX_FLC_VERIFIER_SCHEDULE)
        return index + 1;
    return index;
}

unsigned Flc_Verifier_Schedule_Instance_To_Index(uint32_t object_instance)
{
    unsigned index = MAX_FLC_VERIFIER_SCHEDULE;

    if ((object_instance <= MAX_FLC_VERIFIER_SCHEDULE) &&
        (object_instance > 0))
        index = object_instance - 1;

    return index;
}

bool Flc_Verifier_Schedule_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[32] = "";   /* okay for single thread */
    unsigned int index;
    bool status = false;

    index = Flc_Verifier_Schedule_Instance_To_Index(object_instance);
    if (index < MAX_FLC_VERIFIER_SCHEDULE) {
        sprintf(text_string, "VERIFIER SCHEDULE");
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}


/* return apdu length, or BACNET_STATUS_ERROR on error */
/* assumption - object has already exists */
int Flc_Verifier_Schedule_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    BACNET_CHARACTER_STRING char_string;
    unsigned object_index = 0;
    uint8_t *apdu = NULL;
//	uint32_t i, j;
//    BACNET_TIME time = {11, 25, 0, 0};

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }

    object_index = Flc_Verifier_Schedule_Instance_To_Index(rpdata->object_instance);
    if (object_index >= MAX_FLC_VERIFIER_SCHEDULE)
        return BACNET_STATUS_ERROR;

    apdu = rpdata->application_data;
    switch ((int) rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len = encode_application_object_id(&apdu[0], OBJECT_VERIFIERSCHEDULE,
								rpdata->object_instance + 1);
            break;
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            Flc_Verifier_Schedule_Object_Name(rpdata->object_instance, &char_string);
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len = encode_application_enumerated(&apdu[0], OBJECT_VERIFIERSCHEDULE);
            break;

        case PROP_SCHEDULE:
//            for(i = 1; i <= 5; i++)
//			{
//                apdu_len += encode_opening_tag(&apdu[apdu_len], TAG_PATTERNS);
//				for(j = 0; j < 12; j++)
//                {
//                    apdu_len += encode_opening_tag(&apdu[apdu_len], TAG_PATTERN);
//					apdu_len += encode_application_unsigned(&apdu[apdu_len], j+1);
//                    apdu_len += encode_application_time(&apdu[apdu_len], &time);
//                    apdu_len += encode_application_time(&apdu[apdu_len], &time);
//                    apdu_len += encode_closing_tag(&apdu[apdu_len], TAG_PATTERN);                    
//                }
//                apdu_len += encode_closing_tag(&apdu[apdu_len], TAG_PATTERNS);
//			}
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
bool Flc_Verifier_Schedule_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
	bool status = false;        /* return value */
	unsigned int object_index = 0;
	uint32_t len = 0;
    uint32_t left = 0;
	BACNET_APPLICATION_DATA_VALUE *value;    /* 最多需读取数据 */
    uint32_t value_len = 3;
    uint32_t schedule_no;
    uint32_t day_cnt, mode_cnt;
    uint8_t *data;
    str_schedule_general_def schedule;
    
    value = (BACNET_APPLICATION_DATA_VALUE *)rt_malloc(
                sizeof(BACNET_APPLICATION_DATA_VALUE) * value_len);
    if(value == RT_NULL)
        return false;
    
	
	object_index = Flc_Verifier_Schedule_Instance_To_Index(wp_data->object_instance);
	if (object_index >= MAX_FLC_VERIFIER_SCHEDULE) {
        rt_free(value);
		return false;
	}
    schedule_no = wp_data->object_instance;

	switch ((int) wp_data->object_property) {
        case PROP_SCHEDULE:
            len = wp_data->application_data_len;    /* 数据长度 */
            data = wp_data->application_data;       /* 数据 */
            day_cnt = 0;
            mode_cnt = 0;
            memset(&schedule, 0, sizeof(str_schedule_general_def));
            while(len)
            {
                if((*data) == 0x4E)     /* 数据开始 */
                {
                    /* 应用数据解码 */
                    value_len = bacapp_decode_application_data_complex(data,
                                len, value, 3, &left);   /* 每次最多读取3个数据 */
                    data += (len - left);
                    len = left;
                    if((value_len != 1) && (value_len != 3))
                    {
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                        return false;
                    }
                    status = WPValidateArgType(&value[0], BACNET_APPLICATION_TAG_UNSIGNED_INT,
								&wp_data->error_class, &wp_data->error_code);
                    if (status)         /* 符合要求，解析成功 */
                        schedule.day[day_cnt].mode[mode_cnt].mode = value[0].type.Unsigned_Int;
                    else
                    {
                        rt_free(value);
                        return false;
                    }
                    
                    if(value_len == 3)
                    {
                        /* start time */
                        status = WPValidateArgType(&value[1], BACNET_APPLICATION_TAG_TIME,
								&wp_data->error_class, &wp_data->error_code);
                        if (status)         /* 符合要求，解析成功 */
                        {
                            schedule.day[day_cnt].mode[mode_cnt].start_time.hour = 
                                    value[1].type.Time.hour;
                            schedule.day[day_cnt].mode[mode_cnt].start_time.min = 
                                    value[1].type.Time.min;
                        }
                        else
                        {
                            rt_free(value);
                            return false;
                        }
                        /* end time */
                        status = WPValidateArgType(&value[2], BACNET_APPLICATION_TAG_TIME,
								&wp_data->error_class, &wp_data->error_code);
                        if (status)         /* 符合要求，解析成功 */
                        {
                            schedule.day[day_cnt].mode[mode_cnt].end_time.hour = 
                                    value[2].type.Time.hour;
                            schedule.day[day_cnt].mode[mode_cnt].end_time.min = 
                                    value[2].type.Time.min;
                        }
                        else
                        {
                            rt_free(value);
                            return false;
                        }
                    }
                    day_cnt++;
                }
                else if(*data == 0x2F)
                {
                    data++;
                    mode_cnt++;
                    day_cnt = 0;
                    len--;
                }
                else
                {
                    data++;
                    len--;
                }
            }
            rt_kprintf("verifier_schedule write start****************\r\n");
            for(uint32_t i = 0; i < 12; i++)
            {
                rt_kprintf("day-------%d\r\n", i+1);
                for(uint32_t j = 0; j < 5; j++)
                {
                    rt_kprintf("mode-%d  md:%d  s_t %d:%d   e_t %d:%d\r\n",
                        j+1,   schedule.day[i].mode[j].mode,
                        schedule.day[i].mode[j].start_time.hour, 
                        schedule.day[i].mode[j].start_time.min, 
                        schedule.day[i].mode[j].end_time.hour, 
                        schedule.day[i].mode[j].end_time.min);
                }
            }
            rt_kprintf("end**************************************\r\n\r\n");
            app_schedule_write(&schedule, schedule_no, 1, SCHEDULE_TYPE_VERIFIER);
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
