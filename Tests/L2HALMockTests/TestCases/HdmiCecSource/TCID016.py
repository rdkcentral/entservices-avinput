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
# Testcase Description : Verify that cec enable status is true initially. Then we change the return value
# of HdmiCecOpen as -1 using updateAPIConfig. Deactivate and reactivate the HdmiCecSource plugin.
# After that hit the curl command and verify that cec enable status changed to false. Perform the post-condition
# to revert the changes back to default state

import Config
from Utilities import Utils, ReportGenerator
from HdmiCecSource import CecUtils
from HdmiCecSource import HdmiCecSourceApis

print("Testcase Description - Verify that cec enable status is true initially. Then we change the return value of HdmiCecOpen as -1 using updateAPIConfig. Deactivate and reactivate the HdmiCecSource plugin. After that hit the curl command and verify that cec enable status changed to false. Perform the post-condition to revert the changes back to default state store the expected output response with HdmiCecOpen return value as 0")
print("---------------------------------------------------------------------------------------------------------------------------")
expected_output_response1 = '{"jsonrpc":"2.0","id":42,"result":{"enabled":true,"success":true}}'

# store the expected output response with HdmiCecOpen return value as -1
expected_output_response2 = '{"jsonrpc":"2.0","id":42,"result":{"enabled":false,"success":true}}'
Utils.initiliaze_flask_for_HdmiCecSource()
# send the curl command and fetch the output json response with HdmiCecOpen return value as 0
curl_response1 = Utils.send_curl_command(HdmiCecSourceApis.get_enabled)
if curl_response1:
    Utils.info_log("send the curl command for get_enabled")
else:
    Utils.error_log("send curl command for get_enabled failed")
print("")
# change the return value of HdmiCecOpen hal API to -1 using updateAPIConfig API
update_api=CecUtils.cec_update_api_overrides(Config.cec_minus_one)
if update_api:
    Utils.info_log("configuring the return value of HdmiCecOpen as -1 using flask api")
else:
    Utils.error_log("failed to configure the return value of HdmiCecOpen as -1")
print("")
# send the curl command and fetch the output json response with HdmiCecOpen return value as -1
curl_response2 = Utils.send_curl_command(HdmiCecSourceApis.get_enabled)
if curl_response2:
    Utils.info_log("curl command send for get_enabled")
else:
    Utils.error_log("curl command send for get_enabled failed")
print("---------------------------------------------------------------------------------------------------------------------------")
# compare both expected and received output responses
if str(curl_response1) == str(expected_output_response1) and str(curl_response2) == str(expected_output_response2):
    status = 'Pass'
    message = 'Output response is matching with expected one. The cec enable status is true initially and ' \
              'after changing the return value, cec enable status changed to false'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID016_getEnabled_False_HAL_value'
print("Testcase ID : " + tc_id)
print("Testcase Output Response : " + curl_response2)
print("Testcase Status : " + status)
print("Testcase Message : " + message)

if status == 'Pass':
    ReportGenerator.passed_tc_list.append(tc_id)
else:
    ReportGenerator.failed_tc_list.append(tc_id)

# push the testcase execution details to report file
ReportGenerator.append_test_results_to_csv(tc_id, curl_response2, status, message)

# post condition : change the return value of HdmiCecOpen hal API back to 0
# post = CecUtils.cec_post_condition_for_negative_scenarios()
print("")
Utils.initiliaze_flask_for_HdmiCecSource()
Utils.warning_log("Reset - changed the return value of HdmiCecOpen hal API back to 0")

