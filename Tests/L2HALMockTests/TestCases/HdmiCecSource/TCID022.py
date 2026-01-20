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

# Testcase ID : TCID022
# Testcase Description : To verify the emulation of the processes which are triggered when 3 devices are connected.
# Initially, when all the devices are on,one device will be an inactive source and another will be an active source. 
# After that, the devices go into standby and the user sets the the other device into active source. The devices are then woken up using the one-touch play feature.
# After waking up, one device will be an active source, while the other will be an inactive source.
# As the device enters the active state, it may trigger the text view on and image view on processes also as a result.
import subprocess
import json
import requests
import time
import Config
from Utilities import Utils, ReportGenerator
from HdmiCecSource import HdmiCecSourceApis

# store the expected output response
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"success":true}}'

print("TC Description - To verify the emulation of the processes which are triggered when 3 devices are connected. Send the corresponding messages for HAL. Hit the curlcommand for sendStandbyMessage and performOTPAction. Send the corresponding messages to hal. Verify the output response. One source will be active and the other inactive and vice-versa. Verify that the text view on and the image view on processes are also triggered.")

Utils.initiliaze_flask_for_HdmiCecSource()
time.sleep(3)
#send messages required for getting inactive source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.inactive_source_hisense)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for querying the inactive source")
else:
     Utils.error_log("sendMessage emulation failed for querying the inactive source")
time.sleep(3)
print("")

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.request_active_source_firestick1)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the active source")
else:
     Utils.error_log("sendMessage emulation failed for requesting the active source")
time.sleep(3)
print("")

#send messages required for getting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.active_source_firestick1)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for querying the active source")
else:
     Utils.error_log("sendMessage emulation failed for querying the active source")
time.sleep(3)
print("")

# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSourceApis.send_standby_message)
if curl_response:
     Utils.warning_log("send_standby_message curl command sent from the test runner")
else:
     Utils.error_log("send_standby_message curl command failed")

# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSourceApis.perform_otp_action)
if curl_response:
     Utils.warning_log("perform_otp_action curl command sent from the test runner")
else:
     Utils.error_log("perform_otp_action curl command failed")

#send messages required for getting inactive source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.inactive_source_firestick1)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for querying the inactive source")
else:
     Utils.error_log("sendMessage emulation failed for querying the inactive source")
time.sleep(3)
print("")

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.request_active_source_hisense)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the active source")
else:
     Utils.error_log("sendMessage emulation failed for requesting the active source")
time.sleep(3)
print("")

#send messages required for set stream path
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.set_stream_path_hisense)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for set stream path to hisense")
else:
     Utils.error_log("sendMessage emulation failed for set stream path to hisense")
time.sleep(3)
print("")

#send messages required for routing change
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.routing_change)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for routing change to hisense")
else:
     Utils.error_log("sendMessage emulation failed for routing change to hisense")
time.sleep(3)
print("")

#send messages required for routing information
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.routing_information_hisense)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for querying routing information from hisense")
else:
     Utils.error_log("sendMessage emulation failed for querying routing information from hisense")
time.sleep(3)
print("")

#send messages required for getting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.active_source_hisense)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for querying the active source")
else:
     Utils.error_log("sendMessage emulation failed for querying the active source")
time.sleep(3)
print("")

#send messages required for image view on
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.image_view_on)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for querying the image view on")
else:
     Utils.error_log("sendMessage emulation failed for querying the image view on")
time.sleep(3)
print("")

#send messages required for getting text view on
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.text_view_on)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for querying the text view on")
else:
     Utils.error_log("sendMessage emulation failed for querying the text view on")
time.sleep(3)
print("")

#send messages required for getting osd string
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.set_osd_string)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for getting the osd string")
else:
     Utils.error_log("sendMessage emulation failed for getting the osd string")
time.sleep(3)
print("")


#send messages required for getting device power status
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.give_device_power_status_hisense)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for querying the power status")
else:
     Utils.error_log("sendMessage emulation failed for querying the power status")
time.sleep(3)
print("")

#send messages required for getting osd name
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.give_osd_name_hisense)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for querying the osd name")
else:
     Utils.error_log("sendMessage emulation failed for querying the osd name")
time.sleep(3)
print("")


print("---------------------------------------------------------------------------------------------------------------------------")

# compare both expected and received output responses
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one.'
else:
    status = 'Fail'
    message = 'Output response is different from expected one.'

# generate logs in terminal
tc_id = 'TCID022_process emulation'
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
Utils.initiliaze_flask_for_HdmiCecSource()
op=Utils.netstat_output()
print(op)



