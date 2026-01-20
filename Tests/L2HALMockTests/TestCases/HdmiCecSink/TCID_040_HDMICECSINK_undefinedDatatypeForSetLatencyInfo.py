# Testcase ID : TCID022_setLatencyInfo
# Testcase Description : Sets the Current Latency Values such as Video Latency, Latency Flags,Audio Output Compensated value and Audio Output Delay by sending <Report Current Latency> message for Dynamic Auto LipSync Feature. Verify the output response
from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis
import json
import requests
import time
import Config

Utils.initialize_flask()
#ToDo - send messages required for reporting current latency info
#send messages required for reporting current latency
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.requestcurrentlatency)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the requestcurrentlatency")
else:
     Utils.error_log("sendMessage emulation failed for requesting the requestcurrentlatency")
time.sleep(3)
print("")


#send messages required for getting menu language
#message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
#    Config.flask_server_ip, json.dumps(Config.get_menu_language_hisense)))
#if "200" in str(message1_response):
#     Utils.info_log("sendMessage emulation success for querying the menu language")
#else:
#     Utils.error_log("sendMessage emulation failed for querying the menu language")
#time.sleep(3)
#print("")

#send messages required for setting menu language
#message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
#    Config.flask_server_ip, json.dumps(Config.set_menu_language)))
#if "200" in str(message1_response):
#     Utils.info_log("sendMessage emulation success for setting the menu language")
#else:
#     Utils.error_log("sendMessage emulation failed for setting the menu language")
#time.sleep(3)
#print("")

curl_response = Utils.send_curl_command(HdmiCecSinkApis.set_latency_info)
if curl_response:
    Utils.info_log("curl command send for set latency info")
    status = 'Pass'
    message = 'Output response is matching with expected one. The set latency info ' \
              'is obtained in output response'
else:
    Utils.error_log("curl command send failed {}" .format(HdmiCecSinkApis.set_latency_info))
    status = 'False'
    message = 'Output response is not matching with expected one'

# generate logs in terminal
tc_id = 'TCID022_HdmiCecSink_setLatencyInfo'
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
ReportGenerator.append_test_results_to_csv(tc_id, curl_response, status, message)# Testcase ID : TCID022_setLatencyInfo
# Testcase Description : Sets the Current Latency Values such as Video Latency, Latency Flags,Audio Output Compensated value and Audio Output Delay by sending <Report Current Latency> message for Dynamic Auto LipSync Feature. Verify the output response
from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis
import json
import requests
import time
import Config

Utils.initialize_flask()
#ToDo - send messages required for reporting current latency info
#send messages required for reporting current latency
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.requestcurrentlatency)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the requestcurrentlatency")
else:
     Utils.error_log("sendMessage emulation failed for requesting the requestcurrentlatency")
time.sleep(3)
print("")


#send messages required for getting menu language
#message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
#    Config.flask_server_ip, json.dumps(Config.get_menu_language_hisense)))
#if "200" in str(message1_response):
#     Utils.info_log("sendMessage emulation success for querying the menu language")
#else:
#     Utils.error_log("sendMessage emulation failed for querying the menu language")
#time.sleep(3)
#print("")

#send messages required for setting menu language
#message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
#    Config.flask_server_ip, json.dumps(Config.set_menu_language)))
#if "200" in str(message1_response):
#     Utils.info_log("sendMessage emulation success for setting the menu language")
#else:
#     Utils.error_log("sendMessage emulation failed for setting the menu language")
#time.sleep(3)
#print("")

curl_response = Utils.send_curl_command(HdmiCecSinkApis.set_latency_info_with_undefinedLatencyMode)
if curl_response:
    Utils.info_log("curl command send for set latency info")
    status = 'Pass'
    message = 'Output response is matching with expected one. The set latency info ' \
              'is obtained in output response'
else:
    Utils.error_log("curl command send failed {}" .format(HdmiCecSinkApis.set_latency_info_with_undefinedLatencyMode))
    status = 'False'
    message = 'Output response is not matching with expected one'

# generate logs in terminal
tc_id = 'TCID040_HdmiCecSink_setLatencyInfo_with_erroneousLatencyModes'
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
