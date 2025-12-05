/**
 * @addtogroup  measure_advertise_conn
 * @{
 * @file         ble_gapSvc.c
 * @brief        Ble Generic Access service implementation example
 * @details      This file contains the implementation of the Ble Generic Access service.
 *               The Generic Access service provides information about the device and its capabilities.
 *               It includes characteristics such as device name and appearance.
 *               This implementation demonstrates how to add and remove the Generic Access service
 *               using the rbk_smp290_ble_atts library.
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
#include "rbk_smp290_ble_atts.h"

/* Project includes */
#include "ble_gapSvc.h"
#include "main.h"

/// @defgroup measure_advertise_conn_gap_svc BLE GAP Service definitions
/// @{

/**************************************************************************************************
 * Macros
 **************************************************************************************************/
/// Default device name
#define DEFAULT_DEV_NAME "TPMS-SMP290"
/// Length of default device name
#define DEFAULT_DEV_NAME_LEN (sizeof(DEFAULT_DEV_NAME) - 1)

/*******************************************************************************
 *  Global variables
 ******************************************************************************/

/**************************************************************************************************
 * GAP group
 **************************************************************************************************/
/**************************************************************************************************
 * GAP service definitions
 **************************************************************************************************/
/// GAP service UUID definition
static const uint8_t gapSvc[] = {RBK_SMP290_CONV_U16_TO_BYTES(BLE_GAP_SVC_UUID)};

/// Length of GAP service Attribute value
static const uint16_t gapSvcLen = sizeof(gapSvc);

/**************************************************************************************************
 * Device name characteristic definitions
 **************************************************************************************************/
/// Device name characteristic UUID definition
static const uint8_t gapSvcDnChUuid[RBK_SMP290_BLE_ATTS_16_UUID_LEN] = {RBK_SMP290_CONV_U16_TO_BYTES(BLE_GAP_SVC_DEVICE_NAME_UUID)};

