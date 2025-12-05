/**
 * @addtogroup   measure_advertise_conn
 * @{
 * @file         adv.c
 * @brief        This file contains the advertising routines of the project \ref measure_advertise_conn
 * @details      The advertising routines in this file are responsible for configuring and starting BLE advertising,
 *               as well as preparing the advertising data payload. The advertising data payload includes information
 *               such as flags, sensor data, and appearance. The functions in this file are used to initialize the
 *               advertising parameters, prepare the advertising data, and start the advertising process. The advertising
 *               interval, duration, and MTU size are configurable constants defined in this file.
 * @note         For BLE advertising, the company ID, appearance, and advertising interval are set to specific values.
 * @note         The advertising data payload is constructed using specific structures defined in this file.
 * @note         The advertising data payload includes flags, sensor data, and appearance.
 * @note         The advertising data payload is stored in a global variable for reuse.
 * @note         The desired MTU size is also stored in a global variable for configuration.
 * @copyright    (c) [Robert Bosch GmbH] [2024].
 *               All rights reserved, also regarding any disposal,
 *               exploitation, reproduction, editing, distribution, as well
 *               as in the event of applications for industrial property
 *               rights. The communication of its contents to others without
 *               express authorization is prohibited. Offenders will be held
 *               liable for the payment of damages. All rights reserved in
 *               the event of the grant of a patent, utility model or design.
 **/

/* System includes */
#include <string.h>

/* Library includes */
#include "rbk_smp290_ble.h"
#include "rbk_smp290_ble_atts.h"
#include "rbk_smp290_types.h"

/* Project includes */
#include "ble_custSvc.h"
#include "ble_gpioSvc.h"
#include "ble_maintSvc.h"
#include "main.h"

/// @defgroup measure_advertise_conn_adv_cfg BLE Adv. configuration definitions
/// @{

/******************************************************************************\
 *  Constants
\******************************************************************************/

/// BLE Adv. Company ID Bosch in the Manufacturer Specific Adv. structure
#define BLE_ADV_COMPANY_ID_BOSCH 0x02A6

/// BLE Adv. Appearance TPMS value in the Appearance Adv. structure
#define BLE_ADV_APPEARANCE_TPMS 0x0559

/// The BLE advertising interval is set to 20 ms. One digit is 0.625 ms and the
/// interval-range is 0x0020 to 0x4000 (20 ms to 10.24 s)
#define BLE_ADVERTISING_INTL 0x0020u

/// The BLE advertising duration in ms is set to 60 ms. 0 is infinity
#define BLE_ADVERTISING_DURATION 60u

/// The BLE MTU size
#define BLE_MTU_SIZE 128

/*******************************************************************************
 *  Types
 ******************************************************************************/

/// BLE Advertising data structures
// Pack the following structs
#pragma pack(1)

/// Flags Adv. Structure
typedef struct
{
    uint8_t length;  //!< Adv. structure length
    uint8_t type;    //!< Adv. structure type: Flags(0x01)
    uint8_t flags;   //!< Adv. structure flags
} ble_advStrFlags_tst;

/// BLE Service Data Adv. Structure
typedef struct
{
    uint8_t length;                 //!< Adv. structure length
    uint8_t type;                   //!< Adv. structure type: Manufacturer Specific 0xFF)
    uint16_t companyId;             //!< Company ID (0x02A6)
    ble_sensorData_tst sensorData;  //!< Sensor data
} ble_advStrData_tst;

/// Appearance Adv. Structure
typedef struct
{
    uint8_t length;       //!< Adv. structure length
    uint8_t type;         //!< Adv. structure type: Appearance (0x19)
    uint16_t appearance;  //!< Appearance (0x5905)
} ble_advStrAppearance_tst;

// Restore default pack
#pragma pack()

/// Length of the BLE advertisement message
#define BLE_ADV_DATA_LEN (sizeof(ble_advStrFlags_tst) + sizeof(ble_advStrData_tst) + sizeof(ble_advStrAppearance_tst))

/******************************************************************************\
 *  Global variables
\******************************************************************************/

/// Advertising data payload
SECTION_PERSISTENT static uint8_t ble_advData[BLE_ADV_DATA_LEN];

/// Desired MTU size
uint16_t ble_mtu_size = BLE_MTU_SIZE;

/// @}

/******************************************************************************\
 *  Functions declarations
\******************************************************************************/

