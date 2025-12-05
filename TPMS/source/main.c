/**
 * @addtogroup   measure_advertise_conn
 * @brief        This is the \ref measure_advertise_conn example.
 * @{
 * @file         main.c
 * @brief        This is the main entry-point of the project \ref measure_advertise_conn.
 * @details      This file contains the main entry-point of the project. It initializes 
 *               the project, sets up the sensor driver, and starts the task.
 *               The project measures various sensor values and sends the data over BLE advertising.
 *               It also provides instructions on how to build and flash the project, 
 *               as well as how to capture the BLE advertising data.
 * @copyright    (c) [Robert Bosch GmbH] [2024].
 *               All rights reserved, also regarding any disposal,
 *               exploitation, reproduction, editing, distribution, as well
 *               as in the event of applications for industrial property
 *               rights. The communication of its contents to others without
 *               express authorization is prohibited. Offenders will be held
 *               liable for the payment of damages. All rights reserved in
 *               the event of the grant of a patent, utility model or design.
 *
 * @example      measure_advertise_conn
 * @details      This is the measure_advertise_conn example project.
 * @par{Introduction}
 *  - The example:
 *   + Starts with the sleep mode enabled.
 *   + Creates Task1.
 *   + Task1 \b Q_ENTRY_SIG event performs the \glos{GATT} and sequence initializations.
 *     * The \glos{GATT} initialization performs performs the setup of the attribute server and the services.
 *     * The sequence initialization creates a periodic timer. This timer triggers Task1 every \b SEQ_UPDATE_PERIOD_US [us]
 *       when the sequence is resumed.
 *   + \ref rbk_smp290_ble_evtCbk receives the event \ref RBK_SMP290_BLE_STACK_INITIALIZED when the internal BLE initialization
 *     is done. It then initializes  BLE advertisement.
 *      * The advertisement type is \ref RBK_SMP290_BLE_ADV_CONN_UNDIRECT
 *        @note Transmission output power temperature compensation is activated.
 *
 *      * When the BLE initialization is done, the sequence is resumed.
 *   + The initialization flow is depicted in the following diagram:
 *     @msc
 *      hscale = "2";
 *      Task1, ble_inin, rbk_smp290_ble_evtCbk, sequence;
 *
 *      Task1=>Task1 [label="gatt_init", URL="\ref gatt_init"];
 *      Task1=>Task1 [label="sequence_init", URL="\ref sequence_init"];
 *        ---  [ label = "Task1 ready", ID="*" ];
 *      ble_inin=>rbk_smp290_ble_evtCbk [label="wait for RBK_SMP290_BLE_STACK_INITIALIZED", URL="\ref RBK_SMP290_BLE_STACK_INITIALIZED"];
 *      rbk_smp290_ble_evtCbk=>rbk_smp290_ble_evtCbk [label="adv_init", URL="\ref adv_init"];
 *        ---  [ label = "BLE ready", ID="*" ];
 *      rbk_smp290_ble_evtCbk=>sequence [label="sequence_resume", URL="\ref sequence_resume"];
 *      sequence=>Task1 [label="SIG_TIMER_TICK"];
 *      Task1=>Task1 [label="sequence_run", URL="\ref sequence_run"];
 *     @endmsc
 *
 *   + On every trigger of Task1 by the sequence timer through \b SIG_TIMER_TICK, Task1 runs the sequence logic.
 *     The sequence steps are as follows:
 *     * [  0 ms] Measure T.
 *     * [100 ms] Measure Tpaz.
 *     * [200 ms] Measure Tazax Low.
 *     * [300 ms] Measure Tazax High.
 *     * [400 ms] Measure Vbat.
 *     * [500 ms] Adv. data.
 *     * [600 ms] Do nothing.
 *     * [700 ms] Do nothing.
 *     * [800 ms] Do nothing.
 *     * [900 ms] Do nothing.
 *   + It then loops back to the beginning. This is illustrated in the following sequence diagram:
 *     @ref_image{measure_advertise_conn_sequence.svg}
 *   + The SMP290 then goes to sleep to perform the measurement. After every measurement scheduling, the SMP290 transitions
 *     to sleep to acquire the requested samples, then wakes up from sleep and notifies Task1 through the initialization callback
 *     \b entry_snsrClbk.
 *   + Task1 then gets the measurement done event, captures the measurement status, and gets the measurement values.
 *   + When all the measurements are done, the advertising Data frame is constructed and then sent over the configured \glos{BLE}
 *     advertising. The Advertisement data is constructed as follows:
 *     @ref_image{measure_advertise_advData.svg}
 *     @note To translate the sensor values into physical values, you can refer to the SMP290 datasheet.
 *
 *   + When the device receives a SCAN_REQ, it answers with a Scan Response. The Scan Response data
 *     is constructed as follows:
 *     @ref_image{measure_advertise_conn_scanrsp_data.svg}
 *   + The project uses the following services and characteristics in a profile:
 *     * The Generic Access Service (GAP Service):
 *       @ref_image{gap_service.svg}
 *     * The Generic Attribute Service (GATT Service):
 *       @ref_image{gatt_service.svg}
 *     * Custom Service:
 *       @ref_image{custom_service.svg}

 * @endpar
 * @par{Steps}
 *  - First, the project has to be [built](@ref build_project) and [flashed](@ref flash_project).
 *  - To capture the BLE Advertising data, use any \glos{BLE} sniffing tool or app.
 * @endpar
 * @par{Output}
 *  @ref_image{measure_advertise_sniffer.svg}
 * @endpar
 * @par{Code}
 *  @include measure_advertise_conn/source/main.c
 *  @include measure_advertise_conn/source/task.c
 *  @include measure_advertise_conn/source/sequence.c
 *  @include measure_advertise_conn/source/adv.c
 *  @include measure_advertise_conn/source/gap.c
 *  @include measure_advertise_conn/source/gatt.c
 *  @include measure_advertise_conn/source/ble_gapSvc.c
 *  @include measure_advertise_conn/source/ble_gattSvc.c
 *  @include measure_advertise_conn/source/ble_custSvc.c
 * @endpar
 **/

