/*
 * File    : bac_bi.c
 * function: binary in 关于门和读卡器的DIDO读写
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
#include "rp.h"
#include "wp.h"
#include "cov.h"
#include "config.h"     /* the custom stuff */
#include "bac_bi.h"
#include "handlers.h"
#include "app_card_reader_handler.h"
#include "app_door.h"

#ifndef MAX_BINARY_INPUTS
#define MAX_BINARY_INPUTS 88
#endif


//typedef struct
//{
//    uint32_t door_stat;
//    uint32_t lock_op;
//    uint32_t lock_stat;
//    uint32_t lock_error;
//    uint32_t temp_lock_op;
//    uint32_t open_abnormal_op;
//    uint32_t open_abnormal_error;
//    uint32_t open_timeout_error;
//}bac_bi_door_def;

//typedef struct
//{
//    uint32_t running_op;
//    uint32_t running_stat;
//    uint32_t running_error;
//    uint32_t illigle_card_op;
//    uint32_t illigle_card_error;
//}bac_bi_cr_def;


#define CR1IN_RUNNING_STAT          369
#define CR1OUT_RUNNING_STAT         370
#define CR2IN_RUNNING_STAT          371
#define CR2OUT_RUNNING_STAT         372
#define CR3IN_RUNNING_STAT          373
#define CR3OUT_RUNNING_STAT         374
#define CR4IN_RUNNING_STAT          375
#define CR4OUT_RUNNING_STAT         376
#define CR5IN_RUNNING_STAT          377
#define CR5OUT_RUNNING_STAT         378
#define CR6IN_RUNNING_STAT          379
#define CR6OUT_RUNNING_STAT         380
#define CR7IN_RUNNING_STAT          381
#define CR7OUT_RUNNING_STAT         382
#define CR8IN_RUNNING_STAT          383
#define CR8OUT_RUNNING_STAT         384

#define CR1IN_RUNNING_ERROR         385
#define CR1OUT_RUNNING_ERROR        386
#define CR2IN_RUNNING_ERROR         387
#define CR2OUT_RUNNING_ERROR        388
#define CR3IN_RUNNING_ERROR         389
#define CR3OUT_RUNNING_ERROR        390
#define CR4IN_RUNNING_ERROR         391
#define CR4OUT_RUNNING_ERROR        392
#define CR5IN_RUNNING_ERROR         393
#define CR5OUT_RUNNING_ERROR        394
#define CR6IN_RUNNING_ERROR         395
#define CR6OUT_RUNNING_ERROR        396
#define CR7IN_RUNNING_ERROR         397
#define CR7OUT_RUNNING_ERROR        398
#define CR8IN_RUNNING_ERROR         399
#define CR8OUT_RUNNING_ERROR        400

#define CR1IN_ILLEGLE_ERROR         433
#define CR1OUT_ILLEGLE_ERROR        434
#define CR2IN_ILLEGLE_ERROR         435
#define CR2OUT_ILLEGLE_ERROR        436
#define CR3IN_ILLEGLE_ERROR         437
#define CR3OUT_ILLEGLE_ERROR        438
#define CR4IN_ILLEGLE_ERROR         439
#define CR4OUT_ILLEGLE_ERROR        440
#define CR5IN_ILLEGLE_ERROR         441
#define CR5OUT_ILLEGLE_ERROR        442
#define CR6IN_ILLEGLE_ERROR         443
#define CR6OUT_ILLEGLE_ERROR        444
#define CR7IN_ILLEGLE_ERROR         445
#define CR7OUT_ILLEGLE_ERROR        446
#define CR8IN_ILLEGLE_ERROR         447
#define CR8OUT_ILLEGLE_ERROR        448


#define DOOR1_DOOR_STAT             337
#define DOOR2_DOOR_STAT             338
#define DOOR3_DOOR_STAT             339
#define DOOR4_DOOR_STAT             340
#define DOOR5_DOOR_STAT             341
#define DOOR6_DOOR_STAT             342
#define DOOR7_DOOR_STAT             343
#define DOOR8_DOOR_STAT             344

