/*
 * File    : bac_bo.c
 * function: binary out 关于门和读卡器的DIDO读写
 * Change Logs:
 * Date           Author  Notes
 * 2018-08-11     limian  first version
 * 2018-10-10     limian  调试完成
 *
 * note:   
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "bacapp.h"
#include "config.h"     /* the custom stuff */
#include "rp.h"
#include "wp.h"
#include "bac_bo.h"
#include "handlers.h"
#include "app_card_reader_handler.h"
#include "app_door.h"

#ifndef MAX_BINARY_OUTPUTS
#define MAX_BINARY_OUTPUTS 56
#endif


#define CR1IN_RUNNING_OP            213
#define CR1OUT_RUNNING_OP           214
#define CR2IN_RUNNING_OP            215
#define CR2OUT_RUNNING_OP           216
#define CR3IN_RUNNING_OP            217
#define CR3OUT_RUNNING_OP           218
#define CR4IN_RUNNING_OP            219
#define CR4OUT_RUNNING_OP           220
#define CR5IN_RUNNING_OP            221
#define CR5OUT_RUNNING_OP           222
#define CR6IN_RUNNING_OP            223
#define CR6OUT_RUNNING_OP           224
#define CR7IN_RUNNING_OP            225
#define CR7OUT_RUNNING_OP           226
#define CR8IN_RUNNING_OP            227
#define CR8OUT_RUNNING_OP           228

#define CR1IN_ILLEGLE_OP            245
#define CR1OUT_ILLEGLE_OP           246
#define CR2IN_ILLEGLE_OP            247
#define CR2OUT_ILLEGLE_OP           248
#define CR3IN_ILLEGLE_OP            249
#define CR3OUT_ILLEGLE_OP           250
#define CR4IN_ILLEGLE_OP            251
#define CR4OUT_ILLEGLE_OP           252
#define CR5IN_ILLEGLE_OP            253
#define CR5OUT_ILLEGLE_OP           254
#define CR6IN_ILLEGLE_OP            255
#define CR6OUT_ILLEGLE_OP           256
#define CR7IN_ILLEGLE_OP            257
#define CR7OUT_ILLEGLE_OP           258
#define CR8IN_ILLEGLE_OP            259
#define CR8OUT_ILLEGLE_OP           260



#define DOOR1_LOCK_OP               165
#define DOOR2_LOCK_OP               166
#define DOOR3_LOCK_OP               167
#define DOOR4_LOCK_OP               168
#define DOOR5_LOCK_OP               169
#define DOOR6_LOCK_OP               170
#define DOOR7_LOCK_OP               171
#define DOOR8_LOCK_OP               172

#define DOOR1_TEMP_LOCK             181
#define DOOR2_TEMP_LOCK             182
#define DOOR3_TEMP_LOCK             183
#define DOOR4_TEMP_LOCK             184
#define DOOR5_TEMP_LOCK             185
#define DOOR6_TEMP_LOCK             186
#define DOOR7_TEMP_LOCK             187
#define DOOR8_TEMP_LOCK             188

#define DOOR1_OPEN_ABNORMAL_OP      205
#define DOOR2_OPEN_ABNORMAL_OP      206
#define DOOR3_OPEN_ABNORMAL_OP      207
#define DOOR4_OPEN_ABNORMAL_OP      208
#define DOOR5_OPEN_ABNORMAL_OP      209
#define DOOR6_OPEN_ABNORMAL_OP      210
#define DOOR7_OPEN_ABNORMAL_OP      211
#define DOOR8_OPEN_ABNORMAL_OP      212



/* When all the priorities are level null, the present value returns */
/* the Relinquish Default value */
#define RELINQUISH_DEFAULT BINARY_INACTIVE
/* Here is our Priority Array.*/
static BACNET_BINARY_PV Binary_Output_Level[MAX_BINARY_OUTPUTS];

