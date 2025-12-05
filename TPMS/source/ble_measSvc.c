/**
 * @addtogroup  measure_advertise_conn
 * @{
 * @file         ble_measSvc.c
 * @brief        Ble custom service implementation example
 * @details      This file contains the implementation of a custom BLE service for the measure_advertise_conn example.
 *               The custom service includes characteristics for triggering measurements.
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
#include "rbk_smp290_snsr.h"
#include "rbk_smp290_cfgmgr.h"

/* Project includes */
#include "ble_measSvc.h"
#include "main.h"

/// @defgroup measure_advertise_conn_cust_svc Custom Service definitions
/// @{

/**************************************************************************************************
 Global variables
 **************************************************************************************************/

/// Indication timer id
static rbk_smp290_ble_tmr_tst TP_SlftstCharIndicnTmr;
/// Indication buffer
static uint8_t ble_indicnSnsrBuff[BLE_CUST_SVC_MEAS_CCC_BUFF_SIZE] = {0};
/// Indication buffer
static uint8_t ble_indicnSnsrBuffDouble[BLE_CUST_SVC_MEAS_CCC_BUFF_SIZE_DOUBLE] = {0};
/// Snsr buffer
SECTION_PERSISTENT static uint16_t Snsr_buffer;
/// Sensor measurement done
SECTION_PERSISTENT bool meas_done = true;
/// Buffer for all mearuement results
SECTION_PERSISTENT uint16_t meas_res[4] = {0};

/**************************************************************************************************
  Custom service group
 **************************************************************************************************/
/**************************************************************************************************
  Custom service definition
 **************************************************************************************************/
static const uint8_t custmeasSvc[]   = {BLE_CUST_SVC_MEAS_SERVICE_UUID};
static const uint16_t custmeasSvcLen = sizeof(custmeasSvc);

/**************************************************************************************************
  Temperature definitions
 **************************************************************************************************/
