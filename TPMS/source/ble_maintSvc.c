/**
 * @addtogroup  measure_advertise_conn
 * @{
 * @file         ble_maintSvc.c
 * @brief        Ble custom service implementation example
 * @details      This file contains the implementation of a custom BLE service for the measure_advertise_conn example.
 *               The custom service includes characteristics for maintenance.
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
#include "rbk_smp290_entry.h"
#include "rbk_smp290_cfgmgr.h"
#include "rbk_smp290_slftst.h"

/* Project includes */
#include "ble_maintSvc.h"
#include "main.h"

/// @defgroup measure_advertise_conn_cust_svc Custom Service definitions
/// @{

/**************************************************************************************************
 Global variables
 **************************************************************************************************/
/// Indication timer id
static rbk_smp290_ble_tmr_tst TP_SlftstCharIndicnTmr;
/// Indication buffer
static uint8_t ble_indicnSlftstBuff[BLE_CUST_SVC_MAINT_CCC_BUFF_SIZE] = {0};
/// Data Backup status
SECTION_PERSISTENT static int8_t data_bckup_status;
/// Selftest buffer
SECTION_PERSISTENT static uint16_t Selftest_buffer;
/// Selftest value
SECTION_PERSISTENT static uint8_t Selftest_Value = 0XFF;

/**************************************************************************************************
  Custom service group
 **************************************************************************************************/
/**************************************************************************************************
  Custom service definition
 **************************************************************************************************/
static const uint8_t custmaintSvc[]   = {BLE_CUST_SVC_MAINT_SERVICE_UUID};
static const uint16_t custmaintSvcLen = sizeof(custmaintSvc);

/**************************************************************************************************
  HW Version definitions
 **************************************************************************************************/
