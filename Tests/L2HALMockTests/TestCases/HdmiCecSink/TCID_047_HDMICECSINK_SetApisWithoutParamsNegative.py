# Testcase ID : TCID047_setApisWithoutParams
# Testcase Description : send all set apis with default HAL values and without params in json rpc request
import time
from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis

print("TC Description - send all set apis with default HAL values and without params in json rpc request")
print("---------------------------------------------------------------------------------------------------------------------------")
# store the expected output response
Utils.initialize_flask()

def wait():
  time.sleep(2)

expected_output_response = '{"jsonrpc":"2.0","id":42,"error":{"code":1,"message":"ERROR_GENERAL"}}'
# send the curl command and fetch the output json response
curl_response = Utils.send_curl_command(HdmiCecSinkApis.sendKeyPress_withoutParams)
if curl_response:
     Utils.info_log("curl command to send key press without params is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

wait()

curl_response = Utils.send_curl_command(HdmiCecSinkApis.senduserContolledPress_withoutParams )
if curl_response:
     Utils.info_log("curl command to send usercontrol key press without params is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

curl_response = Utils.send_curl_command(HdmiCecSinkApis.sendusercontrolledReleased_withoutParams )
if curl_response:
     Utils.info_log("curl command to send user key release without params is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

wait()

curl_response = Utils.send_curl_command(HdmiCecSinkApis.setActivepath_withoutParams )
if curl_response:
     Utils.info_log("curl command to set active path without params is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

wait()

curl_response = Utils.send_curl_command(HdmiCecSinkApis.setEnabled_withoutParams )
if curl_response:
     Utils.info_log("curl command to set enable the driver without params is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

wait()

curl_response = Utils.send_curl_command(HdmiCecSinkApis.setMenuLanguage_withoutParams  )
if curl_response:
     Utils.info_log("curl command to set menu language without params is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

wait()

curl_response = Utils.send_curl_command(HdmiCecSinkApis.setMenuLanguage_withoutParams  )
if curl_response:
     Utils.info_log("curl command to set menu language without params is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

wait()

curl_response = Utils.send_curl_command(HdmiCecSinkApis.setOSDName_withoutParams   )
if curl_response:
     Utils.info_log("curl command to set OSD name without params is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

wait()

curl_response = Utils.send_curl_command(HdmiCecSinkApis.setRoutingChange_withoutParams   )
if curl_response:
     Utils.info_log("curl command to set routing change without params is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

wait()

curl_response = Utils.send_curl_command(HdmiCecSinkApis.setupARCRouting_withoutParams   )
if curl_response:
     Utils.info_log("curl command to setup ARC routing without params is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

wait()

curl_response = Utils.send_curl_command(HdmiCecSinkApis.setvendorID_withoutParams   )
if curl_response:
     Utils.info_log("curl command to set vendor id without params is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

wait()

curl_response = Utils.send_curl_command(HdmiCecSinkApis.setMenuLanguage_withoutParams  )
if curl_response:
     Utils.info_log("curl command to set menu language without params is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

wait()

curl_response = Utils.send_curl_command(HdmiCecSinkApis.setLatencyInfo_withoutParams   )
if curl_response:
     Utils.info_log("curl command to set latency info without params is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")


print("---------------------------------------------------------------------------------------------------------------------------")
# compare both expected and received output responses
if str(curl_response) == str(expected_output_response):
    status = 'Pass'
    message = 'Output response is matching with expected one. error value ' \
              'is obtained in output response'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID047_HdmiCecSink_setApis_withoutParams_Negative'
print("Testcase ID : " + tc_id)
print("Testcase Output Response : " + curl_response)
print("Testcase Status : " + status)
print("Testcase Message : " + message)
print("")

if status == 'Pass':
    ReportGenerator.passed_tc_list.append(tc_id)
else:
    ReportGenerator.failed_tc_list.append(tc_id)
  
#Reset the state of the mock system to original
Utils.initialize_flask()
# push the testcase execution details to report file
ReportGenerator.append_test_results_to_csv(tc_id, curl_response, status, message)
