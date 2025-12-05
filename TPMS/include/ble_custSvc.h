/**
 * @addtogroup   measure_advertise_conn
 * @{
 * @file         ble_custSvc.h
 * @brief        \glos{API} of the BLE custom service.
 * @details
 * This file contains the API and definitions for the BLE custom service of measure_advertise_conn. 
 * The custom service provides characteristics for current TX power, counter value, and TSD.
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

#ifndef _BLE_CUSTSVC_H
#define _BLE_CUSTSVC_H

#include "rbk_smp290_gpio.h"
#include "rbk_smp290_nvm.h"
#include "ble_gattSvc.h"
#include "main.h"

/// @defgroup measure_advertise_conn_cust_svc Custom Service definitions
/// @{

/**************************************************************************************************
 Macros
 **************************************************************************************************/
/// Custom Service start handle
#define BLE_CUST_SVC_START_HNDL UINT16_C(BLE_GATT_SVC_MAX_HNDL)
/// Custom Service End handle
#define BLE_CUST_SVC_END_HNDL UINT16_C(BLE_CUST_SVC_MAX_HNDL - 1)

#define BLE_CUST_SVC_SERVICE_UUID_PART    UINT16_C(0x1A00)  //!< Demo Service UUID

#define BLE_CUST_SVC_TXPWR_CHAR_UUID_PART UINT16_C(0x1A01)  //!< Current TX power characteristics UUID
#define BLE_CUST_SVC_CNTR_CHAR_UUID_PART  UINT16_C(0x1A02)  //!<Counter value characteristics UUID
#define BLE_CUST_SVC_TSD_CHAR_UUID_PART   UINT16_C(0x1A03)  //!<Thermal Shut Down characteristics UUID

/// Custom service 02a63290-xxxx-b83e-af18-025703723367
/// Custom base UUID part 1
#define BLE_CUST_SVC_BASE_UUID_PART1 0x67, 0x33, 0x72, 0x03, 0x57, 0x02, 0x18, 0xaf, 0x3e, 0xb8
/// Custom base UUID part 2
#define BLE_CUST_SVC_BASE_UUID_PART2 0x90, 0x32, 0xa6, 0x02

/// Macro for building Custom UUID
#define BLE_CUST_SVC_BUILD(part) BLE_CUST_SVC_BASE_UUID_PART1, RBK_SMP290_CONV_U16_TO_BYTES(part), BLE_CUST_SVC_BASE_UUID_PART2

/// Macro for Building the Custom Service UUID
#define BLE_CUST_SVC_SERVICE_UUID BLE_CUST_SVC_BUILD(BLE_CUST_SVC_SERVICE_UUID_PART)

/// Macro for Building the Custom Characteristics 1  UUID
#define BLE_CUST_SVC_TXPWR_CHAR_UUID BLE_CUST_SVC_BUILD(BLE_CUST_SVC_TXPWR_CHAR_UUID_PART)

/// Macro for Building the Custom Characteristics 2  UUID
#define BLE_CUST_SVC_CNTR_CHAR_UUID BLE_CUST_SVC_BUILD(BLE_CUST_SVC_CNTR_CHAR_UUID_PART)

/// Macro for Building the Custom Characteristics 3  UUID
#define BLE_CUST_SVC_TSD_CHAR_UUID BLE_CUST_SVC_BUILD(BLE_CUST_SVC_TSD_CHAR_UUID_PART)

#define BLE_CUST_SVC_CCC_BUFF_SIZE    UINT8_C(2)      //!< Ble Indication buffer size
#define BLE_CUST_SVC_BLE_TMR_INTERVAL UINT32_C(1000)  //!< Ble Indication timer interval in ms    1 sec

#define BLE_CUST_SVC_READ_REQ  0x01u  //!< Custom service read request
#define BLE_CUST_SVC_WRITE_REQ 0x02u  //!< Custom service write request

/// TSD default value
#define TSD_DEFAULT_STATUS 0x01u

/// Memory address of the TX power level in \glos{NVM}
#define TXPWR_NVM_ADR 0x404800u

/******************************************************************************\
 * Types
 \******************************************************************************/

