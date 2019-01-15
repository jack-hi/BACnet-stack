/*
 * File      : pass_pattern.c
 * function  : bacnet pass_pattern 对象
 * Change Logs:
 * Date         Author  Notes
 * 2018-06-25   limian  create object
 * 2018-10-16  	limian  调试中
 * 
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
#include "bac_pass_pattern.h"
#include "app_pass_pattern.h"


#ifndef MAX_FLC_PASS_PATTERNS
#define MAX_FLC_PASS_PATTERNS MAX_PASS_PATTERN_COUNT
#endif

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
	
    PROP_PASSAVAILABLE,
    PROP_WARNSETAVAILABLE,
    PROP_WARNUNSETAVAILABLE,
    PROP_PASSSCHEDULENO,
    PROP_CALENDARNO,
    PROP_AVAILABLE,
    -1
};

static const int Properties_Optional[] = {
	PROP_DESCRIPTION,
    -1
};

static const int Properties_Proprietary[] = {
    -1
};


void Flc_Pass_Pattern_Property_Lists(
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


void Flc_Pass_Pattern_Init(void)
{
    unsigned i;

    for (i = 0; i < MAX_FLC_PASS_PATTERNS; i++) {
	/* 变量初始化 */
	
    }
}


bool Flc_Pass_Pattern_Valid_Instance(
    uint32_t object_instance)
{
    unsigned int index;

    index = Flc_Pass_Pattern_Instance_To_Index(object_instance);
    if (index < MAX_FLC_PASS_PATTERNS)
        return true;

    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Flc_Pass_Pattern_Count(
    void)
{
    return MAX_FLC_PASS_PATTERNS;
}


uint32_t Flc_Pass_Pattern_Index_To_Instance(
    unsigned index)
{
    if(index < MAX_FLC_PASS_PATTERNS)
        return index + 1;
    return index;
}

unsigned Flc_Pass_Pattern_Instance_To_Index(uint32_t object_instance)
{
    unsigned index = MAX_FLC_PASS_PATTERNS;

    if ((object_instance <= MAX_FLC_PASS_PATTERNS) &&
        (object_instance > 0))
        index = object_instance - 1;

    return index;
}

bool Flc_Pass_Pattern_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[32] = "";   /* okay for single thread */
    unsigned int index;
    bool status = false;

    index = Flc_Pass_Pattern_Instance_To_Index(object_instance);
    if (index < MAX_FLC_PASS_PATTERNS) {
        sprintf(text_string, "PASS PATTERN %lu", (unsigned long) index);
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}


/* return apdu length, or BACNET_STATUS_ERROR on error */
/* assumption - object has already exists */
int Flc_Pass_Pattern_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    BACNET_CHARACTER_STRING char_string;
    unsigned object_index = 0;
    uint8_t *apdu = NULL;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }

    object_index = Flc_Pass_Pattern_Instance_To_Index(rpdata->object_instance);
    if (object_index >= MAX_FLC_PASS_PATTERNS)
        return BACNET_STATUS_ERROR;

    apdu = rpdata->application_data;
    switch ((int) rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len = encode_application_object_id(&apdu[0], OBJECT_PASSPATTERN,
                       rpdata->object_instance + 1);
            break;
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            Flc_Pass_Pattern_Object_Name(rpdata->object_instance, &char_string);
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len = encode_application_enumerated(&apdu[0], OBJECT_PASSPATTERN);
            break;
        
		
        case PROP_PASSAVAILABLE:
        case PROP_WARNSETAVAILABLE:
        case PROP_WARNUNSETAVAILABLE:
        case PROP_PASSSCHEDULENO:
        case PROP_CALENDARNO:
        case PROP_AVAILABLE:
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

#define MAX_VALUE_COUNT		16
/* returns true if successful */
bool Flc_Pass_Pattern_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
	bool status = false;        /* return value */
	unsigned int object_index = 0;
	int len = 0;
	BACNET_APPLICATION_DATA_VALUE *value;    /* 最多需读取16组数据 */
    uint32_t value_len = MAX_VALUE_COUNT;
    uint32_t left;
	static str_pass_pattern_def pattern;
	uint32_t i;
    uint32_t no;
    
    
    value = (BACNET_APPLICATION_DATA_VALUE *)rt_malloc(
                sizeof(BACNET_APPLICATION_DATA_VALUE) * value_len);
    if(value == RT_NULL)
        return false;
   
	/* 应用数据解码 */
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

	object_index = Flc_Pass_Pattern_Instance_To_Index(wp_data->object_instance);
	if (object_index >= MAX_FLC_PASS_PATTERNS) {
		rt_free(value);
		return false;
	}
    no = wp_data->object_instance;
	switch ((int) wp_data->object_property) {
        case PROP_PASSAVAILABLE:
            /* 清零pattern结构体 */
            memset(&pattern, 0, sizeof(str_pass_pattern_def));
            if(value_len != MAX_VALUE_COUNT)
			{
				rt_free(value);
				return false;
			}
			for(i = 0; i < value_len; i++)
			{
				status = WPValidateArgType(&value[i], BACNET_APPLICATION_TAG_BOOLEAN,
								&wp_data->error_class, &wp_data->error_code);
				if (status)         /* 符合要求，解析成功 */
					pattern.pattern[i].pass_available = value[i].type.Boolean;
				else
				{
					rt_free(value);
					return false;
				}
			}
			break;
        case PROP_WARNSETAVAILABLE:
            if(value_len != MAX_VALUE_COUNT)
			{
				rt_free(value);
				return false;
			}
			for(i = 0; i < value_len; i++)
			{
				status = WPValidateArgType(&value[i], BACNET_APPLICATION_TAG_BOOLEAN,
								&wp_data->error_class, &wp_data->error_code);
				if (status)         /* 符合要求，解析成功 */
					pattern.pattern[i].warn_set_available = value[i].type.Boolean;
				else
				{
					rt_free(value);
					return false;
				}
			}
			break;
        case PROP_WARNUNSETAVAILABLE:
			if(value_len != MAX_VALUE_COUNT)
			{
				rt_free(value);
				return false;
			}
			for(i = 0; i < value_len; i++)
			{
				status = WPValidateArgType(&value[i], BACNET_APPLICATION_TAG_BOOLEAN,
								&wp_data->error_class, &wp_data->error_code);
				if (status)         /* 符合要求，解析成功 */
					pattern.pattern[i].warn_unset_available = value[i].type.Boolean;
				else
				{
					rt_free(value);
					return false;
				}
			}
			break;
        case PROP_PASSSCHEDULENO:
            if(value_len != MAX_VALUE_COUNT)
			{
				rt_free(value);
				return false;
			}
			for(i = 0; i < value_len; i++)
			{
				status = WPValidateArgType(&value[i], BACNET_APPLICATION_TAG_UNSIGNED_INT,
								&wp_data->error_class, &wp_data->error_code);
				if (status)         /* 符合要求，解析成功 */
					pattern.pattern[i].pass_schedule_no = value[i].type.Unsigned_Int;
				else
				{
					rt_free(value);
					return false;
				}
			}
            break;
        case PROP_CALENDARNO:
			if(value_len != MAX_VALUE_COUNT)
			{
				rt_free(value);
				return false;
			}
			for(i = 0; i < value_len; i++)
			{
				status = WPValidateArgType(&value[i], BACNET_APPLICATION_TAG_UNSIGNED_INT,
								&wp_data->error_class, &wp_data->error_code);
				if (status)         /* 符合要求，解析成功 */
					pattern.pattern[i].calendar_no = value[i].type.Unsigned_Int;
				else
				{
					rt_free(value);
					return false;
				}
			}
            app_pass_pattern_write(pattern, no);
            break;
			
        case PROP_AVAILABLE:
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
