/**
 * @addtogroup   measure_advertise_conn
 * @{
 * @file         ble_custSvc.h
 * @brief        \glos{API} of the BLE custom service.
 * @details
 * This file contains the API and definitions for the BLE custom service of measure_advertise_conn. 
 * The custom service provides characteristics for triggering measurements.
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

#ifndef _BLE_MEASSVC_H
#define _BLE_MEASSVC_H

#include "rbk_smp290_gpio.h"
#include "rbk_smp290_nvm.h"
#include "ble_maintSvc.h"
#include "main.h"

/// @defgroup measure_advertise_conn_cust_svc Custom Service definitions
/// @{

/**************************************************************************************************
 Macros
 **************************************************************************************************/
/// Custom Service start handle
#define BLE_CUST_SVC_MEAS_START_HNDL UINT16_C(BLE_CUST_SVC_MAINT_MAX_HNDL)
/// Custom Service End handle
#define BLE_CUST_SVC_MEAS_END_HNDL UINT16_C(BLE_CUST_SVC_MEAS_MAX_HNDL - 1)

#define BLE_CUST_SVC_MEAS_SERVICE_UUID_PART    UINT16_C(0x1D00)  //!< Demo Service UUID

#define BLE_CUST_SVC_MEAS_T_CHAR_UUID_PART UINT16_C(0x1D31)     //!< T characteristics UUID
#define BLE_CUST_SVC_MEAS_TPAZ_CHAR_UUID_PART UINT16_C(0x1D32)  //!< TPAZ characteristics UUID
#define BLE_CUST_SVC_MEAS_TAZAX_CHAR_UUID_PART UINT16_C(0x1D33) //!< TAZAX characteristics UUID
#define BLE_CUST_SVC_MEAS_VBAT_CHAR_UUID_PART UINT16_C(0x1D34)  //!< VBAT characteristics UUID

/// Custom service 3bsac86d-xxxx-4876-8b9d-f5799cfa02ba
/// Custom base UUID part 1
#define BLE_CUST_SVC_MEAS_BASE_UUID_PART1 0xba, 0x02, 0xfa, 0x9c, 0x79, 0xf5, 0x9d, 0x8b, 0x76, 0x48
/// Custom base UUID part 2
#define BLE_CUST_SVC_MEAS_BASE_UUID_PART2 0x6d, 0xc8, 0xda, 0x3b

/// Macro for building Custom UUID
#define BLE_CUST_SVC_MEAS_BUILD(part) BLE_CUST_SVC_MEAS_BASE_UUID_PART1, RBK_SMP290_CONV_U16_TO_BYTES(part), BLE_CUST_SVC_MEAS_BASE_UUID_PART2

/// Macro for Building the Custom Service UUID
#define BLE_CUST_SVC_MEAS_SERVICE_UUID BLE_CUST_SVC_MEAS_BUILD(BLE_CUST_SVC_MEAS_SERVICE_UUID_PART)

/// Macro for Building the Custom Characteristics 31  UUID
#define BLE_CUST_SVC_MEAS_T_CHAR_UUID BLE_CUST_SVC_MEAS_BUILD(BLE_CUST_SVC_MEAS_T_CHAR_UUID_PART)

/// Macro for Building the Custom Characteristics 32  UUID
#define BLE_CUST_SVC_MEAS_TPAZ_CHAR_UUID BLE_CUST_SVC_MEAS_BUILD(BLE_CUST_SVC_MEAS_TPAZ_CHAR_UUID_PART)

/// Macro for Building the Custom Characteristics 33  UUID
#define BLE_CUST_SVC_MEAS_TAZAX_CHAR_UUID BLE_CUST_SVC_MEAS_BUILD(BLE_CUST_SVC_MEAS_TAZAX_CHAR_UUID_PART)

/// Macro for Building the Custom Characteristics 34  UUID
#define BLE_CUST_SVC_MEAS_VBAT_CHAR_UUID BLE_CUST_SVC_MEAS_BUILD(BLE_CUST_SVC_MEAS_VBAT_CHAR_UUID_PART)


#define BLE_CUST_SVC_MEAS_CCC_BUFF_SIZE    UINT8_C(3)      //!< Ble Indication buffer size
#define BLE_CUST_SVC_MEAS_CCC_BUFF_SIZE_DOUBLE    UINT8_C(7)      //!< Ble Indication buffer size
#define BLE_CUST_SVC_BLE_TMR_INTERVAL UINT32_C(1000)  //!< Ble Indication timer interval in ms    1 sec

#define BLE_CUST_SVC_READ_REQ  0x01u  //!< Custom service read request
#define BLE_CUST_SVC_WRITE_REQ 0x02u  //!< Custom service write request

