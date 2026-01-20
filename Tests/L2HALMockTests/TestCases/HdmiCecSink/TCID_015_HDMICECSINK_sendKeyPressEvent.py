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

# Testcase ID : TCID015
# Testcase Description : Hit the curl command for sendKeyPressEvent and
# verify that output response is correct

from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis

# store the expected output response
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"success":true}}'
Utils.initialize_flask()

keypress = [HdmiCecSinkApis.send_keypress_VOLUME_UP, HdmiCecSinkApis.send_keypress_VOLUME_DOWN,
            HdmiCecSinkApis.send_keypress_MUTE,
            HdmiCecSinkApis.send_keypress_UP, HdmiCecSinkApis.send_keypress_DOWN,
            HdmiCecSinkApis.send_keypress_LEFT,
            HdmiCecSinkApis.send_keypress_RIGHT, HdmiCecSinkApis.send_keypress_SELECT,
            HdmiCecSinkApis.send_keypress_HOME,
            HdmiCecSinkApis.send_keypress_BACK, HdmiCecSinkApis.send_keypress_NUMBER_0,
            HdmiCecSinkApis.send_keypress_NUMBER_1,
            HdmiCecSinkApis.send_keypress_NUMBER_2, HdmiCecSinkApis.send_keypress_NUMBER_3,
            HdmiCecSinkApis.send_keypress_NUMBER_4,
            HdmiCecSinkApis.send_keypress_NUMBER_5, HdmiCecSinkApis.send_keypress_NUMBER_6,
            HdmiCecSinkApis.send_keypress_NUMBER_7,
            HdmiCecSinkApis.send_keypress_NUMBER_8, HdmiCecSinkApis.send_keypress_NUMBER_9]
print("TC Description - Hit the curl command for sendKeyPressEvent and verify that output response is correct")
print(
    "---------------------------------------------------------------------------------------------------------------------------")
# send the curl command and fetch the output json response
for command in keypress:
    curl_response = Utils.send_curl_command(command)

if curl_response:
    Utils.info_log("curl command send for send_keypress_event")
else:
    Utils.error_log("curl command send failed")
print("")
# compare both expected and received output responses
print(
    "---------------------------------------------------------------------------------------------------------------------------")
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID015_sendKeyPressEvent'
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
