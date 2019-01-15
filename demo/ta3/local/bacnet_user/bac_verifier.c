/*
 * File      : verifier.c
 * function  : bacnet verifier 读卡器对象
 * Change Logs:
 * Date         Author  Notes
 * 2018-06-25   limian  create object
 * 2018-10-10   limian  使用了属性PROP_MODE，用于设置读卡器验证模式
 * 2018-10-15   limian  完成属性 PROP_PATTERNS 的写操作，用于设置门的
 *                      临时时间表
 *                      完成属性 PROP_PATTERNS 的读操作，写之前会进行读取
 * 2018-11-13   limian  增加两个属性PROP_VALID,PROP_RESTORETIME_TIME,
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
#include "bac_verifier.h"
#include "app_card_reader_handler.h"
#include "app_door.h"
#include "app_schedule.h"
/* 读卡器对象 */

#ifndef MAX_FLC_VERIFIERS
#define MAX_FLC_VERIFIERS CARD_READER_NUM_COUNT
#endif

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
	
    PROP_VERIFIERSCHEDULENO,
    PROP_CALENDARNO,
    PROP_GUARDOPERATION,
    PROP_RINGVALID,
    PROP_RINGTIME,
    PROP_PATTERNS,
    PROP_MODE,
    PROP_VALID,
	PROP_RESTORETIME_TIME,
    -1
};

static const int Properties_Optional[] = {
	PROP_DESCRIPTION,
    -1
};

static const int Properties_Proprietary[] = {
    -1
};


