/*
 * File      : route.c
 * function  : bacnet route FLC中一些全局属性的集合
 * Change Logs:
 * Date           Author       Notes
 * 2018-06-27     limian      create object
 * 2018-10-10     limian  未调试，未使用
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
#include "bac_route.h"


#ifndef MAX_FLC_ROUTE
#define MAX_FLC_ROUTE 1
#endif

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
	
    PROP_VALID,
    PROP_CLEAR,     /* destination button */
    -1
};

static const int Properties_Optional[] = {
	PROP_DESCRIPTION,
    -1
};

static const int Properties_Proprietary[] = {
    -1
};


void Flc_Route_Property_Lists(
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


void Flc_Route_Init(void)
{
    unsigned i;

    for (i = 0; i < MAX_FLC_ROUTE; i++) {
	/* 变量初始化 */
	
    }
}


bool Flc_Route_Valid_Instance(
    uint32_t object_instance)
{
    unsigned int index;

    index = Flc_Route_Instance_To_Index(object_instance);
    if (index < MAX_FLC_ROUTE)
        return true;

    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Flc_Route_Count(
    void)
{
    return MAX_FLC_ROUTE;
}


uint32_t Flc_Route_Index_To_Instance(
    unsigned index)
{
    return index;
}

unsigned Flc_Route_Instance_To_Index(uint32_t object_instance)
{
    unsigned index = MAX_FLC_ROUTE;

    if (object_instance < MAX_FLC_ROUTE)
        index = object_instance;

    return index;
}

bool Flc_Route_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[32] = "";   /* okay for single thread */
    unsigned int index;
    bool status = false;

    index = Flc_Route_Instance_To_Index(object_instance);
    if (index < MAX_FLC_ROUTE) {
        sprintf(text_string, "FLC ROUTE");
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}


/* return apdu length, or BACNET_STATUS_ERROR on error */
/* assumption - object has already exists */
int Flc_Route_Read_Property(
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

    object_index = Flc_Route_Instance_To_Index(rpdata->object_instance);
    if (object_index >= MAX_FLC_ROUTE)
        return BACNET_STATUS_ERROR;

    apdu = rpdata->application_data;
    switch ((int) rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len = encode_application_object_id(&apdu[0], OBJECT_ROUTE,
								rpdata->object_instance + 1);
            break;
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            Flc_Route_Object_Name(rpdata->object_instance, &char_string);
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len = encode_application_enumerated(&apdu[0], OBJECT_ROUTE);
            break;

        case PROP_VALID:
            apdu_len = encode_application_boolean(&apdu[0], 1);
            break;
        case PROP_CLEAR:
            apdu_len = encode_application_boolean(&apdu[0], 0);
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
bool Flc_Route_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
	bool status = false;        /* return value */
	unsigned int object_index = 0;
	int len = 0;
	BACNET_APPLICATION_DATA_VALUE value[8];    /* 最多需读取175组数据 */
    uint32_t value_len = 1;
    uint32_t left;
    
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

	object_index = Flc_Route_Instance_To_Index(wp_data->object_instance);
	if (object_index >= MAX_FLC_ROUTE) {
		return false;
	}

	switch ((int) wp_data->object_property) {
        case PROP_VALID:
        case PROP_CLEAR:
            
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
