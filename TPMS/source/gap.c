/**
 * @addtogroup  measure_advertise_conn
 * @{
 * @file         gap.c
 * @brief        Handling the ble generic access profile callback events of the project \ref measure_advertise_conn
 * @details      - This file contains the implementation of the BLE Generic Access Profile (GAP) callback events.
 *               It handles various events in the \ref rbk_smp290_ble_evtCbk function.
 *               The \ref rbk_smp290_ble_evtCbk function is called by the BLE stack to notify the application about these events.
 *               The function prints debug messages for certain events.
 *               This file also defines the \a UpdatedConnPrm structure to store the updated connection parameters.
 *               It also performs specific actions based on the event type, such as stopping measurement, resetting application data,
 *               accepting remote connection parameter requests, etc.
 *               - This file provides the necessary constants and types for
 *               configuring the connection parameters of a BLE device, such as the connection
 *               interval, connection latency, supervision timeout, and connection idle
 *               period. It also includes the required header files for the BLE types and
 *               entry points. The constants defined in this file are the default values for
 *               the connection parameters, which can be modified as per the application requirements.
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

/* Library includes */
#include "rbk_smp290_ble.h"
#include "rbk_smp290_types.h"

/* Project includes */
#include "ble_custSvc.h"
#include "ble_gpioSvc.h"
#include "ble_maintSvc.h"
#include "main.h"

/// @defgroup measure_advertise_conn_conn_params BLE Connection parameter definitions
/// @{

/******************************************************************************\
 *  Constants
 \******************************************************************************/

/// Minimum default connection interval range: 0x0006 to 0x0C80 Time = N * 1.25
/// ms. Time range: 7.5 ms to 4 s. 40 * 1.25 = 50 ms.
#define BLE_DFLT_MIN_CONN_INT 16u

/// Maximum default connection interval range: 0x0006 to 0x0C80 Time = N * 1.25
/// ms. Time range: 7.5 ms to 4 s. 40 * 1.25 = 50 ms.
#define BLE_DFLT_MAX_CONN_INT 16u

/// Maximum peripheral latency for the connection in number of connection
/// events. Range: 0x0000 to 0x01F3.
#define BLE_DFLT_CONN_LTCY 0u

/// Supervision timeout for the LE Link.
/// Range: 0x000A to 0x0C80 Time = N * 10 ms.
/// Time range: 100 ms to 32 s. 500 * 10 = 5000 ms.
#define BLE_DFLT_SUP_TMOUT 500u

/// Connection idle time in ms before attempting connection parameter update.
/// Set to 0 to disable.
#define BLE_DFLT_CONN_IDLE_PERD 5000u

/*******************************************************************************
 *  Global variables
 ******************************************************************************/

/// Default connection parameters
static rbk_smp290_ble_connParam_tst dflConnPrm = {
    .connIntrvMin = BLE_DFLT_MIN_CONN_INT,
    .connIntrvMax = BLE_DFLT_MAX_CONN_INT,
    .connLatency  = BLE_DFLT_CONN_LTCY,
    .supTimeout   = BLE_DFLT_SUP_TMOUT,
};

/// Default connection idle time
SECTION_PERSISTENT static uint32_t dflConnIdlTime = BLE_DFLT_CONN_IDLE_PERD;

/// Updated connection parameters
SECTION_PERSISTENT static rbk_smp290_ble_connParam_tst UpdatedConnPrm;

/// @}

/*******************************************************************************
 *  Function definition
 ******************************************************************************/

