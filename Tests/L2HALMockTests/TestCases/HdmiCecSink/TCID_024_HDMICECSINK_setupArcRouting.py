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

# Testcase ID : TCID024_setupArcRoutingChange
# Testcase Description :Enable (or disable) HDMI-CEC Audio Return Channel (ARC) routing. Upon enabling, triggers arcInitiationEvent and upon disabling, triggers arcTerminationEvent. Verify the output response
from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis
import json
import requests
import time
import Config

Utils.initialize_flask()
print("TC - Description : Enable (or disable) HDMI-CEC Audio Return Channel (ARC) routing. Upon enabling, triggers arcInitiationEvent and upon disabling, triggers arcTerminationEvent. Verify the output response")
#ToDo - send messages required for arpc routing with different params


#ToDo - SendMessage emulation for arcInitiationEvent[[enabled, the CEC <Request ARC Initiation> and <Report ARC Initiated> messages are sent]]	Triggered when routing though the HDMI ARC port is successfully established.
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.initiateArc)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for initiating arc event")
else:
     Utils.error_log("sendMessage emulation failed for initiating arc event")
time.sleep(3)
print("")

curl_response = Utils.send_curl_command(HdmiCecSinkApis.set_arc_routing_params_disabled)
if curl_response:
    Utils.info_log("curl command send for arc routing")
    status = 'Pass'
    message = 'Output response is matching with expected one. The arc routing ' \
              'is obtained in output response'
else:
    Utils.error_log("curl command send failed {}" .format(HdmiCecSinkApis.set_arc_routing_params_disabled))
    status = 'False'
    message = 'Output response is not matching with expected one'

curl_response = Utils.send_curl_command(HdmiCecSinkApis.set_arc_routing_params_enabled)
if curl_response:
    Utils.info_log("curl command send for arc routing")
    status = 'Pass'
    message = 'Output response is matching with expected one. The arc routing ' \
              'is obtained in output response'
else:
    Utils.error_log("curl command send failed {}" .format(HdmiCecSinkApis.set_arc_routing_params_enabled))
    status = 'False'
    message = 'Output response is not matching with expected one'


#ToDo - SendMessage emulation for arcTerminationEvent[[If disabled, the CEC <Request ARC Termination> and <Report ARC Terminated> messages are sent.]].Triggered when routing though the HDMI ARC port terminates.
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.terminateArc)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for terminating Arc Event")
else:
     Utils.error_log("sendMessage emulation failed for terminating Arc Event")
time.sleep(3)
print("")


# generate logs in terminal
tc_id = 'TCID024_HdmiCecSink_setArcRouting'
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
