/**
 * @addtogroup   measure_advertise_conn
 * @{
 * @file         ble_gapSvc.h
 * @brief        \glos{API} of \glos{BLE} \glos{GAP} service
 * @details      This file contains the API for the \glos{BLE} \glos{GAP} service.
 *               The \glos{BLE} \glos{GAP} service provides functionality related
 *               to the Generic Access Profile (GAP) of a Bluetooth Low Energy (BLE) device.
 *               The \glos{GAP} service includes characteristics such as Device Name,
 *               Appearance, and Central Address Resolution.
 *               These characteristics define the basic information and behavior of the
 *               \glos{BLE} device.
 *               The \glos{API} in this file allow adding and removing the GAP service group
 *               to the attribute database.
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
#ifndef _BLE_GAPSVC_H
#define _BLE_GAPSVC_H

#include "rbk_smp290_ble_atts_types.h"
#include "rbk_smp290_ble_atts.h"
#include "rbk_smp290_ble_atts_uuid.h"

/**************************************************************************************************
 Macros
 **************************************************************************************************/
/// @addtogroup measure_advertise_conn_gap_svc BLE GAP Service definitions
/// @{

#define BLE_GAP_SVC_START_HNDL 0x01                        //!< GAP start handle
#define BLE_GAP_SVC_END_HNDL   (BLE_GAP_SVC_MAX_HNDL - 1)  //!< GAP end handle

/// Gap Service UUID
#define BLE_GAP_SVC_UUID              UINT16_C(0x1800)  //!< Generic Access Profile Service
#define BLE_GAP_SVC_DEVICE_NAME_UUID  UINT16_C(0x2A00)  //!< Device Name
#define BLE_GAP_SVC_APPEARANCE_UUID   UINT16_C(0x2A01)  //!< Appearance
#define BLE_GAP_SVC_CEN_ADDR_RES_UUID UINT16_C(0x2AA6)  //!< central address resolution characteristic

/// Appearance characteristics value
#define RBK_SMP290_BLE_PROF_CHAR_APPEARANCE_UNKNOWN UINT16_C(0)

/// GAP service handle
enum
{
    BLE_GAP_SVC_SVC_HNDL = BLE_GAP_SVC_START_HNDL,  //!< GAP service declaration
    BLE_GAP_SVC_DN_CH_HNDL,                         //!< Device name characteristic
    BLE_GAP_SVC_DN_HNDL,                            //!< Device name
    BLE_GAP_SVC_AP_CH_HNDL,                         //!< Appearance characteristic
    BLE_GAP_SVC_AP_HNDL,                            //!< Appearance
    BLE_GAP_SVC_CAR_CH_HDL,                         //!< Central address resolution characteristic
    BLE_GAP_SVC_CAR_HDL,                            //!< Central address resolution
    BLE_GAP_SVC_MAX_HNDL
};

/// @}
/******************************************************************************\
 * Public functions
 \******************************************************************************/

/**
 * @brief  This function adds the GAP service group to the attribute database.
 * @details This function adds the Generic Access service and its
 * characteristics to the attribute table using the rbk_smp290_ble_atts library.
 *
 * return void
 */
void addGapSvc(void);

/**
 * @brief  This function removes the GAP service group from the attribute
 * database.
 * @details This function removes the Generic Access service and its
 * characteristics from the attribute table using the rbk_smp290_ble_atts
 * library.
 *
 * return void
 */
void rmGapSvc(void);

#endif /* _BLE_GAPSVC_H */
/** @} */
