/*
 * File      : door.c
 * function  : bacnet door 门对象实现
 * Change Logs:
 * Date         Author  Notes
 * 2018-06-25   limian  create object
 * 2018-10-10   limian  使用了PROP_OPENVALIDTIME和PROP_OPENOVERTIME
 *                        用来设置开门有效时间和超时报警时间
 * 2018-10-15   limian  完成属性 PROP_PATTERNS 的写操作，用于设置门的
 *                      临时时间表
 * 2018-11-13   limian  增加自动恢复临时时间表的时间表
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
#include "bac_door.h"
#include "app_door.h"
#include "app_schedule.h"

#define MAX_FLC_DOORS   DOOR_NUM_COUNT

/* 4扇门 */

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
	
	PROP_DOORSCHEDULENO,
	PROP_CALENDARNO,
	PROP_OPENVALIDTIME ,
	PROP_OPENOVERTIME,
	PROP_TIMEOUTIN,
	PROP_TIMEOUTOUT,
	PROP_MODEIN,
	PROP_MODEOUT,
	PROP_PATTERNS,
	PROP_ROUTENOIN,
	PROP_ROUTENOOUT,
	PROP_VALIDIN,
	PROP_PRIVILEGEDNUMIN,
	PROP_AUTHTIMEOUTIN,
	PROP_VALIDOUT,
	PROP_PRIVILEGEDNUMOUT,
	PROP_AUTHTIMEOUTOUT,
	PROP_RESTOREOPE,
	PROP_RESTORETIME_UINT,
	PROP_VALID,
	PROP_RESTORETIME_TIME,
	PROP_INPUTDOORNO,
    -1
};

static const int Properties_Optional[] = {
	PROP_DESCRIPTION,
    -1
};

static const int Properties_Proprietary[] = {
    -1
};

