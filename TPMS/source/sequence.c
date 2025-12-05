/**
 * @addtogroup   measure_advertise_conn
 * @{
 * @file         sequence.c
 * @brief        This file contains the sensor measurement sequence of the project \ref measure_advertise_conn.
 * @details      The sequence module is responsible for managing the state machine
 *               sequence of the project. It defines the sequence update period,
 *               idle period, and the sequence definitions. It also includes functions
 *               to initialize, run, stop, and resume the sequence, as well as functions
 *               to handle failed measurements and retrieve output values from the sequence.
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
#include "rbk_smp290_boot.h"
#include "rbk_smp290_snsr.h"
#include "rbk_smp290_timer.h"
#include "rbk_smp290_types.h"

/* Project includes */
#include "main.h"

/// @addtogroup measure_advertise_conn_seq_cfg Sequence configuration definitions
/// @{

/******************************************************************************\
 *  Constants
 \******************************************************************************/

/******************************************************************************\
 * Periodic Sequences configuration constants
 \******************************************************************************/
/// Sequence update period [us].
#define SEQ_UPDATE_PERIOD_US MS_TO_US(100)

/// Sequence definitions
#define SEQ_T           0u   //!< Measurement T
#define SEQ_TPAZ        1u   //!< Measurement TpAz
#define SEQ_TAZAX_LO_LO 2u   //!< Measurement Tazax Low
#define SEQ_TAZAX_LO_HI 3u   //!< Measurement Tazax High
#define SEQ_VBAT        4u   //!< Measurement Vbat
#define SEQ_ADV         5u   //!< Adv. data
#define SEQ_MAX         10u  //!< Iterator max

#define SEQ_VBAT_NREP 1u                            //!< Measurement Vbat number of samples
#define SEQ_VBAT_TREP 0u                            //!< Measurement Vbat sample rate

/******************************************************************************\
 *  Global variables
\******************************************************************************/
/// Static variable to propagate the BLE Adv content.
SECTION_PERSISTENT static ble_sensorData_tst adv_sensorData;

/// Sequence iterator
SECTION_PERSISTENT static volatile uint32_t sequence_iter = SEQ_T;

/// Task 1 Timer Id
SECTION_PERSISTENT static int8_t sequence_timerId = -1;

/// Buffer for Battery voltage reading
static rbk_smp290_snsr_Vbat_buff_tst vbat_buff;

/// @}

/******************************************************************************\
 *  Functions declarations
\******************************************************************************/

/**
 * @brief Cancel the ongoing measurement depending on \ref sequence_iter.
 * @return the cancellation status
 */
static rbk_smp290_snsr_err_ten cancelMeasmt(void)
{
    rbk_smp290_snsr_err_ten ret = RBK_SMP290_SNSR_SUCCESS;

    switch (sequence_iter)
    {
        case SEQ_T:
        {
            ret = rbk_smp290_snsr_cncl_cmpd_T();
        }
        break;
        case SEQ_TPAZ:
        {
            ret = rbk_smp290_snsr_cncl_cmpd_p();
        }
        break;
        case SEQ_TAZAX_LO_LO:
        {
            ret = rbk_smp290_snsr_cncl_cmpd_az_ax();
        }
        break;
        case SEQ_TAZAX_LO_HI:
        {
            ret = rbk_smp290_snsr_cncl_cmpd_az_ax();
        }
        break;
        case SEQ_VBAT:
        {
            ret = rbk_smp290_snsr_cncl_Vbat();
        }
        break;
        default:
        {
            // Nothing to do
        }
        break;
    }
    return ret;
}

/**
 * @brief      Handles the failure of a measurement.
 * @details    This function is called when a measurement fails. It cancels the
 *             measurement and performs a software reset if necessary.
 * @param[in]  status  The status of the measurement.
 * return     None
 */
