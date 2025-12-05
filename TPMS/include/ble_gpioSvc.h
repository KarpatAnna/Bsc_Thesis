/**
 * @addtogroup   measure_advertise_conn
 * @{
 * @file         ble_custSvc.h
 * @brief        \glos{API} of the BLE custom service.
 * @details
 * This file contains the API and definitions for the BLE custom service of measure_advertise_conn. 
 * The custom service provides characteristics for GPIO.
 * It also includes functions for adding and removing the custom service from the attribute database,
 * processing read and write requests, handling CCC (Client Characteristic Configuration) events,
 * and configuring the TX power and GPIO.
 *
 * @copyright    (c) [Robert Bosch GmbH] [2024].
 *               All rights reserved, also regarding any disposal,
 *               exploitation, reproduction, editing, distribution, as well
 *               as in the event of applications for industrial property
 *               rights. The communication of its contents to others without
 *               express authorization is prohibited. Offenders will be held
 *               liable for the payment of damages. All rights reserved in
 *               the event of the grant of a patent, utility model or design.
 **/

#ifndef _BLE_GPIOSVC_H
#define _BLE_GPIOSVC_H

#include "rbk_smp290_gpio.h"
#include "rbk_smp290_nvm.h"
#include "ble_custSvc.h"
#include "main.h"

/// @defgroup measure_advertise_conn_cust_svc Custom Service definitions
/// @{

/**************************************************************************************************
 Macros
 **************************************************************************************************/
/// Custom Service start handle
#define BLE_CUST_SVC_GPIO_START_HNDL UINT16_C(BLE_CUST_SVC_MAX_HNDL)
/// Custom Service End handle
#define BLE_CUST_SVC_GPIO_END_HNDL UINT16_C(BLE_CUST_SVC_GPIO_MAX_HNDL - 1)

#define BLE_CUST_SVC_GPIO_SERVICE_UUID_PART    UINT16_C(0x1B00)  //!< Demo Service UUID

#define BLE_CUST_SVC_GPIO_MODE_CHAR_UUID_PART         UINT16_C(0x1B12) //!<GPIO mode characteristics UUID
#define BLE_CUST_SVC_GPIO_DRV_STRENGTH_CHAR_UUID_PART UINT16_C(0x1B13) //!<GPIO drive strength characteristics UUID
#define BLE_CUST_SVC_GPIO_PULL_CHAR_UUID_PART         UINT16_C(0x1B14) //!<GPIO direction characteristics UUID
#define BLE_CUST_SVC_GPIO_CHAR_UUID_PART              UINT16_C(0x1B03) //!<GPIO value characteristics UUID
#define BLE_CUST_SVC_GPIO_INPUT_CHAR_UUID_PART        UINT16_C(0x1B15) //!<GPIO input value characteristics UUID
#define BLE_CUST_SVC_GPIO_PIN_CHAR_UUID_PART          UINT16_C(0x1B16) //!<GPIO pin value characteristics UUID

/// Custom service 5de23c6e-xxxx-11f0-8de9-0242ac120002
/// Custom base UUID part 1
#define BLE_CUST_SVC_GPIO_BASE_UUID_PART1 0x02, 0x00, 0x12, 0xac, 0x42, 0x02, 0xe9, 0x8d, 0xf0, 0x11
/// Custom base UUID part 2
#define BLE_CUST_SVC_GPIO_BASE_UUID_PART2 0x6e, 0x3c, 0xe2, 0x5d

/// Macro for building Custom UUID
#define BLE_CUST_SVC_GPIO_BUILD(part) BLE_CUST_SVC_GPIO_BASE_UUID_PART1, RBK_SMP290_CONV_U16_TO_BYTES(part), BLE_CUST_SVC_GPIO_BASE_UUID_PART2

/// Macro for Building the Custom Service UUID
#define BLE_CUST_SVC_GPIO_SERVICE_UUID BLE_CUST_SVC_GPIO_BUILD(BLE_CUST_SVC_GPIO_SERVICE_UUID_PART)

/// Macro for Building the Custom Characteristics 12 UUID
#define BLE_CUST_SVC_GPIO_MODE_CHAR_UUID BLE_CUST_SVC_GPIO_BUILD(BLE_CUST_SVC_GPIO_MODE_CHAR_UUID_PART)

