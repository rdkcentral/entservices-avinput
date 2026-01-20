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
# Testcase Description : Set the cec enable status to false and verify that cec enable
# status is false in output response

from Utilities import Utils, ReportGenerator
from HdmiCecSource import HdmiCecSourceApis

print(" TC Description - Set the cec enable status to false and verify that cec enable status is false in output response")
# send the curl command to set the cec enable status to false
Utils.initiliaze_flask_for_HdmiCecSource()
print("---------------------------------------------------------------------------------------------------------------------------")
set_response = Utils.send_curl_command(HdmiCecSourceApis.set_enabled_false)
if set_response:
    Utils.warning_log("send the curl command to set the cec enable status to false is success")
else:
    Utils.error_log("send the curl command to set the cec enable status to false failed")
print("")
# store the expected output response of testcase
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"enabled":false,"success":true}}'

# send the curl command to get enable status of cec and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSourceApis.get_enabled)
if curl_response:
    Utils.info_log("send the curl command to get enable status of cec and fetch the output json response is success")
else:
    Utils.error_log("send the curl command to get enable status of cec and fetch the output json response is failed")
print("---------------------------------------------------------------------------------------------------------------------------")
# compare both expected and received output responses
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one. The cec enabled status is obtained ' \
              'as false in output response'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# set the cec enable status to true as a post condition
Utils.send_curl_command(HdmiCecSourceApis.set_enabled_true)
Utils.info_log("Reset the set enabled to TRUE")

# generate logs in terminal
tc_id = 'TCID007_setEnabled_CEC_False'
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
