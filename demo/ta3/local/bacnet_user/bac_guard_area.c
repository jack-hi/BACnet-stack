/*
 * File      : guard_area.c
 * function  : bacnet guard_area ��������
 * Change Logs:
 * Date           Author  Notes
 * 2018-06-25     limian  create object
 * 2018-10-10     limian  δ���ԣ�δʹ��
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
#include "bac_guard_area.h"


#ifndef MAX_FLC_GUARD_AREAS
#define MAX_FLC_GUARD_AREAS 40
#endif

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
	
    PROP_GUARDSCHEDULENO,
    PROP_CALENDARNO,
    PROP_PATTERNS,
    PROP_ROUTINETIME,
    -1
};

static const int Properties_Optional[] = {
	PROP_DESCRIPTION,
    -1
};

static const int Properties_Proprietary[] = {
    -1
};


void Flc_Guard_Area_Property_Lists(
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


void Flc_Guard_Area_Init(void)
{
    unsigned i;

    for (i = 0; i < MAX_FLC_GUARD_AREAS; i++) {
	/* ������ʼ�� */
	
    }
}


bool Flc_Guard_Area_Valid_Instance(
    uint32_t object_instance)
{
    unsigned int index;

    index = Flc_Guard_Area_Instance_To_Index(object_instance);
    if (index < MAX_FLC_GUARD_AREAS)
        return true;

    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Flc_Guard_Area_Count(
    void)
{
    return MAX_FLC_GUARD_AREAS;
}


uint32_t Flc_Guard_Area_Index_To_Instance(
    unsigned index)
{
    return index;
}

unsigned Flc_Guard_Area_Instance_To_Index(uint32_t object_instance)
{
    unsigned index = MAX_FLC_GUARD_AREAS;

    if (object_instance < MAX_FLC_GUARD_AREAS)
        index = object_instance;

    return index;
}

bool Flc_Guard_Area_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[32] = "";   /* okay for single thread */
    unsigned int index;
    bool status = false;

    index = Flc_Guard_Area_Instance_To_Index(object_instance);
    if (index < MAX_FLC_GUARD_AREAS) {
        sprintf(text_string, "GUARD AREA %lu", (unsigned long) index);
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}


/* return apdu length, or BACNET_STATUS_ERROR on error */
/* assumption - object has already exists */
int Flc_Guard_Area_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    BACNET_CHARACTER_STRING char_string;
    unsigned object_index = 0;
    uint8_t *apdu = NULL;
    uint32_t i, j;
    BACNET_TIME time = {3, 20, 40, 20};

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }

    object_index = Flc_Guard_Area_Instance_To_Index(rpdata->object_instance);
    if (object_index >= MAX_FLC_GUARD_AREAS)
        return BACNET_STATUS_ERROR;

    apdu = rpdata->application_data;
    switch ((int) rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len =
                encode_application_object_id(&apdu[0], OBJECT_GUARDAREA,
                rpdata->object_instance + 1);
            break;
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            Flc_Guard_Area_Object_Name(rpdata->object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0], OBJECT_GUARDAREA);
            break;
        
        case PROP_GUARDSCHEDULENO:
            apdu_len = encode_application_unsigned(&apdu[0], 67);
            break;
        case PROP_CALENDARNO:
            apdu_len = encode_application_unsigned(&apdu[0], 67);
            break;
        case PROP_PATTERNS:
            apdu_len = 0;
			for(i = 0; i < 2; i++)
			{	
				apdu_len += encode_opening_tag(&apdu[apdu_len], TAG_PATTERNS);
				for(j = 0; j < 5; j++)
				{	
					apdu_len += encode_opening_tag(&apdu[apdu_len], TAG_PATTERN);
					apdu_len += encode_application_unsigned(&apdu[apdu_len], 0);
					apdu_len += encode_application_time(&apdu[apdu_len], &time);
					apdu_len += encode_application_time(&apdu[apdu_len], &time);
					apdu_len += encode_closing_tag(&apdu[apdu_len], TAG_PATTERN);
				}
				apdu_len += encode_closing_tag(&apdu[apdu_len], TAG_PATTERNS);
			}
			break;
        case PROP_ROUTINETIME:
            apdu_len = encode_application_time(&apdu[0], &time);
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
bool Flc_Guard_Area_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
	bool status = false;        /* return value */
	unsigned int object_index = 0;
	int len = 0;
	BACNET_APPLICATION_DATA_VALUE value[30];    /* ������ȡ15������ */
    uint32_t value_len = 1;
//    uint32_t i, j;
//    uint32_t ind = 0;
    uint32_t left;
    
    if(wp_data->object_property == PROP_PATTERNS)
        value_len = 30;
    else if(wp_data->object_property == PROP_RINGVALID)
        value_len = 10;
    else if(wp_data->object_property == PROP_RINGTIME)
        value_len = 10;

	/* Ӧ�����ݽ��� */
	len = bacapp_decode_application_data_complex(wp_data->application_data,
	   wp_data->application_data_len, value, value_len, &left);          
	value_len = len;
	if (len < 0) {
	   /* error while decoding - a value larger than we can handle */
	   wp_data->error_class = ERROR_CLASS_PROPERTY;
	   wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
	   return false;
	}

	object_index = Flc_Guard_Area_Instance_To_Index(wp_data->object_instance);
	if (object_index >= MAX_FLC_GUARD_AREAS) {
		return false;
	}

	switch ((int) wp_data->object_property) {
        case PROP_GUARDSCHEDULENO:
            break;
        case PROP_CALENDARNO:
            break;
        case PROP_PATTERNS:
			break;
        case PROP_ROUTINETIME:
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
	return status;
}