/// Temperature characteristic declaration
static const uint8_t TCharUuid[] = {BLE_CUST_SVC_MEAS_T_CHAR_UUID};
static const uint8_t TCharVal[]  = {(uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ,
                                           RBK_SMP290_CONV_U16_TO_BYTES((uint16_t)BLE_CUST_SVC_MEAS_T_CHAR_DATA_HNDL), BLE_CUST_SVC_MEAS_T_CHAR_UUID};
static const uint16_t TCharLen   = sizeof(TCharVal);

/// Temperature characteristic value
static uint8_t TCharData[]   = {0, 0};
static uint16_t TCharDataLen = sizeof(TCharData);

/// Temperature characteristic user description value
static const uint8_t TCharUserDesc[]   = "T";
static const uint16_t TCharUserDescLen = sizeof(TCharUserDesc);

/// Temperature Characteristic Client Characteristic Configuration
static uint8_t TCharCccVal[]      = {0x00, 0x00, 0x00};
static const uint16_t TCharCccLen = sizeof(TCharCccVal);

/**************************************************************************************************
  TPAZ definitions
 **************************************************************************************************/
/// TPAZ characteristic declaration
static const uint8_t TPAZCharUuid[] = {BLE_CUST_SVC_MEAS_TPAZ_CHAR_UUID};
static const uint8_t TPAZCharVal[]  = {(uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ,
                                           RBK_SMP290_CONV_U16_TO_BYTES((uint16_t)BLE_CUST_SVC_MEAS_TPAZ_CHAR_DATA_HNDL), BLE_CUST_SVC_MEAS_TPAZ_CHAR_UUID};
static const uint16_t TPAZCharLen   = sizeof(TPAZCharVal);

/// TPAZ characteristic value
static uint8_t TPAZCharData[]   = {0, 0, 0, 0, 0, 0};
static uint16_t TPAZCharDataLen = sizeof(TPAZCharData);

/// TPAZ characteristic user description value
static const uint8_t TPAZCharUserDesc[]   = "TPAZ";
static const uint16_t TPAZCharUserDescLen = sizeof(TPAZCharUserDesc);

/// TPAZ Characteristic Client Characteristic Configuration
static uint8_t TPAZCharCccVal[]      = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint16_t TPAZCharCccLen = sizeof(TPAZCharCccVal);

/**************************************************************************************************
  TAZAX definitions
 **************************************************************************************************/
/// TAZAX characteristic declaration
static const uint8_t TAZAXCharUuid[] = {BLE_CUST_SVC_MEAS_TAZAX_CHAR_UUID};
static const uint8_t TAZAXCharVal[]  = {(uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ,
                                           RBK_SMP290_CONV_U16_TO_BYTES((uint16_t)BLE_CUST_SVC_MEAS_TAZAX_CHAR_DATA_HNDL), BLE_CUST_SVC_MEAS_TAZAX_CHAR_UUID};
static const uint16_t TAZAXCharLen   = sizeof(TAZAXCharVal);

/// TAZAX characteristic value
static uint8_t TAZAXCharData[]   = {0, 0, 0, 0, 0, 0};
static uint16_t TAZAXCharDataLen = sizeof(TAZAXCharData);

/// TAZAX characteristic user description value
static const uint8_t TAZAXCharUserDesc[]   = "TAZAX";
static const uint16_t TAZAXCharUserDescLen = sizeof(TAZAXCharUserDesc);

/// TAZAX Characteristic Client Characteristic Configuration
static uint8_t TAZAXCharCccVal[]      = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint16_t TAZAXCharCccLen = sizeof(TAZAXCharCccVal);

/**************************************************************************************************
  VBAT definitions
 **************************************************************************************************/
/// VBAT characteristic declaration
static const uint8_t VBATCharUuid[] = {BLE_CUST_SVC_MEAS_VBAT_CHAR_UUID};
static const uint8_t VBATCharVal[]  = {(uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ,
                                           RBK_SMP290_CONV_U16_TO_BYTES((uint16_t)BLE_CUST_SVC_MEAS_VBAT_CHAR_DATA_HNDL), BLE_CUST_SVC_MEAS_VBAT_CHAR_UUID};
static const uint16_t VBATCharLen   = sizeof(VBATCharVal);

/// VBAT characteristic value
static uint8_t VBATCharData[]   = {0, 0};
static uint16_t VBATCharDataLen = sizeof(VBATCharData);

/// VBAT characteristic user description value
static const uint8_t VBATCharUserDesc[]   = "VBAT";
static const uint16_t VBATCharUserDescLen = sizeof(VBATCharUserDesc);

/// VBAT Characteristic Client Characteristic Configuration
static uint8_t VBATCharCccVal[]      = {0x00, 0x00, 0x00};
static const uint16_t VBATCharCccLen = sizeof(VBATCharCccVal);

/**************************************************************************************************
  Custom service attributes list
 **************************************************************************************************/
static const rbk_smp290_ble_attsAttr_tst custmeasSvcAttrGrp[] = {
    /// Add Primary service
    {
        rbk_smp290_ble_attsPrimSvcUuid,           // Primary service declaration UUID: 0x2800
        (uint8_t *)custmeasSvc,                      // Primary service Attribute Value: Custom Service UUID: 3bsac86d-1d00-4876-8b9d-f5799cfa02ba
        (uint16_t *)&custmeasSvcLen,                 // Primary service Attribute Value length
        sizeof(custmeasSvc),                         // Primary service Attribute Value  maximum Length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Primary service Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Primary service Attribute permission
    },

    /// Temperature Characteristic
	{
	   rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
	   (uint8_t *)TCharVal,                  // Characteristic Attribute Value
	   (uint16_t *)&TCharLen,                // Characteristic Attribute Value length
	   sizeof(TCharVal),                     // Characteristic Attribute Value maximum length
	   (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic attribute settings
	   (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic attribute permission
	},
	/// Temperature Characteristic value declaration
	{
        TCharUuid,                      // Characteristic UUID
        (uint8_t *)TCharData,           // Characteristic value
        (uint16_t *)&TCharDataLen,      // Characteristic value length
		sizeof(TCharData),              // Characteristic value maximum Length
        ((uint8_t)RBK_SMP290_BLE_ATTS_SET_UUID_128 | (uint8_t)RBK_SMP290_BLE_ATTS_SET_READ_CBACK |
         (uint8_t)RBK_SMP290_BLE_ATTS_SET_WRITE_CBACK),                                         // Characteristic value Attribute settings
        ((uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE)  // Characteristic value Attribute permission
    },
	/// Temperature Characteristic User Description
    {
        rbk_smp290_ble_attsChUserDescUuid,        // Characteristic User Description: 0x2901
        (uint8_t *)TCharUserDesc,             // Characteristic User Description Value
        (uint16_t *)&TCharUserDescLen,        // Characteristic User Description Value length
        sizeof(TCharUserDesc),                // Characteristic User Description Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic User description Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic User description Attribute Permission
    },
    /// Temperature Characteristic client characteristic configuration
    {
        rbk_smp290_ble_attsCliChCfgUuid,       // Client characteristic configuration UUID: 0x2902
        (uint8_t *)TCharCccVal,            // Client characteristic configuration Value
        (uint16_t *)&TCharCccLen,          // Client characteristic configuration Value length
        sizeof(TCharCccVal),               // Client characteristic configuration Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_CCC,  // Client characteristic configuration Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ |
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE  // Client characteristic configuration Attribute permission
    },
    /// TPAZ Characteristic
	{
	   rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
	   (uint8_t *)TPAZCharVal,                  // Characteristic Attribute Value
	   (uint16_t *)&TPAZCharLen,                // Characteristic Attribute Value length
	   sizeof(TPAZCharVal),                     // Characteristic Attribute Value maximum length
	   (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic attribute settings
	   (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic attribute permission
	},
	/// TPAZ Characteristic value declaration
	{
        TPAZCharUuid,                      // Characteristic UUID
        (uint8_t *)TPAZCharData,           // Characteristic value
        (uint16_t *)&TPAZCharDataLen,      // Characteristic value length
		sizeof(TPAZCharData),              // Characteristic value maximum Length
        ((uint8_t)RBK_SMP290_BLE_ATTS_SET_UUID_128 | (uint8_t)RBK_SMP290_BLE_ATTS_SET_READ_CBACK |
         (uint8_t)RBK_SMP290_BLE_ATTS_SET_WRITE_CBACK),                                         // Characteristic value Attribute settings
        ((uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE)  // Characteristic value Attribute permission
    },
	/// TPAZ Characteristic User Description
    {
        rbk_smp290_ble_attsChUserDescUuid,        // Characteristic User Description: 0x2901
        (uint8_t *)TPAZCharUserDesc,             // Characteristic User Description Value
        (uint16_t *)&TPAZCharUserDescLen,        // Characteristic User Description Value length
        sizeof(TPAZCharUserDesc),                // Characteristic User Description Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic User description Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic User description Attribute Permission
    },
    /// TPAZ Characteristic client characteristic configuration
    {
        rbk_smp290_ble_attsCliChCfgUuid,       // Client characteristic configuration UUID: 0x2902
        (uint8_t *)TPAZCharCccVal,            // Client characteristic configuration Value
        (uint16_t *)&TPAZCharCccLen,          // Client characteristic configuration Value length
        sizeof(TPAZCharCccVal),               // Client characteristic configuration Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_CCC,  // Client characteristic configuration Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ |
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE  // Client characteristic configuration Attribute permission
    },
    /// TAZAX Characteristic
	{
	   rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
	   (uint8_t *)TAZAXCharVal,                  // Characteristic Attribute Value
	   (uint16_t *)&TAZAXCharLen,                // Characteristic Attribute Value length
	   sizeof(TAZAXCharVal),                     // Characteristic Attribute Value maximum length
	   (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic attribute settings
	   (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic attribute permission
	},
	/// TAZAX Characteristic value declaration
	{
        TAZAXCharUuid,                      // Characteristic UUID
        (uint8_t *)TAZAXCharData,           // Characteristic value
        (uint16_t *)&TAZAXCharDataLen,      // Characteristic value length
		sizeof(TAZAXCharData),              // Characteristic value maximum Length
        ((uint8_t)RBK_SMP290_BLE_ATTS_SET_UUID_128 | (uint8_t)RBK_SMP290_BLE_ATTS_SET_READ_CBACK |
         (uint8_t)RBK_SMP290_BLE_ATTS_SET_WRITE_CBACK),                                         // Characteristic value Attribute settings
        ((uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE)  // Characteristic value Attribute permission
    },
	/// TAZAX Characteristic User Description
    {
        rbk_smp290_ble_attsChUserDescUuid,        // Characteristic User Description: 0x2901
        (uint8_t *)TAZAXCharUserDesc,             // Characteristic User Description Value
        (uint16_t *)&TAZAXCharUserDescLen,        // Characteristic User Description Value length
        sizeof(TAZAXCharUserDesc),                // Characteristic User Description Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic User description Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic User description Attribute Permission
    },
    /// TAZAX Characteristic client characteristic configuration
    {
        rbk_smp290_ble_attsCliChCfgUuid,       // Client characteristic configuration UUID: 0x2902
        (uint8_t *)TAZAXCharCccVal,            // Client characteristic configuration Value
        (uint16_t *)&TAZAXCharCccLen,          // Client characteristic configuration Value length
        sizeof(TAZAXCharCccVal),               // Client characteristic configuration Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_CCC,  // Client characteristic configuration Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ |
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE  // Client characteristic configuration Attribute permission
    },
    /// VBAT Characteristic
	{
	   rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
	   (uint8_t *)VBATCharVal,                  // Characteristic Attribute Value
	   (uint16_t *)&VBATCharLen,                // Characteristic Attribute Value length
	   sizeof(VBATCharVal),                     // Characteristic Attribute Value maximum length
	   (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic attribute settings
	   (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic attribute permission
	},
	/// VBAT Characteristic value declaration
	{
        VBATCharUuid,                      // Characteristic UUID
        (uint8_t *)VBATCharData,           // Characteristic value
        (uint16_t *)&VBATCharDataLen,      // Characteristic value length
		sizeof(VBATCharData),              // Characteristic value maximum Length
        ((uint8_t)RBK_SMP290_BLE_ATTS_SET_UUID_128 | (uint8_t)RBK_SMP290_BLE_ATTS_SET_READ_CBACK |
         (uint8_t)RBK_SMP290_BLE_ATTS_SET_WRITE_CBACK),                                         // Characteristic value Attribute settings
        ((uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE)  // Characteristic value Attribute permission
    },
	/// VBAT Characteristic User Description
    {
        rbk_smp290_ble_attsChUserDescUuid,        // Characteristic User Description: 0x2901
        (uint8_t *)VBATCharUserDesc,             // Characteristic User Description Value
        (uint16_t *)&VBATCharUserDescLen,        // Characteristic User Description Value length
        sizeof(VBATCharUserDesc),                // Characteristic User Description Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic User description Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic User description Attribute Permission
    },
    /// VBAT Characteristic client characteristic configuration
    {
        rbk_smp290_ble_attsCliChCfgUuid,       // Client characteristic configuration UUID: 0x2902
        (uint8_t *)VBATCharCccVal,            // Client characteristic configuration Value
        (uint16_t *)&VBATCharCccLen,          // Client characteristic configuration Value length
        sizeof(VBATCharCccVal),               // Client characteristic configuration Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_CCC,  // Client characteristic configuration Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ |
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE  // Client characteristic configuration Attribute permission
    }
};
/**************************************************************************************************
  Custom service Attribute Grouping
 **************************************************************************************************/
/// Custom service Attribute Group
rbk_smp290_ble_attsAttrGrp_tst custmeasSvcGrp = {NULL,
                                             (rbk_smp290_ble_attsAttr_tst *)custmeasSvcAttrGrp,
                                             custmeasSvc_rdCallBack,
                                             custmeasSvc_wrCallBack,
                                             (uint16_t)BLE_CUST_SVC_MEAS_START_HNDL,
                                             (uint16_t)BLE_CUST_SVC_MEAS_END_HNDL};

/// @}

/**************************************************************************************************
 Local static function declaration
 **************************************************************************************************/
// Send indication
static void send_TChar_indication(rbk_smp290_ble_tmrPrm Evt, rbk_smp290_snsr_err_ten status);
// Send indication
static void send_TPAZChar_indication(rbk_smp290_ble_tmrPrm Evt, rbk_smp290_snsr_err_ten status);
// Send indication
static void send_TAZAXChar_indication(rbk_smp290_ble_tmrPrm Evt, rbk_smp290_snsr_err_ten status);
// Send indication
static void send_VBATChar_indication(rbk_smp290_ble_tmrPrm Evt, rbk_smp290_snsr_err_ten status);
// Custom service indication timer call back
static void custmeasSvc_indication_timer_callback(rbk_smp290_ble_tmrPrm prm, rbk_smp290_snsr_err_ten status);

/// Buffer for Battery voltage reading
static rbk_smp290_snsr_Vbat_buff_tst vbat_meas_buff;
#define SEQ_VBAT_NREP_MEAS 1u                            //!< Measurement Vbat number of samples
#define SEQ_VBAT_TREP_MEAS 0u                            //!< Measurement Vbat sample rate
/*******************************************************************************
 *  Function definition
 ******************************************************************************/
rbk_smp290_ble_atts_err_ten custmeasSvc_rdCallBack(rbk_smp290_ble_attsConnId connId, rbk_smp290_ble_attsHndl handle, uint8_t Op, uint16_t offset,
                                              rbk_smp290_ble_attsAttr_tst *pAttr)
{
    (void)(connId);
    (void)(Op);
    (void)(offset);

    
    uint8_t *data = pAttr->pAttValue;
    uint16_t *len = pAttr->pLen;

    switch (handle)
    {
        
        case BLE_CUST_SVC_MEAS_T_CHAR_DATA_HNDL:
            //Get T

            data[0] = ((meas_res[0] >> 8) & 0xFF);
            data[1] = (meas_res[0] & 0xFF);

        	*len  = 2;
        	break;
        case BLE_CUST_SVC_MEAS_TPAZ_CHAR_DATA_HNDL:
            //Get TPAZ

            data[0] = ((meas_res[0] >> 8) & 0xFF);
            data[1] = (meas_res[0] & 0xFF);

            data[2] = ((meas_res[1] >> 8) & 0xFF);
            data[3] = (meas_res[1] & 0xFF);

            data[4] = ((meas_res[2] >> 8) & 0xFF);
            data[5] = (meas_res[2] & 0xFF);

        	*len  = 6;
        	break;
        case BLE_CUST_SVC_MEAS_TAZAX_CHAR_DATA_HNDL:
            //Get TAZAX   

            data[0] = ((meas_res[0] >> 8) & 0xFF);
            data[1] = (meas_res[0] & 0xFF);

            data[2] = ((meas_res[2] >> 8) & 0xFF);
            data[3] = (meas_res[2] & 0xFF);

            data[4] = ((meas_res[3] >> 8) & 0xFF);
            data[5] = (meas_res[3] & 0xFF);

        	*len  = 6;
        	break;
        case BLE_CUST_SVC_MEAS_VBAT_CHAR_DATA_HNDL:
            //Get VBAT

            data[0] = ((vbat_meas_buff.Vbat[0] >> 8) & 0xFF);
            data[1] = (vbat_meas_buff.Vbat[0] & 0xFF);

        	*len  = 2;
        	break;
        default:
        {
            return RBK_SMP290_BLE_ATTS_ERR_HANDLE;
        }
        break;
    }
    return RBK_SMP290_BLE_ATTS_SUCCESS;
}

rbk_smp290_ble_atts_err_ten custmeasSvc_wrCallBack(rbk_smp290_ble_attsConnId connId, rbk_smp290_ble_attsHndl handle, uint8_t Op, uint16_t offset,
                                              uint16_t len, uint8_t *pValue, rbk_smp290_ble_attsAttr_tst *pAttr)
{
    (void)(connId);
    (void)(Op);
    (void)(offset);
    (void)(pAttr);
    (void)(len);

    uint8_t status = 0x00;

    switch (handle)
    {
        
        case BLE_CUST_SVC_MEAS_T_CHAR_DATA_HNDL:
            // Configure T
            if (meas_done)
            {
                meas_done = false;
                Snsr_buffer = (rbk_smp290_ble_tmrPrm)BLE_CUST_SVC_MEAS_T_CHAR_DATA_HNDL;
                status = rbk_smp290_snsr_meas_cmpd_T();
            }
            break;
        case BLE_CUST_SVC_MEAS_TPAZ_CHAR_DATA_HNDL:
            // Configure TPAZ
            if (meas_done)
            {
                meas_done = false;
                Snsr_buffer = (rbk_smp290_ble_tmrPrm)BLE_CUST_SVC_MEAS_TPAZ_CHAR_DATA_HNDL;
                status = rbk_smp290_snsr_meas_cmpd_p(RBK_SMP290_SNSR_EN_ENABLE, RBK_SMP290_SNSR_EN_ENABLE);
            }
            break;
        case BLE_CUST_SVC_MEAS_TAZAX_CHAR_DATA_HNDL:
            // Configure TAZAX
            if (meas_done)
            {
                meas_done = false;
                Snsr_buffer = (rbk_smp290_ble_tmrPrm)BLE_CUST_SVC_MEAS_TAZAX_CHAR_DATA_HNDL;
                status = rbk_smp290_snsr_meas_cmpd_az_ax(RBK_SMP290_SNSR_EN_ENABLE, RBK_SMP290_SNSR_RANGE_HI, RBK_SMP290_SNSR_RANGE_HI);
            }
            break;
        case BLE_CUST_SVC_MEAS_VBAT_CHAR_DATA_HNDL:
            // Configure VBAT
            if (meas_done)
            {
                meas_done = false;
                Snsr_buffer = (rbk_smp290_ble_tmrPrm)BLE_CUST_SVC_MEAS_VBAT_CHAR_DATA_HNDL;
                static const rbk_smp290_snsr_cfg_Vbat_tst vbat_meas_cfg = {.N_rep     = (uint8_t)SEQ_VBAT_NREP_MEAS,
                                                                  .t_rep     = SEQ_VBAT_TREP_MEAS,
                                                                  .osr       = RBK_SMP290_SNSR_OSR_4X,
                                                                  .Vbat_load = RBK_SMP290_SNSR_VBAT_LOAD_DISABLE};
                status                                          = rbk_smp290_snsr_meas_and_get_Vbat(&vbat_meas_cfg, &vbat_meas_buff);
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
static void send_TChar_indication(rbk_smp290_ble_tmrPrm Evt, rbk_smp290_snsr_err_ten status)
{
    rbk_smp290_ble_atts_err_ten ret;
    
    ble_indicnSnsrBuff[0] = status;

    meas_res[0] = rbk_smp290_snsr_get_cmpd_T();
    
    ble_indicnSnsrBuff[1] = ((meas_res[0] >> 8) & 0xFF);
    ble_indicnSnsrBuff[2] = (meas_res[0] & 0xFF);

    ret = rbk_smp290_ble_atts_sendIndication((rbk_smp290_ble_attsHndl)Evt, BLE_CUST_SVC_MEAS_CCC_BUFF_SIZE, ble_indicnSnsrBuff);
    if (ret != RBK_SMP290_BLE_ATTS_SUCCESS)
    {
    	smp290_log(LOG_VERBOSITY_WARNING, "T meas indication failed.\r\n");
    }
    meas_done = true;
}

/**
 * @brief Sends a counter characteristic indication.
 * @details This function increments the indication counter, prepares the indication buffer,
 *          and sends the indication using the BLE stack. If the indication fails, the timer is disabled.
 * @param Evt The event that triggered the indication.
 */
static void send_TPAZChar_indication(rbk_smp290_ble_tmrPrm Evt, rbk_smp290_snsr_err_ten status)
{
    rbk_smp290_ble_atts_err_ten ret;
    
    ble_indicnSnsrBuffDouble[0] = status;

    meas_res[0] = rbk_smp290_snsr_get_cmpd_T();
    ble_indicnSnsrBuffDouble[1] = ((meas_res[0] >> 8) & 0xFF);
    ble_indicnSnsrBuffDouble[2] = (meas_res[0] & 0xFF);

    meas_res[1] = rbk_smp290_snsr_get_cmpd_p();
    ble_indicnSnsrBuffDouble[3] = ((meas_res[1] >> 8) & 0xFF);
    ble_indicnSnsrBuffDouble[4] = (meas_res[1] & 0xFF);

    meas_res[2] = rbk_smp290_snsr_get_cmpd_az(RBK_SMP290_SNSR_RANGE_HI);
    ble_indicnSnsrBuffDouble[5] = ((meas_res[2] >> 8) & 0xFF);
    ble_indicnSnsrBuffDouble[6] = (meas_res[2] & 0xFF);

    ret = rbk_smp290_ble_atts_sendIndication((rbk_smp290_ble_attsHndl)Evt, BLE_CUST_SVC_MEAS_CCC_BUFF_SIZE_DOUBLE, ble_indicnSnsrBuffDouble);
    if (ret != RBK_SMP290_BLE_ATTS_SUCCESS)
    {
    	smp290_log(LOG_VERBOSITY_WARNING, "TPAZ meas indication failed.\r\n");
    }
    meas_done = true;
}

/**
 * @brief Sends a counter characteristic indication.
 * @details This function increments the indication counter, prepares the indication buffer,
 *          and sends the indication using the BLE stack. If the indication fails, the timer is disabled.
 * @param Evt The event that triggered the indication.
 */
static void send_TAZAXChar_indication(rbk_smp290_ble_tmrPrm Evt, rbk_smp290_snsr_err_ten status)
{
    rbk_smp290_ble_atts_err_ten ret;
    
    ble_indicnSnsrBuffDouble[0] = status;

    meas_res[0] = rbk_smp290_snsr_get_cmpd_T();
    ble_indicnSnsrBuffDouble[1] = ((meas_res[0] >> 8) & 0xFF);
    ble_indicnSnsrBuffDouble[2] = (meas_res[0] & 0xFF);

    meas_res[2] = rbk_smp290_snsr_get_cmpd_az(RBK_SMP290_SNSR_RANGE_HI);
    ble_indicnSnsrBuffDouble[3] = ((meas_res[2] >> 8) & 0xFF);
    ble_indicnSnsrBuffDouble[4] = (meas_res[2] & 0xFF);

    meas_res[3] = rbk_smp290_snsr_get_cmpd_ax(RBK_SMP290_SNSR_RANGE_HI);
    ble_indicnSnsrBuffDouble[5] = ((meas_res[3] >> 8) & 0xFF);
    ble_indicnSnsrBuffDouble[6] = (meas_res[3] & 0xFF);
    ret = rbk_smp290_ble_atts_sendIndication((rbk_smp290_ble_attsHndl)Evt, BLE_CUST_SVC_MEAS_CCC_BUFF_SIZE_DOUBLE, ble_indicnSnsrBuffDouble);
    if (ret != RBK_SMP290_BLE_ATTS_SUCCESS)
    {
    	smp290_log(LOG_VERBOSITY_WARNING, "TAZAX meas indication failed.\r\n");
    }
    meas_done = true;
}

/**
 * @brief Sends a counter characteristic indication.
 * @details This function increments the indication counter, prepares the indication buffer,
 *          and sends the indication using the BLE stack. If the indication fails, the timer is disabled.
 * @param Evt The event that triggered the indication.
 */
static void send_VBATChar_indication(rbk_smp290_ble_tmrPrm Evt, rbk_smp290_snsr_err_ten status)
{
    rbk_smp290_ble_atts_err_ten ret;
    
    ble_indicnSnsrBuff[0] = status;
    ble_indicnSnsrBuff[1] = ((vbat_meas_buff.Vbat[0] >> 8) & 0xFF);
    ble_indicnSnsrBuff[2] = (vbat_meas_buff.Vbat[0] & 0xFF); 

    ret = rbk_smp290_ble_atts_sendIndication((rbk_smp290_ble_attsHndl)Evt, BLE_CUST_SVC_MEAS_CCC_BUFF_SIZE, ble_indicnSnsrBuff);
    if (ret != RBK_SMP290_BLE_ATTS_SUCCESS)
    {
    	smp290_log(LOG_VERBOSITY_WARNING, "T meas indication failed.\r\n");
    }
    meas_done = true;
}

//void custmeasSvc_procCccEvt(rbk_smp290_ble_atts_CccVal_ten cccVal, rbk_smp290_ble_attsHndl hndl, uint8_t idx)

//void custmeasSvc_indication_confiramtion()

/**
 * @brief Custom service indication timer callback.
 * @details This function is called when the custom service indication timer expires.
 *          It sends an indication if the event matches the counter characteristic handle.
 * @param evt The event that triggered the callback.
 * @param tmrMsg The timer message containing the event details.
 */
static void custmeasSvc_indication_timer_callback(rbk_smp290_ble_tmrPrm prm, rbk_smp290_snsr_err_ten status)
{
    if (prm == (rbk_smp290_ble_tmrPrm)BLE_CUST_SVC_MEAS_T_CHAR_DATA_HNDL)
    {
        // send indication once the timer expires
        send_TChar_indication(prm, status);
    }
    else if (prm == (rbk_smp290_ble_tmrPrm)BLE_CUST_SVC_MEAS_TPAZ_CHAR_DATA_HNDL)
    {
        // send indication once the timer expires
        send_TPAZChar_indication(prm, status);
    }
    else if (prm == (rbk_smp290_ble_tmrPrm)BLE_CUST_SVC_MEAS_TAZAX_CHAR_DATA_HNDL)
    {
        // send indication once the timer expires
        send_TAZAXChar_indication(prm, status);
    }
    else if (prm == (rbk_smp290_ble_tmrPrm)BLE_CUST_SVC_MEAS_VBAT_CHAR_DATA_HNDL)
    {
        // send indication once the timer expires
        send_VBATChar_indication(prm, status);
    }
}
void addcustmeasSvc()
{
    (void)rbk_smp290_ble_atts_addAttrGrp((rbk_smp290_ble_attsAttrGrp_tst *)&custmeasSvcGrp);
    // Periodic notification timer
    //cntrCharIndicnTmr.prm  = (rbk_smp290_ble_tmrPrm)BLE_CUST_SVC_CNTR_CHAR_DATA_HNDL;
    //(void)rbk_smp290_ble_timer_create(&cntrCharIndicnTmr, custmeasSvc_indication_timer_callback);
    //Set ver default state
    
}
void rmcustmeasSvc()
{
    //
    (void)rbk_smp290_ble_atts_rmvAttrGrp((uint16_t)BLE_CUST_SVC_START_HNDL);
}


/**
 * @brief Meas done callback
 * @param status self return success or error values
 */
void entry_ConnSnsrClbk( rbk_smp290_snsr_err_ten status)
{
    smp290_log(LOG_VERBOSITY_ERROR, "Measurement completed, SnsrErr = (0x%2X)\r\n", status);

    static rbk_smp290_snsr_err_ten snsr_status;
    rbk_smp290_cfgmgr_rtDataBkup();
    snsr_status = status;
    custmeasSvc_indication_timer_callback(Snsr_buffer, snsr_status);
}
/** @} */
