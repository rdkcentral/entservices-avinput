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
# Testcase Description : getEnabled - CEC Already Disabled

import json
import requests
import time
import Config
from Utilities import Utils, ReportGenerator
from HdmiCecSource import HdmiCecSourceApis

# store the expected output response
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"enabled":false,"success":true}}'

print("getEnabled - CEC Already Disabled")
Utils.initiliaze_flask_for_HdmiCecSource()
time.sleep(3)
# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSourceApis.set_enabled_false)
if curl_response:
     Utils.warning_log("set enabled  curl command sent from the test runner")
else:
     Utils.error_log("set enabled  curl command failed")

# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSourceApis.get_enabled)
if curl_response:
     Utils.warning_log("get enabled  curl command sent from the test runner")
else:
     Utils.error_log("get enabled  curl command failed")

# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSourceApis.set_enabled_false)
if curl_response:
     Utils.warning_log("set enabled curl command sent from the test runner")
else:
     Utils.error_log("set enabled curl command failed")

# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSourceApis.get_enabled)
if curl_response:
     Utils.warning_log("get enabled curl command sent from the test runner")
else:
     Utils.error_log("get enabled curl command failed")

post_condition = Utils.send_curl_command(HdmiCecSourceApis.set_enabled_true)

print("---------------------------------------------------------------------------------------------------------------------------")

# compare both expected and received output responses
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one.'
else:
    status = 'Fail'
    message = 'Output response is different from expected one.'

# generate logs in terminal
tc_id = 'TCID029_getEnabled - CEC Already Disabled'
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


