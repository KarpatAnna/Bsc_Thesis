from .util import *
import time

#-----------------------------------------------------------------------------------------------------------------------------------------
## Connection State Machine:
#-----------------------------------------------------------------------------------------------------------------------------------------

class ConnectedStateMachine:
    def __init__(self, collector, client, logger):
        self.state = 'create_dict'
        self.collector = collector
        self.client = client
        self.logger = logger

    def idle(self):
        print("In idle state.")

        # Get user input to determine the next state
        user_input = input("Enter (read, write, chars or exit): ")
        user_input = user_input.lower()

        if user_input=='pin':
            status, data = self.collector.read_char(self.client, smp290_svc_char_pin_uuid, smp290_cust_svc_uuid)
            print(status)
            print(data)        

        elif user_input in ['read', 'write', 'exit', 'chars']:
            self.state = user_input
        else:
            print("Invalid state. Staying in initial state.")

    def create_dict(self):
        print("Creating Dictionary")
        self.logger.info("Creating Dictionary")

        self.smp290_cust_svc_uuid_dict = {}

        current_index = 2

        while current_index < len(self.collector.adapter.db_conns[0].services):
            current_svc_uuid_base = self.collector.adapter.db_conns[0].services[current_index].uuid.base
            current_svc_uuid = base_to_uuid(current_svc_uuid_base)
            
            for char in self.collector.adapter.db_conns[0].services[current_index].chars:

                uuid_base = char.descs[0].uuid.base

                data_uuid = base_to_uuid(uuid_base)

                data = self.collector.read_user_desc(self.client, data_uuid, current_svc_uuid)

                data_string = ''.join(chr(value) for value in data[1] if value != 0)

                self.smp290_cust_svc_uuid_dict.update({data_string : [data_uuid, current_svc_uuid]})

            current_index += 1
            
        self.logger.info(f"SMP290 Characteristics: {list(self.smp290_cust_svc_uuid_dict.keys())}")

        self.state = 'idle'

    def chars(self):
        print("Listing SMP290 Characteristics:")
        print(list(self.smp290_cust_svc_uuid_dict.keys()))

        self.state = 'idle'           

    def write(self):
        print("In write state.")

        # Change GPIO Mode
        user_choice_char = input("What Characteristic would you like to write? ")

        not_found = True
        for key, value in self.smp290_cust_svc_uuid_dict.items():
            if user_choice_char == key:
                smp290_chosen_char = value[0]
                smp290_chosen_svc = value[1]
                not_found = False

        if not_found:
            print("Invalid Characteristic. Returning to idle state.")
            self.state = 'idle'
            return

        if 'GPIO' in user_choice_char and user_choice_char != "GPIO pin":
            pin_select_success = pin_select(self.client, user_choice_char, self.collector, self.logger, self.smp290_cust_svc_uuid_dict, smp290_chosen_svc)
            if not pin_select_success:
                self.state = 'idle'
                return
            print(f"Now set new value for {user_choice_char} Characteristic")

        if 'GPIO' in user_choice_char or 'TSD' in user_choice_char or 'TX power' in user_choice_char:
            user_input_data = input("New value: ")
            data_in = [int(data) for data in user_input_data.split(",")]
        else:
            data_in = 0
        
        self.collector.write_char(self.client, smp290_chosen_char, bytes(data_in), smp290_chosen_svc)

        status, data_out = self.collector.read_char(self.client, smp290_chosen_char, smp290_chosen_svc)

        if 'GPIO' in user_choice_char or 'TSD' in user_choice_char or 'TX power' in user_choice_char:
            if data_in == data_out:
                self.logger.info(f"{user_choice_char} Characteristic Value change SUCCESSFUL, value changed to: {data_in}")
            else:
                self.logger.info(f"{user_choice_char} Characteristic Value change FAILED, value remains: {data_out}")
        else:
            self.logger.info(f"{user_choice_char} Characteristic Value is: {data_out}")

        decode_value(user_choice_char, data_out)
        self.state = 'idle'

    def read(self):
        print("In read state.")

        user_choice_char = input("What Characteristic would you like to read? ")

        not_found = True

        for key, value in self.smp290_cust_svc_uuid_dict.items():
            if user_choice_char == key:
                smp290_chosen_char = value[0]
                smp290_chosen_svc = value[1]
                not_found = False
        
        if not_found:
            print("Invalid Characteristic. Returning to idle state.")
            self.state = 'idle'
            return

        if 'GPIO' in user_choice_char and user_choice_char != "GPIO pin":
            pin_select_success = pin_select(self.client, user_choice_char, self.collector, self.logger, self.smp290_cust_svc_uuid_dict, smp290_chosen_svc)
            if not pin_select_success:
                self.state = 'idle'
                return
                
        status, data = self.collector.read_char(self.client, smp290_chosen_char, smp290_chosen_svc)

        self.logger.info(f"{user_choice_char} Characteristic Value: {data}")

        decode_value(user_choice_char, data)
        self.state = 'idle'
    
    def exit(self):
        print("Exiting...") # Does not get printed

        # Exits state machine
    

    def run(self):
        while self.state != 'exit':
            getattr(self, self.state)()

#-----------------------------------------------------------------------------------------------------------------------------------------
## Functions:
#-----------------------------------------------------------------------------------------------------------------------------------------