#define DOOR1_LOCK_STAT             313
#define DOOR2_LOCK_STAT             314
#define DOOR3_LOCK_STAT             315
#define DOOR4_LOCK_STAT             316
#define DOOR5_LOCK_STAT             317
#define DOOR6_LOCK_STAT             318
#define DOOR7_LOCK_STAT             319
#define DOOR8_LOCK_STAT             320


#define DOOR1_LOCK_ERROR            321
#define DOOR2_LOCK_ERROR            322
#define DOOR3_LOCK_ERROR            323
#define DOOR4_LOCK_ERROR            324
#define DOOR5_LOCK_ERROR            325
#define DOOR6_LOCK_ERROR            326
#define DOOR7_LOCK_ERROR            327
#define DOOR8_LOCK_ERROR            328

#define DOOR1_OPEN_ABNORMAL_ERROR   353
#define DOOR2_OPEN_ABNORMAL_ERROR   354
#define DOOR3_OPEN_ABNORMAL_ERROR   355
#define DOOR4_OPEN_ABNORMAL_ERROR   356
#define DOOR5_OPEN_ABNORMAL_ERROR   357
#define DOOR6_OPEN_ABNORMAL_ERROR   358
#define DOOR7_OPEN_ABNORMAL_ERROR   359
#define DOOR8_OPEN_ABNORMAL_ERROR   360

#define DOOR1_OPEN_TIMEOUT          361
#define DOOR2_OPEN_TIMEOUT          362
#define DOOR3_OPEN_TIMEOUT          363
#define DOOR4_OPEN_TIMEOUT          364
#define DOOR5_OPEN_TIMEOUT          365
#define DOOR6_OPEN_TIMEOUT          366
#define DOOR7_OPEN_TIMEOUT          367
#define DOOR8_OPEN_TIMEOUT          368



