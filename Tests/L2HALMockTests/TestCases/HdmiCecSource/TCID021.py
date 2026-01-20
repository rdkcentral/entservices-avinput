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

# Testcase ID : TCID021
# Testcase Description : Coverage enhancements

import subprocess
import os
import signal
import json
import requests
import time
import Config
from Utilities import Utils, ReportGenerator
from HdmiCecSource import HdmiCecSourceApis


process_name_cec ="CecDaemonMain"
output = subprocess.check_output(["ps", "aux"])

# store the expected output response
expected_output_response = '{"jsonrpc":"2.0","id":3,"result":null}'

print("TC Description - To verify coverage enhancements")
Utils.initiliaze_flask_for_HdmiCecSource()
time.sleep(3)
# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSourceApis.deactivate_command)
if curl_response:
     Utils.warning_log("Deactivate curl command sent from the test runner")
else:
     Utils.error_log("Deactivate curl command failed")

try:
    pids = []
    # Parse the output to find the PID
    for line in output.decode().splitlines():
        if process_name_cec in line:
          pid = int(line.split()[1])
          pids.append(pid)
          print("killing CEC Daemon")
        else:
          pass

    if pids is not None:
        for each_pid in pids:
            try:
                os.system("kill -9 %s" % (each_pid, ))
            except:
                print("process already killed")
    else:
        print("They are no services up wrt HAL Mock setup")
except:
    pass

# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSourceApis.activate_command)
if curl_response:
     Utils.warning_log("activate curl command sent from the test runner")
else:
     Utils.error_log("activate curl command failed")

print("---------------------------------------------------------------------------------------------------------------------------")

# compare both expected and received output responses
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one. We are expecting opcode : 36 in thunder logs'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID021_coverage enhancements'
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