/// Macro for Building the Custom Characteristics 13 UUID
#define BLE_CUST_SVC_GPIO_DRV_STRENGTH_CHAR_UUID BLE_CUST_SVC_GPIO_BUILD(BLE_CUST_SVC_GPIO_DRV_STRENGTH_CHAR_UUID_PART)

/// Macro for Building the Custom Characteristics 14 UUID
#define BLE_CUST_SVC_GPIO_PULL_CHAR_UUID BLE_CUST_SVC_GPIO_BUILD(BLE_CUST_SVC_GPIO_PULL_CHAR_UUID_PART)

/// Macro for Building the Custom Characteristics 3 UUID
#define BLE_CUST_SVC_GPIO_CHAR_UUID BLE_CUST_SVC_GPIO_BUILD(BLE_CUST_SVC_GPIO_CHAR_UUID_PART)

/// Macro for Building the Custom Characteristics 15 UUID
#define BLE_CUST_SVC_GPIO_INPUT_CHAR_UUID BLE_CUST_SVC_GPIO_BUILD(BLE_CUST_SVC_GPIO_INPUT_CHAR_UUID_PART)

/// Macro for Building the Custom Characteristics 16 UUID
#define BLE_CUST_SVC_GPIO_PIN_CHAR_UUID BLE_CUST_SVC_GPIO_BUILD(BLE_CUST_SVC_GPIO_PIN_CHAR_UUID_PART)

/// GPIO mode Default value
#define GPIO_MODE_DIR_DEFAULT_STATUS  RBK_SMP290_GPIO_CFG_IO_DIR_DISABLED
#define GPIO_MODE_MODE_DEFAULT_STATUS RBK_SMP290_GPIO_CFG_OUT_MODE_NA

/// GPIO drive strength Default value
#define GPIO_DRV_STRENGTH_DEFAULT_STATUS RBK_SMP290_GPIO_CFG_IO_VALUE_LOW

/// GPIO pull resistor Default value
#define GPIO_PULL_DEFAULT_STATUS RBK_SMP290_GPIO_CFG_PULL_DOWN
/// GPIO default value
#define GPIO_DEFAULT_STATUS RBK_SMP290_GPIO_CFG_IO_VALUE_LOW
/// GPIO pin default value
#define GPIO_PIN_DEFAULT_STATUS RBK_SMP290_GPIO_0


/******************************************************************************\
 * Types
 \******************************************************************************/

/// Custom service handle values
typedef enum
{
    //Custom Service
	BLE_CUST_SVC_GPIO_SVC_HNDL = BLE_CUST_SVC_GPIO_START_HNDL, //!< Custom service declaration
	BLE_CUST_SVC_GPIO_MODE_CHAR_HNDL,                //!< Custom Characteristic 12 Handle
	BLE_CUST_SVC_GPIO_MODE_CHAR_DATA_HNDL,           //!< Custom Characteristic 12 Data Handle
	BLE_CUST_SVC_GPIO_MODE_CHAR_CUD_HNDL,            //!< Custom Characteristic 12 Characteristic User Description
    BLE_CUST_SVC_GPIO_DRV_STRENGTH_CHAR_HNDL,        //!< Custom Characteristic 13 Handle
    BLE_CUST_SVC_GPIO_DRV_STRENGTH_CHAR_DATA_HNDL,   //!< Custom Characteristic 13 Data Handle
    BLE_CUST_SVC_GPIO_DRV_STRENGTH_CHAR_CUD_HNDL,    //!< Custom Characteristic 13 Characteristic User Description
    BLE_CUST_SVC_GPIO_PULL_CHAR_HNDL,                //!< Custom Characteristic 14 Handle
    BLE_CUST_SVC_GPIO_PULL_CHAR_DATA_HNDL,           //!< Custom Characteristic 14 Data Handle
    BLE_CUST_SVC_GPIO_PULL_CHAR_CUD_HNDL,            //!< Custom Characteristic 14 Characteristic User Description
	BLE_CUST_SVC_GPIO_CHAR_HNDL,                     //!< Custom Characteristic 3 Handle 0x2903
	BLE_CUST_SVC_GPIO_CHAR_DATA_HNDL,                //!< Custom Characteristic 3 Data Handle 0x2903
	BLE_CUST_SVC_GPIO_CHAR_CUD_HNDL,                 //!< Custom Characteristic 3 Characteristic User Description 0x2903
	BLE_CUST_SVC_GPIO_INPUT_CHAR_HNDL,               //!< Custom Characteristic 15 Handle
	BLE_CUST_SVC_GPIO_INPUT_CHAR_DATA_HNDL,          //!< Custom Characteristic 15 Data Handle
	BLE_CUST_SVC_GPIO_INPUT_CHAR_CUD_HNDL,           //!< Custom Characteristic 15 Characteristic User Description
    BLE_CUST_SVC_GPIO_PIN_CHAR_HNDL,               //!< Custom Characteristic 16 Handle
	BLE_CUST_SVC_GPIO_PIN_CHAR_DATA_HNDL,          //!< Custom Characteristic 16 Data Handle
	BLE_CUST_SVC_GPIO_PIN_CHAR_CUD_HNDL,           //!< Custom Characteristic 16 Characteristic User Description
	BLE_CUST_SVC_GPIO_MAX_HNDL
} gpioSvc_ten;