// BLE event call back function
void rbk_smp290_ble_evtCbk(rbk_smp290_ble_evtTyp_ten evt, void *msg_p)
{
    switch (evt)
    {
        case RBK_SMP290_BLE_STACK_INITIALIZED:
        {
            // Initialize Advertisement configurations.
            adv_init();
            // Resume the sequence
            sequence_resume();
        }
        break;

        case RBK_SMP290_BLE_GAP_ADV_START:
        {
            // Log that advertising has started
            smp290_log(LOG_VERBOSITY_TRACE, "\t\tGAP: Adv. started\r\n");

            rbk_smp290_ble_addrTyp_ten addrTyp = rbk_smp290_ble_gap_addr_getTyp();
            rbk_smp290_ble_addr curAddr;
            (void)rbk_smp290_ble_gap_addr_getCurr(curAddr);

            // Log the address type
            smp290_log_append(LOG_VERBOSITY_TRACE, "\t\t\t\t Current address type: %d\r\n", addrTyp);
            // Log the address (in reverse order)
            smp290_log_append(LOG_VERBOSITY_TRACE, "\t\t\t\t Current address: ");

            for (uint8_t i = RBK_SMP290_BLE_ADDR_LEN; i > 0; i--)
            {
                smp290_log_append(LOG_VERBOSITY_TRACE, "%02X ", curAddr[i - 1]);
            }
            smp290_log_append(LOG_VERBOSITY_TRACE, "\r\n");
        }
        break;

        case RBK_SMP290_BLE_GAP_ADV_STOP:
        {
            // Log that advertising has stopped
            smp290_log(LOG_VERBOSITY_TRACE, "\t\tGAP: Adv. stopped\r\n");
        }
        break;

        case RBK_SMP290_BLE_GAP_CONNECTED:
        {
            rbk_smp290_ble_connComplEvt_tst const *connEvt = (rbk_smp290_ble_connComplEvt_tst *)msg_p;

            smp290_log(LOG_VERBOSITY_TRACE, "\t\tGAP: Connected\r\n");
            smp290_log_append(LOG_VERBOSITY_TRACE, "\t\t\t\tconnEvt.connInterval: %d\r\n", connEvt->connIntrv);
            smp290_log_append(LOG_VERBOSITY_TRACE, "\t\t\t\tconnEvt.connLatency:  %d\r\n", connEvt->connLatency);
            smp290_log_append(LOG_VERBOSITY_TRACE, "\t\t\t\tconnEvt.supTimeout:   %d\r\n", connEvt->supTimeout);

            // Check if we should do connection parameter update
            if ((connEvt->connIntrv < dflConnPrm.connIntrvMin) || (connEvt->connIntrv > dflConnPrm.connIntrvMax) ||
                (connEvt->connLatency != dflConnPrm.connLatency) || (connEvt->supTimeout != dflConnPrm.supTimeout))
            {
                // Capture the new Connection Parameters
                dflConnPrm.connIntrvMin = connEvt->connIntrv;
                dflConnPrm.connIntrvMax = connEvt->connIntrv;
                dflConnPrm.connLatency  = connEvt->connLatency;
                dflConnPrm.supTimeout   = connEvt->supTimeout;
                // Set the Connection Parameters
                (void)rbk_smp290_ble_gap_conn_paramUpdate(&dflConnPrm, dflConnIdlTime);
            }
            // Stop Measurement
            sequence_stop();
            connected = true;
        }
        break;

        case RBK_SMP290_BLE_GAP_DISCONNECTED:
        {
            const rbk_smp290_ble_disConnComplEvt_tst *connClosedEvt = (const rbk_smp290_ble_disConnComplEvt_tst *)msg_p;
            (void)(connClosedEvt);
            // Reset the custom service application data
            ble_indicnCntr = 0;
            smp290_log(LOG_VERBOSITY_TRACE, "\t\tGAP: Disconnected: Reason: 0X%2X\r\n", connClosedEvt->reason);
            // Resume the sequence
            connected = false;
            sequence_resume();
        }
        break;

        case RBK_SMP290_BLE_GAP_CONN_PARAM_UPDATE_COMPLD:
        {
            const rbk_smp290_ble_connParamUpdateComplEvt_tst *connUpdateEvt = (const rbk_smp290_ble_connParamUpdateComplEvt_tst *)msg_p;

            // Save the new connection parameters
            UpdatedConnPrm.connIntrvMin = connUpdateEvt->connIntrv;
            UpdatedConnPrm.connIntrvMax = connUpdateEvt->connIntrv;
            UpdatedConnPrm.connLatency  = connUpdateEvt->connLatency;
            UpdatedConnPrm.supTimeout   = connUpdateEvt->supTimeout;

            smp290_log(LOG_VERBOSITY_TRACE, "\t\tGAP: Connection parameters updated\r\n");
            smp290_log_append(LOG_VERBOSITY_TRACE, "\t\t\t\tUpdated Connection Interval: %d\r\n", connUpdateEvt->connIntrv);
            smp290_log_append(LOG_VERBOSITY_TRACE, "\t\t\t\tUpdated Connection Latency:  %d\r\n", connUpdateEvt->connLatency);
            smp290_log_append(LOG_VERBOSITY_TRACE, "\t\t\t\tUpdated Supervision Timeout: %d\r\n", connUpdateEvt->supTimeout);
        }
        break;

        case RBK_SMP290_BLE_GAP_RMT_CONN_PARAM_REQ:
        {
            const rbk_smp290_ble_rmtConnParamReqEvt_tst *connPrmRemReqEvt = (const rbk_smp290_ble_rmtConnParamReqEvt_tst *)msg_p;

            smp290_log(LOG_VERBOSITY_TRACE, "\t\tGAP: Remote connection parameter request: ");

            // Accept the incoming connection request
            rbk_smp290_ble_err_ten ret = rbk_smp290_ble_gap_conn_acceptRmtParamReq(&connPrmRemReqEvt->connPrm);
            if (RBK_SMP290_BLE_SUCCESS != ret)
            {
                // Invalid parameters
                smp290_log_append(LOG_VERBOSITY_TRACE, "invalid\r\n");
            }
            else
            {
                // Valid parameters
                smp290_log_append(LOG_VERBOSITY_TRACE, "valid\r\n");
            }
        }
        break;

        case RBK_SMP290_BLE_GAP_READ_RSSI:

        case RBK_SMP290_BLE_GAP_READ_RMT_FEAT:

        case RBK_SMP290_BLE_GAP_SCAN_START:

        case RBK_SMP290_BLE_GAP_SCAN_STOP:

        case RBK_SMP290_BLE_GAP_SCAN_RPRT:

        case RBK_SMP290_BLE_SM_PAIRING_REQ:

        case RBK_SMP290_BLE_SM_PAIRING_FAILED:

        case RBK_SMP290_BLE_SM_PAIRING_CMPLD:

        case RBK_SMP290_BLE_GAP_PRIV_ENABLED:

        case RBK_SMP290_BLE_GAP_PRIV_DISABLED:

        case RBK_SMP290_BLE_GAP_SCAN_REQ_RXD:

            // Not applicable to this sample project

        default:
        {
            // Nothing to do for other events
        }
        break;
    }
}

/** @} */
