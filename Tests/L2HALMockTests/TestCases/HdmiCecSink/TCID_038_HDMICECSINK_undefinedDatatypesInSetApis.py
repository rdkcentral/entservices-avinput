# Testcase ID : TCID005
# Testcase Description : Verify that default vendor id is obtained in output response

from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis

print("TC Description - Verify that default vendor id AND default OSD name is obtained in output response")
print("---------------------------------------------------------------------------------------------------------------------------")
# store the expected output response
Utils.initialize_flask()
count = 0
expected_output_response_vendor_id = '{"jsonrpc":"2.0","id":42,"result":{"vendorid":"019fb","success":true}}'
expected_output_response_osd_name =  '{"jsonrpc":"2.0","id":42,"result":{"name":"TV Box","success":true}}'
# send the curl command and fetch the output json response
curl_response_vendor_id = Utils.send_curl_command(HdmiCecSinkApis.set_vendor_id_with_undefined_datatype)
curl_response_vendor_id = Utils.send_curl_command(HdmiCecSinkApis.get_vendor_id)
if curl_response_vendor_id:
     Utils.info_log("curl command to get vendorID is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

curl_response_osd_name = Utils.send_curl_command(HdmiCecSinkApis.set_osd_name_with_undefined_datatype)
curl_response_osd_name = Utils.send_curl_command(HdmiCecSinkApis.get_osd_name)
if curl_response_osd_name:
  Utils.info_log("curl command to set OSD name  is sent from the test runner")
else:
     Utils.error_log("curl command invoke failed")

print("---------------------------------------------------------------------------------------------------------------------------")
# compare both expected and received output responses
if str(curl_response_vendor_id) == str(expected_output_response_vendor_id) and (curl_response_osd_name) == str(expected_output_response_osd_name):
    count = count+1
    status = 'Pass'
    message = 'Output response is matching with expected one. The default vendor id ' \
              'is obtained in output response'
else:
    status = 'Fail'
    message = 'Output response is different from expected one'

# generate logs in terminal
tc_id = 'TCID005_HdmiCecSink_getVendorId'
print("Testcase ID : " + tc_id)
print("Testcase Output Response : " + curl_response_osd_name)
print("Testcase Status : " + status)
print("Testcase Message : " + message)
print("")

if status == 'Pass':
    ReportGenerator.passed_tc_list.append(tc_id)
else:
    ReportGenerator.failed_tc_list.append(tc_id)
Utils.initialize_flask()
# push the testcase execution details to report file
ReportGenerator.append_test_results_to_csv(tc_id, curl_response_osd_name, status, message)
