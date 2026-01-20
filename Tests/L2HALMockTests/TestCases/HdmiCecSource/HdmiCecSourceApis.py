#** *****************************************************************************
# *
# * If not stated otherwise in this file or this component's LICENSE file the
# * following copyright and licenses apply:
# *
# * Copyright 2024 RDK Management
# *
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *
# http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *
#* ******************************************************************************

# Curl command for activating HdmiCecSource plugin
activate_command = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc":"2.0","id":"3"
,"method": "Controller.1.activate", "params":{"callsign":"org.rdk.HdmiCecSource"}}' http://127.0.0.1:55555/jsonrpc'''

# Curl command for deactivating HdmiCecSource plugin
deactivate_command = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc":"2.0","id":"3"
,"method": "Controller.1.deactivate", "params":{"callsign":"org.rdk.HdmiCecSource"}}' http://127.0.0.1:55555/jsonrpc'''

# Store the expected output response for activate & deactivate curl command
expected_output_response = '{"jsonrpc":"2.0","id":3,"result":null}'

######################################################################################

# HdmiCecSource Methods :

get_device_list = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.getDeviceList"}' http://127.0.0.1:55555/jsonrpc'''

send_standby_message = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", 
"id": 42,"method":"org.rdk.HdmiCecSource.sendStandbyMessage"}' http://127.0.0.1:55555/jsonrpc'''

get_vendor_id = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.getVendorId"}' http://127.0.0.1:55555/jsonrpc'''

set_vendor_id = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSource.setVendorId","params": {"vendorid": "0x4455"}}' http://127.0.0.1:55555/jsonrpc'''

get_osd_name = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.getOSDName"}' http://127.0.0.1:55555/jsonrpc'''

set_osd_name = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSource.setOSDName","params": {"name": "CUSTOM8 TV"}}' http://127.0.0.1:55555/jsonrpc'''

get_enabled = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.getEnabled"}' http://127.0.0.1:55555/jsonrpc'''

set_enabled_false = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSource.setEnabled","params": {"enabled": false}}' http://127.0.0.1:55555/jsonrpc'''

set_enabled_true = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSource.setEnabled","params": {"enabled": true}}' http://127.0.0.1:55555/jsonrpc'''

get_active_source_status = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0",
"id": 42,"method":"org.rdk.HdmiCecSource.getActiveSourceStatus","params": {"status": true}}' http://127.0.0.1:55555/jsonrpc'''

get_otp_enabled = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.getOTPEnabled"}' http://127.0.0.1:55555/jsonrpc'''

set_otp_enabled_false = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSource.setOTPEnabled","params": {"enabled": false}}' http://127.0.0.1:55555/jsonrpc'''

set_otp_enabled_true = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSource.setOTPEnabled","params": {"enabled": true}}' http://127.0.0.1:55555/jsonrpc'''

send_keypress_VOLUME_UP = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 65}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_VOLUME_DOWN = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 66}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_MUTE = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 67}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_UP = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 1}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_DOWN = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 2}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_LEFT = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 3}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_RIGHT = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 4}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_SELECT = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 0}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_HOME = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 9}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_BACK = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 13}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_0 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 32}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_1 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 33}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_2 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 34}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_3 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 35}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_4 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 36}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_5 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 37}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_6 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 38}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_7 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 39}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_8 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 40}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_9 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSource.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 41}}' http://127.0.0.1:55555/jsonrpc'''

perform_otp_action = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSource.performOTPAction"}' http://127.0.0.1:55555/jsonrpc'''

set_otp_enabled_invalid = '''curl --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0","id": 42,"method": "org.rdk.HdmiCecSource.setOTPEnabled","params": {"ennable": true}}' http://127.0.0.1:55555/jsonrpc'''

set_osd_name_invalid = '''curl --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0","id": 42,"method": "org.rdk.HdmiCecSource.setOSDName","params": {"nnamme": "LG TV"}}' http://127.0.0.1:55555/jsonrpc'''

set_vendor_id_invalid_1 = '''curl --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0","id": 42,"method": "org.rdk.HdmiCecSource.setVendorId","params": {"vendorid": "]]"}}' http://127.0.0.1:55555/jsonrpc'''

set_vendor_id_invalid_2 = '''curl --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0","id": 42,"method": "org.rdk.HdmiCecSource.setVendorId","params": {"vllendorid": "]]"}}' http://127.0.0.1:55555/jsonrpc'''


