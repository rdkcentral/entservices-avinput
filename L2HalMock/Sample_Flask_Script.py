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

import requests
import json

flask_server = "127.0.0.1:8000"

config_data = {
                "apiName": "setDeviceConfig",
                "arguments": {
                    "devices": [
                        {
                            "device": [
                                {"name": "xione_uk"},
                                {"islocal": 1},
                                {"type": "source"},
                                {"powerState": 1},
                                {"physicalAddress": "301"},
                                {"logicalAddress": "3"},
                                {"vendorId": "4567"},
                                {"osdName": "404753747265616D696E672054776F"},
                                {"optionalProperty1": "value1"},
                                {"optionalProperty2": "value2"}
                            ]
                        },
                        {
                            "device": [
                                {"name": "amazon fire stick"},
                                {"islocal": 0},
                                {"type": "source"},
                                {"powerState": 1},
                                {"physicalAddress": "304"},
                                {"logicalAddress": "4"},
                                {"vendorId": "4567"},
                                {"osdName": "53747265616D696E67204F6E65"},
                                {"optionalProperty1": "value1"},
                                {"optionalProperty2": "value2"}
                            ]
                        },
                        {
                            "device": [
                                {"name": "amazon fire stick"},
                                {"islocal": 0},
                                {"type": "source"},
                                {"powerState": 1},
                                {"physicalAddress": "304"},
                                {"logicalAddress": "9"},
                                {"vendorId": "4567"},
                                {"osdName": "53747265616D696E67204F6E65"},
                                {"optionalProperty1": "value1"},
                                {"optionalProperty2": "value2"}
                            ]
                        },
                        {
                            "device": [
              {"name": "hisense"},
                                {"islocal": 0},
                                {"type": "sink"},
                                {"powerState": 1},
                                {"physicalAddress": "0"},
                                {"logicalAddress": "0"},
                                {"vendorId": "0x4567"},
                                {"osdName": "545620426F78"},
                                {"optionalProperty1": "value1"},
                                {"optionalProperty2": "value2"}
                            ]
                        }
                    ]
                }
        }

api_data = {
              "apiName": "setAPIConfig",
              "arguments": {
                "apiOverrides": [
                  {
                    "HdmiCecOpen": [
                      { "return": 1 },
                      { "outParams": [{"handle": 2345678}] }
                    ]
                  },
                  {
                    "HdmiCecGetLogicalAddress": [
                      { "return": 0 },
                      { "outParams": [{"logicalAddress": "0x4"}] }
                    ]
                  },
                  {
                    "HdmiCecGetPhysicalAddress": [
                      { "return": 0 },
                      { "outParams": [{"physicalAddress": "0x304"}] }
                    ]
                  }
                ]
              }
            }


# fetch the cec network data from flask using http get requests
createDevice_response = requests.get("http://{}/Database.setDeviceConfig/{}".format(flask_server, json.dumps(config_data)))
print(createDevice_response.text)

# fetch the api overrides data from flask using http get requests
createApiOverrides_response = requests.get("http://{}/Hdmicec.setAPIConfig/{}".format(flask_server, json.dumps(api_data)))
print(createApiOverrides_response.text)


