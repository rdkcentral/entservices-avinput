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

# Testcase ID : TCID016
# Testcase Description : Hit the curl command for sendUserControlledPressEvent and
# verify that output response is correct

from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis

# store the expected output response
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"success":true}}'
Utils.initialize_flask()

keypress = [HdmiCecSinkApis.send_keypress_VOLUME_UP_USER_Press, HdmiCecSinkApis.send_keypress_VOLUME_DOWN_USER_Press,
            HdmiCecSinkApis.send_keypress_MUTE_USER_Press,
            HdmiCecSinkApis.send_keypress_UP_USER_Press, HdmiCecSinkApis.send_keypress_DOWN_USER_Press,
            HdmiCecSinkApis.send_keypress_LEFT_USER_Press,
            HdmiCecSinkApis.send_keypress_RIGHT_USER_Press, HdmiCecSinkApis.send_keypress_SELECT_USER_Press,
            HdmiCecSinkApis.send_keypress_HOME_USER_Press,
            HdmiCecSinkApis.send_keypress_BACK_USER_Press, HdmiCecSinkApis.send_keypress_NUMBER_0_USER_Press,
            HdmiCecSinkApis.send_keypress_NUMBER_1_USER_Press,
            HdmiCecSinkApis.send_keypress_NUMBER_2_USER_Press, HdmiCecSinkApis.send_keypress_NUMBER_3_USER_Press,
            HdmiCecSinkApis.send_keypress_NUMBER_4_USER_Press,
            HdmiCecSinkApis.send_keypress_NUMBER_5_USER_Press, HdmiCecSinkApis.send_keypress_NUMBER_6_USER_Press,
            HdmiCecSinkApis.send_keypress_NUMBER_7_USER_Press,
            HdmiCecSinkApis.send_keypress_NUMBER_8_USER_Press, HdmiCecSinkApis.send_keypress_NUMBER_9_USER_Press]
print("TC Description - Hit the curl command for sendUserControlPressed and verify that output response is correct")
print(
    "---------------------------------------------------------------------------------------------------------------------------")
# send the curl command and fetch the output json response
for command in keypress:
    curl_response = Utils.send_curl_command(command)

if curl_response:
    Utils.info_log("curl command send for send_user_control_pressed")
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
tc_id = 'TCID016_sendUserControlPressed'
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
