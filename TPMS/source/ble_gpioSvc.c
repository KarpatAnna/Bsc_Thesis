/**
 * @addtogroup  measure_advertise_conn
 * @{
 * @file         ble_gpioSvc.c
 * @brief        Ble custom service implementation example
 * @details      This file contains the implementation of a custom BLE service for the measure_advertise_conn example.
 *               The custom service includes characteristics for GPIO.
 *               It also provides functions for sending indications and handling timer events.
 * @copyright    (c) [Robert Bosch GmbH] [2024].
 *               All rights reserved, also regarding any disposal,
 *               exploitation, reproduction, editing, distribution, as well
 *               as in the event of applications for industrial property
 *               rights. The communication of its contents to others without
 *               express authorization is prohibited. Offenders will be held
 *               liable for the payment of damages. All rights reserved in
 *               the event of the grant of a patent, utility model or design.
 **/

/* System includes */
#include <string.h>

/* Library includes */
#include "rbk_smp290_ble_atts_types.h"
#include "rbk_smp290_ble_atts_uuid.h"
#include "rbk_smp290_ble_atts.h"
#include "rbk_smp290_ble_radio.h"
#include "rbk_smp290_ble_timer.h"
#include "rbk_smp290_gpio.h"

/* Project includes */
#include "ble_gpioSvc.h"
#include "main.h"

/// @defgroup measure_advertise_conn_cust_svc Custom Service definitions
/// @{

/**************************************************************************************************
 Global variables
 **************************************************************************************************/
/// Variable to hold the gpio demo status
int8_t ble_gpioStatus;
/// GPIO set value
SECTION_PERSISTENT static uint8_t gpio_setValue[3] = {0};
/// GPIO active pin
SECTION_PERSISTENT static uint8_t gpio_pin;

/**************************************************************************************************
  Custom service group
 **************************************************************************************************/
/**************************************************************************************************
  Custom service definition
 **************************************************************************************************/
static const uint8_t custSvcgpio[]   = {BLE_CUST_SVC_GPIO_SERVICE_UUID};
static const uint16_t custSvcgpioLen = sizeof(custSvcgpio);
/**************************************************************************************************
  GPIO Characteristic definitions
 **************************************************************************************************/