/// Device name characteristic Attribute value
static const uint8_t gapSvcDnCh[] = {
    (uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ,                      // Characteristic Properties
    RBK_SMP290_CONV_U16_TO_BYTES(BLE_GAP_SVC_DN_HNDL),          // Characteristic Value Attribute Handle
    RBK_SMP290_CONV_U16_TO_BYTES(BLE_GAP_SVC_DEVICE_NAME_UUID)  // Characteristic UUID
};

/// Length of device name characteristic Attribute value
static const uint16_t gapSvcDnChLen = sizeof(gapSvcDnCh);

/// Device name characteristic value
static uint8_t gapSvcDnChVal[RBK_SMP290_BLE_ATTS_DFLT_PAYLOAD_LEN] = DEFAULT_DEV_NAME;
static uint16_t gapSvcDnChValLen                                  = DEFAULT_DEV_NAME_LEN;
/**************************************************************************************************
 * Appearance characteristic definitions
 **************************************************************************************************/
/// Appearance characteristic UUID definition
static const uint8_t gapSvcApChUuid[RBK_SMP290_BLE_ATTS_16_UUID_LEN] = {RBK_SMP290_CONV_U16_TO_BYTES(BLE_GAP_SVC_APPEARANCE_UUID)};

/// Appearance characteristic Attribute value
static const uint8_t gapSvcApCh[] = {
    (uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ,                     // Characteristic Properties
    RBK_SMP290_CONV_U16_TO_BYTES(BLE_GAP_SVC_AP_HNDL),         // Characteristic Value Attribute Handle
    RBK_SMP290_CONV_U16_TO_BYTES(BLE_GAP_SVC_APPEARANCE_UUID)  // Characteristic UUID
};

/// Length of appearance characteristic Attribute value
static const uint16_t gapSvcApChLen = sizeof(gapSvcApCh);

/// Appearance characteristic Value
static uint8_t gapSvcApChVal[]         = {0x00, 0x00};
static const uint16_t gapSvcApChValLen = sizeof(gapSvcApChVal);

/**************************************************************************************************
 * Central address resolution characteristic definitions
 **************************************************************************************************/
///  Central address resolution characteristic UUID definition
static const uint8_t gapSvcCarChUuid[RBK_SMP290_BLE_ATTS_16_UUID_LEN] = {RBK_SMP290_CONV_U16_TO_BYTES(BLE_GAP_SVC_CEN_ADDR_RES_UUID)};

/// Central address resolution characteristic Attribute value
static const uint8_t gapSvcCarCh[] = {
    (uint8_t)RBK_SMP290_BLE_ATTS_PPTY_READ,                       // Characteristic Properties
    RBK_SMP290_CONV_U16_TO_BYTES(BLE_GAP_SVC_CAR_HDL),           // Characteristic Value Attribute Handle
    RBK_SMP290_CONV_U16_TO_BYTES(BLE_GAP_SVC_CEN_ADDR_RES_UUID)  // Characteristic UUID
};

/// Length of central address resolution characteristic Attribute value
static const uint16_t gapSvcCarChLen = sizeof(gapSvcCarCh);

/// Central address resolution characteristic value
static uint8_t gapSvcCarChVal[]         = {0};
static const uint16_t gapSvcCarChValLen = sizeof(gapSvcCarChVal);

/**************************************************************************************************
 * Gap service attributes list
 **************************************************************************************************/
static const rbk_smp290_ble_attsAttr_tst rbk_smp290_prof_gapAttrGrp[] = {

    /// Add Primary service
    /// Generic Access Profile Service
    {
        rbk_smp290_ble_attsPrimSvcUuid,           // Primary service declaration UUID: 0x2800
        (uint8_t *)gapSvc,                       // Primary service Attribute Value: Generic Access Profile Service UUID : 0x1800
        (uint16_t *)&gapSvcLen,                  // Primary service Attribute Value length
        sizeof(gapSvc),                          // Primary service Attribute Value  maximum Length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Primary service Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Primary service Attribute permission
    },
    /// Device Name characteristic
    {
        rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
        (uint8_t *)gapSvcDnCh,                   // Characteristic Attribute Value : Device Name UUID : 0x2A00
        (uint16_t *)&gapSvcDnChLen,              // Characteristic Attribute Value length
        sizeof(gapSvcDnCh),                      // Characteristic Attribute Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic Attribute permission
    },
    /// Device Name characteristic value declaration
    {
        gapSvcDnChUuid,                 // Characteristics UUID 0x2A00
        (uint8_t *)gapSvcDnChVal,       // Characteristic value
        (uint16_t *)&gapSvcDnChValLen,  // Characteristic value length
        sizeof(gapSvcDnChVal),          // Characteristic value maximum Length
        /// Defines if the characteristic value size is variable .
        /// It is useful in case of the attribute is writable to define if the remote is allowed to write
        /// this kind of value Length.
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_VARIABLE_LEN,  // Characteristic value Attribute settings
        (uint8_t)(RBK_SMP290_BLE_ATTS_PERMIT_READ)      // Characteristic value Attribute Permission
    },
    /// Appearance characteristic  0x2A01
    {
        rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
        (uint8_t *)gapSvcApCh,                   // Characteristic Attribute Value :Appearance  UUID : 0x2A01
        (uint16_t *)&gapSvcApChLen,              // Characteristic Attribute Value length
        sizeof(gapSvcApCh),                      // Characteristic Attribute Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic Attribute permission
    },
    /// Appearance characteristic value declaration
    {
        gapSvcApChUuid,                          // Characteristics UUID 0x2A01
        (uint8_t *)gapSvcApChVal,                // Characteristic value
        (uint16_t *)&gapSvcApChValLen,           // Characteristic value length
        sizeof(gapSvcApChVal),                   // Characteristic value maximum Length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic value Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic value Attribute Permission
    },
    /// Central address resolution characteristic
    {
        rbk_smp290_ble_attsChUuid,                // Characteristic declaration UUID: 0x2803
        (uint8_t *)gapSvcCarCh,                  // Characteristic Attribute Value : Central address resolution  UUID : 0x2AA6
        (uint16_t *)&gapSvcCarChLen,             // Characteristic Attribute Value length
        sizeof(gapSvcCarCh),                     // Characteristic Attribute Value maximum length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic Attribute permission
    },
    /// Central address resolution characteristic value declaration
    {
        gapSvcCarChUuid,                         // Characteristics UUID 0x2AA6
        (uint8_t *)gapSvcCarChVal,               // Characteristic value
        (uint16_t *)&gapSvcCarChValLen,          // Characteristic value length
        sizeof(gapSvcCarChVal),                  // Characteristic value maximum Length
        (uint8_t)RBK_SMP290_BLE_ATTS_SET_NONE,    // Characteristic value Attribute settings
        (uint8_t)RBK_SMP290_BLE_ATTS_PERMIT_READ  // Characteristic value Attribute Permission
    }};
/**************************************************************************************************
 * Gap service Attribute Grouping
 **************************************************************************************************/
/// Gap service Attribute Group
rbk_smp290_ble_attsAttrGrp_tst gapSvcGrp = {
    NULL, (rbk_smp290_ble_attsAttr_tst *)rbk_smp290_prof_gapAttrGrp, NULL, NULL, (uint16_t)BLE_GAP_SVC_START_HNDL, (uint16_t)BLE_GAP_SVC_END_HNDL};

/// @}

/*******************************************************************************
 *  Function definition
 ******************************************************************************/

/// Add the Generic Access service to the attribute table.
void addGapSvc()
{
    //
    (void)rbk_smp290_ble_atts_addAttrGrp((rbk_smp290_ble_attsAttrGrp_tst *)&gapSvcGrp);
}

/// Remove the Generic Access service from the attribute table.
void rmGapSvc()
{
    //
    (void)rbk_smp290_ble_atts_rmvAttrGrp(BLE_GAP_SVC_START_HNDL);
}

/** @} */
