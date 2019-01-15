/*
 * File      : bac_user.h
 * function  : bacnet 公共头文件
 * Change Logs:
 * Date           Author       Notes
 * 2018-06-25     limian      first version.
 */
#ifndef __BAC_USER_H__
#define __BAC_USER_H__

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "rp.h"
#include "wp.h"

typedef enum {
    TAG_PATTERN = 0,
    TAG_PATTERNS = 1,
    TAG_TEMP_SCHEDULE = 2,
    TAG_RESTORE_TIME = 3,
    TAG_INPUT_DOORNO = 4,
    TAG_DAY = 5
} FLC_DOOR_TAG;

typedef struct
{
    uint32_t Mode;
    BACNET_TIME Start_Time;
    BACNET_TIME End_Time;
}TEMP_SCHEDULE;


#endif