/// GPIO mode characteristic declaration
static const uint8_t gpioModeCharUuid[] = {BLE_CUST_SVC_GPIO_MODE_CHAR_UUID};
static const uint8_t gpioModeCharVal[]  = {((uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PPTY_WRITE),
                                       RBK_SMP290_CONV_U16_TO_BYTES((uint16_t)BLE_CUST_SVC_GPIO_MODE_CHAR_DATA_HNDL), BLE_CUST_SVC_GPIO_MODE_CHAR_UUID};
static const uint16_t gpioModeCharLen   = sizeof(gpioModeCharVal);

/// GPIO mode characteristics value
static uint8_t gpioModeCharData[]   = {0, 0};
static uint16_t gpioModeCharDataLen = sizeof(gpioModeCharData);

/// GPIO mode characteristics user description value
static const uint8_t gpioModeCharUserDesc[]   = "GPIO mode";
static const uint16_t gpioModeCharUserDescLen = sizeof(gpioModeCharUserDesc);

/// GPIO drive strength characteristic declaration
static const uint8_t gpioDrvStrengthCharUuid[] = {BLE_CUST_SVC_GPIO_DRV_STRENGTH_CHAR_UUID};
static const uint8_t gpioDrvStrengthCharVal[]  = {((uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PPTY_WRITE),
                                       RBK_SMP290_CONV_U16_TO_BYTES((uint16_t)BLE_CUST_SVC_GPIO_DRV_STRENGTH_CHAR_DATA_HNDL), BLE_CUST_SVC_GPIO_DRV_STRENGTH_CHAR_UUID};
static const uint16_t gpioDrvStrengthCharLen   = sizeof(gpioDrvStrengthCharVal);

/// GPIO drive strength characteristics value
static uint8_t gpioDrvStrengthCharData[]   = {0};
static uint16_t gpioDrvStrengthCharDataLen = sizeof(gpioDrvStrengthCharData);

/// GPIO drive strength characteristics user description value
static const uint8_t gpioDrvStrengthCharUserDesc[]   = "GPIO drive strength";
static const uint16_t gpioDrvStrengthCharUserDescLen = sizeof(gpioDrvStrengthCharUserDesc);

/// GPIO pull resistors characteristic declaration
static const uint8_t gpioPullCharUuid[] = {BLE_CUST_SVC_GPIO_PULL_CHAR_UUID};
static const uint8_t gpioPullCharVal[]  = {((uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PPTY_WRITE),
                                          RBK_SMP290_CONV_U16_TO_BYTES((uint16_t)BLE_CUST_SVC_GPIO_PULL_CHAR_DATA_HNDL), BLE_CUST_SVC_GPIO_PULL_CHAR_UUID};
static const uint16_t gpioPullCharLen   = sizeof(gpioPullCharVal);

/// GPIO pull resistors characteristics value
static uint8_t gpioPullCharData[]   = {0};
static uint16_t gpioPullCharDataLen = sizeof(gpioPullCharData);

/// GPIO pull resistors characteristics user description value
static const uint8_t gpioPullCharUserDesc[]   = "GPIO pull resistors";
static const uint16_t gpioPullCharUserDescLen = sizeof(gpioPullCharUserDesc);

/// GPIO value characteristic declaration
static const uint8_t gpioCharUuid[] = {BLE_CUST_SVC_GPIO_CHAR_UUID};
static const uint8_t gpioCharVal[]  = {((uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PPTY_WRITE),
                                      RBK_SMP290_CONV_U16_TO_BYTES((uint16_t)BLE_CUST_SVC_GPIO_CHAR_DATA_HNDL), BLE_CUST_SVC_GPIO_CHAR_UUID};
static const uint16_t gpioCharLen   = sizeof(gpioCharVal);

/// GPIO value characteristic value
static uint8_t gpioCharData[]   = {0};
static uint16_t gpioCharDataLen = sizeof(gpioCharData);

/// GPIO value characteristic user description value
static const uint8_t gpioCharUserDesc[]   = "GPIO value";
static const uint16_t gpioCharUserDescLen = sizeof(gpioCharUserDesc);

/// GPIO input level characteristic declaration
static const uint8_t gpioInputCharUuid[] = {BLE_CUST_SVC_GPIO_INPUT_CHAR_UUID};
static const uint8_t gpioInputCharVal[]  = {(uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ,
                                           RBK_SMP290_CONV_U16_TO_BYTES((uint16_t)BLE_CUST_SVC_GPIO_INPUT_CHAR_DATA_HNDL), BLE_CUST_SVC_GPIO_INPUT_CHAR_UUID};
static const uint16_t gpioInputCharLen   = sizeof(gpioInputCharVal);

/// GPIO input level characteristic value
static uint8_t gpioInputCharData[]   = {0};
static uint16_t gpioInputCharDataLen = sizeof(gpioInputCharData);

/// GPIO input level characteristic user description value
static const uint8_t gpioInputCharUserDesc[]   = "GPIO input level";
static const uint16_t gpioInputCharUserDescLen = sizeof(gpioInputCharUserDesc);

/// GPIO pin characteristic declaration
static const uint8_t gpioPinCharUuid[] = {BLE_CUST_SVC_GPIO_PIN_CHAR_UUID};
static const uint8_t gpioPinCharVal[]  = {((uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PPTY_WRITE),
                                           RBK_SMP290_CONV_U16_TO_BYTES((uint16_t)BLE_CUST_SVC_GPIO_PIN_CHAR_DATA_HNDL), BLE_CUST_SVC_GPIO_PIN_CHAR_UUID};
static const uint16_t gpioPinCharLen   = sizeof(gpioPinCharVal);

/// GPIO pin characteristic value
static uint8_t gpioPinCharData[]   = {0};
static uint16_t gpioPinCharDataLen = sizeof(gpioPinCharData);

/// GPIO pin characteristic user description value
static const uint8_t gpioPinCharUserDesc[]   = "GPIO pin";
static const uint16_t gpioPinCharUserDescLen = sizeof(gpioPinCharUserDesc);
/**************************************************************************************************
  Custom service attributes list
 **************************************************************************************************/
static const rbk_smp290_ble_attsAttr_tst custSvcgpioAttrGrp[] = {
    /// Add Primary service
    {
        rbk_smp290_ble_attsPrimSvcUuid,           // Primary service declaration UUID: 0x2800
        (uint8_t *)custSvcgpio,                      // Primary service Attribute Value: Custom Service UUID: 5de23c6e-1b00-11f0-8de9-0242ac120002
        (uint16_t *)&custSvcgpioLen,                 // Primary service Attribute Value length
        sizeof(custSvcgpio),                         // Primary service Attribute Value  maximum Length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Primary service Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Primary service Attribute permission
    },

    /// GPIO Mode Characteristic
	{
		rbk_smp290_ble_attsChUuid,               // Characteristic declaration UUID: 0x2803
		(uint8_t *)gpioModeCharVal,             // Characteristic Attribute Value
		(uint16_t *)&gpioModeCharLen,           // Characteristic Attribute Value length
		sizeof(gpioModeCharVal),                // Characteristic Attribute Value maximum length
		(uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,   // Characteristic attribute settings
		(uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ // Characteristic attribute permission
	},
	/// GPIO Mode Characteristic value declaration
	{
		gpioModeCharUuid,                 // Characteristic UUID
		(uint8_t *)gpioModeCharData,      // Characteristic value
		(uint16_t *)&gpioModeCharDataLen, // Characteristic value length
		sizeof(gpioModeCharData),         // Characteristic value maximum Length
		((uint8_t)RBK_SMP290_BLE_ATTS_SET_UUID_128 | (uint8_t)RBK_SMP290_BLE_ATTS_SET_READ_CBACK |
		 (uint8_t)RBK_SMP290_BLE_ATTS_SET_WRITE_CBACK),                                        // Characteristic value Attribute settings
		((uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE)  // Characteristic value Attribute permission
	},
	/// GPIO Mode Characteristic User Description
	{
		rbk_smp290_ble_attsChUserDescUuid,       // Characteristic User Description: 0x2901
		(uint8_t *)gpioModeCharUserDesc,        // Characteristic User Description Value
		(uint16_t *)&gpioModeCharUserDescLen,   // Characteristic User Description Value length
		sizeof(gpioModeCharUserDesc),           // Characteristic User Description Value maximum length
		(uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,   // Characteristic User description Attribute settings
		(uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ // Characteristic User description Attribute Permission
	},

    /// GPIO Drive Strength Characteristic
    {
        rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
        (uint8_t *)gpioDrvStrengthCharVal,       // Characteristic Attribute Value
        (uint16_t *)&gpioDrvStrengthCharLen,     // Characteristic Attribute Value length
        sizeof(gpioCharVal),                     // Characteristic Attribute Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic attribute permission
    },
    /// GPIO Drive Strength Characteristic value declaration
    {
    	gpioDrvStrengthCharUuid,                 // Characteristic UUID
        (uint8_t *)gpioDrvStrengthCharData,      // Characteristic value
        (uint16_t *)&gpioDrvStrengthCharDataLen, // Characteristic value length
		sizeof(gpioDrvStrengthCharData),         // Characteristic value maximum Length
        ((uint8_t)RBK_SMP290_BLE_ATTS_SET_UUID_128 | (uint8_t)RBK_SMP290_BLE_ATTS_SET_READ_CBACK |
         (uint8_t)RBK_SMP290_BLE_ATTS_SET_WRITE_CBACK),                                        // Characteristic value Attribute settings
        ((uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE)  // Characteristic value Attribute permission
    },
    /// GPIO Drive Strength Characteristic User Description
    {
        rbk_smp290_ble_attsChUserDescUuid,            // Characteristic User Description: 0x2901
        (uint8_t *)gpioDrvStrengthCharUserDesc,      // Characteristic User Description Value
        (uint16_t *)&gpioDrvStrengthCharUserDescLen, // Characteristic User Description Value length
        sizeof(gpioDrvStrengthCharUserDesc),         // Characteristic User Description Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,        // Characteristic User description Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ      // Characteristic User description Attribute Permission
    },

    /// GPIO Pull Resistor Characteristic
    {
        rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
        (uint8_t *)gpioPullCharVal,              // Characteristic Attribute Value
        (uint16_t *)&gpioPullCharLen,            // Characteristic Attribute Value length
        sizeof(gpioPullCharVal),                 // Characteristic Attribute Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic attribute permission
    },
    /// GPIO Pull Resistor Characteristic value declaration
    {
        gpioPullCharUuid,                  // Characteristic UUID
        (uint8_t *)gpioPullCharData,       // Characteristic value
        (uint16_t *)&gpioPullCharDataLen,  // Characteristic value length
		sizeof(gpioPullCharData),          // Characteristic value maximum Length
        ((uint8_t)RBK_SMP290_BLE_ATTS_SET_UUID_128 | (uint8_t)RBK_SMP290_BLE_ATTS_SET_READ_CBACK |
         (uint8_t)RBK_SMP290_BLE_ATTS_SET_WRITE_CBACK),                                        // Characteristic value Attribute settings
        ((uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE)  // Characteristic value Attribute permission
    },
    /// GPIO Pull Resistor Characteristic User Description
    {
        rbk_smp290_ble_attsChUserDescUuid,        // Characteristic User Description: 0x2901
        (uint8_t *)gpioPullCharUserDesc,         // Characteristic User Description Value
        (uint16_t *)&gpioPullCharUserDescLen,    // Characteristic User Description Value length
        sizeof(gpioPullCharUserDesc),            // Characteristic User Description Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic User description Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic User description Attribute Permission
    },

    /// GPIO Value Characteristic
	{
	   rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
	   (uint8_t *)gpioCharVal,                  // Characteristic Attribute Value
	   (uint16_t *)&gpioCharLen,                // Characteristic Attribute Value length
	   sizeof(gpioCharVal),                     // Characteristic Attribute Value maximum length
	   (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic attribute settings
	   (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic attribute permission
	},
	/// GPIO Value Characteristic value declaration
	{
        gpioCharUuid,                      // Characteristic UUID
        (uint8_t *)gpioCharData,           // Characteristic value
        (uint16_t *)&gpioCharDataLen,      // Characteristic value length
		sizeof(gpioCharData),              // Characteristic value maximum Length
        ((uint8_t)RBK_SMP290_BLE_ATTS_SET_UUID_128 | (uint8_t)RBK_SMP290_BLE_ATTS_SET_READ_CBACK |
         (uint8_t)RBK_SMP290_BLE_ATTS_SET_WRITE_CBACK),                                        // Characteristic value Attribute settings
        ((uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE)  // Characteristic value Attribute permission
    },
	/// GPIO Value Characteristic User Description
    {
        rbk_smp290_ble_attsChUserDescUuid,        // Characteristic User Description: 0x2901
        (uint8_t *)gpioCharUserDesc,             // Characteristic User Description Value
        (uint16_t *)&gpioCharUserDescLen,        // Characteristic User Description Value length
        sizeof(gpioCharUserDesc),                // Characteristic User Description Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic User description Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic User description Attribute Permission
    },

	/// GPIO Input Level Characteristic
	{
		rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
		(uint8_t *)gpioInputCharVal,             // Characteristic Attribute Value
		(uint16_t *)&gpioInputCharLen,           // Characteristic Attribute Value length
		sizeof(gpioInputCharVal),                // Characteristic Attribute Value maximum length
		(uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic attribute settings
		(uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic attribute permission
	},
	/// GPIO Input Level Characteristic value declaration
	{
		gpioInputCharUuid,                 // Characteristic UUID
		(uint8_t *)gpioInputCharData,      // Characteristic value
		(uint16_t *)&gpioInputCharDataLen, // Characteristic value length
		sizeof(gpioInputCharData),         // Characteristic value maximum Length
		((uint8_t)RBK_SMP290_BLE_ATTS_SET_UUID_128 | (uint8_t)RBK_SMP290_BLE_ATTS_SET_READ_CBACK), // Characteristic value Attribute settings
		((uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ)                                                // Characteristic value Attribute permission
	},
	/// GPIO Input Level Characteristic User Description
	{
		rbk_smp290_ble_attsChUserDescUuid,        // Characteristic User Description: 0x2901
		(uint8_t *)gpioInputCharUserDesc,         // Characteristic User Description Value
		(uint16_t *)&gpioInputCharUserDescLen,    // Characteristic User Description Value length
		sizeof(gpioInputCharUserDesc),            // Characteristic User Description Value maximum length
		(uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic User description Attribute settings
		(uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic User description Attribute Permission
	},
    /// GPIO Pin Characteristic
	{
	   rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
	   (uint8_t *)gpioPinCharVal,                  // Characteristic Attribute Value
	   (uint16_t *)&gpioPinCharLen,                // Characteristic Attribute Value length
	   sizeof(gpioPinCharVal),                     // Characteristic Attribute Value maximum length
	   (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic attribute settings
	   (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic attribute permission
	},
	/// GPIO Pin Characteristic value declaration
	{
        gpioPinCharUuid,                      // Characteristic UUID
        (uint8_t *)gpioPinCharData,           // Characteristic value
        (uint16_t *)&gpioPinCharDataLen,      // Characteristic value length
		sizeof(gpioPinCharData),              // Characteristic value maximum Length
        ((uint8_t)RBK_SMP290_BLE_ATTS_SET_UUID_128 | (uint8_t)RBK_SMP290_BLE_ATTS_SET_READ_CBACK |
         (uint8_t)RBK_SMP290_BLE_ATTS_SET_WRITE_CBACK),                                        // Characteristic value Attribute settings
        ((uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE)  // Characteristic value Attribute permission
    },
	/// GPIO Pin Characteristic User Description
    {
        rbk_smp290_ble_attsChUserDescUuid,        // Characteristic User Description: 0x2901
        (uint8_t *)gpioPinCharUserDesc,             // Characteristic User Description Value
        (uint16_t *)&gpioPinCharUserDescLen,        // Characteristic User Description Value length
        sizeof(gpioPinCharUserDesc),                // Characteristic User Description Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic User description Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic User description Attribute Permission
    }};
/**************************************************************************************************
  Custom service Attribute Grouping
 **************************************************************************************************/
/// Custom service Attribute Group
rbk_smp290_ble_attsAttrGrp_tst custgpioSvcGrp = {NULL,
                                             (rbk_smp290_ble_attsAttr_tst *)custSvcgpioAttrGrp,
                                             custgpioSvc_rdCallBack,
                                             custgpioSvc_wrCallBack,
                                             (uint16_t)BLE_CUST_SVC_GPIO_START_HNDL,
                                             (uint16_t)BLE_CUST_SVC_GPIO_END_HNDL};

/// @}
/*******************************************************************************
 *  Function definition
 ******************************************************************************/
rbk_smp290_ble_atts_err_ten custgpioSvc_rdCallBack(rbk_smp290_ble_attsConnId connId, rbk_smp290_ble_attsHndl handle, uint8_t Op, uint16_t offset,
                                              rbk_smp290_ble_attsAttr_tst *pAttr)
{
    (void)(connId);
    (void)(Op);
    (void)(offset);

    rbk_smp290_gpio_io_dir_ten       pin_dir  = RBK_SMP290_GPIO_CFG_IO_DIR_DISABLED;
	rbk_smp290_gpio_out_mode_cfg_ten pin_mode = RBK_SMP290_GPIO_CFG_OUT_MODE_NA;
	rbk_smp290_gpio_pull_ten         pull_cfg = RBK_SMP290_GPIO_CFG_PULL_DOWN;
	rbk_smp290_gpio_io_value_ten     pin_val  = RBK_SMP290_GPIO_CFG_IO_VALUE_LOW;

	rbk_smp290_gpio_err_ten status = 0x00;

    uint8_t pin;
    uint8_t *data = pAttr->pAttValue;
    uint16_t *len = pAttr->pLen;

    switch (handle)
    {
        case BLE_CUST_SVC_GPIO_MODE_CHAR_DATA_HNDL:
			//Get the current GPIO mode
        	status = rbk_smp290_gpio_cfg_get(gpio_pin, &pin_dir, &pin_mode);
			if (RBK_SMP290_GPIO_SUCCESS != status)
			{
				smp290_log(LOG_VERBOSITY_ERROR, "Error reading GPIO mode: 0x%02x!\r\n", status);
			}
			data[0] = (uint8_t)pin_dir;
			if (pin_dir == RBK_SMP290_GPIO_CFG_IO_DIR_OUTPUT)
			{
				data[1] = (uint8_t)pin_mode;
			}
			else
			{
				data[1] = (uint8_t)RBK_SMP290_GPIO_CFG_OUT_MODE_NA;
			}

			smp290_log(LOG_VERBOSITY_INFO, "Reading GPIO mode: direction 0x%02x mode 0x%02x.\r\n", data[0], data[1]);
			*len  = 2;
			break;
        case BLE_CUST_SVC_GPIO_DRV_STRENGTH_CHAR_DATA_HNDL:
			//Get the current GPIO drive strength
            if (gpio_pin == RBK_SMP290_GPIO_4)
            {
                *data = (uint8_t)rbk_smp290_gpio_drive_get();
			    *len  = 1;
            }
			break;
        case BLE_CUST_SVC_GPIO_PULL_CHAR_DATA_HNDL:
            //Get the current GPIO pull configuration
        	status = rbk_smp290_gpio_pull_get(gpio_pin, &pull_cfg);
        	if (RBK_SMP290_GPIO_SUCCESS != status)
			{
        		smp290_log(LOG_VERBOSITY_ERROR, "Error reading GPIO pull resistor setting: 0x%02x!\r\n", status);
			}
            *data = (uint8_t)pull_cfg;
            *len  = 1;
            break;
        case BLE_CUST_SVC_GPIO_CHAR_DATA_HNDL:
            pin = (gpio_pin == 4) ? 2 : gpio_pin;
        	*data = gpio_setValue[pin];
        	*len  = 1;
        	break;
        case BLE_CUST_SVC_GPIO_INPUT_CHAR_DATA_HNDL:
			//Get the current gpio value
        	status = rbk_smp290_gpio_value_get(gpio_pin, &pin_val);
        	if (RBK_SMP290_GPIO_SUCCESS != status)
			{
        		smp290_log(LOG_VERBOSITY_ERROR, "Error reading GPIO input: 0x%02x!\r\n", status);
			}
			*data = (uint8_t)pin_val;
			*len  = 1;
			break;
        case BLE_CUST_SVC_GPIO_PIN_CHAR_DATA_HNDL:
            //Get the current gpio pin
        	*data = gpio_pin;
        	*len  = 1;
        	break;
        default:
        {
            return RBK_SMP290_BLE_ATTS_ERR_HANDLE;
        }
        break;
    }
    return RBK_SMP290_BLE_ATTS_SUCCESS;
}

rbk_smp290_ble_atts_err_ten custgpioSvc_wrCallBack(rbk_smp290_ble_attsConnId connId, rbk_smp290_ble_attsHndl handle, uint8_t Op, uint16_t offset,
                                              uint16_t len, uint8_t *pValue, rbk_smp290_ble_attsAttr_tst *pAttr)
{
    (void)(connId);
    (void)(Op);
    (void)(offset);
    (void)(pAttr);
    (void)(len);

    switch (handle)
    {
        case BLE_CUST_SVC_GPIO_MODE_CHAR_DATA_HNDL:
			config_gpio_mode(pValue[0], pValue[1]);
			break;
        case BLE_CUST_SVC_GPIO_DRV_STRENGTH_CHAR_DATA_HNDL:
			if (gpio_pin == RBK_SMP290_GPIO_4)
            {
                config_gpio_drv_strength(*pValue);
            }
			break;
        case BLE_CUST_SVC_GPIO_PULL_CHAR_DATA_HNDL:
			config_gpio_pull(*pValue);
			break;
        case BLE_CUST_SVC_GPIO_CHAR_DATA_HNDL:
        	config_gpio(*pValue);
            break;
        case BLE_CUST_SVC_GPIO_PIN_CHAR_DATA_HNDL:
        	config_gpio_pin(*pValue);
            break;
        default:
        {
            return RBK_SMP290_BLE_ATTS_ERR_HANDLE;
        }
        break;
    }
    return RBK_SMP290_BLE_ATTS_SUCCESS;
}

void addCustgpioSvc()
{
    (void)rbk_smp290_ble_atts_addAttrGrp((rbk_smp290_ble_attsAttrGrp_tst *)&custgpioSvcGrp);
    // Set GPIO default state
    config_gpio_mode(GPIO_MODE_DIR_DEFAULT_STATUS, GPIO_MODE_MODE_DEFAULT_STATUS);
    config_gpio_drv_strength(GPIO_DRV_STRENGTH_DEFAULT_STATUS);
    config_gpio_pull(GPIO_PULL_DEFAULT_STATUS);
    config_gpio(GPIO_DEFAULT_STATUS);
    config_gpio_pin(GPIO_PIN_DEFAULT_STATUS);
}
void rmCustgpioSvc()
{
    //
    (void)rbk_smp290_ble_atts_rmvAttrGrp((uint16_t)BLE_CUST_SVC_GPIO_START_HNDL);
}

void config_gpio_mode(uint8_t dir_value, uint8_t mode_value)
{
	rbk_smp290_gpio_io_dir_ten       pin_dir  = (rbk_smp290_gpio_io_dir_ten)dir_value;
	rbk_smp290_gpio_out_mode_cfg_ten pin_mode = (rbk_smp290_gpio_out_mode_cfg_ten)mode_value;

    rbk_smp290_gpio_err_ten status = 0x00;
    // Set GPIO mode
    if ((pin_dir == RBK_SMP290_GPIO_CFG_IO_DIR_DISABLED || pin_dir == RBK_SMP290_GPIO_CFG_IO_DIR_INPUT ||
    		pin_dir == RBK_SMP290_GPIO_CFG_IO_DIR_OUTPUT) && (pin_mode == RBK_SMP290_GPIO_CFG_OUT_MODE_PUSH_PULL ||
			pin_mode == RBK_SMP290_GPIO_CFG_OUT_MODE_OPEN_DRAIN || pin_mode == RBK_SMP290_GPIO_CFG_OUT_MODE_NA))
    {
		status = rbk_smp290_gpio_cfg_set(gpio_pin, pin_dir, pin_mode);
		smp290_log(LOG_VERBOSITY_INFO, "GPIO mode set: direction 0x%02x mode 0x%02x.\r\n", pin_dir, pin_mode);
        if (RBK_SMP290_GPIO_SUCCESS != status)
        {
        	smp290_log(LOG_VERBOSITY_ERROR, "Error setting GPIO mode: 0x%02x!\r\n", status);
        }
    }
    else
    {
        // otherwise ignore
    	smp290_log(LOG_VERBOSITY_WARNING, "GPIO mode ignored.\r\n");
    }
}

void config_gpio_drv_strength(uint8_t value)
{
	rbk_smp290_gpio_err_ten status = 0x00;
    // Set GPIO drive strength
    if (RBK_SMP290_GPIO_CFG_DRIVE_STRENGTH_LOW == value || RBK_SMP290_GPIO_CFG_DRIVE_STRENGTH_HIGH  == value)
    {
    	status = rbk_smp290_gpio_drive_set((rbk_smp290_gpio_drive_strength_cfg_ten)value);
    	smp290_log(LOG_VERBOSITY_INFO, "GPIO drive strength set: 0x%02x.\r\n", value);
        if (RBK_SMP290_GPIO_SUCCESS != status)
        {
        	smp290_log(LOG_VERBOSITY_ERROR, "Error setting GPIO drive strength: 0x%02x!\r\n", status);
        }
    }
    else
    {
    	// otherwise ignore
    	smp290_log(LOG_VERBOSITY_WARNING, "GPIO drive strength ignored.\r\n");
    }
}

void config_gpio_pull(uint8_t value)
{
	rbk_smp290_gpio_err_ten status = 0x00;
    // Set GPIO pull configuration
    if (RBK_SMP290_GPIO_CFG_PULL_NONE == value || RBK_SMP290_GPIO_CFG_PULL_UP == value || RBK_SMP290_GPIO_CFG_PULL_DOWN == value)
    {
        status = rbk_smp290_gpio_pull_set(gpio_pin, (rbk_smp290_gpio_pull_ten)value);
        smp290_log(LOG_VERBOSITY_INFO, "GPIO pull configuration set: 0x%02x.\r\n", value);
        if (RBK_SMP290_GPIO_SUCCESS != status)
        {
        	smp290_log(LOG_VERBOSITY_ERROR, "Error setting GPIO pull configuration: 0x%02x!\r\n", status);
        }
    }
    else
    {
        // otherwise ignore
    	smp290_log(LOG_VERBOSITY_WARNING, "GPIO pull configuration ignored.\r\n");
    }
}

void config_gpio(uint8_t value)
{
    rbk_smp290_gpio_err_ten status = 0x00;
    uint8_t pin;
    // Set GPIO value
    if (RBK_SMP290_GPIO_CFG_IO_VALUE_LOW == value || RBK_SMP290_GPIO_CFG_IO_VALUE_HIGH == value)
    {
        status = rbk_smp290_gpio_value_set(gpio_pin, (rbk_smp290_gpio_io_value_ten)value);
        if (RBK_SMP290_GPIO_SUCCESS == status)
        {
            pin = (gpio_pin == 4) ? 2 : gpio_pin;
        	gpio_setValue[pin] = value;
        	smp290_log(LOG_VERBOSITY_INFO, "GPIO set value: 0x%02x.\r\n", gpio_setValue[pin]);
        }
        else
        {
        	smp290_log(LOG_VERBOSITY_ERROR, "Error setting GPIO value: 0x%02x!\r\n", status);
        }
    }
    else
    {
        // otherwise ignore
        smp290_log(LOG_VERBOSITY_WARNING, "GPIO value ignored.\r\n");
    }
}

void config_gpio_pin(uint8_t value)
{
    rbk_smp290_gpio_err_ten status = 0x00;
    // Set GPIO pin
    if (RBK_SMP290_GPIO_0 == value || RBK_SMP290_GPIO_1 == value || RBK_SMP290_GPIO_4 == value)
    {
        
    	gpio_pin = value;
        smp290_log(LOG_VERBOSITY_INFO, "GPIO active pin: 0x%02x.\r\n", gpio_pin);
        
    }
    else
    {
        // otherwise ignore
        smp290_log(LOG_VERBOSITY_WARNING, "GPIO pin ignored.\r\n");
    }
}
/** @} */
