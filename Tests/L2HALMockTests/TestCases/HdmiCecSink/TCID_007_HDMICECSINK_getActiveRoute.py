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

# Testcase ID : TCID007
# Testcase Description : Gets details for the current route from the source to sink devices, and bverify the output response
import requests
import Config
import json
import time

from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis
from HdmiCecSource import HdmiCecSourceApis

print("TC Description - Gets details for the current route from the source to sink devices")
print("---------------------------------------------------------------------------------------------------------------------------")
# store the expected output response
Utils.initialize_flask()
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"success":true}}'
expected_output_response_ = '{"jsonrpc":"2.0","id":42,"result":{"available":true,"ActiveRoute":"TV","success":true}}'


#send messages required for image and text view in active device making sink device active
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.imageViewON)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the imageViewON")
else:
     Utils.error_log("sendMessage emulation failed for requesting the imageViewON")
time.sleep(3)
print("")

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.textViewON)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the textViewON")
else:
     Utils.error_log("sendMessage emulation failed for requesting the textViewON")
time.sleep(3)
print("")

curl_response1 = Utils.send_curl_command(HdmiCecSinkApis.set_routing_change)
if curl_response1:
     Utils.info_log("curl command to set routing change is success")
else:
     Utils.error_log("curl command to set routing change is failed")

# send the curl command and fetch the output json response
curl_response2 = Utils.send_curl_command(HdmiCecSinkApis.get_active_route)
if curl_response2:
     Utils.info_log("curl command to get ActiveRoute is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")



print("---------------------------------------------------------------------------------------------------------------------------")
# compare both expected and received output responses
if str(curl_response1) == str(expected_output_response) and str(curl_response2) == str(expected_output_response_):
    status = 'Pass'
    message = 'Output response is matching with expected one. The default vendor id ' \
              'is obtained in output response'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID007_HdmiCecSink_getActiveRoute'
print("Testcase ID : " + tc_id)
print("Testcase Output Response : " + curl_response2)
print("Testcase Status : " + status)
print("Testcase Message : " + message)
print("")

if status == 'Pass':
    ReportGenerator.passed_tc_list.append(tc_id)
else:
    ReportGenerator.failed_tc_list.append(tc_id)
Utils.initialize_flask()
# push the testcase execution details to report file
ReportGenerator.append_test_results_to_csv(tc_id, curl_response2, status, message)