void Flc_Door_Property_Lists(
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


void Flc_Door_Init(
    void)
{
    unsigned i;

    for (i = 0; i < MAX_FLC_DOORS; i++) {
	/* 变量初始化 */
	
    }
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need validate that the */
/* given instance exists */
bool Flc_Door_Valid_Instance(
    uint32_t object_instance)
{
    unsigned int index;

    index = Flc_Door_Instance_To_Index(object_instance);
    if (index < MAX_FLC_DOORS)
        return true;

    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Flc_Door_Count(
    void)
{
    return MAX_FLC_DOORS;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the instance */
/* that correlates to the correct index */
uint32_t Flc_Door_Index_To_Instance(
    unsigned index)
{
    if(index < MAX_FLC_DOORS)
        return index + 1;
    return 0;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the index */
/* that correlates to the correct instance number */
unsigned Flc_Door_Instance_To_Index(
    uint32_t object_instance)
{
    unsigned index = MAX_FLC_DOORS;

    if ((object_instance <= MAX_FLC_DOORS) && (object_instance > 0))
        index = object_instance - 1;

    return index;
}

bool Flc_Door_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[32] = "";   /* okay for single thread */
    unsigned int index;
    bool status = false;

    index = Flc_Door_Instance_To_Index(object_instance);
    if (index < MAX_FLC_DOORS) {
        sprintf(text_string, "DOOR %lu", (unsigned long) object_instance);
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}

/* return apdu length, or BACNET_STATUS_ERROR on error */
/* assumption - object has already exists */
int Flc_Door_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    BACNET_CHARACTER_STRING char_string;
    unsigned object_index = 0;
    uint8_t *apdu = NULL;
    uint32_t i, j;
    BACNET_TIME time;
    uint32_t door;
    str_schedule_general_def schedule;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }

    object_index = Flc_Door_Instance_To_Index(rpdata->object_instance);
    if (object_index >= MAX_FLC_DOORS)
        return BACNET_STATUS_ERROR;
    door = rpdata->object_instance;

    apdu = rpdata->application_data;
    switch ((int) rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len =
                encode_application_object_id(&apdu[0], OBJECT_DOOR,
                rpdata->object_instance  + 1);
            break;
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            Flc_Door_Object_Name(rpdata->object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0], OBJECT_DOOR);
            break;
        
        
		case PROP_DOORSCHEDULENO:
		case PROP_CALENDARNO:
		case PROP_OPENVALIDTIME:
		case PROP_OPENOVERTIME:
		case PROP_TIMEOUTIN:
		case PROP_TIMEOUTOUT:
		case PROP_MODEIN:
		case PROP_MODEOUT:
        case PROP_ROUTENOIN:
		case PROP_ROUTENOOUT:
		case PROP_VALIDIN:
		case PROP_PRIVILEGEDNUMIN:
		case PROP_AUTHTIMEOUTIN:
		case PROP_VALIDOUT:      
		case PROP_PRIVILEGEDNUMOUT:
		case PROP_AUTHTIMEOUTOUT:
		case PROP_RESTOREOPE:
		case PROP_RESTORETIME_UINT:
		case PROP_VALID:
		case PROP_RESTORETIME_TIME:
            apdu_len = 0;
            break;
		case PROP_PATTERNS:
            if(app_schedule_read(&schedule, door + SCHEDULE_TEMP_DOOR_NO_START, 
                    SCHEDULE_TYPE_TEMP) != RT_EOK)
                return 0;
            apdu_len = 0;
            time.hundredths = 0; 
            time.sec = 0;
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

#define MAX_PROP_VALUE_COUNT        30      /* 结构体内最多24个value值 */

/* returns true if successful */
bool Flc_Door_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
	bool status = false;        /* return value */
	unsigned int object_index = 0;
	int len = 0;
	BACNET_APPLICATION_DATA_VALUE *value;    /* 最多需读取15组数据 */
    uint32_t value_len = 1;
    uint32_t left;
    uint32_t door;
    uint8_t *data;
    uint32_t day_cnt, mode_cnt;
    str_schedule_general_def schedule;
    BACNET_TIME restore_time[5];
    
    /* 分配空间 */
    value = rt_malloc(sizeof(BACNET_APPLICATION_DATA_VALUE) * MAX_PROP_VALUE_COUNT);
    if(value == RT_NULL)
        return false;
    
    /* 判断索引号是否符合要求 */
	object_index = Flc_Door_Instance_To_Index(wp_data->object_instance);
	if (object_index < MAX_FLC_DOORS) {
        door = wp_data->object_instance;
	} else {
        rt_free(value);
		return false;
	}
    
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
		case PROP_DOORSCHEDULENO:
			status = WPValidateArgType(&value[0], BACNET_APPLICATION_TAG_UNSIGNED_INT,
								&wp_data->error_class, &wp_data->error_code);
			if (status) {
				app_door_set_door_setting_para_uint32(door,  DOOR_SETTING_PARA_SCHEDULE_NO,
                    value[0].type.Unsigned_Int);
			}
			break;
            
		case PROP_CALENDARNO:
			status = WPValidateArgType(&value[0], BACNET_APPLICATION_TAG_UNSIGNED_INT,
								&wp_data->error_class, &wp_data->error_code);
			if (status) {
				app_door_set_door_setting_para_uint32(door,  DOOR_SETTING_PARA_CALENDAR_NO,
                    value[0].type.Unsigned_Int);
			}
			break;
            
		case PROP_OPENVALIDTIME:
			status = WPValidateArgType(&value[0], BACNET_APPLICATION_TAG_UNSIGNED_INT,
								&wp_data->error_class, &wp_data->error_code);
			if (status) {
				app_door_set_door_setting_para_uint32(door,  
                DOOR_SETTING_PARA_OPEN_VALID_TIME, value[0].type.Unsigned_Int);
			}
			break;
            
		case PROP_OPENOVERTIME:
			status = WPValidateArgType(&value[0], BACNET_APPLICATION_TAG_UNSIGNED_INT,
								&wp_data->error_class, &wp_data->error_code);
			if (status) {
				app_door_set_door_setting_para_uint32(door,  
                DOOR_SETTING_PARA_OPEN_OVER_TIME, value[0].type.Unsigned_Int);
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
            rt_kprintf("door temp schedule write start****************\r\n");
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
            app_schedule_write(&schedule, wp_data->object_instance + 
                    SCHEDULE_TEMP_DOOR_NO_START, 1, SCHEDULE_TYPE_TEMP);
            status = true;
            
			break;
        
        case PROP_VALID:
            status = WPValidateArgType(&value[0], BACNET_APPLICATION_TAG_BOOLEAN,
								&wp_data->error_class, &wp_data->error_code);
			if (status) {
				app_door_set_door_setting_para_uint32(door,  
                DOOR_SETTING_PARA_SCHEDULE_RESTORE_ENABLE, value[0].type.Boolean);
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
            app_door_set_schedule_restore_time(door, restore_time);
            status = true;
            break;
        /* 未使用的属性 */
 		case PROP_TIMEOUTIN:
		case PROP_TIMEOUTOUT:
		case PROP_MODEIN:
		case PROP_MODEOUT:       
		case PROP_ROUTENOIN:
		case PROP_ROUTENOOUT:
		case PROP_VALIDIN:
		case PROP_PRIVILEGEDNUMIN:
		case PROP_AUTHTIMEOUTIN:
		case PROP_VALIDOUT:      
		case PROP_PRIVILEGEDNUMOUT:
		case PROP_AUTHTIMEOUTOUT:
		
		case PROP_RESTORETIME_UINT:
		case PROP_RESTOREOPE:
		
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



