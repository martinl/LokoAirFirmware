
/**
 ******************************************************************************
 * @file    lora_info.h
 * @author  MCD Application Team
 * @brief   To give info to the application about LoRaWAN configuration
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#ifndef __LORA_INFO_H__
#define __LORA_INFO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported constants --------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/*!
 * To give info to the application about LoraWAN capability
 * it can depend how it has been compiled (e.g. compiled regions ...)
 * Params should be better uint32_t foe easier alignment with info_table concept
 */
typedef struct {
    uint32_t ActivationMode; /*!< 1: ABP, 2 : OTAA, 3: ABP & OTAA   */
    uint32_t Region;         /*!< Combination of regions compiled on MW  */
    uint32_t ClassB;         /*!< 0: not compiled in Mw, 1 : compiled in MW  */
    uint32_t Kms;            /*!< 0: not compiled in Mw, 1 : compiled in MW  */
} LoraInfo_t;

/* External variables --------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
/**
 * @brief initialize the LoraInfo table
 */
void lora_info_init(void);

/**
 * @brief returns the pointer to the LoraInfo capabilities table
 * @retval LoraInfo pointer
 */
LoraInfo_t *LoraInfo_GetPtr(void);

#ifdef __cplusplus
}
#endif

#endif /* __LORA_INFO_H__ */
