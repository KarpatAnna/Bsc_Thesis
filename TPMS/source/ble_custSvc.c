/**
 * @addtogroup  measure_advertise_conn
 * @{
 * @file         ble_custSvc.c
 * @brief        Ble custom service implementation example
 * @details      This file contains the implementation of a custom BLE service for the measure_advertise_conn example.
 *               The custom service includes characteristics for TX power, counter value, and GPIO status.
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
#include "rbk_smp290_tsd.h"
#include "rbk_smp290_ble_types.h"

/* Project includes */
#include "ble_custSvc.h"
#include "main.h"

/// @defgroup measure_advertise_conn_cust_svc Custom Service definitions
/// @{

/**************************************************************************************************
 Global variables
 **************************************************************************************************/
/// Indication timer id
static rbk_smp290_ble_tmr_tst cntrCharIndicnTmr;
/// Ble Indication counter
volatile uint16_t ble_indicnCntr = 0u;
/// Indication buffer
static uint8_t ble_indicnBuff[BLE_CUST_SVC_CCC_BUFF_SIZE] = {0};
/// TX power level
int8_t ble_txPwrLvl;
/// Buffer for TX power level
SECTION_PERSISTENT uint8_t tx_lvl = 1;

/**************************************************************************************************
  Custom service group
 **************************************************************************************************/
/**************************************************************************************************
  Custom service definition
 **************************************************************************************************/
static const uint8_t custSvc[]   = {BLE_CUST_SVC_SERVICE_UUID};
static const uint16_t custSvcLen = sizeof(custSvc);

/**************************************************************************************************
  TX Power Characteristic definitions
 **************************************************************************************************/
