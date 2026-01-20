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

# Testcase ID : TCID028
# Testcase Description : Emulating abort feature with several combinations.Here emulating with invalid vendorId
import time
import json
import Config
import requests
from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis
from HdmiCecSource import HdmiCecSourceApis

print("TC Description - Emulating abort feature with several combinations.Here emulating the feature abort trigger message by giving an invalid vendor ID ,so that system doesnt support")
print("---------------------------------------------------------------------------------------------------------------------------")
# store the expected output response
Utils.initialize_flask()
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"vendorid":"019fb","success":true}}'


def update_config(data):
    time.sleep(10)
    Utils.abort_data(data)

def feature_abort():
    message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
        Config.flask_server_ip, json.dumps(Config.feature_abort)))
    if "200" in str(message1_response):
         Utils.info_log("sendMessage emulation success for requesting the feature_abort")
    else:
         Utils.error_log("sendMessage emulation failed for requesting the feature_abort")
    time.sleep(3)
    print("")

#send messages required for requesting active source
def abort_hisense():
    message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
        Config.flask_server_ip, json.dumps(Config.abort_hisense)))
    if "200" in str(message1_response):
         Utils.info_log("sendMessage emulation success for requesting the abort_hisense")
    else:
         Utils.error_log("sendMessage emulation failed for requesting the abort_hisense")
    time.sleep(3)
    print("")

curl_response = Utils.send_curl_command(HdmiCecSinkApis.set_vendor_id_invalid_2)
if curl_response:
     Utils.warning_log("set vendor id invalid curl command sent from the test runner")
     Utils.info_log("curl command send for arc routing")
     status = 'Pass'
     message = 'Output response is matching with expected one. The arc routing ' \
               'is obtained in output response'
else:
    Utils.error_log("curl command send failed {}".format(HdmiCecSinkApis.set_arc_routing_params_enabled))
    status = 'False'
    message = 'Output response is not matching with expected one'



# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSinkApis.get_vendor_id)
if curl_response == expected_output_response:
     Utils.warning_log("get vendor id curl command sent from the test runner")
     Utils.info_log("curl command send for arc routing")
     status = 'Pass'
     message = 'Output response is matching with expected one. '
else:
     Utils.error_log("get vendor id curl command failed")
     status = 'Fail'
     message = 'Output response is not matching with expected one. The arc routing '


Utils.initialize_flask()
time.sleep(2)
#calling feature abort with existing config
feature_abort()
time.sleep(3)

#calling abort hisense with exsiting config
abort_hisense()
time.sleep(2)

#send the second config data for feature abort

update_config(Config.abort_data_1)

time.sleep(2)
feature_abort()
time.sleep(3)

abort_hisense()
time.sleep(2)

#send the third config data for feature abort


update_config(Config.abort_data_2)

time.sleep(2)
feature_abort()
time.sleep(3)

abort_hisense()
time.sleep(2)

#send the forth config data for feature abort


update_config(Config.abort_data_3)

time.sleep(2)
feature_abort()
time.sleep(3)

abort_hisense()
time.sleep(2)

#send the fifth config data for feature abort

update_config(Config.abort_data_4)

time.sleep(2)
feature_abort()
time.sleep(3)

abort_hisense()
time.sleep(2)

#send the sixth config data for feature abort


update_config(Config.abort_data_5)

time.sleep(2)
feature_abort()
time.sleep(3)

abort_hisense()
time.sleep(2)



# generate logs in terminal
tc_id = 'TCID_028_HDMICECSINK_abortCombinationsEmulation'
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










