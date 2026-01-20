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

# Testcase ID : TCID033_setRoutingChange_negative
# Testcase Description : Changes routing while switching between HDMI inputs and TV. Verify the output response
from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis
from HdmiCecSource import HdmiCecSourceApis
import json
import requests
import time
import Config

Utils.initialize_flask()

expected_output_response = '{"jsonrpc":"2.0","id":42,"error":{"code":1,"message":"ERROR_GENERAL"}}'

#ToDo - send messages required for switching inputs between hdmi and TV, by sending active source message emulation.
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

curl_response = Utils.send_curl_command(HdmiCecSinkApis.set_routing_change_negative)
if curl_response:
    Utils.info_log("curl command send for set routing change")
    status = 'Pass'
    message = 'Output response is matching with expected one. The set latency info ' \
              'is obtained in output response'
else:
    Utils.error_log("curl command send failed {}" .format(HdmiCecSinkApis.set_routing_change_negative))
    status = 'False'
    message = 'Output response is not matching with expected one'

if str(curl_response) == str(expected_output_response):
    Utils.info_log("Test Case Passed")
else:
    Utils.error_log("Test Case Failed")
# generate logs in terminal
tc_id = 'TCID033_setRoutingChange_negative'
print("Testcase ID : " + tc_id)
print("Testcase Output Response : " + curl_response)
print("Testcase Status : " + status)
print("Testcase Message : " + message)
print("")

if status == 'Pass':
    ReportGenerator.passed_tc_list.append(tc_id)
else:
    ReportGenerator.failed_tc_list.append(tc_id)
Utils.initialize_flask()
# push the testcase execution details to report file
ReportGenerator.append_test_results_to_csv(tc_id, curl_response, status, message)
