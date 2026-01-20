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

# Testcase ID : TCID013
# Testcase Description : To verify all remote devices given by test user during hal initialization
# is present in output response with device details

import time
from HdmiCecSource import HdmiCecSourceApis
from Utilities import Utils, ReportGenerator

# Utils.restart_services()
# store the expected output response
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"numberofdevices":3,"deviceList":[{"logicalAddress":0,"osdName":"TV Box","vendorID":"04567"},{"logicalAddress":5,"osdName":"","vendorID":"4567"},{"logicalAddress":9,"osdName":"Streaming One","vendorID":"4567"}],"success":true}}'

# send the curl command and fetch the output json response
# Utils.send_curl_command(HdmiCecSourceApis.get_device_list)
# time.sleep(10)
# Utils.send_curl_command(HdmiCecSourceApis.get_device_list)
# time.sleep(10)
Utils.send_curl_command(HdmiCecSourceApis.get_device_list)
time.sleep(3)
curl_response = Utils.send_curl_command(HdmiCecSourceApis.get_device_list)

# compare both expected and received output responses
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID013_getDeviceList_verify_all_remote_devices'
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



