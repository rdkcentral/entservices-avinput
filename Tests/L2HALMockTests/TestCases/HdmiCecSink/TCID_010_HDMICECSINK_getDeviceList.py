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

# Testcase ID : TCID010
# Testcase Description : Gets the number of connected source devices and system information for each device. The information includes device type, physical address, CEC version, vendor ID, power status and OSD name., and verify the output response

from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis
import time

print("TC Description - Gets the number of connected source devices and system information for each device. The information includes device type, physical address, CEC version, vendor ID, power status and OSD name.")
print("---------------------------------------------------------------------------------------------------------------------------")
# store the expected output response
Utils.initialize_flask()
time.sleep(6)
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"numberofdevices":3,"deviceList":[{"logicalAddress":3,"physicalAddress":"15.15.15.15","deviceType":"Reserved","cecVersion":"Version 1.3a","osdName":"@GStreaming Tw","vendorID":"000","powerStatus":"On","portNumber":-1},{"logicalAddress":5,"physicalAddress":"15.15.15.15","deviceType":"TV","cecVersion":"Unknown","osdName":"","vendorID":"000","powerStatus":"On","portNumber":-1},{"logicalAddress":9,"physicalAddress":"15.15.15.15","deviceType":"TV","cecVersion":"Unknown","osdName":"","vendorID":"000","powerStatus":"On","portNumber":-1}],"success":true}}'

# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSinkApis.get_device_list)
if curl_response:
     Utils.info_log("curl command to get devicelist is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

print("---------------------------------------------------------------------------------------------------------------------------")
# compare both expected and received output responses
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one. The default vendor id ' \
              'is obtained in output response'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID005_HdmiCecSink_getDevicelist'
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