/// Custom service handle values
typedef enum
{
    //Custom Service
	BLE_CUST_SVC_SVC_HNDL = BLE_CUST_SVC_START_HNDL, //!< Custom service declaration
    BLE_CUST_SVC_TXPWR_CHAR_HNDL,                    //!< Custom Characteristic 1 Handle  0x2901
    BLE_CUST_SVC_TXPWR_CHAR_DATA_HNDL,               //!< Custom Characteristic 1 Data Handle  0x2901
    BLE_CUST_SVC_TXPWR_CHAR_CUD_HNDL,                //!< Custom Characteristic 1 Characteristic User Description 0x2901
    BLE_CUST_SVC_CNTR_CHAR_HNDL,                     //!< Custom Characteristic 2 Handle
    BLE_CUST_SVC_CNTR_CHAR_DATA_HNDL,                //!< CustomCharacteristic 2 Data Handle
    BLE_CUST_SVC_CNTR_CHAR_CUD_HNDL,                 //!< Custom Characteristic 2 Characteristic User Description 0x2901
    BLE_CUST_SVC_CNTR_CHAR_CCC_HNDL,                 //!< Custom Characteristic 2 Client Characteristics Configuration 0x2902
    BLE_CUST_SVC_TSD_CHAR_HNDL,                      //!< Custom Characteristic 3 Handle
	BLE_CUST_SVC_TSD_CHAR_DATA_HNDL,                 //!< Custom Characteristic 3 Data Handle
	BLE_CUST_SVC_TSD_CHAR_CUD_HNDL,                  //!< Custom Characteristic 3 Characteristic User Description
	BLE_CUST_SVC_MAX_HNDL
} custSvc_ten;

/// Structure with size NVM word to store the Tx Power in \glos{NVM}
typedef struct
{
    uint8_t TxPwrNvmStrd;                  //!< Is TX Power stored in \glos{NVM} (only a value of 1 means it is stored)
    int8_t TxPwrNvm;                       //!< TX Power value in \glos{NVM}
    uint8_t reserved[NVM_WORD_SIZE - 2u];  //!< Reserved
} ble_TxPwrNvm_tst;

_Static_assert(NVM_WORD_SIZE == sizeof(ble_TxPwrNvm_tst), "The size of ble_TxPwrNvm_tst is not equal to NVM_WORD_SIZE.");

// Restore default pack
#pragma pack()

/// Type definition of the custom service callback
typedef void (*custSvcAppCbk)(uint8_t req, rbk_smp290_ble_attsHndl hndl, uint8_t *value, uint16_t len);

/******************************************************************************\
 * Extern global variables
 \******************************************************************************/
/// BLE Notification counter
extern volatile uint16_t ble_indicnCntr;
/// Tx power level
extern int8_t ble_txPwrLvl;

/// @}

/******************************************************************************\
 * Public functions
 \******************************************************************************/

/**
 * @brief  This \glos{API} adds the custom service to the attribute database.
 * return void
 */
void addCustSvc(void);
/**
 * @brief  This \glos{API} removes the custom service from the attribute database.
 * return void
 */
void rmCustSvc(void);

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
rbk_smp290_ble_atts_err_ten custSvc_rdCallBack(rbk_smp290_ble_attsConnId connId, rbk_smp290_ble_attsHndl handle, uint8_t Op, uint16_t offset,
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
rbk_smp290_ble_atts_err_ten custSvc_wrCallBack(rbk_smp290_ble_attsConnId connId, rbk_smp290_ble_attsHndl handle, uint8_t Op, uint16_t offset,
                                              uint16_t len, uint8_t *pValue, rbk_smp290_ble_attsAttr_tst *pAttr);

/**
 * @brief This \glos{API} processes the incoming CCC (Client Characteristic Configuration) event from \glos{BLE} \glos{ATT} layer
 *
 * @param  cccVal  The CCC value.
 * @param  hndl    The attribute handle.
 * @param  idx     The index.
 *
 * return void
 */
void custSvc_procCccEvt(rbk_smp290_ble_atts_CccVal_ten cccVal, rbk_smp290_ble_attsHndl hndl, uint8_t idx);

/**
 * @brief This \glos{API} to processes the indication confirmation.
 *
 * return void
 */
void custSvc_indication_confiramtion(void);

/**
 * @brief This \glos{API} to registers the application-specific callback.
 *
 * @param  cbk  The callback function.
 *
 * return void
 */
void custSvcRegAppCbk(custSvcAppCbk cbk);

/**
 * @brief  Configures the desired TX power, which is received from write callback event
 *
 * @param  txPwr  The TX power level.
 * return void
 */
void config_txPwr(int8_t txPwr);

/**
 * @brief Writes the TX power level to \glos{NVM}.
 *
 * @param  txPwr  The TX power level to be written.
 *
 * @return @ref RBK_SMP290_NVM_SUCCESS if operation is successful.
 * @return Else, the NVM driver error that took place
 */
rbk_smp290_nvm_err_ten writeTxPwrToNvm(int8_t txPwr);

/**
 * @brief Reads the current TX power level from \glos{NVM}. If the value is
 *        not found, the default \ref RBK_SMP290_BLE_TX_PWR_6_DBM is returned.
 *
 * @return The stored TX power level in \glos{NVM} if it is found.
 * @return \ref RBK_SMP290_BLE_TX_PWR_0_DBM if TX power level in \glos{NVM} is not found.
 */
int8_t readTxPwrFromNvm(void);

/**
 * @brief  Configures Thermal Shut Down
 *
 * @param  value  TSD
 * return void
 *
 */
void config_tsd(uint8_t value);

#endif /* _BLE_CUSTSVC_H */

/** @} */
