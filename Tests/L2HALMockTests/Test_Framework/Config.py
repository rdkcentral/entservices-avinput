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
                    {"physicalAddress": "19"},
                    {"logicalAddress": "3"},
                    {"vendorId": "4567"},
                    {"osdName": "404753747265616D696E672054776F"},
                    {"menuLanguage": "454E47"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "04"}
                ]
            },
            {
                "device": [
                    {"name": "Audio Device"},
                    {"islocal": 0},
                    {"type": "source"},
                    {"powerState": 0},
                    {"physicalAddress": "304"},
                    {"logicalAddress": "5"},
                    {"vendorId": "4567"},
                    {"osdName": "0053616d73756e67"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "04"}
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
                    {"osdString": "48656C6C6F"},
                    {"cecVersion": "04"},
                    {"abortReason": "04"}
                ]
            },
            {
                "device": [
                    {"name": "hisense"},
                    {"islocal": 0},
                    {"type": "sink"},
                    {"powerState": 0},
                    {"physicalAddress": "0"},
                    {"logicalAddress": "0"},
                    {"vendorId": "0x4567"},
                    {"osdName": "545620426F78"},
                    {"menuLanguage": "454E47"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "04"}

                ]
            }
        ]
    }
}

config_data_no_audio = {
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
                    {"menuLanguage": "454E47"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "04"}
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
                    {"osdString": "48656C6C6F"},
                    {"cecVersion": "04"},
                    {"abortReason": "04"}
                ]
            },
            {
                "device": [
                    {"name": "hisense"},
                    {"islocal": 0},
                    {"type": "sink"},
                    {"powerState": 0},
                    {"physicalAddress": "0"},
                    {"logicalAddress": "0"},
                    {"vendorId": "0x4567"},
                    {"osdName": "545620426F78"},
                    {"menuLanguage": "454E47"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "04"}

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
                    {"return": 0},
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

#Hal Api Data for error scenarios
api_data_negative = {
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
                    {"return": -1},
                    {"outParams": [{"logicalAddress": "0x3"}]}
                ]
            },
            {
                "HdmiCecGetPhysicalAddress": [
                    {"return": -1},
                    {"outParams": [{"physicalAddress": "0x304"}]}
                ]
            }
        ]
    }
}


abort_data_1 = {
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
                    {"menuLanguage": "454E47"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "159"}
                ]
            },
            {
                "device": [
                    {"name": "Audio Device"},
                    {"islocal": 0},
                    {"type": "source"},
                    {"powerState": 0},
                    {"physicalAddress": "304"},
                    {"logicalAddress": "5"},
                    {"vendorId": "4567"},
                    {"osdName": "0053616d73756e67"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "159"}
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
                    {"osdString": "48656C6C6F"},
                    {"cecVersion": "04"},
                    {"abortReason": "159"}
                ]
            },
            {
                "device": [
                    {"name": "hisense"},
                    {"islocal": 0},
                    {"type": "sink"},
                    {"powerState": 0},
                    {"physicalAddress": "0"},
                    {"logicalAddress": "0"},
                    {"vendorId": "0x4567"},
                    {"osdName": "545620426F78"},
                    {"menuLanguage": "454E47"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "159"}

                ]
            }
        ]
    }
}

abort_data_2 = {
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
                    {"menuLanguage": "454E47"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "140"}
                ]
            },
            {
                "device": [
                    {"name": "Audio Device"},
                    {"islocal": 0},
                    {"type": "source"},
                    {"powerState": 0},
                    {"physicalAddress": "304"},
                    {"logicalAddress": "5"},
                    {"vendorId": "4567"},
                    {"osdName": "0053616d73756e67"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "140"}
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
                    {"osdString": "48656C6C6F"},
                    {"cecVersion": "04"},
                    {"abortReason": "140"}
                ]
            },
            {
                "device": [
                    {"name": "hisense"},
                    {"islocal": 0},
                    {"type": "sink"},
                    {"powerState": 0},
                    {"physicalAddress": "0"},
                    {"logicalAddress": "0"},
                    {"vendorId": "0x4567"},
                    {"osdName": "545620426F78"},
                    {"menuLanguage": "454E47"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "140"}

                ]
            }
        ]
    }
}

