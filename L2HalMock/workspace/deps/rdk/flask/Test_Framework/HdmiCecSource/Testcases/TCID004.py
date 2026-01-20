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

# Testcase ID : TCID004
# Testcase Description : To verify that standby message is successfully triggered
# and got proper logs in thunder. Hit the curl command for sendStandbyMessage and send the corresponding
# messages to hal. Verify the output response. Also, wake up the devices by sending cec message as post condition

import json
import requests
import time
import Config
import subprocess
from Utilities import Utils, ReportGenerator
from HdmiCecSource import HdmiCecSourceApis

op=subprocess.run(['netstat', '-ntlp'],capture_output=True,text=True)
print(op.stdout)
# store the expected output response
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"success":true}}'

print("TC Description - To verify that standby message is successfully triggered and get proper logs in thunder. Hit the curl command for sendStandbyMessage and send the corresponding messages to hal. Verify the output response. Also, wake up the devices by sending cec message as post condition") 
#send messages required for getting power status of device
print("---------------------------------------------------------------------------------------------------------------------------")
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.getPowerStatusMessage)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for querying the powerstatus of local device")
else:
     Utils.error_log("sendMessage emulation failed for querying the powerstatus of local device")
time.sleep(3)
print("")

# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSourceApis.send_standby_message)
if curl_response:
     Utils.warning_log("send_standby_message curl command sent from the test runner")
else:
     Utils.error_log("send_standby_message curl command failed")
# send messages required for sendStandbyMessage method
message2_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.sendStandbyMessage)))
if "200" in str(message2_response):
     Utils.info_log("sendMessage emulation success for sending the remote device state as standby to local device")
else:
     Utils.error_log("sendMessage emulation failed for sending the remote device state as standby to local device")
time.sleep(3)
print("")

# send messages required for getting power status of device
message3_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.getPowerStatusMessage)))
time.sleep(3)
if "200" in str(message3_response):
     Utils.info_log("sendMessage emulation success for querying the powerstatus of local device")
else:
     Utils.error_log("sendMessage emulation failed for querying the powerstatus of local device")

print("---------------------------------------------------------------------------------------------------------------------------")

# compare both expected and received output responses
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one. We are expecting opcode : 36 in thunder logs'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID004'
print("Testcase ID : " + tc_id)
print("Testcase Output Response : " + curl_response)
print("Testcase Status : " + status)
print("Testcase Message : " + message)
print("")

if status == 'Pass':
    ReportGenerator.passed_tc_list.append(tc_id)
else:
    ReportGenerator.failed_tc_list.append(tc_id)

# push the testcase execution details to report file
ReportGenerator.append_test_results_to_csv(tc_id, curl_response, status, message)

# wake up the device from standby as post condition by sending message
message4_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.performOTPActionMessage)))
Utils.warning_log("Reset the device state to ON from standby")
time.sleep(3)

# send messages required for getting power status of device
message5_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.getPowerStatusMessage)))
time.sleep(3)
Utils.info_log("send the emulated message to get the power status of local device")
op=subprocess.run(['netstat', '-ntlp'],capture_output=True,text=True)
print(op.stdout)

