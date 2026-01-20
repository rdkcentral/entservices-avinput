###
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2024 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
###

RM          := rm -rf
CFLAGS      := -g -fPIC -D_REENTRANT -Wall
OBJS        := $(patsubst %.c,%.o,$(wildcard *.c))
CPPLAGS      := -g -fPIC -D_REENTRANT -Wall
EXECUTABLE  =   dsSrvMain
INCLUDE     =   -I. \
                -I$(IARM_PATH)/core/   \
                -I$(IARM_PATH)/core/include \
				-I$(IARM_MGRS)/sysmgr/include \
				-I$(IARM_MGRS)/hal/include \
				-I$(IARM_MGRS)/mfr/include \
                -I$(DS_PATH)/rpc/include \
				-I$(DS_PATH)/hal/include \
		        -I$(DS_PATH)/ds/include \
				-I$(DS_PATH)/hal/src \
				-I$(GLIB_HEADER_PATH)/ \
				-I$(GLIB_HEADER_PATH)/glib/ \
				-I$(GLIB_CONFIG_PATH)/../lib/glib-2.0/include/ \
				-I$(GLIB_INCLUDE_PATH)/ \
                -I$(DFB_ROOT)/usr/local/include/directfb \
				-I$(SAFEC_INCLUDE_PATH)
	
INSTALL     := $(PWD)/install

CFLAGS      += $(INCLUDE)

# LDFLAGS +=-Wl,--copy-dt-needed-entries,-rpath,$(SDK_FSROOT)/usr/local/lib
# LDFLAGS += -L$(IARM_PATH)/install -lIARMBus
# LDFLAGS += -L$(DS_PATH)/install/lib -lds -ldshalcli

# LDFLAGS     := -L$(CEC_ROOT)/ccec/drivers/iarmbus/install/lib -L$(CEC_ROOT)/osal/src/install/lib -L$(CEC_ROOT)/soc/$(PLATFORM_SOC)/common/install/lib -L$(CEC_ROOT)/ccec/src/install/lib
LDFLAGS     += -L$(IARM_PATH)/install -L$(CEC_ROOT)/ccec/iarmbus 
LDFLAGS     += -L$(FUSION_PATH) -L$(DFB_LIB) -L$(OPENSOURCE_BASE)/lib -L$(GLIB_LIBRARY_PATH) -L$(DS_PATH)/install/lib -L$(SAFEC_LIB_PATH) $(GLIBS) -lIARMBus -lpthread -ldirect -lfusion -ldshalsrv -lds -lds-hal  -lsafec-3.5 -lglib-2.0 
# LDFLAGS     += -lnexus # couldn't find nexus and not sure if it is really needed yet.
# LDFLAGS     += -L$(DS_PATH)/install/lib -lds -ldshalcli

# LDFLAGS += -L /home/administrator/PROJECT/GIT_COMMIT/work/LATEST_COMMIT/rdkservices/L2HalMock/workspace/deps/rdk/iarmbus/install -lIARMBus
# LDFLAGS += -L /home/administrator/PROJECT/GIT_COMMIT/work/LATEST_COMMIT/rdkservices/L2HalMock/workspace/deps/rdk/devicesettings/install/lib -ldshalcli -ldshalsrv -lds       

all:clean executable
	@echo "Build Finished...."

executable: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(EXECUTABLE) $(LDFLAGS)

%.o: %.c
	@echo "Building $@ ...."
	$(CXX) -c $<  $(CFLAGS) -o $@

%.o: %.cpp
	@echo "Building $@ ...."
	$(CXX) -c $<  $(CFLAGS) -o $@

clean:
	@echo "Cleaning the directory..."
	@$(RM) $(OBJS) $(EXECUTABLE)