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

# Testcase ID : TCID019
# Testcase Description : To verify the device list is empty and number of devices is 0 in output response
# for getDeviceList curl command when return value of HdmiCecOpen is changed to -1. Change the return
# value of HdmiCecOpen as -1 using updateAPIConfig. Deactivate and reactivate the HdmiCecSource plugin.
# After that hit the curl command and verify that the device list is empty. Perform the post-condition
# to revert the changes back to default state

import Config
from Utilities import Utils, ReportGenerator
from HdmiCecSource import CecUtils
from HdmiCecSource import HdmiCecSourceApis

post = CecUtils.cec_post_condition_for_negative_scenarios()
# change the return value of HdmiCecOpen hal API to -1 using updateAPIConfig API
print("TC Description - To verify the device list is empty and number of devices is 0 in output response for getDeviceList curl command when return value of HdmiCecOpen is changed to -1. Change the return value of HdmiCecOpen as -1 using updateAPIConfig. Deactivate and reactivate the HdmiCecSource plugin. After that hit the curl command and verify that the device list is empty. Perform the post-condition to revert the changes back to default state")
Utils.initiliaze_flask_for_HdmiCecSource()
CecUtils.cec_update_api_overrides(Config.cec_minus_one)
Utils.info_log("change the return value of HdmiCecOpen hal API to -1 using updateAPIConfig API")
print("")

print("---------------------------------------------------------------------------------------------------------------------------")
# store the expected output response
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"numberofdevices":0,"deviceList":[],"success":true}}'

# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSourceApis.get_device_list)
if curl_response:
    Utils.info_log("curl command send for get_device_list")
else:
    Utils.error_log("curl command send failed for get_device_list")
print("")
print("---------------------------------------------------------------------------------------------------------------------------")
# compare both expected and received output responses
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one. The device list is empty and number of devices' \
              'is 0 in output response'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID019_getDeviceList_HAL_False'
print("Testcase ID : " + tc_id)
print("Testcase Output Response : " + curl_response)
print("Testcase Status : " + status)
print("Testcase Message : " + message)

if status == 'Pass':
    ReportGenerator.passed_tc_list.append(tc_id)
else:
    ReportGenerator.failed_tc_list.append(tc_id)

# push the testcase execution details to report file
ReportGenerator.append_test_results_to_csv(tc_id, curl_response, status, message)

# post condition : change the return value of HdmiCecOpen hal API back to 0
CecUtils.cec_post_condition_for_negative_scenarios()
Utils.initiliaze_flask_for_HdmiCecSource()
Utils.warning_log("Reset - change the return values of HAL APIs back to 0")
print("")