// Restore default pack
#pragma pack()

/// Type definition of the custom service callback
typedef void (*custSvcAppCbk)(uint8_t req, rbk_smp290_ble_attsHndl hndl, uint8_t *value, uint16_t len);

/******************************************************************************\
 * Extern global variables
 \******************************************************************************/


/// @}

/******************************************************************************\
 * Public functions
 \******************************************************************************/

/**
 * @brief  This \glos{API} adds the custom service to the attribute database.
 * return void
 */
void addCustgpioSvc(void);
/**
 * @brief  This \glos{API} removes the custom service from the attribute database.
 * return void
 */
void rmCustgpioSvc(void);

/**
 * @brief This \glos{API} processes the incoming read request from the \glos{BLE} \glos{ATT} layer.
 *
 * @param  connId  The connection ID.
 * @param  handle  The attribute handle.
 * @param  Op      The operation code.
 * @param  offset  The offset.
 * @param  pAttr   Pointer to the attribute structure.
 * @return         The BLE ATT error code.
 */
rbk_smp290_ble_atts_err_ten custgpioSvc_rdCallBack(rbk_smp290_ble_attsConnId connId, rbk_smp290_ble_attsHndl handle, uint8_t Op, uint16_t offset,
                                              rbk_smp290_ble_attsAttr_tst *pAttr);
/**
 * @brief This \glos{API} processes the incoming write request from the \glos{BLE} \glos{ATT} layer.
 *
 * @param  connId  The connection ID.
 * @param  handle  The attribute handle.
 * @param  Op      The operation code.
 * @param  offset  The offset.
 * @param  len     The length of the data.
 * @param  pValue  Pointer to the data buffer.
 * @param  pAttr   Pointer to the attribute structure.
 * @return         The BLE ATT error code.
 */
rbk_smp290_ble_atts_err_ten custgpioSvc_wrCallBack(rbk_smp290_ble_attsConnId connId, rbk_smp290_ble_attsHndl handle, uint8_t Op, uint16_t offset,
                                              uint16_t len, uint8_t *pValue, rbk_smp290_ble_attsAttr_tst *pAttr);



/**
 * @brief Initializes the GPIO.
 *
 * return void
 */
void gpio_init(void);

/**
 * @brief  Configures the desired GPIO mode.
 *
 * @param  dir_value  The GPIO direction.
 * @param  dir_value  The GPIO mode.
 * return void
 *
 */
void config_gpio_mode(uint8_t dir_value, uint8_t mode_value);

/**
 * @brief  Configures the desired GPIO drive strength.
 *
 * @param  value  The GPIO drive strength.
 * return void
 *
 */
void config_gpio_drv_strength(uint8_t value);

/**
 * @brief  Configures the desired GPIO pull resistors.
 *
 * @param  value  The GPIO pull resistors.
 * return void
 *
 */
void config_gpio_pull(uint8_t value);

/**
 * @brief  Configures the desired GPIO value.
 *
 * @param  value  The GPIO value.
 * return void
 *
 */
void config_gpio(uint8_t value);

/**
 * @brief  Configures the desired GPIO pin.
 *
 * @param  value  The GPIO pin.
 * return void
 *
 */
void config_gpio_pin(uint8_t value);

#endif /* _BLE_GPIOSVC_H */

/** @} */
