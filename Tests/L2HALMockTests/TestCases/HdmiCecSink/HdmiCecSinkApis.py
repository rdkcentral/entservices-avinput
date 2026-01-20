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

# Curl command for activating HdmiCecSink plugin
activate_command = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc":"2.0","id":"3"
,"method": "Controller.1.activate", "params":{"callsign":"org.rdk.HdmiCecSource"}}' http://127.0.0.1:55555/jsonrpc'''

# Curl command for deactivating HdmiCecSink plugin
deactivate_command = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc":"2.0","id":"3"
,"method": "Controller.1.deactivate", "params":{"callsign":"org.rdk.HdmiCecSource"}}' http://127.0.0.1:55555/jsonrpc'''

# Store the expected output response for activate & deactivate curl command
expected_output_response = '{"jsonrpc":"2.0","id":3,"result":null}'

######################################################################################

# HdmiCecSink Methods :

#Gets the number of connected source devices and system information for each device
get_device_list = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.getDeviceList"}' http://127.0.0.1:55555/jsonrpc'''

#Sends a CEC <Standby> message to the logical address of the device
send_standby_message = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", 
"id": 42,"method":"org.rdk.HdmiCecSink.sendStandbyMessage"}' http://127.0.0.1:55555/jsonrpc'''

#Gets the current vendor ID used by host device
get_vendor_id = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.getVendorId"}' http://127.0.0.1:55555/jsonrpc'''

#Sets a vendor ID used by host device
set_vendor_id = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setVendorId","params": {"vendorid": "0x4455"}}' http://127.0.0.1:55555/jsonrpc'''

#Sets an undefined vendor ID(not following the standard data type) used by host device
set_vendor_id_with_undefined_datatype = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setVendorId","params": {"vendorid": "12345"}}' http://127.0.0.1:55555/jsonrpc'''

#Returns the OSD name used by host device
get_osd_name = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.getOSDName"}' http://127.0.0.1:55555/jsonrpc'''

#Sets the OSD Name used by host device
set_osd_name = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setOSDName","params": {"name": "Sky TV"}}' http://127.0.0.1:55555/jsonrpc'''

#Sets the an OSD Name which is not in the correct format used by host device
set_osd_name_with_undefined_datatype = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setOSDName","params": {"name": "ABC TV"}}' http://127.0.0.1:55555/jsonrpc'''

#Returns whether HDMI-CEC is enabled on platform or not
get_enabled = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.getEnabled"}' http://127.0.0.1:55555/jsonrpc'''

#Disables HDMI-CEC support in the platform as enabled is FALSE
set_enabled_false = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setEnabled","params": {"enabled": false}}' http://127.0.0.1:55555/jsonrpc'''

#Enables HDMI-CEC support in the platform
set_enabled_true = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setEnabled","params": {"enabled": true}}' http://127.0.0.1:55555/jsonrpc'''

#Disables HDMI-CEC support in the platform, by passing a value other than boolean to get an error response 
set_enabled_with_undefined_datatype = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setEnabled","params": {"enabled": 1234}}' http://127.0.0.1:55555/jsonrpc'''

#Gets the current active source 
get_active_source_status = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0",
"id": 42,"method":"org.rdk.HdmiCecSink.getActiveSourceStatus","params": {"status": true}}' http://127.0.0.1:55555/jsonrpc'''


get_otp_enabled = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.getOTPEnabled"}' http://127.0.0.1:55555/jsonrpc'''

set_otp_enabled_false = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setOTPEnabled","params": {"enabled": false}}' http://127.0.0.1:55555/jsonrpc'''

set_otp_enabled_true = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setOTPEnabled","params": {"enabled": true}}' http://127.0.0.1:55555/jsonrpc'''

set_otp_enabled_true_with_undefined_datatype = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setOTPEnabled","params": {"enabled": 1234}}' http://127.0.0.1:55555/jsonrpc'''

#Sends the CEC <User Control Pressed> message when TV remote key is pressed and all the key events are defined here
send_keypress_VOLUME_UP = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 65}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_VOLUME_DOWN = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 66}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_MUTE = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 67}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_UP = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 1}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_DOWN = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 2}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_LEFT = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 3}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_RIGHT = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 4}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_SELECT = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 0}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_HOME = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 9}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_BACK = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 13}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_0 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 32}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_1 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 33}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_2 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 34}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_3 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 35}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_4 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 36}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_5 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 37}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_6 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 38}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_7 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 39}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_8 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 40}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_9 = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent", "params": {"logicalAddress": 0,"keyCode": 41}}' http://127.0.0.1:55555/jsonrpc'''


