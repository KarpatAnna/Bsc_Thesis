/**
 * @addtogroup   measure_advertise_conn
 * @{
 * @file         main.h
 * @brief        \glos{API}s of the project \ref measure_advertise_conn.
 * @copyright    (c) [Robert Bosch GmbH] [2024].
 *               All rights reserved, also regarding any disposal,
 *               exploitation, reproduction, editing, distribution, as well
 *               as in the event of applications for industrial property
 *               rights. The communication of its contents to others without
 *               express authorization is prohibited. Offenders will be held
 *               liable for the payment of damages. All rights reserved in
 *               the event of the grant of a patent, utility model or design.
**/

#ifndef _MAIN_H
#define _MAIN_H

/* Library includes */
#include "rbk_smp290_snsr_types.h"
#include "rbk_smp290_qpc.h"
#include "rbk_smp290_printf.h"
#include "rbk_smp290_timer.h"
#include "rbk_smp290_gpio.h"
#include "rbk_smp290_slftst.h"

/******************************************************************************\
 * Types
 \******************************************************************************/
/// @addtogroup measure_advertise_conn_adv_cfg BLE Adv. configuration definitions
/// @{

/// BLE advertisement sensor data within the Manufacturer Specific Adv. structure
// Pack the following struct
#pragma pack(1)
typedef struct
{
    int16_t p_out;          //!< Pressure
    int16_t T_out;          //!< Temperature
    int16_t Az_lo_out;      //!< Az low
    int16_t Az_hi_out;      //!< Az hi
    int16_t Ax_lo_out;      //!< Ax low
    int16_t Ax_hi_out;      //!< Ax hi
    int16_t Vbat_out;       //!< Vbat
    uint8_t error;          //!< error status
    uint8_t frame_counter;  //!< frame counter
} ble_sensorData_tst;

// Restore default pack
#pragma pack()
/// @}

/// @addtogroup measure_advertise_conn_qpc_sigs Task signals
/// @{

/// Task signals
typedef enum
{
    SIG_ENTRY = QPC_FIRST_USER_SIGNAL,  //!<  First signal that can be used for user signals
    SIG_TIMER_TICK,                     //!<  Signal triggered when the sequence timer is elapsed
    SIG_MEASMT_DONE,                    //!<  Signal triggered by the measurement callback
    SIG_ADV                             //!<  Signal to trigger Adv. and publish measurement results
} proj_qpcTaskSig_ten;

/// @}

/******************************************************************************\
 * Extern global variables
 \******************************************************************************/
/// @addtogroup measure_advertise_conn_adv_cfg BLE Adv. configuration definitions
/// @{

/// Desired MTU size (used by adv.c and gatt.c)
extern uint16_t ble_mtu_size;

/// Connected
extern bool connected;
/// @}

/******************************************************************************\
 * Public functions
 \******************************************************************************/
/**
 * @brief  Create and start task1
 * return  void
 */
void task_creatAndStrt(void);

/**
 * @brief   Post a signal to task1
 * @param   signal to post
 * @param   pParams data to post
 * return   void
 */
void task_postEvent(enum_t signal, void *pParams);

/**
 * @brief   Post a signal to task1 from an ISR context.
 * @param   signal to post
 * @param   pParams data to post
 * return   void
 */
void task_postEventFromIsr(enum_t signal, void *pParams);

/**
 * @brief   Sensor driver callback
 * @param   status
 * return   void
 */
void entry_snsrClbk(rbk_smp290_snsr_err_ten status);

/**
 * @name entry_slftstClbk measure_advertise
 * @brief Sensor driver callback
 * @param status
 */
void entry_slftstClbk(rbk_smp290_slftst_err_ten status);

/**
 * @brief    Initializes the sequence.
 * @details  This function initializes the sequence by creating and enabling
 *           the sequence timer.
 * return    void
 */
void sequence_init(void);

/**
 * @brief    Runs the sequence.
 * @details  This function runs the sequence by performing the measurements
 *           based on the current sequence iterator. It also handles failed
 *           measurements and updates the sequence iterator.
 * return    void
 */
void sequence_run(void);

/**
 * @brief    Retrieve the output values after an iteration of the sequence.
 * @details  This function retrieves the output values from the sequence based
 *           on the current sequence iterator and updates the adv_sensorData
 *           structure.
 * @param    status: status of the last measurement sequence iteration.
 * return    void
 */
void sequence_getOutVals(rbk_smp290_snsr_err_ten status);

/**
 * @brief    Resumes the sequence.
 * @details  This function resets the sequence iterator and restarts the sequence timer.
 * return    void
 */
void sequence_resume(void);

/**
 * @brief    Stops the sequence.
 * @details  This function disables the sequence timer which halts its execution.
 * return    void
 */
void sequence_stop(void);

/**
 * @brief   Initializes the advertising parameters and configurations.
 * @details This function is responsible for initializing the BLE advertising parameters and configurations.
 *          It sets the desired MTU size, sets the public address, configures the TX power, sets the advertising interval
 *          and duration, sets the advertising channel, sets the advertising type, and enables the temperature compensation TX power.
 * @note    This function should be called before starting the advertising process.
 *          This is done, if the callback-function \ref rbk_smp290_ble_evtCbk
 *          is called with the event \ref RBK_SMP290_BLE_STACK_INITIALIZED.
 *
 * return   void
 */
void adv_init(void);

/**
 * @brief   Prepares the advertising data payload.
 * @details This function is responsible for preparing the advertising data payload.
 *          It constructs the advertising data payload using specific structures defined in this file.
 *          The payload includes flags, sensor data, and appearance.
 *          The advertising data payload is then stored in a global variable for reuse.
 * @note    This function should be called before starting the advertising process.
 * @param   adv_sensorData_p Pointer to the sensor data structure.
 * return   void
 */
void adv_prepSrvData(ble_sensorData_tst const *adv_sensorData_p);

/**
 * @brief   Starts the advertising process.
 * @details This function is responsible for starting the BLE advertising
 * process. It passes the advertising data payload to the RBK SMP290 BLE library
 * for advertising. The advertising process is then started using the RBK SMP290
 * BLE library.
 * @note    This function should be called after preparing the advertising data
 * payload. 
 *
 * return   void
 */
void adv_doAdv(void);

/**
 * @brief  Initializes the \glos{GATT} profile.
 * @details This function initializes the attribute server, sets the ACL MAX length,
 *          adds the Generic Access Service, Generic Attribute Service, and Custom Service,
 *          and registers the Client Characteristics Configuration Descriptor (CCCD).
 * @note   This function should be called before using the GATT profile.
 *
 * return   void
 */
void gatt_init(void);

#endif  // _MAIN_H

/** @} */
