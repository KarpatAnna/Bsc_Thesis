/**
 * @addtogroup  measure_advertise_conn
 * @{
 * @file         gatt.c
 * @brief        BLE generic attribute profile initialization and handling
 *               callback events
 *
 * @details      This file contains the implementation of the BLE generic attribute profile initialization
 *               and handling callback events for the \ref measure_advertise_conn project.
 *               It includes the necessary system, library, and project-specific headers.
 *               It also defines client characteristics index and client characteristics configuration structure.
 *               The gatt_init function initializes the gatt profile by adding services and registering CCC.
 *               The rbk_smp290_ble_atts_attrEvtCbk function handles attribute events such as CCC state indication,
 *               handle value confirmation, MTU update indication, and DB hash calculation completion indication.
 * @copyright    (c) [Robert Bosch GmbH] [2024].
 *               All rights reserved, also regarding any disposal,
 *               exploitation, reproduction, editing, distribution, as well
 *               as in the event of applications for industrial property
 *               rights. The communication of its contents to others without
 *               express authorization is prohibited. Offenders will be held
 *               liable for the payment of damages. All rights reserved in
 *               the event of the grant of a patent, utility model or design.
 **/

/* Library includes */
#include "rbk_smp290_ble.h"
#include "rbk_smp290_ble_atts.h"
#include "rbk_smp290_gpio.h"
#include "rbk_smp290_types.h"

/* Project includes */
#include "ble_custSvc.h"
#include "ble_gapSvc.h"
#include "ble_gattSvc.h"
#include "ble_gpioSvc.h"
#include "ble_maintSvc.h"
#include "ble_measSvc.h"
#include "main.h"

/// @defgroup measure_advertise_conn_gatt BLE GATT definitions
/// @{

/// Client characteristics index
typedef enum
{
    BLE_GATT_SVC_SC_CCC_IDX,         //!< GATT service changed characteristics client configuration index
    BLE_CUST_SVC_CNTR_CHAR_CCC_IDX,  //!< Custom service characteristics client configuration index
    BLE_CUST_SVC_MAINT_TP_SLFTST_CHAR_CCC_IDX,
    BLE_CUST_SVC_MEAS_T_CHAR_CCC_IDX,
    BLE_CUST_SVC_MEAS_TPAZ_CHAR_CCC_IDX,
    BLE_CUST_SVC_MEAS_TAZAX_CHAR_CCC_IDX,
    BLE_CUST_SVC_MEAS_VBAT_CHAR_CCC_IDX,
    BLE_PROF_MAX_CCC_IDX
} rbk_smp290_prof_CccIdx_ten;

/// Client Characteristics Configuration Structure
rbk_smp290_ble_attsCccCfg_tst rbk_smp290_prof_CccCfg[BLE_PROF_MAX_CCC_IDX] = {
    {
        (rbk_smp290_ble_attsHndl)BLE_GATT_SVC_SC_CCC_HNDL,  // cccd handle
        (uint16_t)RBK_SMP290_BLE_ATTS_CCC_VAL_INDICN,      // cccCfg
        RBK_SMP290_BLE_ATTS_SEC_LEVEL_NONE                  // security level
    },
    {
        (rbk_smp290_ble_attsHndl)BLE_CUST_SVC_CNTR_CHAR_CCC_HNDL,  // cccd handle
        (uint16_t)RBK_SMP290_BLE_ATTS_CCC_VAL_INDICN,             // cccCfg
        RBK_SMP290_BLE_ATTS_SEC_LEVEL_NONE                         // security level
    },
    {
        (rbk_smp290_ble_attsHndl)BLE_CUST_SVC_MAINT_TP_SLFTST_CHAR_CCC_HNDL,  // cccd handle
        (uint16_t)RBK_SMP290_BLE_ATTS_CCC_VAL_INDICN,             // cccCfg
        RBK_SMP290_BLE_ATTS_SEC_LEVEL_NONE                         // security level
    },
    {
        (rbk_smp290_ble_attsHndl)BLE_CUST_SVC_MEAS_T_CHAR_CCC_HNDL,  // cccd handle
        (uint16_t)RBK_SMP290_BLE_ATTS_CCC_VAL_INDICN,             // cccCfg
        RBK_SMP290_BLE_ATTS_SEC_LEVEL_NONE                         // security level
    },
    {
        (rbk_smp290_ble_attsHndl)BLE_CUST_SVC_MEAS_TPAZ_CHAR_CCC_HNDL,  // cccd handle
        (uint16_t)RBK_SMP290_BLE_ATTS_CCC_VAL_INDICN,             // cccCfg
        RBK_SMP290_BLE_ATTS_SEC_LEVEL_NONE                         // security level
    },
    {
        (rbk_smp290_ble_attsHndl)BLE_CUST_SVC_MEAS_TAZAX_CHAR_CCC_HNDL,  // cccd handle
        (uint16_t)RBK_SMP290_BLE_ATTS_CCC_VAL_INDICN,             // cccCfg
        RBK_SMP290_BLE_ATTS_SEC_LEVEL_NONE                         // security level
    },
    {
        (rbk_smp290_ble_attsHndl)BLE_CUST_SVC_MEAS_VBAT_CHAR_CCC_HNDL,  // cccd handle
        (uint16_t)RBK_SMP290_BLE_ATTS_CCC_VAL_INDICN,             // cccCfg
        RBK_SMP290_BLE_ATTS_SEC_LEVEL_NONE                         // security level
    },
    
};
/// @}

