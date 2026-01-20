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

# Testcase ID : TCID035
# Testcase Description :Emulating osd string, and arc events
import time
import json
import Config
import requests
from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis
from HdmiCecSource import HdmiCecSourceApis

print("TC Description - Emulating osd string, and arc events")
print("---------------------------------------------------------------------------------------------------------------------------")
# store the expected output response
Utils.initialize_flask_without_audio_device()
time.sleep(3)
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"success":true}}'


message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.reportPhysicalAdd)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for reporting the physical address")
else:
     Utils.error_log("sendMessage emulation failed for reporting the physical address")
time.sleep(3)
print("")
#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.imageViewON)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the imageViewON")
else:
     Utils.error_log("sendMessage emulation failed for requesting the imageViewON")
time.sleep(3)
print("")

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.textViewON)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the textViewON")
else:
     Utils.error_log("sendMessage emulation failed for requesting the textViewON")
time.sleep(3)
print("")

Utils.initialize_flask()
time.sleep(3)
#Emulation curl command
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

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.routing_change)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the routing_change")
else:
     Utils.error_log("sendMessage emulation failed for requesting the routing_change")
time.sleep(3)
print("")

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.feature_abort)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the feature_abort")
else:
     Utils.error_log("sendMessage emulation failed for requesting the feature_abort")
time.sleep(3)
print("")

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.abort_hisense)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the abort_hisense")
else:
     Utils.error_log("sendMessage emulation failed for requesting the abort_hisense")
time.sleep(3)
print("")

curl_response = Utils.send_curl_command(HdmiCecSinkApis.set_routing_change_nav)
if curl_response:
    Utils.info_log("curl command send for set routing change")
    status = 'Pass'
    message = 'Output response is matching with expected one. The set latency info ' \
              'is obtained in output response'
else:
    Utils.error_log("curl command send failed {}" .format(HdmiCecSinkApis.set_routing_change_nav))
    status = 'False'
    message = 'Output response is not matching with expected one'
time.sleep(3)

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.initiateArc)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the initiateArc")
else:
     Utils.error_log("sendMessage emulation failed for requesting the initiateArc")
time.sleep(3)
print("")

curl_response = Utils.send_curl_command(HdmiCecSinkApis.set_arc_routing_params_disabled)
if curl_response:
    Utils.info_log("curl command send for set routing change")
    status = 'Pass'
    message = 'Output response is matching with expected one. The set latency info ' \
              'is obtained in output response'
else:
    Utils.error_log("curl command send failed {}" .format(HdmiCecSinkApis.set_arc_routing_params_disabled))
    status = 'False'
    message = 'Output response is not matching with expected one'
    
#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.terminateArc)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the terminateArc")
else:
     Utils.error_log("sendMessage emulation failed for requesting the terminateArc")
time.sleep(3)
print("")

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.reportShortAudioDes)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the reportShortAudioDes")
else:
     Utils.error_log("sendMessage emulation failed for requesting the reportShortAudioDes")
time.sleep(3)
print("")

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.setSystemAudioMode)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the setSystemAudioMode")
else:
     Utils.error_log("sendMessage emulation failed for requesting the setSystemAudioMode")
time.sleep(3)
print("")

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.reportAudioMode)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the reportAudioMode")
else:
     Utils.error_log("sendMessage emulation failed for requesting the reportAudioMode")
time.sleep(3)
print("")

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.givefeatures)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the givefeatures")
else:
     Utils.error_log("sendMessage emulation failed for requesting the givefeatures")
time.sleep(3)
print("")

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.requestcurrentlatency)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the requestcurrentlatency")
else:
     Utils.error_log("sendMessage emulation failed for requesting the requestcurrentlatency")
time.sleep(3)
print("")