/// TSD default value



/******************************************************************************\
 * Types
 \******************************************************************************/

/// Custom service handle values
typedef enum
{
    //Custom Service
	BLE_CUST_SVC_MEAS_SVC_HNDL = BLE_CUST_SVC_MEAS_START_HNDL, //!< Custom service declaration
    BLE_CUST_SVC_MEAS_T_CHAR_HNDL,                    //!< Custom Characteristic 1 Handle  0x2901
    BLE_CUST_SVC_MEAS_T_CHAR_DATA_HNDL,               //!< Custom Characteristic 1 Data Handle  0x2901
    BLE_CUST_SVC_MEAS_T_CHAR_CUD_HNDL,                //!< Custom Characteristic 1 Characteristic User Description 0x2901
    BLE_CUST_SVC_MEAS_T_CHAR_CCC_HNDL,                 //!< Custom Characteristic 1 Client Characteristics Configuration 0x2902
    BLE_CUST_SVC_MEAS_TPAZ_CHAR_HNDL,                    //!< Custom Characteristic 2 Handle  0x2901
    BLE_CUST_SVC_MEAS_TPAZ_CHAR_DATA_HNDL,               //!< Custom Characteristic 2 Data Handle  0x2901
    BLE_CUST_SVC_MEAS_TPAZ_CHAR_CUD_HNDL,                //!< Custom Characteristic 2 Characteristic User Description 0x2901
    BLE_CUST_SVC_MEAS_TPAZ_CHAR_CCC_HNDL,                 //!< Custom Characteristic 2 Client Characteristics Configuration 0x2902
    BLE_CUST_SVC_MEAS_TAZAX_CHAR_HNDL,                    //!< Custom Characteristic 3 Handle  0x2901
    BLE_CUST_SVC_MEAS_TAZAX_CHAR_DATA_HNDL,               //!< Custom Characteristic 3 Data Handle  0x2901
    BLE_CUST_SVC_MEAS_TAZAX_CHAR_CUD_HNDL,                //!< Custom Characteristic 3 Characteristic User Description 0x2901
    BLE_CUST_SVC_MEAS_TAZAX_CHAR_CCC_HNDL,                 //!< Custom Characteristic 3 Client Characteristics Configuration 0x2902
    BLE_CUST_SVC_MEAS_VBAT_CHAR_HNDL,                    //!< Custom Characteristic 4 Handle  0x2901
    BLE_CUST_SVC_MEAS_VBAT_CHAR_DATA_HNDL,               //!< Custom Characteristic 4 Data Handle  0x2901
    BLE_CUST_SVC_MEAS_VBAT_CHAR_CUD_HNDL,                //!< Custom Characteristic 4 Characteristic User Description 0x2901
    BLE_CUST_SVC_MEAS_VBAT_CHAR_CCC_HNDL,                 //!< Custom Characteristic 4 Client Characteristics Configuration 0x2902
	BLE_CUST_SVC_MEAS_MAX_HNDL
} measSvc_ten;

// Restore default pack
#pragma pack()

/// Type definition of the custom service callback
typedef void (*measSvcAppCbk)(uint8_t req, rbk_smp290_ble_attsHndl hndl, uint8_t *value, uint16_t len);

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
void addCustmeasSvc(void);
/**
 * @brief  This \glos{API} removes the custom service from the attribute database.
 * return void
 */
void rmCustmeasSvc(void);

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
rbk_smp290_ble_atts_err_ten custmeasSvc_rdCallBack(rbk_smp290_ble_attsConnId connId, rbk_smp290_ble_attsHndl handle, uint8_t Op, uint16_t offset,
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
rbk_smp290_ble_atts_err_ten custmeasSvc_wrCallBack(rbk_smp290_ble_attsConnId connId, rbk_smp290_ble_attsHndl handle, uint8_t Op, uint16_t offset,
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
void custmeasSvc_procCccEvt(rbk_smp290_ble_atts_CccVal_ten cccVal, rbk_smp290_ble_attsHndl hndl, uint8_t idx);

/**
 * @brief This \glos{API} to processes the indication confirmation.
 *
 * return void
 */
void custmeasSvc_indication_confiramtion(void);

/**
 * @brief This \glos{API} to registers the application-specific callback.
 *
 * @param  cbk  The callback function.
 *
 * return void
 */
void custmeasSvcRegAppCbk(measSvcAppCbk cbk);


/**
 * @brief Meas done callback
 * @param status self return success or error values
 */
void entry_ConnSnsrClbk( rbk_smp290_snsr_err_ten status);

#endif /* _BLE_CUSTSVC_H */

/** @} */
