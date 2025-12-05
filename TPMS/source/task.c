/**
 * @addtogroup    measure_advertise_conn
 * @{
 * @file         task.c
 * @brief        This file contains the task definitions of the project \ref measure_advertise_conn
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
#include "rbk_smp290_qpc.h"
#include "rbk_smp290_types.h"

/* Project includes */
#include "main.h"
#include "ble_measSvc.h"

/// @defgroup measure_advertise_conn_qpc_sigs Task signals
/// @{

/******************************************************************************\
 *  Constants
\******************************************************************************/
/// Maximum number of events for the Task1
#define EVENTS_NUM_TASK1 (6u)

/******************************************************************************\
 *  Global variables
\******************************************************************************/
/// Event queue for the Task1 task.
static SECTION_NP_NOINIT const QEvt *eveQ_task1[EVENTS_NUM_TASK1];

/// Task1 active object.
static rbk_smp290_qpc_actObj_t actObj_task1;

/// Connected
bool connected = false;
/// @}


/******************************************************************************\
 *  Private functions prototypes
\******************************************************************************/
static QState iniSt_task1(rbk_smp290_qpc_actObj_t *const me);
static QState actSt_task1(rbk_smp290_qpc_actObj_t *const me, QEvt *pEvt);

/******************************************************************************\
 *  Private functions definition
\******************************************************************************/
/**
 * @brief Initial state machine state. The initial transition function performs
 * the actions of the initial transition and initializes the state variable to
 * the default state.
 * @param[in, out] me pointer of task object
 * @return QState: conveys the status of the event handling to the event
 * processor.
 */
static QState iniSt_task1(rbk_smp290_qpc_actObj_t *const me)
{
    //
    return Q_TRAN(&actSt_task1);
}

/**
 * @brief Active state machine state.
 * @param [in, out] me pointer of task object
 * @param [in] pEvt event pointer
 * @return QState: conveys the status of the event handling to the event
 * processor.
 */
static QState actSt_task1(rbk_smp290_qpc_actObj_t *const me, QEvt *pEvt)
{
    QState qstatus                     = Q_HANDLED();
    const rbk_smp290_qpc_event *pEvent = ((const rbk_smp290_qpc_event *)pEvt);

    switch (pEvent->super.sig)
    {
        case Q_ENTRY_SIG:
        {
            gatt_init();
            sequence_init();

            // Init of BLE security and advertising will be done by rbk_smp290_ble_evtCbk
            // after RBK_SMP290_BLE_STACK_INITIALIZED is received

            // At the end of BLE initialization as mentioned above, the sequence
            // will be started by calling sequence_resume
        }
        break;

        case SIG_TIMER_TICK:
        {
            sequence_run();
        }
        break;

        case SIG_MEASMT_DONE:
        {
            rbk_smp290_snsr_err_ten status = *(rbk_smp290_snsr_err_ten *)pEvent->params;
            sequence_getOutVals(status);
        }
        break;

        case SIG_ADV:
        {
            ble_sensorData_tst const *adv_sensorData_p = (ble_sensorData_tst *)pEvent->params;
            adv_prepSrvData(adv_sensorData_p);
            adv_doAdv();
        }
        break;
        case Q_EXIT_SIG:
        case Q_INIT_SIG:
        default:
        {
            qstatus = Q_SUPER(&QHsm_top);
        }
        break;
    }

    return qstatus;
}

/******************************************************************************\
 *  Public functions
\******************************************************************************/
void entry_snsrClbk(rbk_smp290_snsr_err_ten status)
{
    if (connected)
    {
        entry_ConnSnsrClbk(status);
    }
    else
    {
        static rbk_smp290_snsr_err_ten snsr_status;

        snsr_status = status;
        // Post status to Task1
        task_postEvent(SIG_MEASMT_DONE, &snsr_status);
    }
    
}

void task_creatAndStrt(void)
{
    // Create and start task1
    rbk_smp290_qpc_tskCreatAndStrt(EVENTS_NUM_TASK1, (QEvt const **)&eveQ_task1[0], &actObj_task1, iniSt_task1);
}

void task_postEvent(enum_t signal, void *pParams)
{
    // Post signal
    rbk_smp290_qpc_postEve(&actObj_task1, signal, pParams);
}

void task_postEventFromIsr(enum_t signal, void *pParams)
{
    // Post signal from ISR
    rbk_smp290_qpc_postEveFromIsr(&actObj_task1, signal, pParams);
}

/** @} */
