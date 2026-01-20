# Testcase ID : TCID014_getAudioStatusMessage
# Testcase Description : Sends the CEC <Give Audio Status> message to request the audio status., and verify the output response

from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis
from HdmiCecSource import HdmiCecSourceApis
import Config
import requests
import json
import time

print("TC Description - To check if mngr Sends the CEC <Give Audio Status> message to request the audio status if HAL Api values are passed negative")
print("---------------------------------------------------------------------------------------------------------------------------")
# store the expected output response
Utils.initialize_flask_with_HalApiNegativeValues()
expected_output_response = '{"jsonrpc":"2.0","id":42,"error":{"code":1,"message":"ERROR_GENERAL"}}'



# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSinkApis.sendGetAudioStatusMessage)
if curl_response:
     Utils.info_log("curl command to send audio status message is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

time.sleep(3)

#ToDo - send message emulation for report audio status message event[reportAudioStatusEvent	Triggered when CEC message of device is received.]
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.reportShortAudioDes)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the reportShortAudioDes")
else:
     Utils.error_log("sendMessage emulation failed for requesting the reportShortAudioDes")
time.sleep(3)
print("")

time.sleep(3)

message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.reportAudioMode)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for requesting the reportAudioMode")
else:
     Utils.error_log("sendMessage emulation failed for requesting the reportAudioMode")
time.sleep(3)
print("")

print("---------------------------------------------------------------------------------------------------------------------------")
# compare both expected and received output responses
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one. The send audio status message and report audio sytatus event ' \
              'is obtained in output response'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID045_HdmiCecSink_getAudioStatusMessageNegative'
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