static const uint8_t txPwrCharUuid[] = {BLE_CUST_SVC_TXPWR_CHAR_UUID};
static const uint8_t txPwrCharVal[]  = {((uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PPTY_WRITE),
                                       RBK_SMP290_CONV_U16_TO_BYTES((uint16_t)BLE_CUST_SVC_TXPWR_CHAR_DATA_HNDL), BLE_CUST_SVC_TXPWR_CHAR_UUID};
static const uint16_t txPwrCharLen   = sizeof(txPwrCharVal);

/// TX power Characteristic Value
static uint8_t txPwrCharData[]   = {0};
static uint16_t txPwrCharDataLen = sizeof(txPwrCharData);

/// TX power Characteristic User Description Value
static const uint8_t txPwrCharUserDesc[]   = "Current TX power";
static const uint16_t txPwrCharUserDescLen = sizeof(txPwrCharUserDesc);
/**************************************************************************************************
  Counter value Characteristic definitions
 **************************************************************************************************/
static const uint8_t cntrCharUuid[] = {BLE_CUST_SVC_CNTR_CHAR_UUID};
static const uint8_t cntrCharVal[]  = {((uint8_t)RBK_SMP290_BLE_ATTS_PPTY_INDICATE | (uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ),
                                      RBK_SMP290_CONV_U16_TO_BYTES((uint16_t)BLE_CUST_SVC_CNTR_CHAR_DATA_HNDL), BLE_CUST_SVC_CNTR_CHAR_UUID};
static const uint16_t cntrCharLen   = sizeof(cntrCharVal);

/// Counter value Characteristic Value
static uint8_t cntrCharData[2]  = {0};
static uint16_t cntrCharDataLen = sizeof(cntrCharData);

/// Counter value Characteristic User Description Value
static const uint8_t cntrCharUserDesc[]   = "Counter value";
static const uint16_t cntrCharUserDescLen = sizeof(cntrCharUserDesc);

/// Counter value Characteristic Client Characteristic Configuration
static uint8_t cntrCharCccVal[]      = {0x00, 0x00};
static const uint16_t cntrCharCccLen = sizeof(cntrCharCccVal);
/**************************************************************************************************
  Termal Shut Down definitions
 **************************************************************************************************/
/// TSD characteristic declaration
static const uint8_t TSDCharUuid[] = {BLE_CUST_SVC_TSD_CHAR_UUID};
static const uint8_t TSDCharVal[]  = {((uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PPTY_WRITE),
                                           RBK_SMP290_CONV_U16_TO_BYTES((uint16_t)BLE_CUST_SVC_TSD_CHAR_DATA_HNDL), BLE_CUST_SVC_TSD_CHAR_UUID};
static const uint16_t TSDCharLen   = sizeof(TSDCharVal);

/// TSD characteristic value
static uint8_t TSDCharData[]   = {0};
static uint16_t TSDCharDataLen = sizeof(TSDCharData);

/// TSD characteristic user description value
static const uint8_t TSDCharUserDesc[]   = "TSD";
static const uint16_t TSDCharUserDescLen = sizeof(TSDCharUserDesc);

/**************************************************************************************************
  Custom service attributes list
 **************************************************************************************************/
static const rbk_smp290_ble_attsAttr_tst custSvcAttrGrp[] = {
    /// Add Primary service
    {
        rbk_smp290_ble_attsPrimSvcUuid,           // Primary service declaration UUID: 0x2800
        (uint8_t *)custSvc,                      // Primary service Attribute Value: Custom Service UUID: 02a63290-1a00-b83e-af18-025703723367
        (uint16_t *)&custSvcLen,                 // Primary service Attribute Value length
        sizeof(custSvc),                         // Primary service Attribute Value  maximum Length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Primary service Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Primary service Attribute permission
    },

    /// TX power Characteristic
    {
        rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
        (uint8_t *)txPwrCharVal,                 // Characteristic Attribute Value
        (uint16_t *)&txPwrCharLen,               // Characteristic Attribute Value length
        sizeof(txPwrCharVal),                    // Characteristic Attribute Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic Attribute permission
    },
    /// TX power Characteristic value declaration
    {
        txPwrCharUuid,                     // Characteristic UUID: 02a63290-1a01-b83e-af18-025703723367
        (uint8_t *)txPwrCharData,          // Characteristic value
        (uint16_t *)&txPwrCharDataLen,     // Characteristic value length
        RBK_SMP290_BLE_ATTS_VALUE_MAX_LEN,  // Characteristic value maximum Length
        ((uint8_t)RBK_SMP290_BLE_ATTS_SET_UUID_128 | (uint8_t)RBK_SMP290_BLE_ATTS_SET_VARIABLE_LEN | (uint8_t)RBK_SMP290_BLE_ATTS_SET_READ_CBACK |
         (uint8_t)RBK_SMP290_BLE_ATTS_SET_WRITE_CBACK),                                        // Characteristic value Attribute settings
        ((uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE)  // Characteristic value Attribute Permission
    },
    /// TX power Characteristic User Description
    {
        rbk_smp290_ble_attsChUserDescUuid,        // Characteristic User Description: 0x2901
        (uint8_t *)txPwrCharUserDesc,            // Characteristic User Description Value
        (uint16_t *)&txPwrCharUserDescLen,       // Characteristic User Description Value length
        sizeof(txPwrCharUserDesc),               // Characteristic User Description Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic User description Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic User description Attribute Permission
    },

    /// Counter Characteristic
    {
        rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
        (uint8_t *)cntrCharVal,                  // Characteristic Attribute Value
        (uint16_t *)&cntrCharLen,                // Characteristic Attribute Value length
        sizeof(cntrCharVal),                     // Characteristic Attribute Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic attribute permission
    },
    /// Counter Characteristic value declaration
    {
        cntrCharUuid,                      // Characteristic UUID: 02a63290-1a02-b83e-af18-025703723367
        (uint8_t *)cntrCharData,           // Characteristic value
        (uint16_t *)&cntrCharDataLen,      // Characteristic value length
        RBK_SMP290_BLE_ATTS_VALUE_MAX_LEN,  // Characteristic value maximum Length
        ((uint8_t)RBK_SMP290_BLE_ATTS_SET_UUID_128 | (uint8_t)RBK_SMP290_BLE_ATTS_SET_VARIABLE_LEN |
         (uint8_t)RBK_SMP290_BLE_ATTS_SET_READ_CBACK),                                         // Characteristic value Attribute settings
        ((uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE)  // Characteristic value Attribute Permission
    },
    /// Counter Characteristic User Description
    {
        rbk_smp290_ble_attsChUserDescUuid,        // Characteristic User Description: 0x2901
        (uint8_t *)cntrCharUserDesc,             // Characteristic User Description Value
        (uint16_t *)&cntrCharUserDescLen,        // Characteristic User Description Value length
        sizeof(cntrCharUserDesc),                // Characteristic User Description Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic User description Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic User description Attribute Permission
    },
    /// Counter Characteristic client characteristic configuration
    {
        rbk_smp290_ble_attsCliChCfgUuid,       // Client characteristic configuration UUID: 0x2902
        (uint8_t *)cntrCharCccVal,            // Client characteristic configuration Value
        (uint16_t *)&cntrCharCccLen,          // Client characteristic configuration Value length
        sizeof(cntrCharCccVal),               // Client characteristic configuration Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_CCC,  // Client characteristic configuration Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ |
            (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE  // Client characteristic configuration Attribute permission
    },
    /// Thermal Shut Down (TSD) Characteristic
	{
	   rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
	   (uint8_t *)TSDCharVal,                  // Characteristic Attribute Value
	   (uint16_t *)&TSDCharLen,                // Characteristic Attribute Value length
	   sizeof(TSDCharVal),                     // Characteristic Attribute Value maximum length
	   (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic attribute settings
	   (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic attribute permission
	},
	/// Thermal Shut Down (TSD) Characteristic value declaration
	{
        TSDCharUuid,                      // Characteristic UUID: 02a63290-1a04-b83e-af18-025703723367
        (uint8_t *)TSDCharData,           // Characteristic value
        (uint16_t *)&TSDCharDataLen,      // Characteristic value length
		sizeof(TSDCharData),              // Characteristic value maximum Length
        ((uint8_t)RBK_SMP290_BLE_ATTS_SET_UUID_128 | (uint8_t)RBK_SMP290_BLE_ATTS_SET_READ_CBACK |
         (uint8_t)RBK_SMP290_BLE_ATTS_SET_WRITE_CBACK),                                        // Characteristic value Attribute settings
        ((uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE)  // Characteristic value Attribute permission
    },
	/// Thermal Shut Down (TSD) Characteristic User Description
    {
        rbk_smp290_ble_attsChUserDescUuid,        // Characteristic User Description: 0x2901
        (uint8_t *)TSDCharUserDesc,             // Characteristic User Description Value
        (uint16_t *)&TSDCharUserDescLen,        // Characteristic User Description Value length
        sizeof(TSDCharUserDesc),                // Characteristic User Description Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic User description Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic User description Attribute Permission
    }
};
/**************************************************************************************************
  Custom service Attribute Grouping
 **************************************************************************************************/
/// Custom service Attribute Group
rbk_smp290_ble_attsAttrGrp_tst custSvcGrp = {NULL,
                                             (rbk_smp290_ble_attsAttr_tst *)custSvcAttrGrp,
                                             custSvc_rdCallBack,
                                             custSvc_wrCallBack,
                                             (uint16_t)BLE_CUST_SVC_START_HNDL,
                                             (uint16_t)BLE_CUST_SVC_END_HNDL};

/// @}

/**************************************************************************************************
 Local static function declaration
 **************************************************************************************************/
// Send indication
static void send_cntrChar_indication(rbk_smp290_ble_tmrPrm Evt);
// Custom service indication timer call back
static void custSvc_indication_timer_callback(rbk_smp290_ble_tmrPrm prm);

/*******************************************************************************
 *  Function definition
 ******************************************************************************/
rbk_smp290_ble_atts_err_ten custSvc_rdCallBack(rbk_smp290_ble_attsConnId connId, rbk_smp290_ble_attsHndl handle, uint8_t Op, uint16_t offset,
                                              rbk_smp290_ble_attsAttr_tst *pAttr)
{
    (void)(connId);
    (void)(Op);
    (void)(offset);

    
    uint8_t pin;
    bool retTsd;
    uint8_t *data = pAttr->pAttValue;
    uint16_t *len = pAttr->pLen;

    switch (handle)
    {
        case BLE_CUST_SVC_TXPWR_CHAR_DATA_HNDL:
            
            // The configured power level shall be stored in the global variable
            ble_txPwrLvl = rbk_smp290_ble_radio_getTxPwr();
            // Store value in NVM
            if (tx_lvl != ble_txPwrLvl)
                {
                (void)writeTxPwrToNvm(ble_txPwrLvl);// Get the current TX Power Level
                tx_lvl = ble_txPwrLvl;
                }
            *data = (uint8_t)(ble_txPwrLvl);
            *len  = 1;
            break;
        case BLE_CUST_SVC_CNTR_CHAR_DATA_HNDL:
            // Get the current counter value
            ble_indicnCntr++;
            data[0] = ((ble_indicnCntr >> 8) & 0xFF);
            data[1] = (ble_indicnCntr & 0xFF);
            *len    = 2;
            break;
        case BLE_CUST_SVC_TSD_CHAR_DATA_HNDL:
            //Get TSD
            retTsd = rbk_smp290_tsd_isTsdEnabled();
            if (retTsd)
            {
                *data = 1;
            }
            else
            {
                *data = 0;
            }
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

rbk_smp290_ble_atts_err_ten custSvc_wrCallBack(rbk_smp290_ble_attsConnId connId, rbk_smp290_ble_attsHndl handle, uint8_t Op, uint16_t offset,
                                              uint16_t len, uint8_t *pValue, rbk_smp290_ble_attsAttr_tst *pAttr)
{
    (void)(connId);
    (void)(Op);
    (void)(offset);
    (void)(pAttr);
    (void)(len);

    switch (handle)
    {
        case BLE_CUST_SVC_TXPWR_CHAR_DATA_HNDL:
            // Configure the TX power level
        	 config_txPwr(*(int8_t*)pValue);
            break;
        case BLE_CUST_SVC_TSD_CHAR_DATA_HNDL:
            // Configure TSD
        	config_tsd(*pValue);
            break;
        default:
        {
            return RBK_SMP290_BLE_ATTS_ERR_HANDLE;
        }
        break;
    }
    return RBK_SMP290_BLE_ATTS_SUCCESS;
}

/**
 * @brief Sends a counter characteristic indication.
 * @details This function increments the indication counter, prepares the indication buffer,
 *          and sends the indication using the BLE stack. If the indication fails, the timer is disabled.
 * @param Evt The event that triggered the indication.
 */
static void send_cntrChar_indication(rbk_smp290_ble_tmrPrm Evt)
{
    rbk_smp290_ble_atts_err_ten ret;
    ble_indicnCntr++;
    ble_indicnBuff[0] = ((ble_indicnCntr >> 8) & 0xFF);
    ble_indicnBuff[1] = (ble_indicnCntr & 0xFF);
    smp290_log(LOG_VERBOSITY_INFO, "Indication value to be sent ble_indcnCntr=%d\r\n", ble_indicnCntr);
    ret = rbk_smp290_ble_atts_sendIndication((rbk_smp290_ble_attsHndl)Evt, BLE_CUST_SVC_CCC_BUFF_SIZE, ble_indicnBuff);
    if (ret != RBK_SMP290_BLE_ATTS_SUCCESS)
    {
    	 (void)rbk_smp290_ble_timer_disable(&cntrCharIndicnTmr);
    }
}

void custSvc_procCccEvt(rbk_smp290_ble_atts_CccVal_ten cccVal, rbk_smp290_ble_attsHndl hndl, uint8_t idx)
{
    (void)(hndl);
    (void)(idx);

    if (cccVal == RBK_SMP290_BLE_ATTS_CCC_VAL_DISAD)
    {
        // Stop indication timer
        (void)rbk_smp290_ble_timer_disable(&cntrCharIndicnTmr);
    }
    else if (cccVal == RBK_SMP290_BLE_ATTS_CCC_VAL_INDICN)
    {
        // Start indication timer
    	(void)rbk_smp290_ble_timer_enable_ms(&cntrCharIndicnTmr, BLE_CUST_SVC_BLE_TMR_INTERVAL);
    }
    else
    {
        // Do Nothing
    }
}

void custSvc_indication_confiramtion()
{
    //
	(void)rbk_smp290_ble_timer_enable_ms(&cntrCharIndicnTmr, BLE_CUST_SVC_BLE_TMR_INTERVAL);
}

/**
 * @brief Custom service indication timer callback.
 * @details This function is called when the custom service indication timer expires.
 *          It sends an indication if the event matches the counter characteristic handle.
 * @param evt The event that triggered the callback.
 * @param tmrMsg The timer message containing the event details.
 */
static void custSvc_indication_timer_callback(rbk_smp290_ble_tmrPrm prm)
{
    if (prm == (rbk_smp290_ble_tmrPrm)BLE_CUST_SVC_CNTR_CHAR_DATA_HNDL)
    {
        // send indication once the timer expires
        send_cntrChar_indication(prm);
    }
}
void addCustSvc()
{
    (void)rbk_smp290_ble_atts_addAttrGrp((rbk_smp290_ble_attsAttrGrp_tst *)&custSvcGrp);
    // Periodic notification timer
    cntrCharIndicnTmr.prm  = (rbk_smp290_ble_tmrPrm)BLE_CUST_SVC_CNTR_CHAR_DATA_HNDL;
    (void)rbk_smp290_ble_timer_create(&cntrCharIndicnTmr, custSvc_indication_timer_callback);
    //Set TSD default state
    config_tsd(TSD_DEFAULT_STATUS);
}
void rmCustSvc()
{
    //
    (void)rbk_smp290_ble_atts_rmvAttrGrp((uint16_t)BLE_CUST_SVC_START_HNDL);
}

void config_txPwr(int8_t txPwr)
{
    smp290_log(LOG_VERBOSITY_INFO, "TX Power :Received %d \r\n", txPwr);

    if (ble_txPwrLvl != txPwr)
    {
        // Set the RF out power
        (void)rbk_smp290_ble_radio_setTxPwr(txPwr);
        
    }
    smp290_log(LOG_VERBOSITY_INFO, "TX Power :Configured :%d \r\n", ble_txPwrLvl);
}

rbk_smp290_nvm_err_ten writeTxPwrToNvm(int8_t txPwr)
{
    rbk_smp290_nvm_err_ten ret;
    // Create a NVM word sized buffer
    ble_TxPwrNvm_tst buf;

    // Set all bytes to 0
    memset((void *)&buf, 0, sizeof(ble_TxPwrNvm_tst));

    buf.TxPwrNvm     = txPwr;
    buf.TxPwrNvmStrd = true;

    ret = rbk_smp290_nvm_writeWithErase((void *)TXPWR_NVM_ADR, (const void *)&buf, 1u);

    return ret;
}

int8_t readTxPwrFromNvm()
{
    const ble_TxPwrNvm_tst *txPwrNvmStruct = (ble_TxPwrNvm_tst *)TXPWR_NVM_ADR;

    // Initialize with the default value
    int8_t ret = RBK_SMP290_BLE_TX_PWR_6_DBM;

    if (1u == txPwrNvmStruct->TxPwrNvmStrd)
    {
        ret = txPwrNvmStruct->TxPwrNvm;
        smp290_log(LOG_VERBOSITY_INFO, "Read TxPwr from NVM: %d\r\n", ret);
    }
    else
    {
        smp290_log(LOG_VERBOSITY_INFO, "No TxPwr in NVM. Using default:%d\r\n", ret);
    }
    return ret;
}

void config_tsd(uint8_t value)
{
    // Configure TSD
    if (value == 1)
    {
        
    	rbk_smp290_tsd_enable();
        smp290_log(LOG_VERBOSITY_INFO, "TSD enabled");
        
    }
    if (value == 0)
    {
        
    	rbk_smp290_tsd_disable();
        smp290_log(LOG_VERBOSITY_INFO, "TSD disabled");
        
    }
    else
    {
        // otherwise ignore
        smp290_log(LOG_VERBOSITY_WARNING, "TSD ignored.\r\n");
    }
}
/** @} */
