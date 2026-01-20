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

# Testcase ID : TCID006
# Testcase Description : Set the cec enable status to false and verify that cec enable
# status is false in output response

from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis
import json

print(" TC Description - Sets a vendor ID used by host device.")
# send the curl command to set the cec enable status to false
Utils.initialize_flask()
print("---------------------------------------------------------------------------------------------------------------------------")
pre_expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"vendorid":"019fb","success":true}}'
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"vendorid":"04455","success":true}}'

pre_get_response = Utils.send_curl_command(HdmiCecSinkApis.get_vendor_id)
if pre_get_response:
    Utils.warning_log("send curl command to get the vendor id  is success")
else:
    Utils.error_log("send curl command to get the vendor id is false failed")
print("")

set_response = Utils.send_curl_command(HdmiCecSinkApis.set_vendor_id)
print("---------------------------------------------------------------------------------------------------------------------------")
if set_response:
    Utils.warning_log("send the curl command to set the vendor id is success")
else:
    Utils.error_log("send the curl command to set the vendor id to false failed")
print("")

get_response = Utils.send_curl_command(HdmiCecSinkApis.get_vendor_id)
if get_response:
    Utils.warning_log("send curl command to get the vendor id  is success")
else:
    Utils.error_log("send curl command to get the vendor id is false failed")
print("")

# compare both expected and received output responses
if str(pre_get_response) == str(pre_expected_output_response) or str(get_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one. The cec enabled status is obtained ' \
              'as false in output response'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# set the cec enable status to true as a post condition
Utils.send_curl_command(HdmiCecSinkApis.set_enabled_true)
Utils.info_log("Reset the set enabled to TRUE")

# generate logs in terminal
tc_id = 'TCID006_setVendorid'
print("Testcase ID : " + tc_id)
print("Testcase Output Response : " + set_response)
print("Testcase Status : " + status)
print("Testcase Message : " + message)
print("")

if status == 'Pass':
    ReportGenerator.passed_tc_list.append(tc_id)
else:
    ReportGenerator.failed_tc_list.append(tc_id)
Utils.initialize_flask()
# push the testcase execution details to report file
ReportGenerator.append_test_results_to_csv(tc_id, set_response, status, message)                                                                                     
