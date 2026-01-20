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

# Testcase ID : TCID037
# Testcase Description : To verify the emulation of sending of events

import subprocess
import Config
from Utilities import Utils, ReportGenerator
from HdmiCecSource import HdmiCecSourceApis
from HdmiCecSink import HdmiCecSinkApis
import time


# store the expected output response
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"enabled":true,"success":true}}'

print("TC Description - To verify the emulation of sending of events.")
Utils.initialize_flask()
time.sleep(3)
# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSinkApis.get_enabled)
time.sleep(1)
if curl_response:
     Utils.warning_log("activate curl command sent from the test runner")
else:
     Utils.error_log("activate curl command failed")

#Define the script to be executed
execute_script = '../../../../../sendEvents.sh'


#Execute the script
try:
    result = subprocess.run(['/bin/bash', execute_script], check=True, capture_output=True, text=True)
    print("sendEvents.sh executed successfully.")
    print("Output:\n", result.stdout)
    
except subprocess.CalledProcessError as e:
    print("Error occured while executing the shell script.")
    print("Error message:\n", e.stderr)

print("---------------------------------------------------------------------------------------------------------------------------")

# compare both expected and received output responses
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one.'
else:
    status = 'Fail'
    message = 'Output response is different from expected one.'

# generate logs in terminal
tc_id = 'TCID025_sending events'
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

