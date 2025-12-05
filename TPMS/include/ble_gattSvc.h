/**
 * @addtogroup   measure_advertise_conn
 * @{
 * @file         ble_gattSvc.h
 * @brief        \glos{API} of \glos{BLE} \glos{GATT} service
 * @details
 * This file contains the API for the Ble Gatt service. The Ble Gatt service provides functionality
 * for managing GATT attributes and handling read and write requests from the BLE GATT layer.
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
#ifndef _BLE_GATTSVC_H
#define _BLE_GATTSVC_H

#include "ble_gapSvc.h"

/**************************************************************************************************
 Macros
 **************************************************************************************************/
/// @addtogroup measure_advertise_conn_gatt_svc BLE GATT Service definitions
/// @{

#define BLE_GATT_SVC_START_HDL BLE_GAP_SVC_MAX_HNDL         //!< GATT start handle.
#define BLE_GATT_SVC_END_HDL   (BLE_GATT_SVC_MAX_HNDL - 1)  //!< GATT end handle

#define BLE_GATT_SVC_CSF_LEN           0x01  //!< GATT client supported feature data length
#define BLE_GATT_SVC_DATABASE_HASH_LEN 16    //!< GATT Data Base hash value length

#define BLE_GATT_SVC_SERVICE_UUID               0x1801  //!< Generic Attribute Profile Service
#define BLE_GATT_SVC_SERVICE_CHANGED_UUID       0x2A05  //!< Service Changed
#define BLE_GATT_SVC_CLIENT_SUPP_FEAT           0x2B29  //!< Client Supported Features
#define BLE_GATT_SVC_DATABASE_HASH_UUID         0x2B2A  //!< Database Hash
#define BLE_GATT_SVC_SERVER_SUPPORTED_FEAT_UUID 0x2B3A  //!< Server Supported Features

enum
{
    BLE_GATT_SVC_SVC_HNDL = BLE_GATT_SVC_START_HDL,  //!< GATT service declaration
    BLE_GATT_SVC_SC_CH_HNDL,                         //!< Service changed characteristic
    BLE_GATT_SVC_SC_HNDL,                            //!< Service changed
    BLE_GATT_SVC_SC_CCC_HNDL,                        //!< Service changed client characteristic configuration descriptor
    BLE_GATT_SVC_CSF_CH_HDL,                         //!< Client supported features characteristic
    BLE_GATT_SVC_CSF_HDL,                            //!< Client supported features
    BLE_GATT_SVC_DBH_CH_HNDL,                        //!< Database hash characteristic
    BLE_GATT_SVC_DBH_HNDL,                           //!< Database hash
    BLE_GATT_SVC_SSF_CH_HNDL,                        //!< Server supported features characteristic
    BLE_GATT_SVC_SSF_HNDL,                           //!< Server supported features
    BLE_GATT_SVC_MAX_HNDL                            //!< GATT maximum handle
};

/// @}

/******************************************************************************\
 * Public functions
 \******************************************************************************/
/**
 * @brief  This function adds the GATT service group to the attribute database.
 *
 * @details This function adds the GATT service group to the attribute database. It initializes the GATT
 * service and its characteristics.
 *
 * @note   This function should be called during initialization to set up the GATT service.
 *
 * return void
 */
void addGattSvc(void);

/**
 * @brief  This function removes the GATT service group from the attribute database.
 *
 * @details This function removes the GATT service group from the attribute database. It cleans up the GATT
 * service and its characteristics.
 *
 * @note   This function should be called during cleanup to remove the GATT service.
 *
 * return void
 */
void rmGattSvc(void);

/**
 * @brief  This function is the callback for processing incoming read requests from the BLE GATT layer.
 *
 * @details This function is called when a read request is received from the BLE GATT layer. It processes the
 * read request and returns the attribute value to be sent back to the client.
 *
 * @param  connId  The connection ID.
 * @param  handle  The handle of the attribute being read.
 * @param  Op      The read operation code.
 * @param  offset  The offset within the attribute value.
 * @param  pAttr   Pointer to the attribute structure.
 *
 * @return The error code indicating the result of the read operation.
 */
rbk_smp290_ble_atts_err_ten gattSvc_rdCallBack(rbk_smp290_ble_attsConnId connId, rbk_smp290_ble_attsHndl handle, uint8_t Op, uint16_t offset,
                                              rbk_smp290_ble_attsAttr_tst *pAttr);

/**
 * @brief \glos{API} to process the incoming write request from \glos{BLE} \glos{ATT} layer
 *
 * @details This function is called when a write request is received from the BLE ATT layer. It processes the
 * write request and updates the attribute value accordingly.
 *
 * @param  connId   The connection ID.
 * @param  handle   The handle of the attribute being written.
 * @param  Op       The write operation code.
 * @param  offset   The offset within the attribute value.
 * @param  len      The length of the data being written.
 * @param  pValue   Pointer to the data buffer containing the new attribute value.
 * @param  pAttr    Pointer to the attribute structure.
 *
 * @return The error code indicating the result of the write operation.
 */
rbk_smp290_ble_atts_err_ten gattSvc_wrCallBack(rbk_smp290_ble_attsConnId connId, rbk_smp290_ble_attsHndl handle, uint8_t Op, uint16_t offset,
                                              uint16_t len, uint8_t *pValue, rbk_smp290_ble_attsAttr_tst *pAttr);

/**
 * @brief  This \glos{API} to send the server service changed indication to the client
 *
 * @details This function sends a service changed indication to the client. It is an optional feature and
 * should be called if the database has changed and the peer has configured service indications.
 *
 * @note   This function should be called after the connection is established.
 *
 * @param  start  The start handle of the changed service.
 * @param  end    The end handle of the changed service.
 *
 * return void
 */
void gattSvc_send_service_changed_indication(uint16_t start, uint16_t end);

#endif /* _BLE_GATTSVC_H */
/** @} */