static uint32_t BO_ID[MAX_BINARY_OUTPUTS] = {
    CR1IN_RUNNING_OP,
    CR1IN_ILLEGLE_OP,
    CR1OUT_RUNNING_OP,
    CR1OUT_ILLEGLE_OP,
    CR2IN_RUNNING_OP,
    CR2IN_ILLEGLE_OP,
    CR2OUT_RUNNING_OP,
    CR2OUT_ILLEGLE_OP,
    DOOR1_LOCK_OP,
    DOOR1_TEMP_LOCK,
    DOOR1_OPEN_ABNORMAL_OP,
    DOOR2_LOCK_OP,
    DOOR2_TEMP_LOCK,
    DOOR2_OPEN_ABNORMAL_OP,
    
    CR3IN_RUNNING_OP,
    CR3IN_ILLEGLE_OP,
    CR3OUT_RUNNING_OP,
    CR3OUT_ILLEGLE_OP,
    CR4IN_RUNNING_OP,
    CR4IN_ILLEGLE_OP,
    CR4OUT_RUNNING_OP,
    CR4OUT_ILLEGLE_OP,
    DOOR3_LOCK_OP,
    DOOR3_TEMP_LOCK,
    DOOR3_OPEN_ABNORMAL_OP,
    DOOR4_LOCK_OP,
    DOOR4_TEMP_LOCK,
    DOOR4_OPEN_ABNORMAL_OP,
    
    CR5IN_RUNNING_OP,
    CR5IN_ILLEGLE_OP,
    CR5OUT_RUNNING_OP,
    CR5OUT_ILLEGLE_OP,
    CR6IN_RUNNING_OP,
    CR6IN_ILLEGLE_OP,
    CR6OUT_RUNNING_OP,
    CR6OUT_ILLEGLE_OP,
    DOOR5_LOCK_OP,
    DOOR5_TEMP_LOCK,
    DOOR5_OPEN_ABNORMAL_OP,
    DOOR6_LOCK_OP,
    DOOR6_TEMP_LOCK,
    DOOR6_OPEN_ABNORMAL_OP,
    
    CR7IN_RUNNING_OP,
    CR7IN_ILLEGLE_OP,
    CR7OUT_RUNNING_OP,
    CR7OUT_ILLEGLE_OP,
    CR8IN_RUNNING_OP,
    CR8IN_ILLEGLE_OP,
    CR8OUT_RUNNING_OP,
    CR8OUT_ILLEGLE_OP,
    DOOR7_LOCK_OP,
    DOOR7_TEMP_LOCK,
    DOOR7_OPEN_ABNORMAL_OP,
    DOOR8_LOCK_OP,
    DOOR8_TEMP_LOCK,
    DOOR8_OPEN_ABNORMAL_OP,
};

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Binary_Output_Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    
    PROP_PRESENT_VALUE,
    PROP_STATUS_FLAGS,
    PROP_EVENT_STATE,
    PROP_RELIABILITY,
    PROP_OUT_OF_SERVICE,
    PROP_POLARITY,
    PROP_CHANGE_OF_STATE_COUNT,
    PROP_ELAPSED_ACTIVE_TIME,
    PROP_MINIMUM_OFF_TIME,
    PROP_MINIMUM_ON_TIME,
    PROP_RELINQUISH_DEFAULT,
    PROP_TIME_DELAY,
    PROP_NOTIFICATION_CLASS,
    PROP_NOTIFY_TYPE,
//    PROP_NOTIFICATION_PERIOD,
    PROP_ELAPSED_STATE,
    PROP_CHANGE_STATE,
    PROP_ACTIVE_STATE,
    -1
};

static const int Binary_Output_Properties_Optional[] = {
    PROP_DESCRIPTION,
    -1
};

static const int Binary_Output_Properties_Proprietary[] = {
    -1
}; 

