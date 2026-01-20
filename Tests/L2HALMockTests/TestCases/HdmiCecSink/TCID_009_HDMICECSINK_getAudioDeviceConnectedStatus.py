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

# Testcase ID : TCID009
# Testcase Description : Verify that default vendor id is obtained in output response

from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis

print("TC Description - Get status of audio device connection.")
print("---------------------------------------------------------------------------------------------------------------------------")
# store the expected output response
Utils.initialize_flask()
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"connected":true,"success":true}}'

#ToDo - Send Message emulation to connect an audio device and send its status message with sink device or give it from device config  by specifying the exact logical address required.

# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSinkApis.get_audio_device_connected_status)
if curl_response:
     Utils.info_log("curl command to get audio device connected status is sent from the test runner")
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
tc_id = 'TCID009_HdmiCecSink_getAudioDeviceConnectedStatus'
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