#Sends the CEC <User Control Pressed> message when TV remote key is pressed
send_keypress_VOLUME_UP_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 65}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_VOLUME_DOWN_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 66}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_MUTE_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 67}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_UP_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 1}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_DOWN_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 2}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_LEFT_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 3}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_RIGHT_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 4}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_SELECT_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 0}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_HOME_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 9}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_BACK_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 13}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_0_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 32}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_1_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 33}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_2_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 34}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_3_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 35}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_4_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 36}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_5_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 37}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_6_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 38}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_7_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 39}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_8_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 40}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_9_USER_Press = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed", "params": {"logicalAddress": 0,"keyCode": 41}}' http://127.0.0.1:55555/jsonrpc'''


#Sends the CEC <User Control released> message when TV remote key is released
send_keypress_VOLUME_UP_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 65}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_VOLUME_DOWN_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 66}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_MUTE_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 67}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_UP_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 1}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_DOWN_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 2}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_LEFT_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 3}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_RIGHT_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 4}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_SELECT_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 0}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_HOME_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 9}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_BACK_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 13}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_0_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 32}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_1_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 33}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_2_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 34}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_3_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 35}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_4_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 36}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_5_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 37}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_6_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 38}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_7_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 39}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_8_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 40}}' http://127.0.0.1:55555/jsonrpc'''
send_keypress_NUMBER_9_USER_Release = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased", "params": {"logicalAddress": 0,"keyCode": 41}}' http://127.0.0.1:55555/jsonrpc'''


#waking up the device from standby
perform_otp_action = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.performOTPAction"}' http://127.0.0.1:55555/jsonrpc'''

#Sends the CEC Request Short Audio Descriptor (SAD) message as an event
request_short_audio_descriptor = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.requestShortAudioDescriptor"}' http://127.0.0.1:55555/jsonrpc'''

#This message is used to power on the connected audio device
send_audio_device_power_on_message = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.sendAudioDevicePowerOnMessage"}' http://127.0.0.1:55555/jsonrpc'''

#Sends the CEC <Give Audio Status> message to request the audio status
get_audio_status_message = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.getAudioStatusMessage"}' http://127.0.0.1:55555/jsonrpc'''

#Requests the active source in the network
request_active_source = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.requestActiveSource"}' http://127.0.0.1:55555/jsonrpc'''

#Get status of audio device connection
get_audio_device_connected_status =  '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.getAudioDeviceConnectedStatus"}' http://127.0.0.1:55555/jsonrpc'''

#enabling otp with undefined data types
set_otp_enabled_with_undefined_datatype = '''curl --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0","id": 42,"method": "org.rdk.HdmiCecSink.setOTPEnabled","params": {"ennable": true}}' http://127.0.0.1:55555/jsonrpc'''

set_osd_name_invalid = '''curl --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0","id": 42,"method": "org.rdk.HdmiCecSink.setOSDName","params": {"nnamme": "LG TV"}}' http://127.0.0.1:55555/jsonrpc'''

set_vendor_id_invalid_1 = '''curl --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0","id": 42,"method": "org.rdk.HdmiCecSink.setVendorId","params": {"vendorid": "]]"}}' http://127.0.0.1:55555/jsonrpc'''

set_vendor_id_invalid_2 = '''curl --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0","id": 42,"method": "org.rdk.HdmiCecSink.setVendorId","params": {"vllendorid": "]]"}}' http://127.0.0.1:55555/jsonrpc'''

#Gets details for the current route from the source to sink devices
get_active_route = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.getActiveRoute"}' http://127.0.0.1:55555/jsonrpc'''

#Sets the source device to active (setStreamPath)
set_active_path = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,"method":"org.rdk.HdmiCecSink.setActivePath", "params": {"activePath": "1.0.0.0"}}' http://127.0.0.1:55555/jsonrpc'''

#Sets the current active source as TV (physical address 0
set_active_source = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.setActiveSource"}' http://127.0.0.1:55555/jsonrpc'''

#Updates the internal data structure with the new menu Language and also broadcasts the <Set Menu Language> CEC message.Events
set_menu_language = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setMenuLanguage","params": {"language": "chi"}}' http://127.0.0.1:55555/jsonrpc'''

#Updates the internal data structure with the new wrong menu Language and also broadcasts the <Set Menu Language> CEC message.Events
set_menu_language_with_undefined_datatype = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setMenuLanguage","params": {"language": "abc"}}' http://127.0.0.1:55555/jsonrpc'''

