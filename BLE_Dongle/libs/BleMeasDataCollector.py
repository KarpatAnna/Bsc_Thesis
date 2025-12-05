import sys
import time
import serial
import logging
import logging.handlers
import datetime
import binascii
import traceback

# Import helper functions
from .util import *
import logging
import datetime
import traceback
import time
from threading import Thread
# Main class for Bosch demo application
class BleMeasDataCollector(BleDataCollector):
    pressure_char_version = 2 # Can either be 1 for lower clipping at 100 kPa or 2 for lower clipping at 90 kPa.
    def __init__(self, serial_port, logger, data_logger, log_severity_level='error'):
        self.known_devices = {}
        self.connectable_devices = {}
        self.logger = logger
        self.data_logger = data_logger
        super(BleMeasDataCollector, self).__init__(serial_port, log_severity_level)

    def on_gap_evt_adv_report(self, ble_driver, conn_handle, peer_addr, rssi, adv_type, adv_data):
        
        # Type of advertising message
        adv_type_name = BleMeasDataCollector.ADV_TYPE_NAMES[adv_type]

        address_string = ':'.join('{0:02X}'.format(b) for b in peer_addr.addr)

        # Check if this is the advertising message of a Bosch Demo Application
        if nrf_ble_driver.BLEAdvData.Types.manufacturer_specific_data in adv_data.records:
            man_spec_data = adv_data.records[nrf_ble_driver.BLEAdvData.Types.manufacturer_specific_data]
            if bytes(man_spec_data)[:2][::-1] == b'\x02\xA6':
                if len(man_spec_data) in range(18,33):
                    self.myAddr = peer_addr
                    # print('Value received:\t',bytes(man_spec_data).hex(':'))
                    # Store device address as known Bosch Demo Application
                    if address_string not in self.known_devices:
                        self.known_devices[address_string] = peer_addr
                    if adv_type_name == 'ADV_IND' and address_string and address_string not in self.connectable_devices:
                        self.connectable_devices[address_string] = peer_addr
                        
                    
                    # Collect meas data from device
                    pressure_raw     = int.from_bytes(man_spec_data[ 2: 4], 'little', signed=True)
                    temperature_raw  = int.from_bytes(man_spec_data[ 4: 6], 'little', signed=True)
                    z_acc_low_raw    = int.from_bytes(man_spec_data[ 6: 8], 'little', signed=True)
                    z_acc_high_raw   = int.from_bytes(man_spec_data[ 8:10], 'little', signed=True)
                    x_acc_low_raw    = int.from_bytes(man_spec_data[10:12], 'little', signed=True)
                    x_acc_high_raw   = int.from_bytes(man_spec_data[12:14], 'little', signed=True)
                    batt_voltage_raw = int.from_bytes(man_spec_data[14:16], 'little', signed=True)
                    meas_raw = int.from_bytes(man_spec_data[16:18], 'little', signed=True)
                    counter = int.from_bytes(man_spec_data[20:22], 'little', signed=False)
                    if self.pressure_char_version == 1:
                        pressure = 0.40059 * pressure_raw + 100.
                    elif self.pressure_char_version == 2:
                        pressure = 0.40547 * pressure_raw + 90.
                    else:
                        raise ValueError('Unknown pressure characteristic version: expecting either 1 or 2!')
                    temperature  = 1. * temperature_raw
                    z_acc_low    = 640./2048. * z_acc_low_raw
                    z_acc_high   = 1920./2048. * z_acc_high_raw
                    x_acc_low    = 220./2048. * x_acc_low_raw
                    x_acc_high   = 660./2048. * x_acc_high_raw
                    batt_voltage = 0.005 * batt_voltage_raw + 1.9

                    log_string = 'Received {}, address: {}, RSSI: {} dBm\r\n'.format(
                            adv_type_name, address_string, rssi
                        )
                    log_string += ', data: {:6.2f} kPa {:3.0f} °C z {:3.1f}/{:3.1f} g x {:3.1f}/{:3.1f} g {:1.1f} V {}\r\n'.format(
                            pressure, temperature, z_acc_low, z_acc_high, x_acc_low, x_acc_high, batt_voltage, meas_raw
                        )
                    if len(man_spec_data) > 18:
                        log_string += ', error: 0x{:02X}, identifier: {:d}, counter: {:d}\r\n'.format(man_spec_data[18], man_spec_data[19], counter)

                    self.logger.debug(log_string)

                    data_string = '"{}",{},{:.2f},{:.0f},{:.1f},{:.1f},{:.1f},{:.1f},{:.1f},{}'.format(
                        address_string, rssi,
                        pressure, temperature, z_acc_low, z_acc_high, x_acc_low, x_acc_high, batt_voltage, meas_raw
                    )

                    if len(man_spec_data) > 18:
                        data_string += ',{:02X},{:d},{:d}'.format(man_spec_data[18], man_spec_data[19], counter)
                    else:
                        data_string += ','
                    self.data_logger.info(data_string)
                