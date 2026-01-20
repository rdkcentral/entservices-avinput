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

# Testcase ID : TCID024
# Testcase Description : To verify the emulation processes - abort and feature abort
import subprocess
import json
import requests
import time
import Config
from Utilities import Utils, ReportGenerator
from HdmiCecSource import HdmiCecSourceApis

# store the expected output response
expected_output_response = '{"jsonrpc":"2.0","id":3,"result":null}'

print("TC Description - To verify the emulation of the processes - abort and feature abort.")
Utils.initiliaze_flask_for_HdmiCecSource()
time.sleep(3)
# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSourceApis.activate_command)
if curl_response:
     Utils.warning_log("activate curl command sent from the test runner")
else:
     Utils.error_log("activate curl command failed")

#send messages required for abort
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.abort_hisense)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for abort")
else:
     Utils.error_log("sendMessage emulation failed for abort")
time.sleep(3)
print("")

#send messages required for feature abort
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.feature_abort_hisense)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for querying the abort reason")
else:
     Utils.error_log("sendMessage emulation failed for querying the abort reason")
time.sleep(3)
print("")

print("---------------------------------------------------------------------------------------------------------------------------")

# compare both expected and received output responses
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one.'
else:
    status = 'Fail'
    message = 'Output response is different from expected one.'

# generate logs in terminal
tc_id = 'TCID024_process emulation'
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
Utils.initiliaze_flask_for_HdmiCecSource()
op=Utils.netstat_output()
print(op)