// Initialize the advertising parameters and configurations.
void adv_init()
{
    rbk_smp290_ble_rfOutPwr PwrLvl;
    // Read the TX power from NVM
    ble_txPwrLvl = readTxPwrFromNvm();
    // Set the desired MTU
    (void)rbk_smp290_ble_atts_set_mtu(ble_mtu_size);
    // Initialize the Device address
    // Set the public address  0    9    2     P    M    S
    //rbk_smp290_ble_addr addr = {0x30, 0x39, 0x32, 0x50, 0x4D, 0x53};
    //rbk_smp290_ble_gap_addr_setPublic(addr);

    // Configure the TX Power
    PwrLvl = rbk_smp290_ble_radio_setTxPwr(ble_txPwrLvl);
    smp290_log(LOG_VERBOSITY_INFO, "TX Power Level: %d\r\n", PwrLvl);

    // Configure the BLE Advertisement parameters
    // Set the Adv. Interval
    (void)rbk_smp290_ble_gap_adv_setIntrv(BLE_ADVERTISING_INTL, BLE_ADVERTISING_DURATION);

    // Set the Adv. Channel
    (void)rbk_smp290_ble_gap_adv_setChannel(RBK_SMP290_BLE_ADV_CH_ALL);

    // Set the Adv. Type
    (void)rbk_smp290_ble_gap_adv_setTyp(RBK_SMP290_BLE_ADV_CONN_UNDIRECT);

    // Enable the Temperature Compensation TX Power
    rbk_smp290_ble_radio_enable_cmpd_T();

    // Calculate database hash
    rbk_smp290_ble_atts_calcDbHash();

    // Set the Adv Filter policy
    (void)rbk_smp290_ble_gap_adv_setFiltPolicy(RBK_SMP290_BLE_ADV_FILT_NONE);
}

// Prepare the advertising data payload.
void adv_prepSrvData(ble_sensorData_tst const *adv_sensorData_p)
{
    uint8_t index = 0u;

    // GAP advertising flags for discoverable
    static const ble_advStrFlags_tst advStr_flags = {
        .length = sizeof(ble_advStrFlags_tst) - 1u,
        .type   = (uint8_t)RBK_SMP290_BLE_ADV_TYP_FLAGS,
        .flags  = ((uint8_t)RBK_SMP290_BLE_FLAG_LE_GENERAL_DISC | (uint8_t)RBK_SMP290_BLE_FLAG_LE_BREDR_NOT_SUP)};

    // Manufacturer Specific sensor data struct
    static ble_advStrData_tst advStr_data = {
        .length = sizeof(ble_advStrData_tst) - 1u, 
        .type = (uint8_t)RBK_SMP290_BLE_ADV_TYP_MANUFACTURER, 
        .companyId = BLE_ADV_COMPANY_ID_BOSCH};

    // Appearance
    static const ble_advStrAppearance_tst advStr_appearance = {
        .length = sizeof(ble_advStrAppearance_tst) - 1u, 
        .type = (uint8_t)RBK_SMP290_BLE_ADV_TYP_APPEARANCE, 
        .appearance = BLE_ADV_APPEARANCE_TPMS};

    // Copy the advStr_flags to the advertising data buffer
    memcpy(&ble_advData[index], &advStr_flags, sizeof(ble_advStrFlags_tst));
    index += sizeof(ble_advStrFlags_tst);

    if (NULL == adv_sensorData_p)
    {
        memset((void *)&(advStr_data.sensorData), 0x00, sizeof(ble_sensorData_tst));
    }
    else
    {
        memcpy((void *)&(advStr_data.sensorData), (void *)adv_sensorData_p, sizeof(ble_sensorData_tst));
    }
    // Copy the advStr_data to the advertising data buffer
    memcpy(&ble_advData[index], &advStr_data, sizeof(ble_advStrData_tst));
    index += sizeof(ble_advStrData_tst);

    // Copy the advStr_appearance to the advertising data buffer
    memcpy(&ble_advData[index], &advStr_appearance, sizeof(ble_advStrAppearance_tst));
}

// Start the advertising process.
void adv_doAdv()
{
    // Set the advertising data
    (void)rbk_smp290_ble_gap_adv_setData((uint8_t *)ble_advData, (const uint8_t)sizeof(ble_advData));

    // Start advertising
    rbk_smp290_ble_gap_adv_start();
}

/** @} */
