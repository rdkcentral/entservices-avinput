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

import subprocess
import os
import json
import requests
import Config
from os import path
import logging
from colorama import Fore, Style
import time


def error_log(message):
    print(Fore.RED + f"ERROR: {message}" + Style.RESET_ALL)

def warning_log(message):
    print(Fore.YELLOW + f"LOGGER: {message}" + Style.RESET_ALL)

def info_log(message):
    print(Fore.CYAN + f"INFO: {message}" + Style.RESET_ALL)

def initiliaze_flask_for_HdmiCecSource():
    '''This function is used to push the initial data towards the Flask server'''
    try:
        # Push the initial cec network data to Flask using http get requests - setDeviceConfig API
        create_device_response = requests.get("http://{}/Database.setDeviceConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.config_data_hdmicecsource)))
        print("Inside Utils.py : " + create_device_response.text + " : " + str(Config.config_data_hdmicecsource))
        print("configured device data using - http://{}/Database.setDeviceConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.config_data_hdmicecsource)))
        print("")

        # Push the api overrides data to Flask using http get requests - setAPIConfig API
        create_api_overrides_response = requests.get("http://{}/Hdmicec.setAPIConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.api_data_hdmicecsource)))
        print("Inside Utils.py : " + create_api_overrides_response.text + " : " + str(Config.api_data_hdmicecsource))
        print("configured hal api data using - http://{}/Hdmicec.setAPIConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.api_data_hdmicecsource)))
        print("")

        # Compare the obtained response with actual response
        if "Success" in str(create_device_response.text) and "Success" in str(create_api_overrides_response.text):
            print("Inside Utils.py : Successfully pushed the initial data (setDeviceConfig & setAPIConfig) to Flask")
        else:
            print("Inside Utils.py : Failed to push the initial data (setDeviceConfig & setAPIConfig) to Flask")

    except:
        print("Inside Utils.py : Exception in initialize_flask function")

def initialize_flask():
    '''This function is used to push the initial data towards the Flask server'''
    try:
        # Push the initial cec network data to Flask using http get requests - setDeviceConfig API
        create_device_response = requests.get("http://{}/Database.setDeviceConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.config_data)))
        print("Inside Utils.py : " + create_device_response.text + " : " + str(Config.config_data))
        print("configured device data using - http://{}/Database.setDeviceConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.config_data)))
        print("")

        # Push the api overrides data to Flask using http get requests - setAPIConfig API
        create_api_overrides_response = requests.get("http://{}/Hdmicec.setAPIConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.api_data)))
        print("Inside Utils.py : " + create_api_overrides_response.text + " : " + str(Config.api_data))
        print("configured hal api data using - http://{}/Hdmicec.setAPIConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.api_data)))
        print("")

        # Compare the obtained response with actual response
        if "Success" in str(create_device_response.text) and "Success" in str(create_api_overrides_response.text):
            print("Inside Utils.py : Successfully pushed the initial data (setDeviceConfig & setAPIConfig) to Flask")
        else:
            print("Inside Utils.py : Failed to push the initial data (setDeviceConfig & setAPIConfig) to Flask")

    except:
        print("Inside Utils.py : Exception in initialize_flask function")

def initialize_hal_apis_with_negative_values():
    '''This function is used to push the initial data towards the Flask server'''
    try:
        # Push the initial cec network data to Flask using http get requests - setDeviceConfig API
        create_device_response = requests.get("http://{}/Database.setDeviceConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.config_data)))
        print("Inside Utils.py : " + create_device_response.text + " : " + str(Config.config_data))
        print("configured device data using - http://{}/Database.setDeviceConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.config_data)))
        print("")

        # Push the api overrides data to Flask using http get requests - setAPIConfig API
        create_api_overrides_response = requests.get("http://{}/Hdmicec.setAPIConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.api_data_crash_instance)))
        print("Inside Utils.py : " + create_api_overrides_response.text + " : " + str(Config.api_data_crash_instance))
        print("configured hal api data using - http://{}/Hdmicec.setAPIConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.api_data_crash_instance)))
        print("")

        # Compare the obtained response with actual response
        if "Success" in str(create_device_response.text) and "Success" in str(create_api_overrides_response.text):
            print("Inside Utils.py : Successfully pushed the initial data (setDeviceConfig & setAPIConfig) to Flask")
        else:
            print("Inside Utils.py : Failed to push the initial data (setDeviceConfig & setAPIConfig) to Flask")

    except:
        print("Inside Utils.py : Exception in initialize_flask function")

def send_curl_command(curl_command):
    '''This function is used to send the curl commands to get the output response using os module'''
    output_response = ""
    try:
        # Send the curl command using os.system module
        response = os.popen(curl_command)

        # Find the line that is a valid JSON for extracting only the json response
        for line in response.readlines():
            try:
                # Try to parse the current line as JSON
                json.loads(line)
                output_response = line
                # Exit the loop as we found the JSON line
                break
            except json.JSONDecodeError:
                # If current line is not a valid JSON, just pass and continue with the next line
                pass

        # Check the output response and add a message if the obtained output response is null
        if len(output_response) < 5:
            output_response = "< No response from WPEFramework >"
    except:
        print("Inside Utils.py : Exception in send_curl_command function")
    finally:
        # Return the output json response of given curl command as a string
        return output_response

#abort Device config with 5 different device configurations.
def abort_data(data):
    print("Sending abort configurations")
    create_device_response = requests.get("http://{}/Database.setDeviceConfig/{}".format(Config.flask_server_ip, json.dumps(data)))
    print("Inside Utils.py : " + create_device_response.text + " : " + str(data))
    print("configured device data using - http://{}/Database.setDeviceConfig/{}".format(
        Config.flask_server_ip, json.dumps(data)))
    print("")

def initialize_flask_without_audio_device():
    '''This function is used to push the initial data towards the Flask server'''
    try:
        # Push the initial cec network data to Flask using http get requests - setDeviceConfig API
        create_device_response = requests.get("http://{}/Database.setDeviceConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.config_data_no_audio)))
        print("Inside Utils.py : " + create_device_response.text + " : " + str(Config.config_data))
        print("configured device data using - http://{}/Database.setDeviceConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.config_data_no_audio)))
        print("")

        # Push the api overrides data to Flask using http get requests - setAPIConfig API
        create_api_overrides_response = requests.get("http://{}/Hdmicec.setAPIConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.api_data)))
        print("Inside Utils.py : " + create_api_overrides_response.text + " : " + str(Config.api_data))
        print("configured hal api data using - http://{}/Hdmicec.setAPIConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.api_data)))
        print("")

        # Compare the obtained response with actual response
        if "Success" in str(create_device_response.text) and "Success" in str(create_api_overrides_response.text):
            print(
                "Inside Utils.py : Successfully pushed the initial data (setDeviceConfig & setAPIConfig) to Flask")
        else:
            print("Inside Utils.py : Failed to push the initial data (setDeviceConfig & setAPIConfig) to Flask")

    except:
        print("Inside Utils.py : Exception in initialize_flask function")

def restart_services():
    '''This function is used to kill the WPEFramework & Websocket services and to restart those'''
    try:
        # Kill the port WPEFramework with -QUIT to generate .gcda filek
        os.system("killall -QUIT WPEFramework")
        
        # Kill the port 55555 which runs the WPEFramework
        os.system("fuser -k 55555/tcp")

        # Kill the port 9000 which runs the Websocket server
        os.system("fuser -k 9000/tcp")

        # Start the websocket server using python subprocess module
        subprocess.Popen(["python3", "websocket_server.py"], cwd=Config.directory_websocket)
        time.sleep(5)
        subprocess.run(["chmod", "+x", "restart.sh"], check=True)
        subprocess.Popen(["./restart.sh"], cwd=Config.WPEFramework_restart)
        time.sleep(5)
        # file_path = path.relpath(Config.WPEFramework_logs_path)

        # Open a file for writing the output and error of WPEFramework process
        # with open(file_path, "w") as logfile:

            # Run the command to start the WPEFramework and redirect the output and error to the file
            # subprocess.Popen(["WPEFramework", "-f", "-c", "config.json"],
            #                  cwd=Config.directory_thunder, stdout=logfile, stderr=logfile)
            # subprocess.Popen(["./restart.sh"], cwd=Config.WPEFramework_restart, stdout=logfile, stderr=logfile)
    except:
        print("Inside Utils.py : Exception in restart_services function")

def netstat_output():

    #Run the netstat -ntlp command

    output = subprocess.run(['netstat','-ntlp'],capture_output=True,text=True)

    #Return the output as a string
    
    return output.stdout

def initialize_flask_with_HalApiNegativeValues():
    '''This function is used to push the initial data towards the Flask server'''
    try:
        # Push the initial cec network data to Flask using http get requests - setDeviceConfig API
        create_device_response = requests.get("http://{}/Database.setDeviceConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.config_data)))
        print("Inside Utils.py : " + create_device_response.text + " : " + str(Config.config_data))
        print("configured device data using - http://{}/Database.setDeviceConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.config_data)))
        print("")

        # Push the api overrides data to Flask using http get requests - setAPIConfig API
        create_api_overrides_response = requests.get("http://{}/Hdmicec.setAPIConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.api_data_negative)))
        print("Inside Utils.py : " + create_api_overrides_response.text + " : " + str(Config.api_data_negative))
        print("configured hal api data using - http://{}/Hdmicec.setAPIConfig/{}".format(
            Config.flask_server_ip, json.dumps(Config.api_data_negative)))
        print("")

        # Compare the obtained response with actual response
        if "Success" in str(create_device_response.text) and "Success" in str(create_api_overrides_response.text):
            print("Inside Utils.py : Successfully pushed the initial data (setDeviceConfig & setAPIConfig) to Flask")
        else:
            print("Inside Utils.py : Failed to push the initial data (setDeviceConfig & setAPIConfig) to Flask")

    except:
        print("Inside Utils.py : Exception in initialize_flask function")