/*******************************************************************************
 *  Function definition
 ******************************************************************************/

// Initialize the GATT profile.
void gatt_init()
{
    rbk_smp290_ble_atts_err_ten ret;

    // Initialize attribute server
    rbk_smp290_ble_atts_inin();

    // Add Generic Access Service
    addGapSvc();

    // Add Generic Attribute Service
    addGattSvc();

    // Add Custom Service
    addCustSvc();
    addCustgpioSvc();
    addCustmaintSvc();
    addcustmeasSvc();
    

    // Register CCC
    ret = rbk_smp290_ble_atts_addCccdAttr((uint8_t)BLE_PROF_MAX_CCC_IDX, (rbk_smp290_ble_attsCccCfg_tst *)rbk_smp290_prof_CccCfg);
    if (ret != RBK_SMP290_BLE_ATTS_SUCCESS)
    {
        smp290_log(LOG_VERBOSITY_ERROR, "\t\tGATT: Adding Client Characteristics Configuration failed\r\n");
    }

    rbk_smp290_ble_atts_calcDbHash();
}

// This function handles the attribute events and prints corresponding messages.
void rbk_smp290_ble_atts_attrEvtCbk(rbk_smp290_ble_atts_evt_ten attsEvt, void *pAttMsg)
{
    rbk_smp290_ble_attsEvt_tst const *msg_p;

    switch (attsEvt)
    {
        case RBK_SMP290_BLE_ATTS_CCC_STATE_IND:
        {
            rbk_smp290_ble_attsCccEvt_tst const *cccEvt = (rbk_smp290_ble_attsCccEvt_tst *)pAttMsg;
            if (cccEvt->idx == (uint8_t)BLE_CUST_SVC_CNTR_CHAR_CCC_IDX)
            {
                custSvc_procCccEvt(cccEvt->value, cccEvt->handle, cccEvt->idx);
            }
            
        }
        break;

        case RBK_SMP290_BLE_ATTS_MULT_VALUE_CNF:
        case RBK_SMP290_BLE_ATTS_HANDLE_VALUE_CNF:
        {
            msg_p = (rbk_smp290_ble_attsEvt_tst *)pAttMsg;
            //smp290_log(LOG_VERBOSITY_INFO, "\t\tGATT: Handle value confirmation received: status:%d\r\n", msg_p->status);
            (void)(msg_p);
            uint16_t ret = rbk_smp290_ble_atts_getCccdVal((uint8_t)BLE_CUST_SVC_CNTR_CHAR_CCC_IDX);
            //uint16_t ret1 = rbk_smp290_ble_atts_getCccdVal((uint8_t)BLE_CUST_SVC_MAINT_TP_SLFTST_CHAR_CCC_IDX);

            if (ret == (uint16_t)RBK_SMP290_BLE_ATTS_CCC_VAL_INDICN)
            {
                custSvc_indication_confiramtion();
                
            }

        }
        break;

        case RBK_SMP290_BLE_ATTS_MTU_UPDATE_IND:
        {
            msg_p = (rbk_smp290_ble_attsEvt_tst *)pAttMsg;

            ble_mtu_size = rbk_smp290_ble_atts_get_mtu();
            smp290_log(LOG_VERBOSITY_INFO, "\t\tGATT: MTU updated: value:%d\r\n", msg_p->mtu);
            (void)(msg_p);
        }
        break;

        case RBK_SMP290_BLE_ATTS_DB_HASH_CALC_CMPL_IND:
        {
            msg_p = (rbk_smp290_ble_attsEvt_tst *)pAttMsg;
            //smp290_log(LOG_VERBOSITY_INFO, "\t\tGATT: DB calculation completed status: %d\r\n", msg_p->status);
            (void)(msg_p);
        }
        break;

        default:
        {
            // Nothing to do
        }
        break;
    }
}
/** @} */
