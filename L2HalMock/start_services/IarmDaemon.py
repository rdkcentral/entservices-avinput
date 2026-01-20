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
import sys
import os
directory_path_iarm = "/home/teuser/.local/bin/act-hdmicecsource-test-poc/deps/rdk/iarmbus/install/bin"
directory_path_cec_daemon = "/home/teuser/.local/bin/act-hdmicecsource-test-poc/deps/rdk/hdmicec/install/bin"

# Change the current working directory
os.chdir(directory_path_iarm)
result = subprocess.run(['./IARMDaemonMain'], stdout=subprocess.PIPE)
output = result.communicate()[0].decode()
print(result.stdout.decode())
print(result.stdout.read().decode())
sys.stdout.write(output)
