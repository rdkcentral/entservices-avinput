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

# This config file contains all the configurable parameters required for Test Framework
import os
# IP & port details of Flask server
flask_server_ip = "127.0.0.1:8000"

# Define the paths of Websocket server & WPEFramework
# Define the paths of Websocket server & WPEFramework
WPEFramework_restart = os.getcwd()
os.chdir("../Flask")

directory_websocket = os.getcwd()
os.chdir("../../../../install/etc/WPEFramework")

directory_thunder = os.getcwd()
os.chdir(WPEFramework_restart)

# Define the path where WPEFramework logs needs to be stored
file_name = 'log_file.txt'
WPEFramework_logs_path = os.path.abspath(file_name)

# Data required for setDeviceConfig api
config_data = {
                "apiName": "setDeviceConfig",
                "arguments": {
                    "devices": [
                        {
                            "device": [
                                {"name": "xione_uk"},
                                {"islocal": 1},
                                {"type": "source"},
                                {"powerState": 0},
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
                                {"powerState": 0},
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
                                {"powerState": 0},
                                {"physicalAddress": "304"},
                                {"logicalAddress": "9"},
                                {"vendorId": "4567"},
                                {"osdName": "53747265616D696E67204F6E65"},
                                {"optionalProperty1": "value1"},
                                {"optionalProperty2": "value2"}
                            ]
                        }
                    ]
                }
        }

# Data required for setAPIConfig api
api_data = {
              "apiName": "setAPIConfig",
              "arguments": {
                "apiOverrides": [
                  {
                    "HdmiCecOpen": [
                      { "return": 0 },
                      { "outParams": [{"handle": 2345678}] }
                    ]
                  },
                  {
                    "HdmiCecGetLogicalAddress": [
                      { "return": 0 },
                      { "outParams": [{"logicalAddress": "0x3"}] }
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

# Device data for HiSense TV
hisense_device_data = {
                    "device": [
                        {"name": "hisense"},
                        {"islocal": 0},
                        {"type": "sink"},
                        {"powerState": 0},
                        {"physicalAddress": "0"},
                        {"logicalAddress": "6"},
                        {"vendorId": "0x4567"},
                        {"osdName": "545620426F78"},
                        {"optionalProperty1": "value1"},
                        {"optionalProperty2": "value2"}
                    ]
                }

# Invalid device data for Xione US
invalid_device_data = {
              "device": [
                {"name": "xione_us"},
                {"islocal": 0},
                {"type": "sink"},
                {"powerState": 0},
                {"physicalAddress": "333"},
                {"logicalAddress": "77"},
                {"vendorId": "4444"},
                {"osdName": "3333"},
                {"optionalProperty1": "value1"},
                {"optionalProperty2": "value2"}
              ]
            }

# Send message data for sendStandbyMessage
sendStandbyMessage = {
                      "apiName": "sendMessage",
                      "arguments": {
                        "remoteDevices": [
                          {
                            "device": {
                              "name": "hisense",
                              "messages": [
                                {
                                  "message1": {
                                    "buf": [
                                      "0x03",
                                      "0x36"
                                    ],
                                    "len": 2,
                                    "repeat": 2,
                                    "delay": 1
                                  }
                                }
                              ]
                            }
                          }
                        ]
                      }
                    }

# message send to know the current power status of device
getPowerStatusMessage = {
                      "apiName": "sendMessage",
                      "arguments": {
                        "remoteDevices": [
                          {
                            "device": {
                              "name": "hisense",
                              "messages": [
                                {
                                  "message1": {
                                    "buf": [
                                      "0x03",
                                      "0x8F"
                                    ],
                                    "len": 2,
                                    "repeat": 2,
                                    "delay": 1
                                  }
                                }
                              ]
                            }
                          }
                        ]
                      }
                    }

# message for perform OTP Action
performOTPActionMessage = {
                      "apiName": "sendMessage",
                      "arguments": {
                        "remoteDevices": [
                          {
                            "device": {
                              "name": "hisense",
                              "messages": [
                                {
                                  "message1": {
                                    "buf": [
                                      "0x03",
                                      "0x04"
                                    ],
                                    "len": 2,
                                    "repeat": 2,
                                    "delay": 1
                                  }
                                }
                              ]
                            }
                          }
                        ]
                      }
                    }

# api overrides data in which return value for HdmiCecOpen is set to -1
cec_minus_one = {
            "apiName": "setAPIConfig",
            "arguments": {
                "apiOverrides": [
                    {
                        "HdmiCecOpen": [
                            {"return": -1},
                            {"outParams": [{"handle": 2345678}]}
                        ]
                    },
                    {
                        "HdmiCecGetLogicalAddress": [
                            {"return": 0},
                            {"outParams": [{"logicalAddress": "0x3"}]}
                        ]
                    },
                    {
                        "HdmiCecGetPhysicalAddress": [
                            {"return": 0},
                            {"outParams": [{"physicalAddress": "0x304"}]}
                        ]
                    }
                ]
            }
        }
