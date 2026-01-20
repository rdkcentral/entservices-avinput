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

# Testcase ID : TCID027
# Testcase Description : To verify that the getActiveourceStatus scenario is true after set stream path and routing change.
# Initially, the getActiveSourceStatus curl command will return a false status. When all the devices are on,one device will be an inactive source and another will be an active source.
# After that, the stream path and routing is changed. The device which was inactive becomes active and vice-versa. The devices are then woken up using the one-touch play feature.
# After waking up, the getActiveSourceStatus curl command will return a true status.

import subprocess
import json
import requests
import time
import Config
from Utilities import Utils, ReportGenerator
from HdmiCecSource import HdmiCecSourceApis

# store the expected output response
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"status":true,"success":true}}'

print("TC Description - To verify that the getActiveSourceStatus curl command returns a true status after set stream path and routing change.")
Utils.initiliaze_flask_for_HdmiCecSource()
time.sleep(3)
# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSourceApis.get_active_source_status)
if curl_response:
    Utils.info_log("curl command send for get_active_source")
else:
    Utils.error_log("curl command send failed for get_active_source")

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


message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.active_source_hisense)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for querying the active source")
else:
     Utils.error_log("sendMessage emulation failed for querying the active source")
time.sleep(3)
print("")

curl_response = Utils.send_curl_command(HdmiCecSourceApis.perform_otp_action)
if curl_response:
    Utils.info_log("curl command send for perform_otp_action")
else:
    Utils.error_log("curl command send failed for perform_otp_action")


print("---------------------------------------------------------------------------------------------------------------------------")
# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSourceApis.get_active_source_status)
if curl_response:
    Utils.info_log("curl command send for get_active_source")
else:
    Utils.error_log("curl command send failed for get_active_source")

# compare both expected and received output responses
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID027_getActiveSourceStatus_True'
print("Testcase ID : " + tc_id)
print("Testcase Output Response : " + curl_response)
print("Testcase Status : " + status)
print("Testcase Message : " + message)
print("")

if status == 'Pass':
    ReportGenerator.passed_tc_list.append(tc_id)
else:
    ReportGenerator.failed_tc_list.append(tc_id)
Utils.initiliaze_flask_for_HdmiCecSource()
# push the testcase execution details to report file
ReportGenerator.append_test_results_to_csv(tc_id, curl_response, status, message)
