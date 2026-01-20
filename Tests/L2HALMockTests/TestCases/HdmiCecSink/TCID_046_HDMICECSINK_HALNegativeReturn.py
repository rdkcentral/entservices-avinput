# Testcase ID : TCID046_HalNegative
# Testcase Description : Returns HDMI-CEC driver enabled status

from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis

print("TC Description - Returns whether HDMI-CEC is enabled on platform or not.")
print("---------------------------------------------------------------------------------------------------------------------------")
# store the expected output response
Utils.initialize_hal_apis_with_negative_values()
expected_output_response = '{"jsonrpc":"2.0","id":42,"error":{"code":1,"message":"ERROR_GENERAL"}}'
# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSinkApis.get_enabled)
if curl_response:
     Utils.info_log("curl command to get enabled is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

print("---------------------------------------------------------------------------------------------------------------------------")
# compare both expected and received output responses
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one. The default driver status ' \
              'is obtained in output response'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID046_HdmiCecSink_HAL_ReturnsNegative_and_string'
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
