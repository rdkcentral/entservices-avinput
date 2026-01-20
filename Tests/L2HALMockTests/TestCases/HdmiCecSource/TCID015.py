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

# Testcase ID : TCID015
# Testcase Description : Add a new invalid device to cec network and verify that invalid device data
# is ignored in output response

import requests
import json
import time
import Config
from Utilities import Utils, ReportGenerator
from HdmiCecSource import HdmiCecSourceApis
from HdmiCecSource import CecUtils

# store the expected output response before adding invalid device data
expected_output_response = Utils.send_curl_command(HdmiCecSourceApis.get_device_list)

# add an invalid device to network
add_device_response = requests.get("http://{}/Database.updateDeviceConfig/addDevice/{}".format(
    Config.flask_server_ip, json.dumps(Config.invalid_device_data)))
print("Inside TCID015 : " + add_device_response.text + " : " + str(Config.invalid_device_data))
time.sleep(3)

# send the curl command after adding invalid device data and fetch the output json response
# Utils.send_curl_command(HdmiCecSourceApis.get_device_list)
# time.sleep(10)
# Utils.send_curl_command(HdmiCecSourceApis.get_device_list)
# time.sleep(10)
Utils.send_curl_command(HdmiCecSourceApis.get_device_list)
time.sleep(3)
curl_response = Utils.send_curl_command(HdmiCecSourceApis.get_device_list)

# compare both expected and received output responses
if str(curl_response) == str(expected_output_response) and len(curl_response) > 5:
    status = 'Pass'
    message = 'Output response is matching with expected one. The newly added invalid device ' \
              'is not listed in output response'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID015_getDeviceList_Add_an_invalid_device'
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

# post condition - restart thunder & websocket - then activate plugin again
# time.sleep(15)
# Utils.restart_services()
# time.sleep(15)
# CecUtils.activate_cec()
# time.sleep(15)
print("")