static uint8_t get_cr_no_by_id(uint32_t id)
{
    switch(id)
    {
        case CR1IN_RUNNING_OP:
        case CR1IN_ILLEGLE_OP:
            return 1;
        case CR1OUT_RUNNING_OP:
        case CR1OUT_ILLEGLE_OP:
            return 2;
        case CR2IN_RUNNING_OP:
        case CR2IN_ILLEGLE_OP:
            return 3;
        case CR2OUT_RUNNING_OP:
        case CR2OUT_ILLEGLE_OP:
            return 4;
        
        case CR3IN_RUNNING_OP:
        case CR3IN_ILLEGLE_OP:
            return 5;
        case CR3OUT_RUNNING_OP:
        case CR3OUT_ILLEGLE_OP:
            return 6;
        case CR4IN_RUNNING_OP:
        case CR4IN_ILLEGLE_OP:
            return 7;
        case CR4OUT_RUNNING_OP:
        case CR4OUT_ILLEGLE_OP:
            return 8;
        
        case CR5IN_RUNNING_OP:
        case CR5IN_ILLEGLE_OP:
            return 9;
        case CR5OUT_RUNNING_OP:
        case CR5OUT_ILLEGLE_OP:
            return 10;
        case CR6IN_RUNNING_OP:
        case CR6IN_ILLEGLE_OP:
            return 11;
        case CR6OUT_RUNNING_OP:
        case CR6OUT_ILLEGLE_OP:
            return 12;
        
        case CR7IN_RUNNING_OP:
        case CR7IN_ILLEGLE_OP:
            return 13;
        case CR7OUT_RUNNING_OP:
        case CR7OUT_ILLEGLE_OP:
            return 14;
        case CR8IN_RUNNING_OP:
        case CR8IN_ILLEGLE_OP:
            return 15;
        case CR8OUT_RUNNING_OP:
        case CR8OUT_ILLEGLE_OP:
            return 16;
        default:
            break;
    }
    return 0;
}

static uint8_t get_door_no_by_id(uint32_t id)
{
    switch(id)
    {
        case DOOR1_LOCK_OP:
        case DOOR1_TEMP_LOCK:
        case DOOR1_OPEN_ABNORMAL_OP:
            return 1;
        case DOOR2_LOCK_OP:
        case DOOR2_TEMP_LOCK:
        case DOOR2_OPEN_ABNORMAL_OP:
            return 2;
        
        case DOOR3_LOCK_OP:
        case DOOR3_TEMP_LOCK:
        case DOOR3_OPEN_ABNORMAL_OP:
            return 3;
        case DOOR4_LOCK_OP:
        case DOOR4_TEMP_LOCK:
        case DOOR4_OPEN_ABNORMAL_OP:
            return 4;
        
        case DOOR5_LOCK_OP:
        case DOOR5_TEMP_LOCK:
        case DOOR5_OPEN_ABNORMAL_OP:
            return 5;
        case DOOR6_LOCK_OP:
        case DOOR6_TEMP_LOCK:
        case DOOR6_OPEN_ABNORMAL_OP:
            return 6;
        
        case DOOR7_LOCK_OP:
        case DOOR7_TEMP_LOCK:
        case DOOR7_OPEN_ABNORMAL_OP:
            return 7;
        case DOOR8_LOCK_OP:
        case DOOR8_TEMP_LOCK:
        case DOOR8_OPEN_ABNORMAL_OP:
            return 8;
        default:
            break;
    }
    return 0;
}



void Binary_Output_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    if (pRequired)
        *pRequired = Binary_Output_Properties_Required;
    if (pOptional)
        *pOptional = Binary_Output_Properties_Optional;
    if (pProprietary)
        *pProprietary = Binary_Output_Properties_Proprietary;

    return;
}

