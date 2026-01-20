# Testcase ID : TCID021_setMenuLanguage
# Testcase Description : Updates the internal data structure with the new menu Language and also broadcasts the <Set Menu Language> CEC message.and verify the outpu response
from Utilities import Utils, ReportGenerator
from HdmiCecSink import HdmiCecSinkApis
import json
import requests
import time
import Config

Utils.initialize_flask_with_HalApiNegativeValues()
#send messages required for getting physical address
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.give_physical_address_hisense)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for querying the physical address")
else:
     Utils.error_log("sendMessage emulation failed for querying the physical address")
time.sleep(3)
print("")

#send messages required for getting menu language
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.get_menu_language_hisense)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for querying the menu language")
else:
     Utils.error_log("sendMessage emulation failed for querying the menu language")
time.sleep(3)
print("")

#send messages required for setting menu language
message1_response = requests.get("http://{}/Hdmicec.sendMessage/{}".format(
    Config.flask_server_ip, json.dumps(Config.set_menu_language)))
if "200" in str(message1_response):
     Utils.info_log("sendMessage emulation success for setting the menu language")
else:
     Utils.error_log("sendMessage emulation failed for setting the menu language")
time.sleep(3)
print("")

curl_response = Utils.send_curl_command(HdmiCecSinkApis.set_menu_language)
if curl_response:
    Utils.info_log("curl command send for set menu language")
    status = 'Pass'
    message = 'Output response is matching with expected one. The set menu language ' \
              'is obtained in output response'
else:
    Utils.error_log("curl command send failed {}" .format(HdmiCecSinkApis.set_menu_language))
    status = 'False'
    message = 'Output response is not matching with expected one'

# generate logs in terminal
tc_id = 'TCID021_HdmiCecSink_setMenuLanguage_withNegativeHapApis'
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
