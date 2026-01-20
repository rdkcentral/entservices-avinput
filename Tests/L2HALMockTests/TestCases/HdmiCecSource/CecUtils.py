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

# This file contains all the common functions required for test framework

import requests
import json
import time
import Config
from Utilities import Utils
from HdmiCecSource import HdmiCecSourceApis


def cec_update_api_overrides(data):
    '''This function is used to update the api overrides data. After updating the data we will
    deactivate and then reactivate the HdmiCecSource plugin to reflect pushed changes'''
    try:
        # Change the values of api overrides for hdmi-cec plugin using updateAPIConfig API
        api_overrides_response = requests.get("http://{}/Hdmicec.updateAPIConfig/{}".format(
            Config.flask_server_ip, json.dumps(data)))
        print("Inside CecUtils.py : " + api_overrides_response.text + " : " + str(data))
        time.sleep(3)

        # Deactivate the plugin using curl command
        Utils.send_curl_command(HdmiCecSourceApis.deactivate_command)

        # Activate the plugin using curl command
        Utils.send_curl_command(HdmiCecSourceApis.activate_command)
    except:
        print("Inside CecUtils.py : Exception in cec_update_api_overrides function")


def cec_sink_tx_fail(data):
    '''This function is used to update the api overrides data. After updating the data we will
    deactivate and then reactivate the HdmiCecSource plugin to reflect pushed changes'''
    try:
        # Change the values of api overrides for hdmi-cec plugin using updateAPIConfig API
        api_overrides_response = requests.get("http://{}/Hdmicec.updateAPIConfig/{}".format(
            Config.flask_server_ip, json.dumps(data)))
        print("Inside CecUtils.py : " + api_overrides_response.text + " : " + str(data))
        time.sleep(3)

        # Deactivate the plugin using curl command
        Utils.send_curl_command(HdmiCecSourceApis.deactivate_command)

        # Activate the plugin using curl command
        Utils.send_curl_command(HdmiCecSourceApis.activate_command)
        time.sleep(5)
    except:
        print("Inside CecUtils.py : Exception in cec_update_api_overrides function")



def cec_post_condition_for_negative_scenarios():
    '''This function is used to update the api overrides data back to default. After updating the data
     we will restart WPEFramework & Websocket services and then activate the HdmiCecSource plugin
     again to reflect pushed changes'''
    try:
        # Change the values of api overrides data for hdmi-cec plugin back to default 0
        api_overrides_response = requests.get("http://{}/Hdmicec.updateAPIConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.api_data)))
        print("Inside CecUtils.py : Post-condition : " + api_overrides_response.text + " : " + str(Config.api_data))
        # time.sleep(3)

        # Restart the WPEFramework & Websocket services
        Utils.restart_services()
        # time.sleep(3)

        # Store the expected output response for activate command
        expected_output_response = '{"jsonrpc":"2.0","id":3,"result":null}'

        # Send the controller activate curl command and fetch the output json response
        curl_response = Utils.send_curl_command(HdmiCecSourceApis.activate_command)

        # Compare both expected and received output responses
        if str(curl_response) == str(expected_output_response):
            print("Inside CecUtils.py : Post-condition : Successfully restarted the WPEFramework & "
                  "Websocket server services. Successfully activated the HdmiCecSource plugin")
        else:
            print("Inside CecUtils.py : Post-condition : Failed to restart the WPEFramework & "
                  "Websocket server services. Failed to activate the HdmiCecSource plugin")
    except:
        print("Inside CecUtils.py : Post-condition : Exception in cec_post_condition_for_negative_scenarios function")


def activate_cec():
    '''This function is used to activate the HdmiCecSource plugin'''
    try:
        # send the controller activate curl command and fetch the output json response
        curl_response = Utils.send_curl_command(HdmiCecSourceApis.activate_command)

        # compare both expected and received output responses
        if str(curl_response) == str(HdmiCecSourceApis.expected_output_response):
            print("Inside CecUtils.py : Successfully activated the HdmiCecSource plugin")
        else:
            print("Inside CecUtils.py : Failed to activate the HdmiCecSource plugin")
    except:
        print("Inside CecUtils.py : Exception in activate_cec function")