/* Change of Value flag */
static bool Change_Of_Value[MAX_BINARY_INPUTS];
static uint32_t BI_ID[MAX_BINARY_INPUTS] = {
    CR1IN_RUNNING_STAT,
    CR1IN_RUNNING_ERROR,
    CR1IN_ILLEGLE_ERROR,
    CR1OUT_RUNNING_STAT,
    CR1OUT_RUNNING_ERROR,
    CR1OUT_ILLEGLE_ERROR,
    CR2IN_RUNNING_STAT,
    CR2IN_RUNNING_ERROR,
    CR2IN_ILLEGLE_ERROR,
    CR2OUT_RUNNING_STAT,
    CR2OUT_RUNNING_ERROR,
    CR2OUT_ILLEGLE_ERROR,
    DOOR1_DOOR_STAT,
    DOOR1_LOCK_STAT,
    DOOR1_LOCK_ERROR,
    DOOR1_OPEN_ABNORMAL_ERROR,
    DOOR1_OPEN_TIMEOUT,
    DOOR2_DOOR_STAT,
    DOOR2_LOCK_STAT,
    DOOR2_LOCK_ERROR,
    DOOR2_OPEN_ABNORMAL_ERROR,
    DOOR2_OPEN_TIMEOUT,
    
    CR3IN_RUNNING_STAT,
    CR3IN_RUNNING_ERROR,
    CR3IN_ILLEGLE_ERROR,
    CR3OUT_RUNNING_STAT,
    CR3OUT_RUNNING_ERROR,
    CR3OUT_ILLEGLE_ERROR,
    CR4IN_RUNNING_STAT,
    CR4IN_RUNNING_ERROR,
    CR4IN_ILLEGLE_ERROR,
    CR4OUT_RUNNING_STAT,
    CR4OUT_RUNNING_ERROR,
    CR4OUT_ILLEGLE_ERROR,
    DOOR3_DOOR_STAT,
    DOOR3_LOCK_STAT,
    DOOR3_LOCK_ERROR,
    DOOR3_OPEN_ABNORMAL_ERROR,
    DOOR3_OPEN_TIMEOUT,
    DOOR4_DOOR_STAT,
    DOOR4_LOCK_STAT,
    DOOR4_LOCK_ERROR,
    DOOR4_OPEN_ABNORMAL_ERROR,
    DOOR4_OPEN_TIMEOUT,
    
    CR5IN_RUNNING_STAT,
    CR5IN_RUNNING_ERROR,
    CR5IN_ILLEGLE_ERROR,
    CR5OUT_RUNNING_STAT,
    CR5OUT_RUNNING_ERROR,
    CR5OUT_ILLEGLE_ERROR,
    CR6IN_RUNNING_STAT,
    CR6IN_RUNNING_ERROR,
    CR6IN_ILLEGLE_ERROR,
    CR6OUT_RUNNING_STAT,
    CR6OUT_RUNNING_ERROR,
    CR6OUT_ILLEGLE_ERROR,
    DOOR5_DOOR_STAT,
    DOOR5_LOCK_STAT,
    DOOR5_LOCK_ERROR,
    DOOR5_OPEN_ABNORMAL_ERROR,
    DOOR5_OPEN_TIMEOUT,
    DOOR6_DOOR_STAT,
    DOOR6_LOCK_STAT,
    DOOR6_LOCK_ERROR,
    DOOR6_OPEN_ABNORMAL_ERROR,
    DOOR6_OPEN_TIMEOUT,
    
    CR7IN_RUNNING_STAT,
    CR7IN_RUNNING_ERROR,
    CR7IN_ILLEGLE_ERROR,
    CR7OUT_RUNNING_STAT,
    CR7OUT_RUNNING_ERROR,
    CR7OUT_ILLEGLE_ERROR,
    CR8IN_RUNNING_STAT,
    CR8IN_RUNNING_ERROR,
    CR8IN_ILLEGLE_ERROR,
    CR8OUT_RUNNING_STAT,
    CR8OUT_RUNNING_ERROR,
    CR8OUT_ILLEGLE_ERROR,
    DOOR7_DOOR_STAT,
    DOOR7_LOCK_STAT,
    DOOR7_LOCK_ERROR,
    DOOR7_OPEN_ABNORMAL_ERROR,
    DOOR7_OPEN_TIMEOUT,
    DOOR8_DOOR_STAT,
    DOOR8_LOCK_STAT,
    DOOR8_LOCK_ERROR,
    DOOR8_OPEN_ABNORMAL_ERROR,
    DOOR8_OPEN_TIMEOUT,
};

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Binary_Input_Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    
    PROP_PRESENT_VALUE,
    PROP_STATUS_FLAGS,
    PROP_EVENT_STATE,
    PROP_RELIABILITY,
    PROP_OUT_OF_SERVICE,
    PROP_POLARITY,
    PROP_CHANGE_OF_STATE_TIME,
    PROP_ELAPSED_ACTIVE_TIME,
    PROP_TIME_DELAY,
    PROP_NOTIFICATION_CLASS,
    PROP_ALARM_VALUE,
    PROP_EVENT_ENABLE,
    PROP_NOTIFY_TYPE,
//    PROP_INTERLOCK_REFERENCE_POINT_ID,
    PROP_INTERLOCK_REFERENCE,
    PROP_INTERLOCK_VALUE,
    PROP_NOTIFICATIONPERIOD,
    PROP_ELAPSED_STATE,
    PROP_CHANGE_STATE,
    PROP_ACTIVE_STATE,
    PROP_CHANGE_OF_STATE_COUNT,
    PROP_CHANGE_ENABLE,
    PROP_CHANGE_LIMIT,
    PROP_COV_RESUBSCRIPTION_INTERVAL,
    PROP_ELAPSED_ENABLE,
    -1
};

static const int Binary_Input_Properties_Optional[] = {
    PROP_DESCRIPTION,
    -1
};

static const int Binary_Input_Properties_Proprietary[] = {
    -1
};


