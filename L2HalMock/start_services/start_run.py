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

import subprocess
import logging
import time
# Configure logging
logging.basicConfig(level=logging.DEBUG)
# Run first Python script


# Run the script with arguments

Flask = subprocess.Popen(['python3', 'startup.py'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

if Flask:
    print("$$$ Flask service is UP $$$")

time.sleep(2)

processA = subprocess.Popen(['python3', 'IarmDaemon.py'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

time.sleep(2)
if processA:
    print("$$$ IarmDaemon is up $$$")

processB = subprocess.Popen(['python3', 'CecDaemon.py'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

time.sleep(5)

if processB:
    print("$$$ CecDaemon is up $$$")

processC = subprocess.Popen(['python3','Thunder.py'], stdout = subprocess.PIPE, stderr=subprocess.STDOUT)

if processC:
    print("$$$ WPEFramework is up $$$")

time.sleep(5)


