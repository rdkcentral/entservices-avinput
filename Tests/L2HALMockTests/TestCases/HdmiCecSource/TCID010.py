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

# Testcase ID : TCID010
# Testcase Description : Set the OTP enabled status to false and verify that otp enabled
# status is false in output response. Change the OTP enabled status back to true as post condition

from Utilities import Utils, ReportGenerator
from HdmiCecSource import HdmiCecSourceApis

print("TC Description - Set the OTP enabled status to false and verify that otp enabled status is false in output response. Change the OTP enabled status back to true as post condition")
print("---------------------------------------------------------------------------------------------------------------------------")
# send the curl command to set the otp enabled status to false
set_response = Utils.send_curl_command(HdmiCecSourceApis.set_otp_enabled_false)
if set_response:
    Utils.warning_log("send the curl command to set the otp enabled status to false is success")
else:
    Utils.error_log("send the curl command to set the otp enabled status to false is failed")
print("")
# store the expected output response of testcase
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"enabled":false,"success":true}}'

# send the curl command to get enabled status of otp and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSourceApis.get_otp_enabled)
if curl_response:
    Utils.info_log("curl command send for get_otp_enabled")
else:
    Utils.error_log("curl command send for get_otp_enabled failed")
print("---------------------------------------------------------------------------------------------------------------------------")
# compare both expected and received output responses
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one. The otp enabled status is obtained ' \
              'as false in output response'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'
print("")
# set the otp enable status to true as a post condition
Utils.send_curl_command(HdmiCecSourceApis.set_otp_enabled_true)
Utils.info_log("Reset OTP to its initial state")

# generate logs in terminal
tc_id = 'TCID010_setOTPEnabled_false'
print("Testcase ID : " + tc_id)
print("Testcase Output Response : " + curl_response)
print("Testcase Status : " + status)
print("Testcase Message : " + message)
print("")

if status == 'Pass':
    ReportGenerator.passed_tc_list.append(tc_id)
else:
    ReportGenerator.failed_tc_list.append(tc_id)

# push the testcase execution details to report file
ReportGenerator.append_test_results_to_csv(tc_id, curl_response, status, message)
