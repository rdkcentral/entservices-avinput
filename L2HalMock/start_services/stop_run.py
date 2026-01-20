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
import os
import signal

process_name = "python3"
process_name_thunder = "WPEFramework"
process_name_iARM ="IARMDaemonMain"
process_name_cec ="CecDaemonMain"
output = subprocess.check_output(["ps", "aux"])
try:
    pids = []
    # Parse the output to find the PID
    for line in output.decode().splitlines():
        if process_name in line:

          pid = int(line.split()[1])
          pids.append(pid)
        else:
          pass
        if process_name_thunder in line:
          pid = int(line.split()[1])
          pids.append(pid)
          print("Killing WPEFramework service")
        else:
          pass
        if process_name_iARM in line:
          pid = int(line.split()[1])
          pids.append(pid)
          print("Killing IARM Daemon")
        else:
          pass
        if process_name_cec in line:
          pid = int(line.split()[1])
          pids.append(pid)
          print("killing CEC Daemon")
        else:
          pass
    if pids is not None:
        for each_pid in pids:
        # os.system("sudo kill %s" % (each_pid, ))
            try:
                os.system("kill -9 %s" % (each_pid, ))
            except:
                print("process already killed")
    else:
        print("They are no services up wrt HAL Mock setup")
except:
    pass
   # os.kill(each_pid, signal.SIGTERM)


#kill /home/teuser/.local/bin/start_shell_Script.py
#time.sleep(5)
