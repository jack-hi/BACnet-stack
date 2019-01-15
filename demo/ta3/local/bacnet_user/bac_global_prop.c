/*
 * File      : global_prop.c
 * function  : bacnet global_prop FLC中一些全局属性的集合
 * Change Logs:
 * Date           Author  Notes
 * 2018-06-26     limian  create object
 * 2018-06-26     limian  未调试，未使用
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
#include "bac_global_prop.h"


#ifndef MAX_FLC_GLOBAL_PROP
#define MAX_FLC_GLOBAL_PROP 1
#endif

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
	
    PROP_INPUTDOORNO,
    PROP_AVAILABLETIME,     /* destination button */
    PROP_COUNT,             /* card_count */
    -1
};

static const int Properties_Optional[] = {
	PROP_DESCRIPTION,
    -1
};

static const int Properties_Proprietary[] = {
    -1
};


void Flc_Global_Prop_Property_Lists(
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


void Flc_Global_Prop_Init(void)
{
    unsigned i;

    for (i = 0; i < MAX_FLC_GLOBAL_PROP; i++) {
	/* 变量初始化 */
	
    }
}


bool Flc_Global_Prop_Valid_Instance(
    uint32_t object_instance)
{
    unsigned int index;

    index = Flc_Global_Prop_Instance_To_Index(object_instance);
    if (index < MAX_FLC_GLOBAL_PROP)
        return true;

    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Flc_Global_Prop_Count(
    void)
{
    return MAX_FLC_GLOBAL_PROP;
}


uint32_t Flc_Global_Prop_Index_To_Instance(
    unsigned index)
{
    return index;
}

unsigned Flc_Global_Prop_Instance_To_Index(uint32_t object_instance)
{
    unsigned index = MAX_FLC_GLOBAL_PROP;

    if (object_instance < MAX_FLC_GLOBAL_PROP)
        index = object_instance;

    return index;
}

bool Flc_Global_Prop_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[32] = "";   /* okay for single thread */
    unsigned int index;
    bool status = false;

    index = Flc_Global_Prop_Instance_To_Index(object_instance);
    if (index < MAX_FLC_GLOBAL_PROP) {
        sprintf(text_string, "GLOBAL PROP");
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}


/* return apdu length, or BACNET_STATUS_ERROR on error */
/* assumption - object has already exists */
int Flc_Global_Prop_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    BACNET_CHARACTER_STRING char_string;
    unsigned object_index = 0;
    uint8_t *apdu = NULL;
    uint32_t i;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }

    object_index = Flc_Global_Prop_Instance_To_Index(rpdata->object_instance);
    if (object_index >= MAX_FLC_GLOBAL_PROP)
        return BACNET_STATUS_ERROR;

    apdu = rpdata->application_data;
    switch ((int) rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len = encode_application_object_id(&apdu[0], OBJECT_GLOBALPROP,
								rpdata->object_instance + 1);
            break;
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            Flc_Global_Prop_Object_Name(rpdata->object_instance, &char_string);
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len = encode_application_enumerated(&apdu[0], OBJECT_GLOBALPROP);
            break;

        case PROP_INPUTDOORNO:
            for(i = 0; i < 8; i++)
                apdu_len += encode_application_unsigned(&apdu[apdu_len], 56 + i);
            break;
        case PROP_AVAILABLETIME:
            apdu_len = encode_application_unsigned(&apdu[0], 67);
            break;
        case PROP_COUNT:             /* card_count */
            apdu_len = encode_application_unsigned(&apdu[0], 150000);
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
bool Flc_Global_Prop_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
	bool status = false;        /* return value */
	unsigned int object_index = 0;
	int len = 0;
	BACNET_APPLICATION_DATA_VALUE value[8];    /* 最多需读取175组数据 */
    uint32_t value_len = 1;
    uint32_t left;

    if(wp_data->object_property == PROP_INPUTDOORNO)
        value_len = 8;
    
	/* 应用数据解码 */
	len = bacapp_decode_application_data_complex(wp_data->application_data,
	   wp_data->application_data_len, value, value_len, &left);          
	value_len = len;
	if (len < 0) {
	   /* error while decoding - a value larger than we can handle */
	   wp_data->error_class = ERROR_CLASS_PROPERTY;
	   wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
	   return false;
	}

	object_index = Flc_Global_Prop_Instance_To_Index(wp_data->object_instance);
	if (object_index >= MAX_FLC_GLOBAL_PROP) {
		return false;
	}

	switch ((int) wp_data->object_property) {
        case PROP_INPUTDOORNO:
        case PROP_AVAILABLETIME:
        case PROP_COUNT:             /* card_count */
            
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
	return status;
}
