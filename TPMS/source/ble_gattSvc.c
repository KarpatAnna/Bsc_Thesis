/**
 * @addtogroup  measure_advertise_conn
 * @{
 * @file         ble_gattSvc.c
 * @brief        Ble Generic Attribute service implementation example
 * @details      This file contains the implementation of the Ble Generic Attribute service.
 *               It defines the attributes and characteristics of the service, along with their values and permissions.
 *               The service includes the Service Changed characteristic, Client Supported Features characteristic,
 *               Database Hash characteristic, and Server Supported Features characteristic.
 *               It also provides utility macros for converting uint16_t to little endian byte stream.
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

/* Library Includes */
#include "rbk_smp290_ble_atts.h"
#include "rbk_smp290_types.h"

/* Project includes */
#include "ble_gattSvc.h"
#include "main.h"

/// @defgroup measure_advertise_conn_gatt_svc BLE GATT Service definitions
/// @{

/**************************************************************************************************
 * Macros
 **************************************************************************************************/

/// Convert uint16_t to little endian byte stream, incrementing two bytes.
#define BLE_CONV_U16_TO_BYTESTREAM(p, n) \
    {                                    \
        *(p)++ = (uint8_t)(n);           \
        *(p)++ = (uint8_t)((n) >> 8);    \
    }

/*******************************************************************************
 *  Global variables
 ******************************************************************************/

/**************************************************************************************************
 * Generic Attribute Service Group
 **************************************************************************************************/
/**************************************************************************************************
 * Generic Attribute Service definitions
 **************************************************************************************************/
/// Generic Attribute Service Attribute Value 16-bit Bluetooth UUID
static const uint8_t gattSvc[] = {RBK_SMP290_CONV_U16_TO_BYTES(BLE_GATT_SVC_SERVICE_UUID)};
/// Length of an Attribute Value
static const uint16_t gattSvcLen = sizeof(gattSvc);

/**************************************************************************************************
 * Generic Attribute service changed characteristic definitions
 **************************************************************************************************/
/// Service changed characteristic UUID definition
static const uint8_t gattSvcScChUuid[RBK_SMP290_BLE_ATTS_16_UUID_LEN] = {RBK_SMP290_CONV_U16_TO_BYTES(BLE_GATT_SVC_SERVICE_CHANGED_UUID)};

/// Service changed characteristic Attribute value
static const uint8_t gattSvcScCh[] = {
    (uint8_t)RBK_SMP290_BLE_ATTS_PPTY_INDICATE,                                   // Characteristic Properties
    RBK_SMP290_CONV_U16_TO_BYTES((rbk_smp290_ble_attsHndl)BLE_GATT_SVC_SC_HNDL),  // Characteristic Value Attribute Handle
    RBK_SMP290_CONV_U16_TO_BYTES(BLE_GATT_SVC_SERVICE_CHANGED_UUID)};            // Characteristic UUID

/// Length of service changed characteristic Attribute value
static const uint16_t gattSvcScChLen = sizeof(gattSvcScCh);

/// Service changed Characteristic Value
static uint8_t gattSvcScVal[] = {0x01, 0x00, 0xFF, 0xFF};

/// Length of service changed characteristic data
static const uint16_t gattSvcScValLen = sizeof(gattSvcScVal);

/// Service changed characteristics client characteristic configuration
static uint8_t gattSvcScCccVal[]      = {0x00, 0x00};
static const uint16_t gattSvcScCccLen = sizeof(gattSvcScCccVal);

/**************************************************************************************************
 * Generic Attribute Client supported features characteristic definitions
 **************************************************************************************************/
/// Client supported features characteristic UUID definition
static const uint8_t gattValCsfChUuid[RBK_SMP290_BLE_ATTS_16_UUID_LEN] = {RBK_SMP290_CONV_U16_TO_BYTES(BLE_GATT_SVC_CLIENT_SUPP_FEAT)};

