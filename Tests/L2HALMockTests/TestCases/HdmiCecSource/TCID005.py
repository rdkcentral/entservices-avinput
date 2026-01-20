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

# Testcase ID : TCID005
# Testcase Description : Set the OSD Name to new one using curl command and verify that new
# OSD Name set by the test user is obtained in output response

from Utilities import Utils, ReportGenerator
from HdmiCecSource import HdmiCecSourceApis

print("TC Description - Set the OSD Name to new one using curl command and verify that new OSD Name set by the test user is obtained in output response")
Utils.initiliaze_flask_for_HdmiCecSource()
print("---------------------------------------------------------------------------------------------------------------------------")
# send the curl command to set the new OSD Name
set_response = Utils.send_curl_command(HdmiCecSourceApis.set_osd_name)
if set_response:
    Utils.info_log(" sent the curl command to set the new OSD Name")
else:
    Utils.error_log("curl command sent to get the new OSD name failed")
print("")
# store the expected output response of testcase
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"name":"CUSTOM8 TV","success":true}}'

# send the curl command to get OSD Name and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSourceApis.get_osd_name)
if curl_response:
    Utils.warning_log("send the curl command to get_osd_name")
else:
    Utils.warning_log("curl command send failed to get_osd_name")

print("---------------------------------------------------------------------------------------------------------------------------")
# compare both expected and received output responses
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one. The new OSD Name given by user ' \
              'is obtained in output response'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'
Utils.initiliaze_flask_for_HdmiCecSource()
# generate logs in terminal
tc_id = 'TCID005_setOSDName_to_new'
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