/* Library includes */
#include "rbk_smp290_entry.h"
#include "rbk_smp290_pml.h"
#include "rbk_smp290_snsr.h"
#include "rbk_smp290_types.h"
#include "rbk_smp290_tsd.h"
#include "rbk_smp290_ble.h"
/* Project includes */
#include "main.h"

/******************************************************************************\
 *  Global variables
 \******************************************************************************/
/// @defgroup measure_advertise_conn_proj_cfg Project configuration
/// @{

/// Project config
const rbk_smp290_featCfg_tst config = {
    .enableJtag       = false,                               // Disable JTAG debugging
    .enableUartPrintf = false,                                // Enable UART printf functionality
    .enableUartApp    = false,                               // Disable UART functionality
    .enableI2cMaster  = false,                               // Disable I2C functionality
    .enableSnsr       = true,                                // Enable sensor functionality
    .enableBle        = true,                                // Enable BLE functionality
    .bleFeatCfg       = {.broadcaster = false,               // Disable BLE broadcaster role
                   .observer    = false,               // Disable BLE observer role
                   .peripheral  = true,                // Enable BLE peripheral role
                   .security    = false,               // Disable BLE security features
                   .privacy     = false,               // Disable BLE privacy features
                   .stackCfg =
                       {
                           .aclBuffLen       = RBK_SMP290_BLE_ACL_MAX_LEN,              // ACL buffer length configuration
                           .numOfTxBuff      = 1,                                       // Number of ACL TX buffers
                           .numOfRxBuff      = 1,                                       // Number of ACL RX buffers
                           .filtAcptListSize = RBK_SMP290_BLE_MAX_FILT_ACPT_LIST_SIZE,  // Filter accept list size
                           .rslvListSize     = RBK_SMP290_BLE_MAX_RSLV_LIST_SIZE,       // Resolving list size
                           .bleTmrCnt        = RBK_SMP290_BLE_TMR_MAX_CNT,              // BLE timer count
                           .hashableAttrLen  = 1500,                                     // GATT hashable attribute length
                       }},
};

/// @}

/******************************************************************************\
 *  Public functions
\******************************************************************************/
// Project boot after Reset logic
void rbk_smp290_entry_initAfterReset(void)
{
    // Print project name
    printf("Project: %s\r\n", PROJECT_NAME);

    // Sensor driver initialization
    (void)rbk_smp290_snsr_inin(entry_snsrClbk);
    // BLE stack initialization
    rbk_smp290_ble_stack_inin();
    //Self-test initialization
    rbk_smp290_slftst_inin(entry_slftstClbk);
    // Enable Sleep
    (void)rbk_smp290_pml_setAutoSleepMod(RBK_SMP290_PML_SLEEP);

    // Create and start task
    task_creatAndStrt();
}

// Project boot after Sleep logic
void rbk_smp290_entry_initAfterSleep(void)
{
    // No action after sleep
}

/** @} */