def pin_select(client, user_choice_char, collector, logger, smp290_cust_svc_uuid_dict, smp290_chosen_svc):
    if user_choice_char == "GPIO drive strength":
        user_input_pin = '4'

    else:
        user_input_pin = input("What GPIO pin would you like to choose (0, 1 or 4)? ")

        if user_input_pin not in ['0', '1', '4']:
            print("Invalid GPIO pin. Returning to idle state.")
            return False

        pin_change = [int(data) for data in user_input_pin.split(",")]
        collector.write_char(client, smp290_cust_svc_uuid_dict['GPIO pin'][0], bytes(pin_change), smp290_chosen_svc)

        status, pin = collector.read_char(client, smp290_cust_svc_uuid_dict['GPIO pin'][0], smp290_chosen_svc)

        if pin == pin_change:
            logger.info(f" GPIO pin {pin} SUCCESSFULLY chosen ")
        else:
            logger.info(f"GPIO pin change FAILED, GPIO pin remains: {pin}")
    
    return True

def base_to_uuid(uuid_base):
    hex_string = ''.join(f'{num:02x}' for num in uuid_base.base)
    uuid_number = int(hex_string, 16)

    data_uuid = uuid_base_from_16bytes(uuid_number, uuid_base.type)
    return data_uuid

def decode_value(char, data):
    print("Decoding value")

    match char:
        case "Current TX power":
            print(f"\nTX power is: {data[0]} dBm\n")

        case "Counter value":
            print(f"\nCounter is: {counter_calculation(data)}\n")

        case "TSD":
            if data == [0]:
                print(f"\nTSD is disabled!\n")
            elif data == [1]:
                print(f"\nTSD is enabled!\n")

        case "HW Version":
            result_string = ""
            for char_code in data:
                if char_code == 0:
                    break
                result_string += chr(char_code)
            print(f"\nHW Version is: {result_string}\n")

        case "FW Version":
            result_string = ""
            for char_code in data:
                if char_code == 0:
                    break
                result_string += chr(char_code)
            print(f"\nFW Version is: {result_string}\n")

        case "Data Backup":
            if data == [0]:
                print(f"\nData backup Success!\n")
            elif data == [1]:
                print(f"\nData backup Failed!\n")

        case "TP Selftest":
            if data == [0]:
                print(f"\nSelftest Success!\n")
            else:
                error_code = data
                print(f"\nSelftest Failed. Error code: 0x{error_code:02x}\n")

        case "T":
            print(f"\nTemperature is: {t_calculation(data)} °C\n")

        case "TPAZ":
            print(f"\nTemperature is: {t_calculation(data[0:2])} °C\n")
            print(f"Pressure is: {p_calculation(data[2:4])} kPa\n")
            print(f"Z axis acceleration is: {az_calculation(data[4:6])} g\n")

        case "TAZAX":
            print(f"\nTemperature is: {t_calculation(data[0:2])} °C\n")
            print(f"Z axis acceleration is: {az_calculation(data)[2:4]} g\n")
            print(f"X axis acceleration is: {ax_calculation(data[4:6])} g\n")

        case "VBAT":
            print(f"\nBattery is: {vbat_calculation(data)} V\n")

        case "GPIO pin":
            print(f"\nGPIO pin chosen is: {data}\n")

        case "GPIO mode":
            print(f"\nGPIO direction is:\n")
            if data[0] == 1:
                print(f"Input\n")
            elif data[0] == 2:
                print(f"Output\n")
            else:
                print(f"Pin direction disabled\n")
            print(f"\nGPIO mode is:\n")
            if data[1] == 0:
                print(f"Push Pull\n")
            elif data[1] == 1:
                print(f"Open Drain\n")
            else:
                print(f"NA\n")

        case "GPIO drive strength":
            print(f"\nGPIO drive strength is:\n")
            if data[0] == 0:
                print(f"Low\n")
            elif data[0] == 1:
                print(f"High\n")

        case "GPIO pull resistors":
            print(f"\nGPIO pull resistor is:\n")
            if data[0] == 0:
                print(f"None\n")
            elif data[0] == 1:
                print(f"Pull Up\n")
            elif data[0] == 2:
                print(f"Pull Down\n")

        case "GPIO value":
            print(f"\nGPIO value is:\n")
            if data[0] == 0:
                print(f"Low\n")
            elif data[0] == 1:
                print(f"High\n")

        case "GPIO input level":
            print(f"\nGPIO input level is:\n")
            if data[0] == 0:
                print(f"Low\n")
            elif data[0] == 1:
                print(f"High\n")
        case _:
            print("Couldn't decode value\n")

def counter_calculation(data):
    hex_string = "".join(f"{byte:02X}" for byte in data)
    counter_dec = int(hex_string, 16)
    return counter_dec

def vbat_calculation(data):
    hex_string = "".join(f"{byte:02X}" for byte in data)
    vbat_dec = int(hex_string, 16)
    vbat = 0.005 * vbat_dec + 1.9
    return vbat

def t_calculation(data):
    hex_string = "".join(f"{byte:02X}" for byte in data)
    t_dec = int(hex_string, 16)
    return t_dec

def p_calculation(data):
    hex_string = "".join(f"{byte:02X}" for byte in data)
    p_dec = int(hex_string, 16)
    p = 0.40547 * p_dec + 90
    return p

def az_calculation(data):
    hex_string = "".join(f"{byte:02X}" for byte in data)
    az_dec = int(hex_string, 16)
    az = 1920./2048. * az_dec
    return az

def ax_calculation(data):
    hex_string = "".join(f"{byte:02X}" for byte in data)
    ax_dec = int(hex_string, 16)
    ax = 660./2048. * ax_dec
    return ax