static uint8_t get_cr_no_by_id(uint32_t id)
{
    switch(id)
    {
        case CR1IN_RUNNING_STAT:
        case CR1IN_RUNNING_ERROR:
        case CR1IN_ILLEGLE_ERROR:
            return 1;
        case CR1OUT_RUNNING_STAT:
        case CR1OUT_RUNNING_ERROR:
        case CR1OUT_ILLEGLE_ERROR:
            return 2;
        case CR2IN_RUNNING_STAT:
        case CR2IN_RUNNING_ERROR:
        case CR2IN_ILLEGLE_ERROR:
            return 3;
        case CR2OUT_RUNNING_STAT:
        case CR2OUT_RUNNING_ERROR:
        case CR2OUT_ILLEGLE_ERROR:
            return 4;
        
        case CR3IN_RUNNING_STAT:
        case CR3IN_RUNNING_ERROR:
        case CR3IN_ILLEGLE_ERROR:
            return 5;
        case CR3OUT_RUNNING_STAT:
        case CR3OUT_RUNNING_ERROR:
        case CR3OUT_ILLEGLE_ERROR:
            return 6;
        case CR4IN_RUNNING_STAT:
        case CR4IN_RUNNING_ERROR:
        case CR4IN_ILLEGLE_ERROR:
            return 7;
        case CR4OUT_RUNNING_STAT:
        case CR4OUT_RUNNING_ERROR:
        case CR4OUT_ILLEGLE_ERROR:
            return 8;
        
        case CR5IN_RUNNING_STAT:
        case CR5IN_RUNNING_ERROR:
        case CR5IN_ILLEGLE_ERROR:
            return 9;
        case CR5OUT_RUNNING_STAT:
        case CR5OUT_RUNNING_ERROR:
        case CR5OUT_ILLEGLE_ERROR:
            return 10;
        case CR6IN_RUNNING_STAT:
        case CR6IN_RUNNING_ERROR:
        case CR6IN_ILLEGLE_ERROR:
            return 11;
        case CR6OUT_RUNNING_STAT:
        case CR6OUT_RUNNING_ERROR:
        case CR6OUT_ILLEGLE_ERROR:
            return 12;
        
        case CR7IN_RUNNING_STAT:
        case CR7IN_RUNNING_ERROR:
        case CR7IN_ILLEGLE_ERROR:
            return 13;
        case CR7OUT_RUNNING_STAT:
        case CR7OUT_RUNNING_ERROR:
        case CR7OUT_ILLEGLE_ERROR:
            return 14;
        case CR8IN_RUNNING_STAT:
        case CR8IN_RUNNING_ERROR:
        case CR8IN_ILLEGLE_ERROR:
            return 15;
        case CR8OUT_RUNNING_STAT:
        case CR8OUT_RUNNING_ERROR:
        case CR8OUT_ILLEGLE_ERROR:
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
        case DOOR1_DOOR_STAT:
        case DOOR1_LOCK_STAT:
        case DOOR1_LOCK_ERROR:
        case DOOR1_OPEN_ABNORMAL_ERROR:
        case DOOR1_OPEN_TIMEOUT:
            return 1;
        case DOOR2_DOOR_STAT:
        case DOOR2_LOCK_STAT:
        case DOOR2_LOCK_ERROR:
        case DOOR2_OPEN_ABNORMAL_ERROR:
        case DOOR2_OPEN_TIMEOUT:
            return 2;
        case DOOR3_DOOR_STAT:
        case DOOR3_LOCK_STAT:
        case DOOR3_LOCK_ERROR:
        case DOOR3_OPEN_ABNORMAL_ERROR:
        case DOOR3_OPEN_TIMEOUT:
            return 3;
        case DOOR4_DOOR_STAT:
        case DOOR4_LOCK_STAT:
        case DOOR4_LOCK_ERROR:
        case DOOR4_OPEN_ABNORMAL_ERROR:
        case DOOR4_OPEN_TIMEOUT:
            return 4;
        
        case DOOR5_DOOR_STAT:
        case DOOR5_LOCK_STAT:
        case DOOR5_LOCK_ERROR:
        case DOOR5_OPEN_ABNORMAL_ERROR:
        case DOOR5_OPEN_TIMEOUT:
            return 5;
        case DOOR6_DOOR_STAT:
        case DOOR6_LOCK_STAT:
        case DOOR6_LOCK_ERROR:
        case DOOR6_OPEN_ABNORMAL_ERROR:
        case DOOR6_OPEN_TIMEOUT:
            return 6;
        case DOOR7_DOOR_STAT:
        case DOOR7_LOCK_STAT:
        case DOOR7_LOCK_ERROR:
        case DOOR7_OPEN_ABNORMAL_ERROR:
        case DOOR7_OPEN_TIMEOUT:
            return 7;
        case DOOR8_DOOR_STAT:
        case DOOR8_LOCK_STAT:
        case DOOR8_LOCK_ERROR:
        case DOOR8_OPEN_ABNORMAL_ERROR:
        case DOOR8_OPEN_TIMEOUT:
            return 8;
        default:
            break;
    }
    return 0;
}


