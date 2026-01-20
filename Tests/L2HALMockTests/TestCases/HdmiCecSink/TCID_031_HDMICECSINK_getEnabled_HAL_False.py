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

# Testcase ID : TCID031_getEnabled_HAL_False
# Testcase Description : Verify the driver enabled status, with different enum values for HDMICECGETLOGICALADDRESS HAL API

from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis
from HdmiCecSource import CecUtils,HdmiCecSourceApis
import Config
import time

print("TC Description - Returns HDMI-CEC enabled status with different enum values for HDMICECGETLOGICALADDRESS HAL API")
print("---------------------------------------------------------------------------------------------------------------------------")
# store the expected output response
Utils.initialize_flask()
time.sleep(3)
    #ToDo- how to dynamically pass the config here find a way config already written in python side , to do changes in hal c side.
def sink_update(data):
    CecUtils.cec_sink_tx_fail(data)
    Utils.info_log("Updated Outparams result with next enum combination")
#CecUtils.cec_sink_tx_fail(Config.api_data_sink)
#Utils.info_log("Updated HdmiCecOpen HAL API return values to -1")
# store the expected output response
#expected_output_response = '{"jsonrpc":"2.0","id":42,"error":{"code":1,"message":"ERROR_GENERAL"}}'

expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"enabled":true,"success":true}}'

def main_scenario():
    curl_response1 = Utils.send_curl_command(HdmiCecSinkApis.get_enabled)
    if curl_response1:
        Utils.info_log("curl command to get enabled is sent from the test runner")
    else:
        Utils.error_log("curl command invoke failed")

# send the curl command and fetch the output json response
    curl_response = Utils.send_curl_command(HdmiCecSinkApis.get_enabled)
    if curl_response:
        Utils.info_log("curl command to get enabled is sent from the test runner")
    else:
        Utils.error_log("curl command invoke failed")

    return curl_response1
sink_update(Config.api_data_sink_1)
time.sleep(2)
curl_response1 = main_scenario()


sink_update(Config.api_data_sink_2)
time.sleep(2)
curl_response1 = main_scenario()


sink_update(Config.api_data_sink_3)
time.sleep(2)
curl_response1 = main_scenario()


sink_update(Config.api_data_sink_4)
time.sleep(2)
curl_response1 = main_scenario()


sink_update(Config.api_data_sink_5)
time.sleep(2)
curl_response1 = main_scenario()


sink_update(Config.api_data_sink_6)
time.sleep(2)
curl_response1 = main_scenario()


sink_update(Config.api_data_sink_7)
time.sleep(2)
curl_response1 = main_scenario()


print("---------------------------------------------------------------------------------------------------------------------------")
# compare both expected and received output responses
if str(curl_response1) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one. The driver enabled status ' \
              'is obtained in output response'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

Utils.info_log("Bringing the system state back to normal")
Utils.initialize_flask()
# generate logs in terminal
tc_id = 'TCID031_HdmiCecSink_getEnabled_HAL_False'
print("Testcase ID : " + tc_id)
print("Testcase Output Response : " + curl_response1)
print("Testcase Status : " + status)
print("Testcase Message : " + message)
print("")

if status == 'Pass':
    ReportGenerator.passed_tc_list.append(tc_id)
else:
    ReportGenerator.failed_tc_list.append(tc_id)
Utils.initialize_flask()
# push the testcase execution details to report file
ReportGenerator.append_test_results_to_csv(tc_id, curl_response1, status, message)