static void handleFailedMeasmt(rbk_smp290_snsr_err_ten status)
{
    rbk_smp290_snsr_err_ten ret = RBK_SMP290_SNSR_SUCCESS;

    if (RBK_SMP290_SNSR_ERR_BUSY == status)
    {
        // Cancel the ongoing measurement
        ret = cancelMeasmt();

        if (RBK_SMP290_SNSR_SUCCESS != ret)
        {
            smp290_log(LOG_VERBOSITY_ERROR, "Resetting! ");
            // In case of an expected error during cancellation, perform a SW Reset
            rbk_smp290_boot_swRst();
        }
        else
        {
            // Measurement cancelled
        }
    }
    else if (RBK_SMP290_SNSR_SUCCESS != status)
    {
        smp290_log(LOG_VERBOSITY_ERROR, "Resetting! ");
        // In case of errors other than Busy, perform a SW Reset
        rbk_smp290_boot_swRst();
    }
}

/**
 * @brief      Callback function for the sequence timer.
 * @details    This function is called when the sequence timer expires. It posts
 *             a timer tick event to the task.
 * @param[in]  status  The status of the timer.
 * return     None
 */
static void timerCallback(rbk_smp290_timerStatus_t status)
{
    (void)(status);
    task_postEventFromIsr((enum_t)SIG_TIMER_TICK, NULL);
}

// Initializes the sequence.
void sequence_init(void)
{
    // Create the sequence timer
    sequence_timerId = rbk_smp290_timer_create(SEQ_UPDATE_PERIOD_US, timerCallback);
    smp290_log(LOG_VERBOSITY_DEBUG, "\tSequence initialized\r\n");
}

// Run the sequence.
void sequence_run(void)
{
    rbk_smp290_snsr_err_ten ret = RBK_SMP290_SNSR_SUCCESS;
    bool isMeasmt               = true;
    smp290_log(LOG_VERBOSITY_DEBUG, "\tSequence step: ");
    switch (sequence_iter)
    {
        case SEQ_T:
        {
            smp290_log_append(LOG_VERBOSITY_DEBUG, "T\r\n");
            ret = rbk_smp290_snsr_meas_cmpd_T();
        }
        break;
        case SEQ_TPAZ:
        {
            smp290_log_append(LOG_VERBOSITY_DEBUG, "TpAz\r\n");
            ret = rbk_smp290_snsr_meas_cmpd_p(RBK_SMP290_SNSR_EN_ENABLE, RBK_SMP290_SNSR_EN_ENABLE);
        }
        break;
        case SEQ_TAZAX_LO_LO:
        {
            smp290_log_append(LOG_VERBOSITY_DEBUG, "Tazax_lo\r\n");
            ret = rbk_smp290_snsr_meas_cmpd_az_ax(RBK_SMP290_SNSR_EN_ENABLE, RBK_SMP290_SNSR_RANGE_LO, RBK_SMP290_SNSR_RANGE_LO);
        }
        break;
        case SEQ_TAZAX_LO_HI:
        {
            smp290_log_append(LOG_VERBOSITY_DEBUG, "Tazax_hi\r\n");
            ret = rbk_smp290_snsr_meas_cmpd_az_ax(RBK_SMP290_SNSR_EN_ENABLE, RBK_SMP290_SNSR_RANGE_HI, RBK_SMP290_SNSR_RANGE_HI);
        }
        break;
        case SEQ_VBAT:
        {
            smp290_log_append(LOG_VERBOSITY_DEBUG, "Vbat\r\n");
            /// Vbat sequence configuration
            static const rbk_smp290_snsr_cfg_Vbat_tst vbat_cfg = {.N_rep     = (uint8_t)SEQ_VBAT_NREP,
                                                                  .t_rep     = SEQ_VBAT_TREP,
                                                                  .osr       = RBK_SMP290_SNSR_OSR_4X,
                                                                  .Vbat_load = RBK_SMP290_SNSR_VBAT_LOAD_DISABLE};
            ret                                                = rbk_smp290_snsr_meas_and_get_Vbat(&vbat_cfg, &vbat_buff);
        }
        break;
        case SEQ_ADV:
        {
            smp290_log_append(LOG_VERBOSITY_DEBUG, "Adv\r\n");
            // Increment the frame counter
            adv_sensorData.frame_counter++;
            // Post event to prepare and start the advertising
            task_postEvent((enum_t)SIG_ADV, &adv_sensorData);
            isMeasmt = false;
        }
        break;
        default:
        {
            smp290_log_append(LOG_VERBOSITY_DEBUG, "Idle\r\n");
            // Until SEQ_MAX is reached, do nothing
            isMeasmt = false;
        }
        break;
    }

    // In case of a measurement sequence, check the status and take action if needed
    if (isMeasmt)
    {
        // Update the error
        adv_sensorData.error |= (uint8_t)ret;
        handleFailedMeasmt(ret);

        // The sequence iterator is incremented by the \ref sequence_getOutVals for measurement
    }
    else
    {
        // Increment the sequence iterator
        sequence_iter = (sequence_iter + 1u) % (SEQ_MAX);
    }
}

