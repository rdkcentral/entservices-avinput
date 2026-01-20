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

# Testcase ID : TCID014_getAudioStatusMessage
# Testcase Description : Sends the CEC <Give Audio Status> message to request the audio status., and verify the output response

from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis
from HdmiCecSource import HdmiCecSourceApis
import Config
import requests
import json
import time

print("TC Description - Sends the CEC <Give Audio Status> message to request the audio status.")
print("---------------------------------------------------------------------------------------------------------------------------")
# store the expected output response
Utils.initialize_flask()
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"success":true}}'



# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSinkApis.sendGetAudioStatusMessage)
if curl_response:
     Utils.info_log("curl command to send audio status message is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

time.sleep(3)

#ToDo - send message emulation for report audio status message event[reportAudioStatusEvent	Triggered when CEC message of device is received.]
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.reportShortAudioDes)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the reportShortAudioDes")
else:
     Utils.error_log("sendMessage emulation failed for requesting the reportShortAudioDes")
time.sleep(3)
print("")

time.sleep(3)

message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.reportAudioMode)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the reportAudioMode")
else:
     Utils.error_log("sendMessage emulation failed for requesting the reportAudioMode")
time.sleep(3)
print("")

print("---------------------------------------------------------------------------------------------------------------------------")
# compare both expected and received output responses
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one. The send audio status message and report audio sytatus event ' \
              'is obtained in output response'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID014_HdmiCecSink_getAudioStatusMessage'
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