void Binary_Input_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    if (pRequired) {
        *pRequired = Binary_Input_Properties_Required;
    }
    if (pOptional) {
        *pOptional = Binary_Input_Properties_Optional;
    }
    if (pProprietary) {
        *pProprietary = Binary_Input_Properties_Proprietary;
    }

    return;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need validate that the */
/* given instance exists */
bool Binary_Input_Valid_Instance(
    uint32_t object_instance)
{
    uint32_t i;
    for(i = 0; i < MAX_BINARY_INPUTS; i++)
        if(BI_ID[i] == object_instance)
            return true;
    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Binary_Input_Count(
    void)
{
    return MAX_BINARY_INPUTS;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the instance */
/* that correlates to the correct index */
uint32_t Binary_Input_Index_To_Instance(
    unsigned index)
{
    return BI_ID[index];
}

void Binary_Input_Init(
    void)
{
    static bool initialized = false;

    if (!initialized) {
        initialized = true;
    }

    return;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the index */
/* that correlates to the correct instance number */
unsigned Binary_Input_Instance_To_Index(
    uint32_t object_instance)
{
    uint32_t i;
    for(i = 0; i < MAX_BINARY_INPUTS; i++)
    {    
        if(BI_ID[i] == object_instance)
            break;
    }
    return i;
}

BACNET_BINARY_PV Binary_Input_Present_Value(
    uint32_t object_instance)
{
    BACNET_BINARY_PV value = BINARY_INACTIVE;
    uint8_t temp;
    uint8_t status;
    
    temp = get_cr_no_by_id(object_instance);
    /* 是读卡器操作 */
    if(temp != 0)
    {
        temp = find_card_reader_by_door(temp);
        if(temp != 0)
        {
            switch(object_instance)
            {
                case CR1IN_RUNNING_STAT:
                case CR1OUT_RUNNING_STAT:
                case CR2IN_RUNNING_STAT:
                case CR2OUT_RUNNING_STAT:
                case CR3IN_RUNNING_STAT:
                case CR3OUT_RUNNING_STAT:
                case CR4IN_RUNNING_STAT:
                case CR4OUT_RUNNING_STAT:
                case CR5IN_RUNNING_STAT:
                case CR5OUT_RUNNING_STAT:
                case CR6IN_RUNNING_STAT:
                case CR6OUT_RUNNING_STAT:
                case CR7IN_RUNNING_STAT:
                case CR7OUT_RUNNING_STAT:
                case CR8IN_RUNNING_STAT:
                case CR8OUT_RUNNING_STAT:
                    status = app_get_card_reader_enable_status(temp);
                    break;
                case CR1IN_RUNNING_ERROR:
                case CR1OUT_RUNNING_ERROR:
                case CR2IN_RUNNING_ERROR:    
                case CR2OUT_RUNNING_ERROR:  
                case CR3IN_RUNNING_ERROR:
                case CR3OUT_RUNNING_ERROR:
                case CR4IN_RUNNING_ERROR:    
                case CR4OUT_RUNNING_ERROR:  
                case CR5IN_RUNNING_ERROR:
                case CR5OUT_RUNNING_ERROR:
                case CR6IN_RUNNING_ERROR:    
                case CR6OUT_RUNNING_ERROR:  
                case CR7IN_RUNNING_ERROR:
                case CR7OUT_RUNNING_ERROR:
                case CR8IN_RUNNING_ERROR:    
                case CR8OUT_RUNNING_ERROR: 
                    status = app_get_card_reader_error_status(temp);
                    break;
                case CR1IN_ILLEGLE_ERROR:
                case CR1OUT_ILLEGLE_ERROR:
                case CR2IN_ILLEGLE_ERROR:
                case CR2OUT_ILLEGLE_ERROR:
                case CR3IN_ILLEGLE_ERROR:
                case CR3OUT_ILLEGLE_ERROR:
                case CR4IN_ILLEGLE_ERROR:
                case CR4OUT_ILLEGLE_ERROR:
                case CR5IN_ILLEGLE_ERROR:
                case CR5OUT_ILLEGLE_ERROR:
                case CR6IN_ILLEGLE_ERROR:
                case CR6OUT_ILLEGLE_ERROR:
                case CR7IN_ILLEGLE_ERROR:
                case CR7OUT_ILLEGLE_ERROR:
                case CR8IN_ILLEGLE_ERROR:
                case CR8OUT_ILLEGLE_ERROR:
                    status = app_get_illegle_card_error(temp);
                    break;
            }
        }
        if(status == 0)
            return BINARY_INACTIVE;
        else
            return BINARY_ACTIVE;
    }
    
    temp = get_door_no_by_id(object_instance);
    /* 是读卡器操作 */
    if(temp != 0)
    {
        switch(object_instance)
        {
            case DOOR1_DOOR_STAT:
            case DOOR2_DOOR_STAT:
            case DOOR3_DOOR_STAT:
            case DOOR4_DOOR_STAT:
            case DOOR5_DOOR_STAT:
            case DOOR6_DOOR_STAT:
            case DOOR7_DOOR_STAT:
            case DOOR8_DOOR_STAT:
                status = app_door_get_door_status(temp);
                break;
            case DOOR1_LOCK_STAT:
            case DOOR2_LOCK_STAT: 
            case DOOR3_LOCK_STAT:
            case DOOR4_LOCK_STAT: 
            case DOOR5_LOCK_STAT:
            case DOOR6_LOCK_STAT: 
            case DOOR7_LOCK_STAT:
            case DOOR8_LOCK_STAT:
                status = app_door_get_open_close_stat(temp);
                break;
            case DOOR1_LOCK_ERROR:
            case DOOR2_LOCK_ERROR:    
            case DOOR3_LOCK_ERROR:
            case DOOR4_LOCK_ERROR:    
            case DOOR5_LOCK_ERROR:
            case DOOR6_LOCK_ERROR:    
            case DOOR7_LOCK_ERROR:
            case DOOR8_LOCK_ERROR:  
                status = app_door_get_oepn_close_error(temp);
                break;
            case DOOR1_OPEN_ABNORMAL_ERROR:
            case DOOR2_OPEN_ABNORMAL_ERROR: 
            case DOOR3_OPEN_ABNORMAL_ERROR:
            case DOOR4_OPEN_ABNORMAL_ERROR: 
            case DOOR5_OPEN_ABNORMAL_ERROR:
            case DOOR6_OPEN_ABNORMAL_ERROR: 
            case DOOR7_OPEN_ABNORMAL_ERROR:
            case DOOR8_OPEN_ABNORMAL_ERROR:
                status = app_door_get_abnomal_error(temp);
                break;
            case DOOR1_OPEN_TIMEOUT:
            case DOOR2_OPEN_TIMEOUT:
            case DOOR3_OPEN_TIMEOUT:
            case DOOR4_OPEN_TIMEOUT:
            case DOOR5_OPEN_TIMEOUT:
            case DOOR6_OPEN_TIMEOUT:
            case DOOR7_OPEN_TIMEOUT:
            case DOOR8_OPEN_TIMEOUT:
                status = app_door_get_timeout_open_error(temp);
            break;
        }
        if(status == 0)
            return BINARY_INACTIVE;
        else
            return BINARY_ACTIVE;
    }
    
    
    
    return value;
}

bool Binary_Input_Change_Of_Value(
    uint32_t object_instance)
{
    bool status = false;
    unsigned index;

    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < MAX_BINARY_INPUTS) {
        status = Change_Of_Value[index];
    }

    return status;
}

void Binary_Input_Change_Of_Value_Clear(
    uint32_t object_instance)
{
    unsigned index;

    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < MAX_BINARY_INPUTS) {
        Change_Of_Value[index] = false;
    }

    return;
}

/**
 * Encode the Value List for Present-Value and Status-Flags
 *
 * @param object_instance - object-instance number of the object
 * @param  value_list - #BACNET_PROPERTY_VALUE with at least 2 entries
 *
 * @return true if values were encoded
 */
bool Binary_Input_Encode_Value_List(
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE * value_list)
{
    bool status = false;

    if (value_list) {
        value_list->propertyIdentifier = PROP_PRESENT_VALUE;
        value_list->propertyArrayIndex = BACNET_ARRAY_ALL;
        value_list->value.context_specific = false;
        value_list->value.tag = BACNET_APPLICATION_TAG_ENUMERATED;
        value_list->value.next = NULL;
        value_list->value.type.Enumerated =
            Binary_Input_Present_Value(object_instance);
        value_list->priority = BACNET_NO_PRIORITY;
        value_list = value_list->next;
    }
    if (value_list) {
        value_list->propertyIdentifier = PROP_STATUS_FLAGS;
        value_list->propertyArrayIndex = BACNET_ARRAY_ALL;
        value_list->value.context_specific = false;
        value_list->value.tag = BACNET_APPLICATION_TAG_BIT_STRING;
        value_list->value.next = NULL;
        bitstring_init(&value_list->value.type.Bit_String);
        bitstring_set_bit(&value_list->value.type.Bit_String,
            STATUS_FLAG_IN_ALARM, false);
        bitstring_set_bit(&value_list->value.type.Bit_String,
            STATUS_FLAG_FAULT, false);
        bitstring_set_bit(&value_list->value.type.Bit_String,
            STATUS_FLAG_OVERRIDDEN, false);

        bitstring_set_bit(&value_list->value.type.Bit_String,
            STATUS_FLAG_OUT_OF_SERVICE, false);
        value_list->priority = BACNET_NO_PRIORITY;
        value_list->next = NULL;
        status = true;
    }

    return status;
}



bool Binary_Input_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    static char text_string[32] = "";   /* okay for single thread */
    bool status = false;
    unsigned index = 0;

    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < MAX_BINARY_INPUTS) {
        sprintf(text_string, "BINARY INPUT %lu",
            (unsigned long) object_instance);
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}


