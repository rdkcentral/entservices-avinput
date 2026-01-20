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

# Testcase ID : TCID029
# Testcase Description : Verify the current active source and its output response
import time
import json
import Config
import requests
from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis
from HdmiCecSource import HdmiCecSourceApis

print("TC Description - Verify the current active source and its output response")
print("---------------------------------------------------------------------------------------------------------------------------")
# store the expected output response
Utils.initialize_flask()
#expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"available":true,"logicalAddress":0,"physicalAddress":"0.0.1.0","deviceType":"TV","cecVersion":"Version 1.4","osdName":"TV Box","vendorID":"019fb","powerStatus":"On","port":"TV","success":true}}'
expected_output_response_with_aud_phyAddr = '{"jsonrpc":"2.0","id":42,"result":{"available":true,"logicalAddress":0,"physicalAddress":"0.0.0.0","deviceType":"TV","cecVersion":"Version 1.4","osdName":"TV Box","vendorID":"019fb","powerStatus":"Standby","port":"TV","success":true}}'
curl_response = Utils.send_curl_command(HdmiCecSinkApis.set_routing_change)
if curl_response:
    Utils.info_log("curl command send for set routing change")
    status = 'Pass'
    message = 'Output response is matching with expected one. The set latency info ' \
              'is obtained in output response'
else:
    Utils.error_log("curl command send failed {}" .format(HdmiCecSinkApis.set_routing_change))
    status = 'False'
    message = 'Output response is not matching with expected one'
time.sleep(3)

curl_response = Utils.send_curl_command(HdmiCecSinkApis.get_Active_Source)
if curl_response:
    Utils.info_log("curl command sent for get active source")
else:
    Utils.error_log("Curl command not send {}" .format(HdmiCecSinkApis.get_Active_Source))

#sendMessage emulations for image view on and text view on 
#send messages required for requesting active source
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

#send messages required for osd string
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.set_osd_string)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the set_osd_string")
else:
     Utils.error_log("sendMessage emulation failed for requesting the set_osd_string")
time.sleep(3)
print("")

print("---------------------------------------------------------------------------------------------------------------------------")
# compare both expected and received output responses
if str(curl_response) == str(expected_output_response_with_aud_phyAddr):
    status = 'Pass'
    message = 'Output response is matching with expected one. current active source status ' \
              'is obtained in output response'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID_029_HDMICECSINK_getActiveSourcewithroutingChange.py'
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