abort_data_3 = {
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
                    {"menuLanguage": "454E47"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "70"}
                ]
            },
            {
                "device": [
                    {"name": "Audio Device"},
                    {"islocal": 0},
                    {"type": "source"},
                    {"powerState": 0},
                    {"physicalAddress": "304"},
                    {"logicalAddress": "5"},
                    {"vendorId": "4567"},
                    {"osdName": "0053616d73756e67"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "70"}
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
                    {"osdString": "48656C6C6F"},
                    {"cecVersion": "04"},
                    {"abortReason": "70"}
                ]
            },
            {
                "device": [
                    {"name": "hisense"},
                    {"islocal": 0},
                    {"type": "sink"},
                    {"powerState": 0},
                    {"physicalAddress": "0"},
                    {"logicalAddress": "0"},
                    {"vendorId": "0x4567"},
                    {"osdName": "545620426F78"},
                    {"menuLanguage": "454E47"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "70"}

                ]
            }
        ]
    }
}

abort_data_4 = {
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
                    {"menuLanguage": "454E47"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "143"}
                ]
            },
            {
                "device": [
                    {"name": "Audio Device"},
                    {"islocal": 0},
                    {"type": "source"},
                    {"powerState": 0},
                    {"physicalAddress": "304"},
                    {"logicalAddress": "5"},
                    {"vendorId": "4567"},
                    {"osdName": "0053616d73756e67"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "143"}
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
                    {"osdString": "48656C6C6F"},
                    {"cecVersion": "04"},
                    {"abortReason": "143"}
                ]
            },
            {
                "device": [
                    {"name": "hisense"},
                    {"islocal": 0},
                    {"type": "sink"},
                    {"powerState": 0},
                    {"physicalAddress": "0"},
                    {"logicalAddress": "0"},
                    {"vendorId": "0x4567"},
                    {"osdName": "545620426F78"},
                    {"menuLanguage": "454E47"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "143"}

                ]
            }
        ]
    }
}

abort_data_5 = {
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
                    {"menuLanguage": "454E47"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "164"}
                ]
            },
            {
                "device": [
                    {"name": "Audio Device"},
                    {"islocal": 0},
                    {"type": "source"},
                    {"powerState": 0},
                    {"physicalAddress": "304"},
                    {"logicalAddress": "5"},
                    {"vendorId": "4567"},
                    {"osdName": "0053616d73756e67"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "164"}
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
                    {"osdString": "48656C6C6F"},
                    {"cecVersion": "04"},
                    {"abortReason": "164"}
                ]
            },
            {
                "device": [
                    {"name": "hisense"},
                    {"islocal": 0},
                    {"type": "sink"},
                    {"powerState": 0},
                    {"physicalAddress": "0"},
                    {"logicalAddress": "0"},
                    {"vendorId": "0x4567"},
                    {"osdName": "545620426F78"},
                    {"menuLanguage": "454E47"},
                    {"osdString": "484953454E5345"},
                    {"cecVersion": "04"},
                    {"abortReason": "164"}

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
        #         {"optionalProperty1": "value1"},
        #        {"optionalProperty2": "value2"}
    ]
}