/// HW Version characteristic declaration
static const uint8_t HW_VerCharUuid[] = {BLE_CUST_SVC_MAINT_HW_VER_CHAR_UUID};
static const uint8_t HW_VerCharVal[]  = {(uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ,
                                           RBK_SMP290_CONV_U16_TO_BYTES((uint16_t)BLE_CUST_SVC_MAINT_HW_VER_CHAR_DATA_HNDL), BLE_CUST_SVC_MAINT_HW_VER_CHAR_UUID};
static const uint16_t HW_VerCharLen   = sizeof(HW_VerCharVal);

/// HW Version characteristic value
static uint8_t HW_VerCharData[32]   = {0};
static uint16_t HW_VerCharDataLen = sizeof(HW_VerCharData);

/// HW Version characteristic user description value
static const uint8_t HW_VerCharUserDesc[]   = "HW Version";
static const uint16_t HW_VerCharUserDescLen = sizeof(HW_VerCharUserDesc);
/**************************************************************************************************
  FW Version definitions
 **************************************************************************************************/
/// FW Version characteristic declaration
static const uint8_t FW_VerCharUuid[] = {BLE_CUST_SVC_MAINT_FW_VER_CHAR_UUID};
static const uint8_t FW_VerCharVal[]  = {(uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ,
                                           RBK_SMP290_CONV_U16_TO_BYTES((uint16_t)BLE_CUST_SVC_MAINT_FW_VER_CHAR_DATA_HNDL), BLE_CUST_SVC_MAINT_FW_VER_CHAR_UUID};
static const uint16_t FW_VerCharLen   = sizeof(FW_VerCharVal);

/// FW Version characteristic value
static uint8_t FW_VerCharData[32]   = {0};
static uint16_t FW_VerCharDataLen = sizeof(FW_VerCharData);

/// FW Version characteristic user description value
static const uint8_t FW_VerCharUserDesc[]   = "FW Version";
static const uint16_t FW_VerCharUserDescLen = sizeof(FW_VerCharUserDesc);
/**************************************************************************************************
  Data Backup definitions
 **************************************************************************************************/
/// Data Backup characteristic declaration
static const uint8_t Data_BckupCharUuid[] = {BLE_CUST_SVC_MAINT_DATA_BCKUP_CHAR_UUID};
static const uint8_t Data_BckupCharVal[]  = {((uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PPTY_WRITE),
                                       RBK_SMP290_CONV_U16_TO_BYTES((uint16_t)BLE_CUST_SVC_MAINT_DATA_BCKUP_CHAR_DATA_HNDL), BLE_CUST_SVC_MAINT_DATA_BCKUP_CHAR_UUID};
static const uint16_t Data_BckupCharLen   = sizeof(Data_BckupCharVal);

/// Data Backup characteristic value
static uint8_t Data_BckupCharData[]   = {0};
static uint16_t Data_BckupCharDataLen = sizeof(Data_BckupCharData);

/// Data Backup characteristic user description value
static const uint8_t Data_BckupCharUserDesc[]   = "Data Backup";
static const uint16_t Data_BckupCharUserDescLen = sizeof(Data_BckupCharUserDesc);
/**************************************************************************************************
  TP Selftest definitions
 **************************************************************************************************/
/// TP Selftest characteristic declaration
static const uint8_t TP_SlftstCharUuid[] = {BLE_CUST_SVC_MAINT_TP_SLFTST_CHAR_UUID};
static const uint8_t TP_SlftstCharVal[]  = {((uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PPTY_WRITE),
                                       RBK_SMP290_CONV_U16_TO_BYTES((uint16_t)BLE_CUST_SVC_MAINT_TP_SLFTST_CHAR_DATA_HNDL), BLE_CUST_SVC_MAINT_TP_SLFTST_CHAR_UUID};
static const uint16_t TP_SlftstCharLen   = sizeof(TP_SlftstCharVal);

/// TP Selftest characteristic value
static uint8_t TP_SlftstCharData[]   = {0};
static uint16_t TP_SlftstCharDataLen = sizeof(TP_SlftstCharData);

/// TP Selftest characteristic user description value
static const uint8_t TP_SlftstCharUserDesc[]   = "TP Selftest";
static const uint16_t TP_SlftstCharUserDescLen = sizeof(TP_SlftstCharUserDesc);

/// TP Selftest Characteristic Client Characteristic Configuration
static uint8_t TP_SlftstCharCccVal[]      = {0x00};
static const uint16_t TP_SlftstCharCccLen = sizeof(TP_SlftstCharCccVal);

/**************************************************************************************************
  Custom service attributes list
 **************************************************************************************************/
static const rbk_smp290_ble_attsAttr_tst custmaintSvcAttrGrp[] = {
    /// Add Primary service
    {
        rbk_smp290_ble_attsPrimSvcUuid,           // Primary service declaration UUID: 0x2800
        (uint8_t *)custmaintSvc,                      // Primary service Attribute Value: Custom Service UUID: 0fd4d14e-1c00-11f0-8de9-0242ac120002
        (uint16_t *)&custmaintSvcLen,                 // Primary service Attribute Value length
        sizeof(custmaintSvc),                         // Primary service Attribute Value  maximum Length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Primary service Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Primary service Attribute permission
    },
    /// HW Version Characteristic
	{
	   rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
	   (uint8_t *)HW_VerCharVal,                  // Characteristic Attribute Value
	   (uint16_t *)&HW_VerCharLen,                // Characteristic Attribute Value length
	   sizeof(HW_VerCharVal),                     // Characteristic Attribute Value maximum length
	   (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic attribute settings
	   (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic attribute permission
	},
	/// HW Version Characteristic value declaration
	{
        HW_VerCharUuid,                      // Characteristic UUID
        (uint8_t *)HW_VerCharData,           // Characteristic value
        (uint16_t *)&HW_VerCharDataLen,      // Characteristic value length
		sizeof(HW_VerCharData),              // Characteristic value maximum Length
        ((uint8_t)RBK_SMP290_BLE_ATTS_SET_UUID_128 | (uint8_t)RBK_SMP290_BLE_ATTS_SET_READ_CBACK), // Characteristic value Attribute settings
		((uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ)   // Characteristic value Attribute permission
    },
	/// HW Version Characteristic User Description
    {
        rbk_smp290_ble_attsChUserDescUuid,        // Characteristic User Description: 0x2901
        (uint8_t *)HW_VerCharUserDesc,             // Characteristic User Description Value
        (uint16_t *)&HW_VerCharUserDescLen,        // Characteristic User Description Value length
        sizeof(HW_VerCharUserDesc),                // Characteristic User Description Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic User description Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic User description Attribute Permission
    },
    /// FW Version Characteristic
	{
	   rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
	   (uint8_t *)FW_VerCharVal,                  // Characteristic Attribute Value
	   (uint16_t *)&FW_VerCharLen,                // Characteristic Attribute Value length
	   sizeof(FW_VerCharVal),                     // Characteristic Attribute Value maximum length
	   (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic attribute settings
	   (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic attribute permission
	},
	/// FW Version Characteristic value declaration
	{
        FW_VerCharUuid,                      // Characteristic UUID: 02a63290-1a04-b83e-af18-025703723367
        (uint8_t *)FW_VerCharData,           // Characteristic value
        (uint16_t *)&FW_VerCharDataLen,      // Characteristic value length
		sizeof(FW_VerCharData),              // Characteristic value maximum Length
        ((uint8_t)RBK_SMP290_BLE_ATTS_SET_UUID_128 | (uint8_t)RBK_SMP290_BLE_ATTS_SET_READ_CBACK), // Characteristic value Attribute settings
		((uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ)   // Characteristic value Attribute permission
    },
	/// FW Version Characteristic User Description
    {
        rbk_smp290_ble_attsChUserDescUuid,        // Characteristic User Description: 0x2901
        (uint8_t *)FW_VerCharUserDesc,             // Characteristic User Description Value
        (uint16_t *)&FW_VerCharUserDescLen,        // Characteristic User Description Value length
        sizeof(FW_VerCharUserDesc),                // Characteristic User Description Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic User description Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic User description Attribute Permission
    },
    /// Data Backup Characteristic
	{
	   rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
	   (uint8_t *)Data_BckupCharVal,                  // Characteristic Attribute Value
	   (uint16_t *)&Data_BckupCharLen,                // Characteristic Attribute Value length
	   sizeof(Data_BckupCharVal),                     // Characteristic Attribute Value maximum length
	   (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic attribute settings
	   (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic attribute permission
	},
	/// Data Backup Characteristic value declaration
	{
        Data_BckupCharUuid,                      // Characteristic UUID
        (uint8_t *)Data_BckupCharData,           // Characteristic value
        (uint16_t *)&Data_BckupCharDataLen,      // Characteristic value length
		sizeof(Data_BckupCharData),              // Characteristic value maximum Length
        ((uint8_t)RBK_SMP290_BLE_ATTS_SET_UUID_128 | (uint8_t)RBK_SMP290_BLE_ATTS_SET_READ_CBACK |
         (uint8_t)RBK_SMP290_BLE_ATTS_SET_WRITE_CBACK),                                        // Characteristic value Attribute settings
        ((uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE)    // Characteristic value Attribute permission
    },
	/// Data Backup Characteristic User Description
    {
        rbk_smp290_ble_attsChUserDescUuid,        // Characteristic User Description: 0x2901
        (uint8_t *)Data_BckupCharUserDesc,             // Characteristic User Description Value
        (uint16_t *)&Data_BckupCharUserDescLen,        // Characteristic User Description Value length
        sizeof(Data_BckupCharUserDesc),                // Characteristic User Description Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic User description Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic User description Attribute Permission
    },
    /// TP Selftest Characteristic
    {
        rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
        (uint8_t *)TP_SlftstCharVal,                  // Characteristic Attribute Value
        (uint16_t *)&TP_SlftstCharLen,                // Characteristic Attribute Value length
        sizeof(TP_SlftstCharVal),                     // Characteristic Attribute Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic attribute permission
    },
    /// TP Selftest Characteristic value declaration
    {
        TP_SlftstCharUuid,                      // Characteristic UUID
        (uint8_t *)TP_SlftstCharData,           // Characteristic value
        (uint16_t *)&TP_SlftstCharDataLen,      // Characteristic value length
        RBK_SMP290_BLE_ATTS_VALUE_MAX_LEN,  // Characteristic value maximum Length
        ((uint8_t)RBK_SMP290_BLE_ATTS_SET_UUID_128 | (uint8_t)RBK_SMP290_BLE_ATTS_SET_READ_CBACK |
         (uint8_t)RBK_SMP290_BLE_ATTS_SET_WRITE_CBACK),                                         // Characteristic value Attribute settings
        ((uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE)  // Characteristic value Attribute Permission
    },
    /// TP Selftest Characteristic User Description
    {
        rbk_smp290_ble_attsChUserDescUuid,        // Characteristic User Description: 0x2901
        (uint8_t *)TP_SlftstCharUserDesc,             // Characteristic User Description Value
        (uint16_t *)&TP_SlftstCharUserDescLen,        // Characteristic User Description Value length
        sizeof(TP_SlftstCharUserDesc),                // Characteristic User Description Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic User description Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic User description Attribute Permission
    },
    /// TP Selftest Characteristic client characteristic configuration
    {
        rbk_smp290_ble_attsCliChCfgUuid,       // Client characteristic configuration UUID: 0x2902
        (uint8_t *)TP_SlftstCharCccVal,            // Client characteristic configuration Value
        (uint16_t *)&TP_SlftstCharCccLen,          // Client characteristic configuration Value length
        sizeof(TP_SlftstCharCccVal),               // Client characteristic configuration Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_CCC,  // Client characteristic configuration Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ |
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE  // Client characteristic configuration Attribute permission
    }
};
/**************************************************************************************************
  Custom service Attribute Grouping
 **************************************************************************************************/
/// Custom service Attribute Group
rbk_smp290_ble_attsAttrGrp_tst custmaintSvcGrp = {NULL,
                                             (rbk_smp290_ble_attsAttr_tst *)custmaintSvcAttrGrp,
                                             custmaintSvc_rdCallBack,
                                             custmaintSvc_wrCallBack,
                                             (uint16_t)BLE_CUST_SVC_MAINT_START_HNDL,
                                             (uint16_t)BLE_CUST_SVC_MAINT_END_HNDL};

/// @}

/**************************************************************************************************
 Local static function declaration
 **************************************************************************************************/
// Send indication
static void send_TP_SlftstChar_indication(rbk_smp290_ble_tmrPrm Evt, rbk_smp290_slftst_err_ten status);
// Custom service indication timer call back
static void custmaintSvc_indication_timer_callback(rbk_smp290_ble_tmrPrm prm, rbk_smp290_slftst_err_ten status);

/*******************************************************************************
 *  Function definition
 ******************************************************************************/
rbk_smp290_ble_atts_err_ten custmaintSvc_rdCallBack(rbk_smp290_ble_attsConnId connId, rbk_smp290_ble_attsHndl handle, uint8_t Op, uint16_t offset,
                                              rbk_smp290_ble_attsAttr_tst *pAttr)
{
    (void)(connId);
    (void)(Op);
    (void)(offset);

    
    uint8_t status = 0x00;
    uint8_t *data = pAttr->pAttValue;
    uint16_t *len = pAttr->pLen;

    switch (handle)
    {
        
        case BLE_CUST_SVC_MAINT_HW_VER_CHAR_DATA_HNDL:
            //Get HW VER
            status =  rbk_smp290_entry_getHwVers(data, 32);
            
            smp290_log(LOG_VERBOSITY_ERROR, "Error reading HW version: 0x%02x!\r\n", status);

        	*len  =  32;
        	break;
        case BLE_CUST_SVC_MAINT_FW_VER_CHAR_DATA_HNDL:
            //Get FW VER
            
            status = rbk_smp290_entry_getFwVers(data, 32);
            
        	smp290_log(LOG_VERBOSITY_ERROR, "Error reading FW version: 0x%02x!\r\n", status);
			
        	*len  = 32;
        	break;
        case BLE_CUST_SVC_MAINT_DATA_BCKUP_CHAR_DATA_HNDL:
            //Get Data backup
            if (data_bckup_status < 0)
            {
                *data = 1;
            }
            else
            {
                *data = 0;
            }
        	*len  = 1;
        	break;
        case BLE_CUST_SVC_MAINT_TP_SLFTST_CHAR_DATA_HNDL:
            // Start PT selftest
            *data = Selftest_Value;
            *len = 1;
            break;
        default:
        {
            return RBK_SMP290_BLE_ATTS_ERR_HANDLE;
        }
        break;
    }
    return RBK_SMP290_BLE_ATTS_SUCCESS;
}

rbk_smp290_ble_atts_err_ten custmaintSvc_wrCallBack(rbk_smp290_ble_attsConnId connId, rbk_smp290_ble_attsHndl handle, uint8_t Op, uint16_t offset,
                                              uint16_t len, uint8_t *pValue, rbk_smp290_ble_attsAttr_tst *pAttr)
{
    (void)(connId);
    (void)(Op);
    (void)(offset);
    (void)(pAttr);
    (void)(len);

    switch (handle)
    {
        case BLE_CUST_SVC_MAINT_DATA_BCKUP_CHAR_DATA_HNDL:
            // Configure data backup
        	config_data_bckup(*pValue);
            break;
        case BLE_CUST_SVC_MAINT_TP_SLFTST_CHAR_DATA_HNDL:
            // Start TP selftest
            if (!rbk_smp290_slftst_isRunning()){
                Selftest_buffer = (rbk_smp290_ble_tmrPrm)BLE_CUST_SVC_MAINT_TP_SLFTST_CHAR_DATA_HNDL;
                rbk_smp290_slftst_T_p();
            }
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
static void send_TP_SlftstChar_indication(rbk_smp290_ble_tmrPrm Evt, rbk_smp290_slftst_err_ten status)
{
    rbk_smp290_ble_atts_err_ten ret;
    
    Selftest_Value = status;
    ble_indicnSlftstBuff[0] = status;
    ret = rbk_smp290_ble_atts_sendIndication((rbk_smp290_ble_attsHndl)Evt, BLE_CUST_SVC_MAINT_CCC_BUFF_SIZE, ble_indicnSlftstBuff);
    if (ret != RBK_SMP290_BLE_ATTS_SUCCESS)
    {
    	smp290_log(LOG_VERBOSITY_WARNING, "TP Selftest indication failed.\r\n");
    }
}

//void custmaintSvc_procCccEvt(rbk_smp290_ble_atts_CccVal_ten cccVal, rbk_smp290_ble_attsHndl hndl, uint8_t idx)


void custmaintSvc_indication_confiramtion()
{
    //
	(void)rbk_smp290_ble_timer_enable_ms(&TP_SlftstCharIndicnTmr, BLE_CUST_SVC_BLE_TMR_INTERVAL);
}


/**
 * @brief Custom service indication timer callback.
 * @details This function is called when the custom service indication timer expires.
 *          It sends an indication if the event matches the counter characteristic handle.
 * @param evt The event that triggered the callback.
 * @param tmrMsg The timer message containing the event details.
 */
static void custmaintSvc_indication_timer_callback(rbk_smp290_ble_tmrPrm prm, rbk_smp290_slftst_err_ten status)
{
    if (prm == (rbk_smp290_ble_tmrPrm)BLE_CUST_SVC_MAINT_TP_SLFTST_CHAR_DATA_HNDL)
    {
        // send indication once the timer expires
        send_TP_SlftstChar_indication(prm, status);
    }
}

void addCustmaintSvc()
{
    (void)rbk_smp290_ble_atts_addAttrGrp((rbk_smp290_ble_attsAttrGrp_tst *)&custmaintSvcGrp);
    // Periodic notification timer
    //TP_SlftstCharIndicnTmr.prm  = (rbk_smp290_ble_tmrPrm)BLE_CUST_SVC_MAINT_TP_SLFTST_CHAR_DATA_HNDL;
    //(void)rbk_smp290_ble_timer_create(&TP_SlftstCharIndicnTmr, custmaintSvc_indication_timer_callback);
    //Set default state
}
void rmCustmaintSvc()
{
    //
    (void)rbk_smp290_ble_atts_rmvAttrGrp((uint16_t)BLE_CUST_SVC_START_HNDL);
}


void config_data_bckup(uint8_t value)
{
    // Configure data backup
    data_bckup_status = rbk_smp290_cfgmgr_rtDataBkup();
}

/**
 * @brief Self-test done callback
 * @param status self return success or error values
 */
void entry_slftstClbk(rbk_smp290_slftst_err_ten status)
{
    smp290_log(LOG_VERBOSITY_ERROR, "Self-test completed, slfTstErr = (0x%2X)\r\n", status);

    static rbk_smp290_slftst_err_ten slftst_status;
    rbk_smp290_cfgmgr_rtDataBkup();
    slftst_status = status;
    custmaintSvc_indication_timer_callback(Selftest_buffer, slftst_status);
}
/** @} */
