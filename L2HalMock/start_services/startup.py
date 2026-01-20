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

directory_path_flask = "/home/teuser/L2Testing/Flask"  #update the path to FLASK repository here (wherever its being cloned in the VM)

# Change the current working directory
os.chdir(directory_path_flask)

with open("log_file_flask", "w") as logfile:
   # Run the command and redirect the output and error to the file
   result = subprocess.run(["python3", "startup.py"], cwd=directory_path_flask, stdout=logfile, stderr=logfile)

print(result)
