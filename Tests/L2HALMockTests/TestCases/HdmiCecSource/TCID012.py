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

# Testcase ID : TCID012
# Testcase Description : Set the devices to standby mode and wake up the remote device using otp feature
# Hit the curl command for sendStandbyMessage and send corresponding cec messages using sendMessage API
# to hal. Then hit the curl command for perform OTP Action and send corresponding cec messages to hal.
# Then hit the curl command for getActiveSourceStatus and verify that status is true in output response.
# Deactivate and reactivate the HdmiCecSource plugin. Then hit the curl command for getActiveSourceStatus
# and verify that status is back to default(false) in output response.Verify the thunder logs for more info.

import json
import requests
import time
import Config
from Utilities import Utils, ReportGenerator
from HdmiCecSource import HdmiCecSourceApis

# Utils.restart_services()
print(" TC Description - Set the devices to standby mode and wake up the remote device using otp feature.Hit the curl command for sendStandbyMessage and send corresponding cec messages using sendMessage API to hal. Then hit the curl command for perform OTP Action and send corresponding cec messages to hal.Then hit the curl command for getActiveSourceStatus and verify that status is true in output response.Deactivate and reactivate the HdmiCecSource plugin. Then hit the curl command for getActiveSourceStatus and verify that status is back to default(false) in output response.Verify the thunder logs for more info. send the curl command to send the standby messages to devices")
Utils.initiliaze_flask_for_HdmiCecSource()
print("---------------------------------------------------------------------------------------------------------------------------")
set_response = Utils.send_curl_command(HdmiCecSourceApis.send_standby_message)
if set_response:
    Utils.info_log("curl command send for standby_message")
else:
    Utils.error_log("curl command send failed for sending standby_message")
print("")

# send messages required for getting power status of device
print("---------------------------------------------------------------------------------------------------------------------------")
message2_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.getPowerStatusMessage)))
time.sleep(3)
if "200" in str(message2_response):
    Utils.info_log("send the emulated message for get power status success")
else:
    Utils.error_log("emulated message for get power status is failed")
print("")

# send messages required for reporting power status of device
print("---------------------------------------------------------------------------------------------------------------------------")
message2_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.reportPowerStatusMessage)))
time.sleep(3)
if "200" in str(message2_response):
    Utils.info_log("send the emulated message for report power status success")
else:
    Utils.error_log("emulated message for report power status is failed")
print("")

# send the curl command to perform OTP action
print("---------------------------------------------------------------------------------------------------------------------------")
curl_response = Utils.send_curl_command(HdmiCecSourceApis.perform_otp_action)
if curl_response:
    Utils.warning_log("curl command send for perform_otp_action")
else:
    Utils.error_log("curl command send failed for perform_otp_action")
print("")

# send messages required for getting power status of device
print("---------------------------------------------------------------------------------------------------------------------------")
message4_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.getPowerStatusMessage)))
time.sleep(3)
if "200" in str(message4_response):
    Utils.info_log("send emulated message for get power status success")
else:
    Utils.error_log("send emulated message for get power status failed")
print("")

# send messages required for reporting power status of device
print("---------------------------------------------------------------------------------------------------------------------------")
message4_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.reportPowerStatusMessage)))
time.sleep(3)
if "200" in str(message2_response):
    Utils.info_log("send the emulated message for report power status success")
else:
    Utils.error_log("emulated message for report power status is failed")
print("")

# store the expected output response of testcase before deactivating plugin
expected_output_response1 = '{"jsonrpc":"2.0","id":42,"result":{"status":true,"success":true}}'

# store the expected output response of testcase after deactivating and reactivating plugin
expected_output_response2 = '{"jsonrpc":"2.0","id":42,"result":{"status":false,"success":true}}'

# send the curl command for getActiveSourceStatus before deactivating plugin
curl_response1 = Utils.send_curl_command(HdmiCecSourceApis.get_active_source_status)

if curl_response1:
    Utils.info_log("send the curl command for getActiveSourceStatus before deactivating plugin success")
else:
    Utils.error_log("send the curl command for getActiveSourceStatus before deactivating plugin failed")
print("")

Utils.warning_log("deactivate and activate the plugin")
Utils.send_curl_command(HdmiCecSourceApis.deactivate_command)
Utils.send_curl_command(HdmiCecSourceApis.activate_command)

# send the curl command for getActiveSourceStatus after deactivating and reactivating plugin
curl_response2 = Utils.send_curl_command(HdmiCecSourceApis.get_active_source_status)
if curl_response2:
    Utils.info_log("send the curl command for getActiveSourceStatus after deactivating and reactivating plugin success")
else:
    Utils.error_log("send the curl command for getActiveSourceStatus after deactivating and reactivating plugin failed")
print("---------------------------------------------------------------------------------------------------------------------------")
# compare both expected and received output responses
if str(curl_response1) == str(expected_output_response1) and str(curl_response2) == str(expected_output_response2):
    status = 'Pass'
    message = 'Output response is matching with expected one. The active source status is switching properly'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID012_sendStandbyMessage_performOTPAction_getActiveSourceStatus'
print("Testcase ID : " + tc_id)
print("Testcase Output Response : " + curl_response1)
print("Testcase Status : " + status)
print("Testcase Message : " + message)
print("")

if status == 'Pass':
    ReportGenerator.passed_tc_list.append(tc_id)
else:
    ReportGenerator.failed_tc_list.append(tc_id)
Utils.initiliaze_flask_for_HdmiCecSource()
# push the testcase execution details to report file
ReportGenerator.append_test_results_to_csv(tc_id, curl_response1, status, message)
