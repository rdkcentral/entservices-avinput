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

# Testcase ID : TCID011
# Testcase Description : Hit the curl command for sendKeyPressEvent and
# verify that output response is correct

from Utilities import Utils, ReportGenerator
from HdmiCecSource import HdmiCecSourceApis

# store the expected output response
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"success":true}}'
Utils.initiliaze_flask_for_HdmiCecSource()

keypress = [HdmiCecSourceApis.send_keypress_VOLUME_UP, HdmiCecSourceApis.send_keypress_VOLUME_DOWN, HdmiCecSourceApis.send_keypress_MUTE,
            HdmiCecSourceApis.send_keypress_UP, HdmiCecSourceApis.send_keypress_DOWN, HdmiCecSourceApis.send_keypress_LEFT,
            HdmiCecSourceApis.send_keypress_RIGHT, HdmiCecSourceApis.send_keypress_SELECT, HdmiCecSourceApis.send_keypress_HOME,
            HdmiCecSourceApis.send_keypress_BACK, HdmiCecSourceApis.send_keypress_NUMBER_0, HdmiCecSourceApis.send_keypress_NUMBER_1,
            HdmiCecSourceApis.send_keypress_NUMBER_2, HdmiCecSourceApis.send_keypress_NUMBER_3, HdmiCecSourceApis.send_keypress_NUMBER_4,
            HdmiCecSourceApis.send_keypress_NUMBER_5, HdmiCecSourceApis.send_keypress_NUMBER_6, HdmiCecSourceApis.send_keypress_NUMBER_7,
            HdmiCecSourceApis.send_keypress_NUMBER_8, HdmiCecSourceApis.send_keypress_NUMBER_9]
print("TC Description - Hit the curl command for sendKeyPressEvent and verify that output response is correct")
print("---------------------------------------------------------------------------------------------------------------------------")
# send the curl command and fetch the output json response
for command in keypress:
    curl_response = Utils.send_curl_command(command)
    
if curl_response:
    Utils.info_log("curl command send for send_keypress_event")
else:
    Utils.error_log("curl command send failed")
print("")
# compare both expected and received output responses
print("---------------------------------------------------------------------------------------------------------------------------")
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID011_sendKeyPressEvent'
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