void Flc_Verifier_Property_Lists(
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


void Flc_Verifier_Init(void)
{
    unsigned i;

    for (i = 0; i < MAX_FLC_VERIFIERS; i++) {
	/* 变量初始化 */
	
    }
}


bool Flc_Verifier_Valid_Instance(
    uint32_t object_instance)
{
    unsigned int index;

    index = Flc_Verifier_Instance_To_Index(object_instance);
    if (index < MAX_FLC_VERIFIERS)
        return true;

    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Flc_Verifier_Count(
    void)
{
    return MAX_FLC_VERIFIERS;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the instance */
/* that correlates to the correct index */
uint32_t Flc_Verifier_Index_To_Instance(
    unsigned index)
{
    return index + 1;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the index */
/* that correlates to the correct instance number */
unsigned Flc_Verifier_Instance_To_Index(uint32_t object_instance)
{
    unsigned index = MAX_FLC_VERIFIERS;

    if ((object_instance <= MAX_FLC_VERIFIERS) && (object_instance > 0))
        index = object_instance - 1;

    return index;
}



bool Flc_Verifier_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[32] = "";   /* okay for single thread */
    unsigned int index;
    bool status = false;

    index = Flc_Verifier_Instance_To_Index(object_instance);
    if (index < MAX_FLC_VERIFIERS) {
        sprintf(text_string, "VERIFIER %lu", (unsigned long) index);
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}


/* return apdu length, or BACNET_STATUS_ERROR on error */
/* assumption - object has already exists */
int Flc_Verifier_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    BACNET_CHARACTER_STRING char_string;
    unsigned object_index = 0;
    uint8_t *apdu = NULL;
    uint32_t i, j;
    uint32_t verifier = 0;
    BACNET_TIME time = {0, 0, 0, 0};
    str_schedule_general_def schedule;
    uint32_t temp;
    
    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }

    object_index = Flc_Verifier_Instance_To_Index(rpdata->object_instance);
    if (object_index > MAX_FLC_VERIFIERS)
        return BACNET_STATUS_ERROR;
    
    verifier = rpdata->object_instance;

    apdu = rpdata->application_data;
    switch ((int) rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len =
                encode_application_object_id(&apdu[0], OBJECT_VERIFIER,
                rpdata->object_instance + 1);
            break;
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            Flc_Verifier_Object_Name(rpdata->object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            verifier = find_card_reader_by_door(verifier);
            apdu_len =
                encode_application_enumerated(&apdu[0], OBJECT_VERIFIER);
            break;
        
        
        case PROP_VERIFIERSCHEDULENO:
        case PROP_CALENDARNO:
        case PROP_GUARDOPERATION:
        case PROP_RINGVALID:
        case PROP_RINGTIME:
        case PROP_VALID:
        case PROP_RESTORETIME_TIME:
            apdu_len = 0;
            break;
        case PROP_MODE:
            verifier = find_card_reader_by_door(verifier);
            temp = app_card_reader_get_para_uint32(verifier, 
                        CARD_READER_SETTING_PARA_MODE_DEFAULT);
            apdu_len = encode_application_unsigned(&apdu[0], temp);
            break;
        
        case PROP_PATTERNS:
            if(app_schedule_read(&schedule, verifier + SCHEDULE_TEMP_VERIFIER_NO_START, 
                    SCHEDULE_TYPE_TEMP) != RT_EOK)
                return 0;
            apdu_len = 0;
			for(i = 0; i < 2; i++)
			{	
				apdu_len += encode_opening_tag(&apdu[apdu_len], 1);
				for(j = 0; j < 5; j++)
				{	
					apdu_len += encode_opening_tag(&apdu[apdu_len], 2);
					apdu_len += encode_application_unsigned(&apdu[apdu_len], schedule.day[i].mode[j].mode);
                    if((schedule.day[i].mode[j].start_time.hour == 0) &&
                       (schedule.day[i].mode[j].start_time.min == 0) &&
                       (schedule.day[i].mode[j].end_time.hour == 0) &&
                       (schedule.day[i].mode[j].end_time.min == 0))
                    {
                        apdu_len += encode_closing_tag(&apdu[apdu_len], 2);
                        continue;
                    }
                    /* 起始时间 */
                    time.hour = schedule.day[i].mode[j].start_time.hour;
                    time.min = schedule.day[i].mode[j].start_time.min;
					apdu_len += encode_application_time(&apdu[apdu_len], &time);
                    /* 结束时间 */
                    time.hour = schedule.day[i].mode[j].end_time.hour;
                    time.min = schedule.day[i].mode[j].end_time.min;
					apdu_len += encode_application_time(&apdu[apdu_len], &time);
					apdu_len += encode_closing_tag(&apdu[apdu_len], 2);
				}
				apdu_len += encode_closing_tag(&apdu[apdu_len], 1);
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
bool Flc_Verifier_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
	bool status = false;        /* return value */
	unsigned int object_index = 0;
	int len = 0;
	BACNET_APPLICATION_DATA_VALUE *value;    /* 最多需读取15组数据 */
    uint32_t value_len = 1;
    uint32_t left;
    uint32_t verifier;
    uint8_t *data;
    uint32_t day_cnt, mode_cnt;
    str_schedule_general_def schedule;
    BACNET_TIME restore_time[5];
    
    value = rt_malloc(sizeof(BACNET_APPLICATION_DATA_VALUE) * 30);
    if(value == RT_NULL)
        return false;
    
	object_index = Flc_Verifier_Instance_To_Index(wp_data->object_instance);
    
	if (object_index < MAX_FLC_VERIFIERS) {
        verifier = wp_data->object_instance;
	} else {
        rt_free(value);
		return false;
	}

	/* 应用数据解码 */
    if(wp_data->object_property != PROP_PATTERNS)
    {
        if(wp_data->object_property == PROP_RESTORETIME_TIME)
            value_len = 5;              /* restore time有5个 */
        else
            value_len = 1;
        
        len = bacapp_decode_application_data_complex(wp_data->application_data,
           wp_data->application_data_len, value, value_len, &left);          
        value_len = len;
        if (len < 0) {
           /* error while decoding - a value larger than we can handle */
           wp_data->error_class = ERROR_CLASS_PROPERTY;
           wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
           rt_free(value);
           return false;
        }
    }



	switch ((int) wp_data->object_property) {
        case PROP_VERIFIERSCHEDULENO:
            status = WPValidateArgType(&value[0], BACNET_APPLICATION_TAG_UNSIGNED_INT,
								&wp_data->error_class, &wp_data->error_code);
            if (status)         /* 符合要求，解析成功 */
            {
                verifier = find_card_reader_by_door(verifier);
                app_card_reader_set_para_uint32(verifier, 
                    CARD_READER_SETTING_PARA_SCHEDULE_NO, value[0].type.Unsigned_Int);
            }
            else
            {
                rt_free(value);
                return false;
            }
            break;
        case PROP_CALENDARNO:
            status = WPValidateArgType(&value[0], BACNET_APPLICATION_TAG_UNSIGNED_INT,
								&wp_data->error_class, &wp_data->error_code);
            if (status)         /* 符合要求，解析成功 */
            {
                verifier = find_card_reader_by_door(verifier);
                app_card_reader_set_para_uint32(verifier, 
                    CARD_READER_SETTING_PARA_CALENDAR_NO, value[0].type.Unsigned_Int);
            }
            else
            {
                rt_free(value);
                return false;
            }
            break;
        case PROP_GUARDOPERATION:
        case PROP_RINGVALID:
        case PROP_RINGTIME:
            status = true;
            break;
        case PROP_MODE:
            status = WPValidateArgType(&value[0], BACNET_APPLICATION_TAG_UNSIGNED_INT,
								&wp_data->error_class, &wp_data->error_code);
			if (status) {
                verifier = find_card_reader_by_door(verifier);
                app_card_reader_set_para_uint32(verifier, 
                        CARD_READER_SETTING_PARA_MODE_DEFAULT, value[0].type.Unsigned_Int);
			}
			break;
        case PROP_PATTERNS:
            len = wp_data->application_data_len;    /* 数据长度 */
            data = wp_data->application_data;       /* 数据 */
            day_cnt = 0;
            mode_cnt = 0;
            memset(&schedule, 0, sizeof(str_schedule_general_def));
            while(len)
            {
                if((*data) == 0x2E)     /* 数据开始 */
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
                    mode_cnt++;
                }
                else if(*data == 0x1F)
                {
                    data++;
                    mode_cnt = 0;
                    day_cnt++;
                    len--;
                }
                else
                {
                    data++;
                    len--;
                }
            }
            rt_kprintf("verifier temp schedule write start****************\r\n");
            for(uint32_t i = 0; i < 2; i++)
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
            verifier = find_card_reader_by_door(verifier);
            app_schedule_write(&schedule, verifier + 
                        SCHEDULE_TEMP_VERIFIER_NO_START, 1, SCHEDULE_TYPE_TEMP);
            status = true;
			break;
            
        case PROP_VALID:
            status = WPValidateArgType(&value[0], BACNET_APPLICATION_TAG_BOOLEAN,
								&wp_data->error_class, &wp_data->error_code);
			if (status) {
                verifier = find_card_reader_by_door(verifier);
				app_card_reader_set_para_uint32(verifier,  
                            CARD_READER_SETTING_PARA_SCHEDULE_RESTORE_ENABLE, 
                            value[0].type.Boolean);
			}
			break;
        case PROP_RESTORETIME_TIME:
            for(uint32_t i = 0; i < 5; i++)
            {
                status = WPValidateArgType(&value[i], BACNET_APPLICATION_TAG_TIME,
								&wp_data->error_class, &wp_data->error_code);
                if (status) 
                    restore_time[i] = value[i].type.Time;
                else
                    return false;
            }
            verifier = find_card_reader_by_door(verifier);
            app_set_card_reader_schedule_restore_time(verifier, restore_time);
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
            status = true;
//		   wp_data->error_class = ERROR_CLASS_PROPERTY;
//		   wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
		   break;
	}
    rt_free(value);
	return status;
}
