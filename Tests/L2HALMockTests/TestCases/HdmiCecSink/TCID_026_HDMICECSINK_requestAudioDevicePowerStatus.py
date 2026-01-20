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

# Testcase ID : TCID026
# Testcase Description : requesting the audio device power status
from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis
import time
import Config
import json
import requests

print("TC Description - Requesting the audio device power status")
print("---------------------------------------------------------------------------------------------------------------------------")
# store the expected output response
Utils.initialize_flask()
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"success":true}}'

# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSinkApis.request_audio_device_power_status)
if curl_response:
     Utils.info_log("curl command to request audio device power status is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.reportShortAudioDes)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the reportShortAudioDes")
else:
     Utils.error_log("sendMessage emulation failed for requesting the reportShortAudioDes")
time.sleep(3)
print("")

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.setSystemAudioMode)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the setSystemAudioMode")
else:
     Utils.error_log("sendMessage emulation failed for requesting the setSystemAudioMode")
time.sleep(3)
print("")

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.reportAudioMode)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the reportAudioMode")
else:
     Utils.error_log("sendMessage emulation failed for requesting the reportAudioMode")
time.sleep(3)
print("")

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.givefeatures)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the givefeatures")
else:
     Utils.error_log("sendMessage emulation failed for requesting the givefeatures")
time.sleep(3)
print("")

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.requestcurrentlatency)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the requestcurrentlatency")
else:
     Utils.error_log("sendMessage emulation failed for requesting the requestcurrentlatency")
time.sleep(3)
print("")

print("---------------------------------------------------------------------------------------------------------------------------")
# compare both expected and received output responses
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one. The audio power device power status details ' \
              'are obtained in output response'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID026_HdmiCecSink_requestAudioDevicePowerStatus'
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