/* return apdu length, or BACNET_STATUS_ERROR on error */
/* assumption - object already exists, and has been bounds checked */
int Binary_Input_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    uint8_t *apdu = NULL;
    BACNET_DATE date;
    uint32_t index;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }
    apdu = rpdata->application_data;
    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            index = Binary_Input_Instance_To_Index(rpdata->object_instance);
            if(index >= MAX_BINARY_INPUTS)
                apdu_len = encode_application_object_id(&apdu[0], 
                        OBJECT_BINARY_INPUT, 0);
            else
                apdu_len =encode_application_object_id(&apdu[0], 
                        OBJECT_BINARY_INPUT, BI_ID[index]);    
            break;
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            /* note: object name must be unique in our device */
            Binary_Input_Object_Name(rpdata->object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0], OBJECT_BINARY_INPUT);
            break;
        
        
        
        
        case PROP_PRESENT_VALUE:
            apdu_len = encode_application_enumerated(&apdu[0],
                Binary_Input_Present_Value(rpdata->object_instance));
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
        case PROP_CHANGE_OF_STATE_TIME:
            get_date_now(&date);
            apdu_len = encode_application_date(&apdu[0], &date);
            break;
        case PROP_ELAPSED_ACTIVE_TIME:
            apdu_len = encode_application_signed(&apdu[0], 0);
            break;
        case PROP_TIME_DELAY:
            apdu_len = encode_application_signed(&apdu[0], 255);
            break;
        case PROP_NOTIFICATION_CLASS:
            apdu_len = encode_application_signed(&apdu[0], 0);
            break;
        case PROP_ALARM_VALUE:
            apdu_len = encode_application_signed(&apdu[0], 0);
            break;
        case PROP_EVENT_ENABLE:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, EVENT_STATE_FILTER_OFFNORMAL, false);
            bitstring_set_bit(&bit_string, EVENT_STATE_FILTER_FAULT, false);
            bitstring_set_bit(&bit_string, EVENT_STATE_FILTER_NORMAL, false);
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_NOTIFY_TYPE:
            apdu_len = encode_application_signed(&apdu[0], 0);
            break;
    //    PROP_INTERLOCK_REFERENCE_POINT_ID,
        case PROP_INTERLOCK_REFERENCE:
            break;
        case PROP_INTERLOCK_VALUE:
            break;
    //    PROP_NOTIFICATION_PERIOD,
        case PROP_ELAPSED_STATE:
            apdu_len = encode_application_signed(&apdu[0], 0);
            break;
        case PROP_CHANGE_STATE:
            apdu_len = encode_application_signed(&apdu[0], 0);
            break;
        case PROP_ACTIVE_STATE:
            apdu_len = encode_application_signed(&apdu[0], 0);
            break;
        case PROP_CHANGE_OF_STATE_COUNT:
            apdu_len = encode_application_unsigned(&apdu[0], 0);
            break;
        case PROP_CHANGE_ENABLE:
            apdu_len = encode_application_boolean(&apdu[0], 0);
            break;
        case PROP_CHANGE_LIMIT:
            apdu_len = encode_application_unsigned(&apdu[0], 0);
            break;
        case PROP_COV_RESUBSCRIPTION_INTERVAL:
            apdu_len = encode_application_unsigned(&apdu[0], 0);
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
bool Binary_Input_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
    int len = 0;
    BACNET_APPLICATION_DATA_VALUE value;
    uint8_t door;

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
    /*  only array properties can have array options */
    if (wp_data->array_index != BACNET_ARRAY_ALL) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
    door = get_door_no_by_id(wp_data->object_instance);
	if (door == 0) {
		return false;
	}
    
    switch (wp_data->object_property) {
        case PROP_PRESENT_VALUE:
        case PROP_EVENT_STATE:
        case PROP_RELIABILITY:
        case PROP_OUT_OF_SERVICE:
        case PROP_POLARITY:
        case PROP_CHANGE_OF_STATE_TIME:
        case PROP_ELAPSED_ACTIVE_TIME:
        case PROP_TIME_DELAY:
        case PROP_NOTIFICATION_CLASS:
        case PROP_ALARM_VALUE:
        case PROP_EVENT_ENABLE:
        case PROP_NOTIFY_TYPE:
    //    PROP_INTERLOCK_REFERENCE_POINT_ID,
        case PROP_INTERLOCK_REFERENCE:
        case PROP_INTERLOCK_VALUE:
        case PROP_ELAPSED_STATE:
        case PROP_CHANGE_STATE:
        case PROP_ACTIVE_STATE:
        case PROP_CHANGE_OF_STATE_COUNT:
        case PROP_COV_RESUBSCRIPTION_INTERVAL:
        case PROP_NOTIFICATIONPERIOD:
        case PROP_ELAPSED_ENABLE:
            status = true;
            break;
        case PROP_CHANGE_ENABLE:
            status = WPValidateArgType(&value, BACNET_APPLICATION_TAG_BOOLEAN,
								&wp_data->error_class, &wp_data->error_code);
			if (status) {
				app_door_set_door_setting_para_uint32(door,  DOOR_SETTING_PARA_LOCK_OP_COUNT_ENABLE,
                    value.type.Unsigned_Int);
			}
			break;
        case PROP_CHANGE_LIMIT:
            status = WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
								&wp_data->error_class, &wp_data->error_code);
			if (status) {
				app_door_set_door_setting_para_uint32(door,  DOOR_SETTING_PARA_LOCK_OP_COUNT_MAX,
                    value.type.Unsigned_Int);
			}
			break;
        case PROP_OBJECT_IDENTIFIER:
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
        case PROP_OBJECT_TYPE:
        case PROP_STATUS_FLAGS:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            break;
        default:
//            wp_data->error_class = ERROR_CLASS_PROPERTY;
//            wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            status = true;
            break;
    }

    return status;
}