void Binary_Output_Init(
    void)
{
    unsigned i;
    static bool initialized = false;

    if (!initialized) {
        initialized = true;

        /* initialize all the analog output priority arrays to NULL */
        for (i = 0; i < MAX_BINARY_OUTPUTS; i++) {
                Binary_Output_Level[i] = BINARY_NULL;
        }
    }

    return;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need validate that the */
/* given instance exists */
bool Binary_Output_Valid_Instance(
    uint32_t object_instance)
{
    uint32_t i;
    for(i = 0; i < MAX_BINARY_OUTPUTS; i++)
        if(BO_ID[i] == object_instance)
            return true;
    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Binary_Output_Count(
    void)
{
    return MAX_BINARY_OUTPUTS;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the instance */
/* that correlates to the correct index */
uint32_t Binary_Output_Index_To_Instance(
    unsigned index)
{
    if(index < MAX_BINARY_OUTPUTS)
        return BO_ID[index];
    return 0;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the index */
/* that correlates to the correct instance number */
unsigned Binary_Output_Instance_To_Index(
    uint32_t object_instance)
{
    uint32_t i;
    for(i = 0; i < MAX_BINARY_OUTPUTS; i++)
    {    
        if(BO_ID[i] == object_instance)
            break;
    }
    return i;
}

BACNET_BINARY_PV Binary_Output_Present_Value(
    uint32_t object_instance)
{
    BACNET_BINARY_PV value = RELINQUISH_DEFAULT;
    unsigned index = 0;

    index = Binary_Output_Instance_To_Index(object_instance);
    if (index < MAX_BINARY_OUTPUTS) {
        if (Binary_Output_Level[index] != BINARY_NULL) {
            value = Binary_Output_Level[index];
        }
    }
    return value;
}

void Binary_Output_present_Value_Write(uint32_t object_instance, BACNET_BINARY_PV level)
{
    uint8_t temp;
    
    temp = get_cr_no_by_id(object_instance);
    /* 是读卡器操作 */
    if(temp != 0)
    {
        temp = find_card_reader_by_door(temp);
        if(temp != 0)
        {
            switch(object_instance)
            {
                case CR1IN_RUNNING_OP:
                case CR1OUT_RUNNING_OP:
                case CR2IN_RUNNING_OP:
                case CR2OUT_RUNNING_OP:
                case CR3IN_RUNNING_OP:
                case CR3OUT_RUNNING_OP:
                case CR4IN_RUNNING_OP:
                case CR4OUT_RUNNING_OP:
                case CR5IN_RUNNING_OP:
                case CR5OUT_RUNNING_OP:
                case CR6IN_RUNNING_OP:
                case CR6OUT_RUNNING_OP:
                case CR7IN_RUNNING_OP:
                case CR7OUT_RUNNING_OP:
                case CR8IN_RUNNING_OP:
                case CR8OUT_RUNNING_OP:
                    app_card_reader_enable_disable(temp, level);
                    break;
                case CR1IN_ILLEGLE_OP:
                case CR1OUT_ILLEGLE_OP:
                case CR2IN_ILLEGLE_OP:
                case CR2OUT_ILLEGLE_OP:
                case CR3IN_ILLEGLE_OP:
                case CR3OUT_ILLEGLE_OP:
                case CR4IN_ILLEGLE_OP:
                case CR4OUT_ILLEGLE_OP:
                case CR5IN_ILLEGLE_OP:
                case CR5OUT_ILLEGLE_OP:
                case CR6IN_ILLEGLE_OP:
                case CR6OUT_ILLEGLE_OP:
                case CR7IN_ILLEGLE_OP:
                case CR7OUT_ILLEGLE_OP:
                case CR8IN_ILLEGLE_OP:
                case CR8OUT_ILLEGLE_OP:
                    if(level == BINARY_ACTIVE)
                        app_card_reader_recover_illegle_card_error(temp);
                    break;
            }
        }
    }
    
    temp = get_door_no_by_id(object_instance);
    /* 是门操作 */
    if(temp != 0)
    {
        switch(object_instance)
        {
            
            case DOOR1_LOCK_OP:
            case DOOR2_LOCK_OP:
            case DOOR3_LOCK_OP:
            case DOOR4_LOCK_OP:
            case DOOR5_LOCK_OP:
            case DOOR6_LOCK_OP:
            case DOOR7_LOCK_OP:
            case DOOR8_LOCK_OP:
                app_door_open_close(temp, level);
                break;
            case DOOR1_TEMP_LOCK:
            case DOOR2_TEMP_LOCK:
            case DOOR3_TEMP_LOCK:
            case DOOR4_TEMP_LOCK:
            case DOOR5_TEMP_LOCK:
            case DOOR6_TEMP_LOCK:
            case DOOR7_TEMP_LOCK:
            case DOOR8_TEMP_LOCK:
                app_door_temp_open(temp);
                break;
            case DOOR1_OPEN_ABNORMAL_OP:
            case DOOR2_OPEN_ABNORMAL_OP:
            case DOOR3_OPEN_ABNORMAL_OP:
            case DOOR4_OPEN_ABNORMAL_OP:
            case DOOR5_OPEN_ABNORMAL_OP:
            case DOOR6_OPEN_ABNORMAL_OP:
            case DOOR7_OPEN_ABNORMAL_OP:
            case DOOR8_OPEN_ABNORMAL_OP:
                app_door_abnormal_open_op(temp);
                break;
        }
    }
}



/* note: the object name must be unique within this device */
bool Binary_Output_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[32] = "";   /* okay for single thread */
    bool status = false;
    uint32_t index;
    index = Binary_Output_Instance_To_Index(object_instance);
    if (index < MAX_BINARY_OUTPUTS) {
        sprintf(text_string, "BINARY OUTPUT %lu", (unsigned long) object_instance);
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}

/* return apdu len, or BACNET_STATUS_ERROR on error */
int Binary_Output_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    BACNET_BINARY_PV present_value = BINARY_INACTIVE;
    uint8_t *apdu = NULL;
    uint32_t index;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }
    apdu = rpdata->application_data;
    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            index = Binary_Output_Instance_To_Index(rpdata->object_instance);
            if(index >= MAX_BINARY_OUTPUTS)
                apdu_len = encode_application_object_id(&apdu[0], 
                        OBJECT_BINARY_OUTPUT, 0);
            else
                apdu_len =encode_application_object_id(&apdu[0], 
                        OBJECT_BINARY_OUTPUT, BO_ID[index]);
            break;
            /* note: Name and Description don't have to be the same.
               You could make Description writable and different */
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            Binary_Output_Object_Name(rpdata->object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0], OBJECT_BINARY_OUTPUT);
            break;
        
        case PROP_PRESENT_VALUE:
            present_value =
                Binary_Output_Present_Value(rpdata->object_instance);
            apdu_len = encode_application_enumerated(&apdu[0], present_value);
            break;
        case PROP_STATUS_FLAGS:
            /* note: see the details in the standard on how to use these */
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE, false);
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;

        case PROP_EVENT_STATE:
            apdu_len = encode_application_signed(&apdu[0], 0);
            break;
        case PROP_RELIABILITY:
            apdu_len = encode_application_signed(&apdu[0], 0);
            break;
        case PROP_OUT_OF_SERVICE:
            apdu_len = encode_application_boolean(&apdu[0], 0);
            break;
        case PROP_POLARITY:
            apdu_len = encode_application_signed(&apdu[0], 0);
            break;
        case PROP_CHANGE_OF_STATE_COUNT:
            apdu_len = encode_application_signed(&apdu[0], 0);
            break;
        case PROP_ELAPSED_ACTIVE_TIME:
            apdu_len = encode_application_signed(&apdu[0], rt_tick_get()/1000);
            break;
        case PROP_MINIMUM_OFF_TIME:
            apdu_len = encode_application_signed(&apdu[0], 100);
            break;
        case PROP_MINIMUM_ON_TIME:
            apdu_len = encode_application_signed(&apdu[0], 100);
            break;
        case PROP_RELINQUISH_DEFAULT:
            apdu_len = encode_application_signed(&apdu[0], 0);
            break;
        case PROP_TIME_DELAY:
            apdu_len = encode_application_signed(&apdu[0], 255);
            break;
        case PROP_NOTIFICATION_CLASS:
            apdu_len = encode_application_signed(&apdu[0], 0);
            break;
        case PROP_NOTIFY_TYPE:
            apdu_len = encode_application_signed(&apdu[0], 0);
            break;
    //    case PROP_NOTIFICATION_PERIOD,
        case PROP_ELAPSED_STATE:
            apdu_len = encode_application_signed(&apdu[0], 0);
            break;
        case PROP_CHANGE_STATE:
            apdu_len = encode_application_signed(&apdu[0], 0);
            break;
        case PROP_ACTIVE_STATE:
            apdu_len = encode_application_signed(&apdu[0], 0);
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
bool Binary_Output_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
    unsigned int object_index = 0;
    unsigned int priority = 0;
    BACNET_BINARY_PV level = BINARY_NULL;
    int len = 0;
    BACNET_APPLICATION_DATA_VALUE value;

    /* decode the some of the request */
    len =
        bacapp_decode_application_data(wp_data->application_data,
        wp_data->application_data_len, &value);
    /* FIXME: len < application_data_len: more data? */
    if (len < 0) {
        /* error while decoding - a value larger than we can handle */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }
    if ((wp_data->object_property != PROP_PRIORITY_ARRAY) &&
        (wp_data->array_index != BACNET_ARRAY_ALL)) {
        /*  only array properties can have array options */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
    switch (wp_data->object_property) {
        case PROP_PRESENT_VALUE:
            if (value.tag == BACNET_APPLICATION_TAG_ENUMERATED) {
                priority = wp_data->priority;
                /* Command priority 6 is reserved for use by Minimum On/Off
                   algorithm and may not be used for other purposes in any
                   object. */
                if (priority && (priority <= BACNET_MAX_PRIORITY) &&
                    (priority != 6 /* reserved */ ) &&
                    (value.type.Enumerated <= MAX_BINARY_PV)) {
                    level = (BACNET_BINARY_PV) value.type.Enumerated;
//                    object_index =
//                        Binary_Output_Instance_To_Index
//                        (wp_data->object_instance);
                    priority--;
                    Binary_Output_present_Value_Write(wp_data->object_instance, level);
//                    Binary_Output_Level[object_index] = level;
                    /* Note: you could set the physical output here if we
                       are the highest priority.
                       However, if Out of Service is TRUE, then don't set the
                       physical output.  This comment may apply to the
                       main loop (i.e. check out of service before changing output) */
                    status = true;
                } else if (priority == 6) {
                    /* Command priority 6 is reserved for use by Minimum On/Off
                       algorithm and may not be used for other purposes in any
                       object. */
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                status =
                    WPValidateArgType(&value, BACNET_APPLICATION_TAG_NULL,
                    &wp_data->error_class, &wp_data->error_code);
                if (status) {
                    level = BINARY_NULL;
                    object_index =
                        Binary_Output_Instance_To_Index
                        (wp_data->object_instance);
                    priority = wp_data->priority;
                    if (priority && (priority <= BACNET_MAX_PRIORITY)) {
                        priority--;
                        Binary_Output_Level[object_index] = level;
                        /* Note: you could set the physical output here to the next
                           highest priority, or to the relinquish default if no
                           priorities are set.
                           However, if Out of Service is TRUE, then don't set the
                           physical output.  This comment may apply to the
                           main loop (i.e. check out of service before changing output) */
                    } else {
                        status = false;
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                    }
                }
            }
            break;
    
        case PROP_STATUS_FLAGS:
        case PROP_EVENT_STATE:
        case PROP_RELIABILITY:
        case PROP_OUT_OF_SERVICE:
        case PROP_POLARITY:
        case PROP_CHANGE_OF_STATE_COUNT:
        case PROP_ELAPSED_ACTIVE_TIME:
        case PROP_MINIMUM_OFF_TIME:
        case PROP_MINIMUM_ON_TIME:
        case PROP_RELINQUISH_DEFAULT:
        case PROP_TIME_DELAY:
        case PROP_NOTIFICATION_CLASS:
        case PROP_NOTIFY_TYPE:
    //    case PROP_NOTIFICATION_PERIOD,
        case PROP_ELAPSED_STATE:
        case PROP_CHANGE_STATE:
        case PROP_ACTIVE_STATE:
            status = true;
            break;
        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            break;
    }

    return status;
}
