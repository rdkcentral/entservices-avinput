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

# Testcase ID : TCID012_requestShortAudioDescriptor
# Testcase Description : Sends the CEC Request Short Audio Descriptor (SAD) message as an event and verify the output response

from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis
import Config
import requests
import json
import time

print("TC Description - Sends the CEC Request Short Audio Descriptor (SAD) message,and verify the output response")
print("---------------------------------------------------------------------------------------------------------------------------")
# store the expected output response
Utils.initialize_flask()
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"success":true}}'

# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSinkApis.request_short_audio_descriptor)
if curl_response:
     Utils.info_log("curl command to request short audio descriptor is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

time.sleep(3)
#ToDo - sendMessage emulation for shortAudio descriptor event [Triggered when SAD is received from the connected audio device.]
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.reportShortAudioDes)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the reportShortAudioDes")
else:
     Utils.error_log("sendMessage emulation failed for requesting the reportShortAudioDes")
time.sleep(3)
print("")


print("---------------------------------------------------------------------------------------------------------------------------")
# compare both expected and received output responses
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one. The default vendor id ' \
              'is obtained in output response'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID012_HDMICECSINK_requestShortAudioDescriptor'
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