#Sets the Current Latency Values such as Video Latency, Latency Flags,Audio Output Compensated value and Audio Output Delay by sending <Report Current Latency> message for Dynamic Auto LipSync Feature.
set_latency_info ='''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setLatencyInfo","params": {"videoLatency": "2","lowLatencyMode": "1","audioOutputCompensated": "1","audioOutputDelay": "20"}}' http://127.0.0.1:55555/jsonrpc'''

#Sets the Current Latency Values such as invalid which includes Video Latency, Latency Flags,Audio Output Compensated value and Audio Output Delay by sending <Report Current Latency> message for Dynamic Auto LipSync Feature.
set_latency_info_with_undefinedLatencyMode ='''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setLatencyInfo","params": {"videoLatency": "987666","lowLatencyMode": "888888","audioOutputCompensated": "abc","audioOutputDelay": "invalid"}}' http://127.0.0.1:55555/jsonrpc'''

#Changes routing while switching between HDMI inputs and TV
set_routing_change = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setRoutingChange","params": {"oldPort": "HDMI0","newPort":"TV"}}' http://127.0.0.1:55555/jsonrpc'''

#Changes routing while switching between HDMI inputs and TV with new ports
set_routing_change_nav = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setRoutingChange","params": {"oldPort": "HDMI0","newPort":"TV"}}' http://127.0.0.1:55555/jsonrpc'''

#Changes routing while switching between unknown devices/port
set_routing_change_with_undefined_datatype = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setRoutingChange","params": {"oldPort": "ABCD","newPort":"zxcv"}}' http://127.0.0.1:55555/jsonrpc'''

#Enable (or disable) HDMI-CEC Audio Return Channel (ARC) routing
set_arc_routing_params_enabled = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setupARCRouting","params":{"enabled":true}' http://127.0.0.1:55555/jsonrpc'''

set_arc_routing_params_disabled = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setupARCRouting","params":{"enabled":false}' http://127.0.0.1:55555/jsonrpc'''

#It prints the list of connected devices and properties of connected devices like deviceType, VendorID, CEC version, PowerStatus, OSDName, PhysicalAddress etc.
print_device_list = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.printDeviceList"}' http://127.0.0.1:55555/jsonrpc'''

#Get status of audio device connection
request_audio_device_power_status = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.requestAudioDevicePowerStatus"}' http://127.0.0.1:55555/jsonrpc'''

#Gets the current active source
get_Active_Source = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.getActiveSource"}' http://127.0.0.1:55555/jsonrpc'''

#Sends the CEC <Give Audio Status> message to request the audio status.
sendGetAudioStatusMessage = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendGetAudioStatusMessage"}' http://127.0.0.1:55555/jsonrpc'''

#Set an invalid/undefined vendor ID in the format of INT instead of hexa
set_vendor_id_invalid_2 = '''curl --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0","id": 42,"method": "org.rdk.HdmiCecSource.setVendorId","params": {"vllendorid": "]]"}}' http://127.0.0.1:55555/jsonrpc'''

#Routing change api from Hdmi to an undefined device for failing the scenario
set_routing_change_negative = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id":42,
"method":"org.rdk.HdmiCecSink.setRoutingChange","params": {"oldPort": "HDMI0","newPort":"NEW"}}' http://127.0.0.1:55555/jsonrpc'''

#All set curl command Json RPC request without params for negtaive scenarios
sendKeyPress_withoutParams =  '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendKeyPressEvent"}' http://127.0.0.1:55555/jsonrpc'''
senduserContolledPress_withoutParams =  '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlPressed"}' http://127.0.0.1:55555/jsonrpc'''
sendusercontrolledReleased_withoutParams = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.sendUserControlReleased"}' http://127.0.0.1:55555/jsonrpc'''
setActivepath_withoutParams = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.setActivePath"}' http://127.0.0.1:55555/jsonrpc'''
setEnabled_withoutParams = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.setEnabled"}' http://127.0.0.1:55555/jsonrpc'''
setMenuLanguage_withoutParams = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.setMenuLanguage"}' http://127.0.0.1:55555/jsonrpc'''
setOSDName_withoutParams = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.setOSDName"}' http://127.0.0.1:55555/jsonrpc'''
setRoutingChange_withoutParams = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.setRoutingChange"}' http://127.0.0.1:55555/jsonrpc'''
setupARCRouting_withoutParams = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.setupARCRouting"}' http://127.0.0.1:55555/jsonrpc'''
setvendorID_withoutParams = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.setVentorId"}' http://127.0.0.1:55555/jsonrpc'''
setLatencyInfo_withoutParams = '''curl --silent --header "Content-Type: application/json" --request POST -d '{"jsonrpc": "2.0", "id": 42,
"method":"org.rdk.HdmiCecSink.setLatencyInfo"}' http://127.0.0.1:55555/jsonrpc'''