/// Client supported features Attribute value
static const uint8_t gattValCsfCh[] = {
    (uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ | (uint8_t)RBK_SMP290_BLE_ATTS_PPTY_WRITE,  // Characteristic Properties
    RBK_SMP290_CONV_U16_TO_BYTES((rbk_smp290_ble_attsHndl)BLE_GATT_SVC_CSF_HDL),      // Characteristic Value Attribute Handle
    RBK_SMP290_CONV_U16_TO_BYTES(BLE_GATT_SVC_CLIENT_SUPP_FEAT)};                    // Characteristic UUID

/// Length of Client supported features Attribute value
static const uint16_t gattValCsfChLen = sizeof(gattSvcScCh);

/// Client supported features Value
static uint8_t gattCsfVal[] = {0};
/// Length of Client supported features characteristic data
static const uint16_t gattCsfValLen = sizeof(gattCsfVal);

/**************************************************************************************************
 * Generic Attribute database hash characteristic definitions
 **************************************************************************************************/

/// Database hash characteristic UUID definition - 0x0x2B2A
static const uint8_t gattSvcDbhChUuid[RBK_SMP290_BLE_ATTS_16_UUID_LEN] = {RBK_SMP290_CONV_U16_TO_BYTES(BLE_GATT_SVC_DATABASE_HASH_UUID)};

/// Database hash characteristic Attribute value
static const uint8_t gattSvcDbhCh[] = {
    (uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ,                                        // Characteristic Properties
    RBK_SMP290_CONV_U16_TO_BYTES((rbk_smp290_ble_attsHndl)BLE_GATT_SVC_DBH_HNDL),  // Characteristic Value Attribute Handle
    RBK_SMP290_CONV_U16_TO_BYTES(BLE_GATT_SVC_DATABASE_HASH_UUID)};               // Characteristic UUID

/// Length database hash characteristic Attribute value
static const uint16_t gattSvcDbhChLen = sizeof(gattSvcDbhCh);

/// Database hash Characteristic Value
static uint8_t gattSvcDbhChVal[BLE_GATT_SVC_DATABASE_HASH_LEN] = {0};
/// Length  Database hash Characteristic Value
static const uint16_t gattSvcDbhChValLen = sizeof(gattSvcDbhChVal);

/**************************************************************************************************
 * Generic Attribute server supported features characteristic definitions
 **************************************************************************************************/

/// Server supported features characteristic UUID - definition 0x2B3A
static const uint8_t gattSvcSsfChUuid[RBK_SMP290_BLE_ATTS_16_UUID_LEN] = {RBK_SMP290_CONV_U16_TO_BYTES(BLE_GATT_SVC_SERVER_SUPPORTED_FEAT_UUID)};

/// Server supported features Attribute value
static const uint8_t gattSvcSsfCh[] = {

    (uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ,                                   // Characteristic Properties
    RBK_SMP290_CONV_U16_TO_BYTES(BLE_GATT_SVC_SSF_HNDL),                     // Characteristic Value Attribute Handle
    RBK_SMP290_CONV_U16_TO_BYTES(BLE_GATT_SVC_SERVER_SUPPORTED_FEAT_UUID)};  // Characteristic UUID

/// Length of Server supported features Attribute value
static const uint16_t gattSvcSsfChLen = sizeof(gattSvcSsfCh);

/// Server supported features Characteristic Value
static uint8_t const gattSvcSsfChVal[] = {0xFF};
/// Length of Server supported features Characteristic Value
static const uint16_t gattSvcSsfChValLen = sizeof(gattSvcSsfChVal);

/**************************************************************************************************
 * Generic Attribute Service attributes list
 **************************************************************************************************/
static const rbk_smp290_ble_attsAttr_tst gattSvcAttrGrp[] = {

    /// Add Primary service
    {
        rbk_smp290_ble_attsPrimSvcUuid,           // Primary service declaration UUID: 0x2800
        (uint8_t *)gattSvc,                      // Primary service Attribute Value: Generic Access Profile Service UUID : 0x1801
        (uint16_t *)&gattSvcLen,                 // Primary service Attribute Value length
        sizeof(gattSvc),                         // Primary service Attribute Value  maximum Length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Primary service Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Primary service Attribute permission
    },
    /// Service changed Characteristic
    {
        rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
        (uint8_t *)gattSvcScCh,                  // Characteristic Attribute Value : Service Changed UUID : 0x2A05
        (uint16_t *)&gattSvcScChLen,             // Characteristic Attribute Value length
        sizeof(gattSvcScCh),                     // Characteristic Attribute Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic Attribute permission
    },
    /// Service changed Characteristic value declaration
    {
        gattSvcScChUuid,                         // Characteristics UUID 0x2A05
        (uint8_t *)gattSvcScVal,                 // Characteristic value
        (uint16_t *)&gattSvcScValLen,            // Characteristic value length
        sizeof(gattSvcScVal),                    // Characteristic value maximum Length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic value Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_NONE  // Characteristic value Attribute Permission
    },
    /// Service changed Client Characteristic Configuration
    {
        rbk_smp290_ble_attsCliChCfgUuid,       // Client characteristic configuration UUID: 0x2902
        (uint8_t *)gattSvcScCccVal,           // Client characteristic configuration Value
        (uint16_t *)&gattSvcScCccLen,         // Client characteristic configuration Value length
        sizeof(gattSvcScCccVal),              // Client characteristic configuration Value length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_CCC,  // Client characteristic configuration Attribute settings
        ((uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ |
         (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_WRITE)  // Client characteristic configuration Attribute permission
    },
    /// Client supported features Characteristic
    {
        rbk_smp290_ble_attsChUuid,                  // Characteristic declaration UUID: 0x2803
        (uint8_t *)gattValCsfCh,                   // Characteristic Attribute Value : Client Supported Features UUID : 0x2B29
        (uint16_t *)&gattValCsfChLen,              // Characteristic Attribute Value length
        sizeof(gattValCsfCh),                      // Characteristic Attribute Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,      // Characteristic Attribute settings
        (uint8_t)(RBK_SMP290_BLE_ATTS_PERMIT_READ)  // Characteristic Attribute permission
    },
    /// Client supported features Characteristic value declaration
    {
        gattValCsfChUuid,                                                                          // Characteristics UUID 0x2B29
        (uint8_t *)gattCsfVal,                                                                     // Characteristic value
        (uint16_t *)&gattCsfValLen,                                                                // Characteristic value length
        sizeof(gattCsfVal),                                                                        // Characteristic value maximum Length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_READ_CBACK | (uint8_t)RBK_SMP290_BLE_ATTS_SET_WRITE_CBACK,  // Characteristic value Attribute settings
        (uint8_t)(RBK_SMP290_BLE_ATTS_PERMIT_READ) | (uint8_t)(RBK_SMP290_BLE_ATTS_PERMIT_WRITE)     // Characteristic value Attribute Permission
    },
    /// Database hash characteristic
    {
        rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
        (uint8_t *)gattSvcDbhCh,                 // Characteristic Attribute Value :Database Hash  UUID : 0x2B2A
        (uint16_t *)&gattSvcDbhChLen,            // Characteristic Attribute Value length
        sizeof(gattSvcDbhCh),                    // Characteristic Attribute Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic Attribute permission
    },
    /// Database hash Characteristic Value
    {
        gattSvcDbhChUuid,                            // Characteristics UUID 0x2B2A
        (uint8_t *)gattSvcDbhChVal,                  // Characteristic value
        (uint16_t *)&gattSvcDbhChValLen,             // Characteristic value length
        sizeof(gattSvcDbhChVal),                     // Characteristic value maximum Length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_READ_CBACK,  // Characteristic value Attribute settings
        (uint8_t)(RBK_SMP290_BLE_ATTS_PERMIT_READ)    // Characteristic value Attribute Permission
    },
    /// Server supported features Characteristic
    {
        rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
        (uint8_t *)gattSvcSsfCh,                 // Characteristic Attribute Value : Server Supported Features  UUID : 0x2B3A
        (uint16_t *)&gattSvcSsfChLen,            // Characteristic Attribute Value length
        sizeof(gattSvcSsfCh),                    // Characteristic Attribute Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic Attribute permission
    },
    /// Server supported features Characteristic Value
    {
        gattSvcSsfChUuid,                        // Characteristics UUID 0x2B3A
        (uint8_t *)gattSvcSsfChVal,              // Characteristic value
        (uint16_t *)&gattSvcSsfChValLen,         // Characteristic value length
        sizeof(gattSvcSsfChVal),                 // Characteristic value maximum Length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic value Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic value Attribute Permission
    }};

/**************************************************************************************************
 * Gatt service Attribute Grouping
 **************************************************************************************************/
/// Gatt service Attribute Group
rbk_smp290_ble_attsAttrGrp_tst gattSvcGrp = {
    // Service group
    NULL,
    (rbk_smp290_ble_attsAttr_tst *)gattSvcAttrGrp,
    gattSvc_rdCallBack,
    NULL,
    BLE_GATT_SVC_START_HDL,
    BLE_GATT_SVC_END_HDL
    //
};

/// @}

/*******************************************************************************
 *  Function definition
 ******************************************************************************/

rbk_smp290_ble_atts_err_ten gattSvc_rdCallBack(rbk_smp290_ble_attsConnId connId, rbk_smp290_ble_attsHndl handle, uint8_t Op, uint16_t offset,
                                              rbk_smp290_ble_attsAttr_tst *pAttr)
{
    (void)(connId);
    (void)(Op);
    (void)(offset);

    switch (handle)
    {
        case BLE_GATT_SVC_DBH_HNDL:
        {
            memcpy(pAttr->pAttValue, gattSvcDbhChVal, BLE_GATT_SVC_DATABASE_HASH_LEN);
            *pAttr->pLen = sizeof(gattSvcDbhChVal);
            smp290_log(LOG_VERBOSITY_INFO, "Gatt Service DB hash ReadCb: %d\r\n", handle);
        }
        break;
        case BLE_GATT_SVC_CSF_HDL:
            memcpy(pAttr->pAttValue, gattCsfVal, sizeof(gattCsfVal));
            *pAttr->pLen = sizeof(gattCsfVal);
            smp290_log(LOG_VERBOSITY_INFO, "Gatt Service Client Supported Feature ReadCb: %d\r\n", handle);
            break;
        default:
        {
        }
        break;
    }
    return RBK_SMP290_BLE_ATTS_SUCCESS;
}

rbk_smp290_ble_atts_err_ten gattSvc_wrCallBack(rbk_smp290_ble_attsConnId connId, rbk_smp290_ble_attsHndl handle, uint8_t Op, uint16_t offset,
                                              uint16_t len, uint8_t *pValue, rbk_smp290_ble_attsAttr_tst *pAttr)
{
    (void)(connId);
    (void)(Op);
    (void)(offset);
    (void)(pAttr);

    switch (handle)
    {
        case BLE_GATT_SVC_CSF_HDL:
            memcpy(gattCsfVal, pValue, len);
            smp290_log(LOG_VERBOSITY_INFO, "gatt_WriteCb handle: %d | value :%d | length :%d\r\n", handle, *pValue, len);
            break;
        default:
        {
        }
        break;
    }
    return RBK_SMP290_BLE_ATTS_SUCCESS;
}

void addGattSvc()
{
    //
    (void)rbk_smp290_ble_atts_addAttrGrp((rbk_smp290_ble_attsAttrGrp_tst *)&gattSvcGrp);
}
void rmGattSvc()
{
    //
    rbk_smp290_ble_atts_rmvAttrGrp((uint16_t)BLE_GATT_SVC_START_HDL);
}
void gattSvc_send_service_changed_indication(uint16_t start, uint16_t end)
{
    rbk_smp290_ble_atts_err_ten ret;
    uint8_t *ptr = gattSvcScVal;
    BLE_CONV_U16_TO_BYTESTREAM(ptr, start);
    BLE_CONV_U16_TO_BYTESTREAM(ptr, end);
    // send indication
    ret = rbk_smp290_ble_atts_sendIndication((rbk_smp290_ble_attsHndl)BLE_GATT_SVC_SC_HNDL, sizeof(gattSvcScVal), gattSvcScVal);
    if (ret != RBK_SMP290_BLE_ATTS_SUCCESS)
    {
        smp290_log(LOG_VERBOSITY_INFO, "Failed to send gatt service changed indication \r\n");
    }
}
/** @} */
