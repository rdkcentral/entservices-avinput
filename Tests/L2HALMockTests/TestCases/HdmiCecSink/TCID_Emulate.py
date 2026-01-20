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

# Testcase ID : TCID005
# Testcase Description : Verify that default vendor id is obtained in output response
import time
import json
import Config
import requests
from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis
from HdmiCecSource import HdmiCecSourceApis

print("TC Description - Verify that default vendor id is obtained in output response")
print("---------------------------------------------------------------------------------------------------------------------------")
# store the expected output response
Utils.initialize_flask()
expected_output_response = '{"jsonrpc":"2.0","id":42,"result":{"success":true}}'

message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.reportPhysicalAdd)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for reporting the physical address")
else:
     Utils.error_log("sendMessage emulation failed for reporting the physical address")
time.sleep(3)
print("")

message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.DeviceVendorID)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for reporting the DeviceVendorID")
else:
     Utils.error_log("sendMessage emulation failed for reporting the DeviceVendorID")
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
    Config.flask_server_ip, json.dumps(Config.ignore_set_menu_language)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the ignore_set_menu_language")
else:
     Utils.error_log("sendMessage emulation failed for requesting the ignore_set_menu_language")
time.sleep(3)
print("")

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.set_osd_string)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the set_osd_string")
else:
     Utils.error_log("sendMessage emulation failed for requesting the set_osd_string")
time.sleep(3)
print("")

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.get_menu_language_hisense)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the get_menu_language_hisense")
else:
     Utils.error_log("sendMessage emulation failed for requesting the get_menu_language_hisense")
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

#send messages required for requesting active source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.initiateArc)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the initiateArc")
else:
     Utils.error_log("sendMessage emulation failed for requesting the initiateArc")
time.sleep(3)
print("")

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