api_data_sink = {
    "apiName": "setAPIConfig",
    "arguments": {
        "apiOverrides": [
            {
                "HdmiCecTx": [
                    {"return": -1},
                    {"outParams": [{"handle": 2345678}]}
                ]
            },
            {
                "HdmiCecOpen": [
                    {"return": -1},
                    {"outParams": [{"handle": 2345678}]}
                ]
            },
            {
                "HdmiCecGetLogicalAddress": [
                    {"return": -1},
                    {"outParams": [{"logicalAddress": "0x3"}]}
                ]
            },
            {
                "HdmiCecGetPhysicalAddress": [
                    {"return": -1},
                    {"outParams": [{"physicalAddress": "0x304"}]}
                ]
            }
        ]
    }
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
        #  {"optionalProperty1": "value1"},
        #  {"optionalProperty2": "value2"}
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
                                    "0x30",
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

# message send to know the current power status of device
reportPowerStatusMessage = {
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
                                    "0x30",
                                    "0x90"
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
                                    "0x30",
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

# messages for active source
active_source_firestick1 = {
    "apiName": "sendMessage",
    "arguments": {
        "remoteDevices": [
            {
                "device": {
                    "name": "amazon fire stick",
                    "messages": [
                        {
                            "message1": {
                                "buf": [
                                    "0x34",
                                    "0x82"
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

active_source_hisense = {
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
                                    "0x30",
                                    "0x82"
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

active_source_xione_uk = {
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
                                    "0xF3",
                                    "0x82"
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
# messages for inactive source
inactive_source_firestick1 = {
    "apiName": "sendMessage",
    "arguments": {
        "remoteDevices": [
            {
                "device": {
                    "name": "amazon fire stick",
                    "messages": [
                        {
                            "message1": {
                                "buf": [
                                    "0x34",
                                    "0x9D"
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

inactive_source_hisense = {
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
                                    "0x30",
                                    "0x9D"
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

# messages for request active source
request_active_source_firestick1 = {
    "apiName": "sendMessage",
    "arguments": {
        "remoteDevices": [
            {
                "device": {
                    "name": "amazon fire stick",
                    "messages": [
                        {
                            "message1": {
                                "buf": [
                                    "0x34",
                                    "0x85"
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

request_active_source_hisense = {
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
                                    "0x30",
                                    "0x85"
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

image_view_on = {
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
                                    "0x30",
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

text_view_on = {
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
                                    "0x30",
                                    "0x0D"
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

routing_change = {
    "apiName": "sendMessage",
    "arguments": {
        "remoteDevices": [
            {
                "device": {
                    "name": "amazon fire stick",
                    "messages": [
                        {
                            "message1": {
                                "buf": [
                                    "0x30",
                                    "0x80"
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

set_stream_path_hisense = {
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
                                    "0x30",
                                    "0x86"
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

routing_information_hisense = {
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
                                    "0x30",
                                    "0x81"
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

give_device_power_status_hisense = {

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
                                    "0x30",
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

set_menu_language = {
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
                                    "0x30",
                                    "0x32"
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

ignore_set_menu_language = {
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
                                    "0x34",
                                    "0x32"
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

get_menu_language_hisense = {
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
                                    "0x30",
                                    "0x91"
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

get_cec_version = {
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
                                    "0x30",
                                    "0x9F"
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

cec_version = {
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
                                    "0x30",
                                    "0x9E"
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

give_physical_address_hisense = {
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
                                    "0x30",
                                    "0x83"
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

give_osd_name_hisense = {
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
                                    "0x30",
                                    "0x46"
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

set_osd_string = {
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
                                    "0x30",
                                    "0x64"
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

feature_abort = {
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
                                    "0x30",
                                    "0x00"
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

abort_hisense = {
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
                                    "0x30",
                                    "0xFF"
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

feature_abort_hisense = {
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
                                    "0x30",
                                    "0x00"
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

polling = {
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
                                    "0x30",
                                    "0x200"
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

sinkActiveSource = {
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
                                    "0x50",
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

sinkActiveSource = {
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
                                    "0x50",
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

imageViewON = {
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
                                    "0x50",
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

textViewON = {
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
                                    "0x30",
                                    "0x0D"
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

reportPhysicalAdd = {
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
                                    "0xF0",
                                    "0x84"
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

DeviceVendorID = {
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
                                    "0xF0",
                                    "0x87"
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

initiateArc = {
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
                                    "0x05",
                                    "0xC0"
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

terminateArc = {
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
                                    "0x05",
                                    "0xC5"
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

reportShortAudioDes = {
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
                                    "0x05",
                                    "0xA3"
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

setSystemAudioMode = {
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
                                    "0x50",
                                    "0x72"
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

reportAudioMode = {
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
                                    "0x50",
                                    "0x7A"
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

givefeatures = {
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
                                    "0x50",
                                    "0xA5"
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

requestcurrentlatency = {
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
                                    "0x50",
                                    "0xA7"
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


api_data_sink_1 = {
    "apiName": "setAPIConfig",
    "arguments": {
        "apiOverrides": [
            {
                "HdmiCecTx": [
                    {"return": 0},
                    {"outParams": [{"handle": 2345678}]}
                ]
            },
            {
                "HdmiCecOpen": [
                    {"return": 0},
                    {"outParams": [{"handle": 2345678}]}
                ]
            },
            {
                "HdmiCecGetLogicalAddress": [
                    {"return": 0},
                    {"outParams": [{"logicalAddress": "0x3"},{"result":0}]}
                ]
            },
            {
                "HdmiCecGetPhysicalAddress": [
                    {"return": -1},
                    {"outParams": [{"physicalAddress": "0x304"}]}
                ]
            }
        ]
    }
}


api_data_sink_2 = {
    "apiName": "setAPIConfig",
    "arguments": {
        "apiOverrides": [
            {
                "HdmiCecTx": [
                    {"return": 0},
                    {"outParams": [{"handle": 2345678}]}
                ]
            },
            {
                "HdmiCecOpen": [
                    {"return": 0},
                    {"outParams": [{"handle": 2345678}]}
                ]
            },
            {
                "HdmiCecGetLogicalAddress": [
                    {"return": 0},
                    {"outParams": [{"logicalAddress": "0x3"},{"result":1}]}
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


api_data_sink_3 = {
    "apiName": "setAPIConfig",
    "arguments": {
        "apiOverrides": [
            {
                "HdmiCecTx": [
                    {"return": 0},
                    {"outParams": [{"handle": 2345678}]}
                ]
            },
            {
                "HdmiCecOpen": [
                    {"return": 0},
                    {"outParams": [{"handle": 2345678}]}
                ]
            },
            {
                "HdmiCecGetLogicalAddress": [
                    {"return": 0},
                    {"outParams": [{"logicalAddress": "0x3"},{"result":2}]}
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


api_data_sink_4 = {
    "apiName": "setAPIConfig",
    "arguments": {
        "apiOverrides": [
            {
                "HdmiCecTx": [
                    {"return": 0},
                    {"outParams": [{"handle": 2345678}]}
                ]
            },
            {
                "HdmiCecOpen": [
                    {"return": 0},
                    {"outParams": [{"handle": 2345678}]}
                ]
            },
            {
                "HdmiCecGetLogicalAddress": [
                    {"return": 0},
                    {"outParams": [{"logicalAddress": "0x3"},{"result":3}]}
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


api_data_sink_5 = {
    "apiName": "setAPIConfig",
    "arguments": {
        "apiOverrides": [
            {
                "HdmiCecTx": [
                    {"return": 0},
                    {"outParams": [{"handle": 2345678}]}
                ]
            },
            {
                "HdmiCecOpen": [
                    {"return": 0},
                    {"outParams": [{"handle": 2345678}]}
                ]
            },
            {
                "HdmiCecGetLogicalAddress": [
                    {"return": 0},
                    {"outParams": [{"logicalAddress": "0x3"},{"result":4}]}
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


api_data_sink_6 = {
    "apiName": "setAPIConfig",
    "arguments": {
        "apiOverrides": [
            {
                "HdmiCecTx": [
                    {"return": 0},
                    {"outParams": [{"handle": 2345678}]}
                ]
            },
            {
                "HdmiCecOpen": [
                    {"return": 0},
                    {"outParams": [{"handle": 2345678}]}
                ]
            },
            {
                "HdmiCecGetLogicalAddress": [
                    {"return": 0},
                    {"outParams": [{"logicalAddress": "0x3"},{"result":5}]}
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


api_data_sink_7 = {
    "apiName": "setAPIConfig",
    "arguments": {
        "apiOverrides": [
            {
                "HdmiCecTx": [
                    {"return": 0},
                    {"outParams": [{"handle": 2345678}]}
                ]
            },
            {
                "HdmiCecOpen": [
                    {"return": 0},
                    {"outParams": [{"handle": 2345678}]}
                ]
            },
            {
                "HdmiCecGetLogicalAddress": [
                    {"return": 0},
                    {"outParams": [{"logicalAddress": "0x3"},{"result":6}]}
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


# Data required for setDeviceConfig api HdmiCecSource
config_data_hdmicecsource = {
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
                                {"menuLanguage": "454E47"},
                                {"osdString": "484953454E5345"},
                                {"cecVersion": "04"},
                                {"abortReason": "04"}
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
                                {"osdString": "484953454E5345"},
                                {"cecVersion": "04"},
                                {"abortReason": "04"}
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
                                {"osdString": "48656C6C6F"},
                                {"cecVersion": "04"},
                                {"abortReason": "04"}
                            ]
                        },
                        {
                            "device": [
                                {"name": "hisense"},
                                {"islocal": 0},
                                {"type": "sink"},
                                {"powerState": 0},
                                {"physicalAddress": "0"},
                                {"logicalAddress": "0"},
                                {"vendorId": "0x4567"},
                                {"osdName": "545620426F78"},
                                {"menuLanguage": "454E47"},
                                {"osdString": "484953454E5345"},
                                {"cecVersion": "04"},
                                {"abortReason": "04"}

                            ]
                        }
                    ]
                }
        }

# Data required for setAPIConfig api HdmiCecSource
api_data_hdmicecsource = {
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