// Retrieve the output values after a measurement iteration of the sequence.
void sequence_getOutVals(rbk_smp290_snsr_err_ten status)
{
    // If the measurement fails, print a message
    if (RBK_SMP290_SNSR_SUCCESS != status)
    {
        smp290_log(LOG_VERBOSITY_WARNING, "\tMeasurement failed!(0X%02X)\r\n", status);
    }
    else
    {
        // Nothing to do
    }

    // In both cases, we retrieve the sensor values and increment the iterator.
    // If an error occurred, the sensor values will reflect this.
    switch (sequence_iter)
    {
        case SEQ_T:
        {
            adv_sensorData.T_out = rbk_smp290_snsr_get_cmpd_T();
        }
        break;
        case SEQ_TPAZ:
        {
            adv_sensorData.T_out     = rbk_smp290_snsr_get_cmpd_T();
            adv_sensorData.p_out     = rbk_smp290_snsr_get_cmpd_p();
            adv_sensorData.Az_hi_out = rbk_smp290_snsr_get_cmpd_az(RBK_SMP290_SNSR_RANGE_HI);
        }
        break;
        case SEQ_TAZAX_LO_LO:
        {
            adv_sensorData.T_out     = rbk_smp290_snsr_get_cmpd_T();
            adv_sensorData.Az_hi_out = rbk_smp290_snsr_get_cmpd_az(RBK_SMP290_SNSR_RANGE_LO);
            adv_sensorData.Ax_lo_out = rbk_smp290_snsr_get_cmpd_ax(RBK_SMP290_SNSR_RANGE_LO);
        }
        break;
        case SEQ_TAZAX_LO_HI:
        {
            adv_sensorData.T_out     = rbk_smp290_snsr_get_cmpd_T();
            adv_sensorData.Az_hi_out = rbk_smp290_snsr_get_cmpd_az(RBK_SMP290_SNSR_RANGE_HI);
            adv_sensorData.Ax_hi_out = rbk_smp290_snsr_get_cmpd_ax(RBK_SMP290_SNSR_RANGE_HI);
        }
        break;
        case SEQ_VBAT:
        {
            adv_sensorData.Vbat_out = vbat_buff.Vbat[0];
        }
        break;
        default:
        {
            // Nothing to do
        }
        break;
    }

    // Increment the sequence iterator
    sequence_iter = (sequence_iter + 1u) % (SEQ_MAX);
}

// Stop the sequence.
void sequence_stop(void)
{
    smp290_log(LOG_VERBOSITY_DEBUG, "\tSequence stopped\r\n");
    // Disable sequence timer
    rbk_smp290_timer_disable(sequence_timerId);

    // Cancel the ongoing measurement.
    // Here we ignore the return since we do not know for sure if a measurement
    // is really scheduled when this function is called.
    (void)cancelMeasmt();
}

// Resumes the sequence.
void sequence_resume(void)
{
    smp290_log(LOG_VERBOSITY_DEBUG, "\tSequence resumed\r\n");
    // Reset the sequence iterator
    sequence_iter = SEQ_T;

    // Reset the remaining time
    rbk_smp290_timer_restart(sequence_timerId);
    // Enable sequence timer
    rbk_smp290_timer_enable(sequence_timerId);
}

/** @} */
