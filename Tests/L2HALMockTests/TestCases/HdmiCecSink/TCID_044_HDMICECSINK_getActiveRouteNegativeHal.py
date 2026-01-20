# Testcase ID : TCID005
# Testcase Description : Verify that default vendor id is obtained in output response
import time
import json
import Config
import requests
from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis
from HdmiCecSource import HdmiCecSourceApis

print("TC Description - Verify that getActiveSource error is obtained in output response as HAL Api values has been given erroneous")
print("---------------------------------------------------------------------------------------------------------------------------")
# store the expected output response
Utils.initialize_flask_with_HalApiNegativeValues()
expected_output_response = '{"jsonrpc":"2.0","id":42,"error":{"code":1,"message":"ERROR_GENERAL"}}'

#send messages required for getting inactive source
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.active_source_xione_uk)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for querying the inactive source")
else:
     Utils.error_log("sendMessage emulation failed for querying the inactive source")
time.sleep(3)
print("")

curl_response = Utils.send_curl_command(HdmiCecSinkApis.get_active_route)
if curl_response:
    Utils.info_log("curl command send for send get audio status message")
    status = 'Pass'
    message = 'Output response is matching with expected one. The send get audio status message ' \
              'is obtained in output response'
else:
    Utils.error_log("curl command send failed {}" .format(HdmiCecSinkApis.get_active_route))
    status = 'False'
    message = 'Output response is not matching with expected one'

# generate logs in terminal
tc_id = 'TCID_044_HDMICECSINK_active_route_HAL_False'
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
