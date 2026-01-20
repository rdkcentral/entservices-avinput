#!/bin/bash

# Access the passed plugins inside the script
SelectedPlugins="$1"

echo "Building for Plugins: $SelectedPlugins"

# Check for the presence of "string to find"
if grep -q "HdmiCecSource" <<< "$SelectedPlugins"; then
  echo "Found: $SelectedPlugins"
  HdmiCecSource="ON"
fi

if grep -q "HdmiCecSink" <<< "$SelectedPlugins"; then
  echo "Found: $SelectedPlugins"
  HdmiCecSink="ON"
fi

# Define ANSI color codes for green
GREEN='\033[0;32m'     # Green text
NC='\033[0m'           # No color (resets to default)

SCRIPT=$(readlink -f "$0")
SCRIPTS_DIR=`dirname "$SCRIPT"`
RDK_DIR=$SCRIPTS_DIR/../
WORKSPACE=$SCRIPTS_DIR/workspace
rm -rf $WORKSPACE; 
mkdir $WORKSPACE;

cd $WORKSPACE
echo -e "${GREEN}========================================Building thunder tools===============================================${NC}"
git clone -b R4_4 https://github.com/rdkcentral/ThunderTools.git
cmake -G Ninja -S ThunderTools -B $WORKSPACE/build/ThunderTools -DCMAKE_INSTALL_PREFIX="$WORKSPACE/install/usr" 
cmake --build $WORKSPACE/build/ThunderTools --target install 

cd $WORKSPACE
echo -e "${GREEN}========================================Building Thunder===============================================${NC}"
git clone -b R4_4 https://github.com/rdkcentral/Thunder.git 
cmake -G Ninja -S Thunder -B $WORKSPACE/build/Thunder -DBINDING="127.0.0.1" -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_INSTALL_PREFIX="$WORKSPACE/install/usr" -DCMAKE_MODULE_PATH="${WORKSPACE}/install/usr/include/WPEFramework/Modules" -DDATA_PATH="${WORKSPACE}/install/usr/share/WPEFramework" -DPERSISTENT_PATH="${WORKSPACE}/install/var/wpeframework" -DPORT="55555" -DPROXYSTUB_PATH="${WORKSPACE}/install/usr/lib/wpeframework/proxystubs" -DSYSTEM_PATH="${WORKSPACE}/install/usr/lib/wpeframework/plugins" -DVOLATILE_PATH="tmp"
cmake --build $WORKSPACE/build/Thunder --target install

echo -e "${GREEN}========================================Do peru sync===============================================${NC}"
(cp $SCRIPTS_DIR/peru.yaml . && peru sync && peru sync --no-cache)


echo -e "${GREEN}========================================Building flux===============================================${NC}"
(cd $WORKSPACE/deps/third-party/flux && autoreconf; autoreconf -f -i && ./configure && make &&  make install && cd -)


echo -e "${GREEN}========================================Building directfb===============================================${NC}"
(cd $WORKSPACE/deps/third-party/directfb && autoreconf; autoreconf -f -i && ./configure && make &&  make install && cd -)


echo -e "${GREEN}========================================Building log4c===============================================${NC}"
(cd $WORKSPACE/deps/third-party/log4c && autoreconf; autoreconf -f -i && ./configure && make &&  make install && cd -)

set +e #exit on error
echo -e "${GREEN}========================================Building safeclib===============================================${NC}"
(cd $WORKSPACE/deps/third-party/safeclib && autoreconf; autoreconf -f -i && ./configure && make &&  make install && cd -)

echo -e "${GREEN}========================================Building dbus===============================================${NC}"
(cd $WORKSPACE/deps/third-party/dbus/ && autoreconf; autoreconf -f -i && ./configure && make &&  make install && cd -)

echo -e "${GREEN}========================================Build glib===============================================${NC}"
(cd $WORKSPACE/deps/third-party/glib && meson _build && ninja -C _build )

echo -e "${GREEN}========================================SafeC header===============================================${NC}"
# cp -r $WORKSPACE/deps/rdk/safec/recipes-common/safec-common-wrapper/files/safec_lib.h /usr/include
cp -r $SCRIPTS_DIR/patches/rdkservices/iarmbus/safec_lib.h /usr/include
cd $WORKSPACE/deps/rdk/safec/recipes-common/safec-common-wrapper/files
# ls
cp -r $WORKSPACE/deps/third-party/safeclib/include/safe_str_lib.h /usr/include
# cp -r $SCRIPTS_DIR/patches/rdkservices/iarmbus/safe_str_lib.h /usr/include
cp -r $WORKSPACE/deps/third-party/safeclib/include/safe_config.h /usr/include
cp -r $WORKSPACE/deps/third-party/safeclib/include/safe_lib_errno.h /usr/include
cp -r $WORKSPACE/deps/third-party/safeclib/include/safe_types.h /usr/include
cp -r $WORKSPACE/deps/third-party/safeclib/include/safe_compile.h /usr/include
cp -r $WORKSPACE/deps/third-party/safeclib/include/safe_mem_lib.h /usr/include
cp -r $WORKSPACE/deps/third-party/dbus/dbus/*.h /usr/include/dbus-1.0/dbus
cp -r /usr/local/lib/libsafec* /usr/lib/
cp -r $WORKSPACE/deps/third-party/glib/_build/glib/glibconfig.h /usr/include
cp -r $WORKSPACE/deps/rdk/halif-power_manager/include/*.h /usr/include
cp -r $WORKSPACE/deps/rdk/halif-deepsleep_manager/include/deepSleepMgr.h /usr/include
cp -r $WORKSPACE/deps/rdk/SyscallWrapper/source/secure_wrapper.h /usr/include
cp -r $WORKSPACE/deps/rdk/iarmmgrs/mfr/include/*.h /usr/include
cp -r $WORKSPACE/deps/rdk/iarmmgrs/hal/include/*.h /usr/include
cp -r $WORKSPACE/deps/rdk/iarmmgrs/sysmgr/include/*.h /usr/include
set +e #exit on error


echo -e "${GREEN}========================================iarmmgrs-emulator===============================================${NC}"
(cd $WORKSPACE/deps/rdk/iarmmgrs-emulator && ./build.sh)
mkdir -p $WORKSPACE/deps/rdk/iarmmgrs/install/lib
cp -r $WORKSPACE/deps/rdk/iarmmgrs-emulator/power/*.so $WORKSPACE/deps/rdk/iarmmgrs/install/lib
cp -r $WORKSPACE/deps/rdk/iarmmgrs-emulator/ir/*.so $WORKSPACE/deps/rdk/iarmmgrs/install/lib

echo "Entering directory $PWD"
cp $SCRIPTS_DIR/env.sh $WORKSPACE/env.sh
cp $SCRIPTS_DIR/invokeEnv.sh $WORKSPACE/deps/rdk/env.sh
cd $WORKSPACE
set -e #exit on error
source ./env.sh

set -x #enable debugging mode
# Build IARM
echo -e "${GREEN}========================================Build IARM===============================================${NC}"
cd $WORKSPACE/deps/rdk/iarmbus/
rm build.sh
cp $SCRIPTS_DIR/patches/rdkservices/iarmbus/build.sh $WORKSPACE/deps/rdk/iarmbus/
(cd $WORKSPACE/deps/rdk/iarmbus/ && ./build.sh)

# Build DSHalMrg
echo -e "${GREEN}========================================Build DSHalMrg===============================================${NC}"
# Build DSHalMrg
(cd $WORKSPACE/deps/rdk/devicesettings/hal && make && make install)

echo -e "${GREEN}========================================Build DeviceSettings===============================================${NC}"
# Build DeviceSettings
cd /usr/include/
mkdir wdmp-c
cp $SCRIPTS_DIR/patches/rdkservices/files/wdmp-c.h /usr/include/wdmp-c
cp $SCRIPTS_DIR/patches/rdkservices/rfcapi.h /usr/include
cp $WORKSPACE/deps/rdk/halif-deepsleep_manager/include/deepSleepMgr.h /usr/include
(cd $WORKSPACE/deps/rdk/devicesettings/ && ./build.sh)

echo -e "${GREEN}========================================Build glib===============================================${NC}"
(cd $WORKSPACE/deps/third-party/glib && meson _build && ninja -C _build && ninja -C _build install)
cp $WORKSPACE/deps/third-party/glib/_build/glib/glibconfig.h /usr/include/glib-2.0

echo -e "${GREEN}========================================iarm_event_Sender===============================================${NC}"
(cd $WORKSPACE/deps/rdk/sender/ && ./build.sh)


echo -e "${GREEN}========================================Build iarmmgr===============================================${NC}"
# Build iarmmgr
cd $WORKSPACE/deps/rdk/iarmmgrs
patch -s -p1 < $SCRIPTS_DIR/patches/rdkservices/iarmmgrs/ds.patch
rm build.sh
cp $SCRIPTS_DIR/patches/rdkservices/iarmmgrs/build.sh $WORKSPACE/deps/rdk/iarmmgrs
(cd $WORKSPACE/deps/rdk/iarmmgrs && ./build.sh)

echo -e "${GREEN}========================================Build HdmiCec===============================================${NC}"
# Build HdmiCec
mv $WORKSPACE/deps/rdk/hdmicec/soc/L2HalMock/common $WORKSPACE/deps/rdk/hdmicec/soc/L2HalMock/common_bkp; 
mv $WORKSPACE/deps/rdk/hdmicec/soc/L2HalMock/hdmicec-hal-emulator $WORKSPACE/deps/rdk/hdmicec/soc/L2HalMock/common
(cd $WORKSPACE/deps/rdk/hdmicec/ && ./build.sh)

echo -e "${GREEN}========================================Build HdmiCecSource===============================================${NC}"
# Build HdmiCecSource 
cd $WORKSPACE/install/usr/include/WPEFramework/Modules/
patch -s -p0 < $SCRIPTS_DIR/patches/rdkservices/FindConfigGenerator_cmake.patch

if grep -q "HdmiCecSource" <<< "$SelectedPlugins"; then
cp $SCRIPTS_DIR/patches/rdkservices/properties/HdmiCecSource/device.properties /etc/
fi

if grep -q "HdmiCecSink" <<< "$SelectedPlugins"; then
cp $SCRIPTS_DIR/patches/rdkservices/properties/HdmiCecSink/device.properties /etc/
fi

cd $WORKSPACE/
#Run time dependency 
mkdir -p $WORKSPACE/install/etc/WPEFramework/plugins
cp $SCRIPTS_DIR/patches/rdkservices/files/HdmiCecSource.json $WORKSPACE/install/etc/WPEFramework/plugins/
cp $SCRIPTS_DIR/patches/rdkservices/files/HdmiCecSink.json $WORKSPACE/install/etc/WPEFramework/plugins/

#Code Coverage patch
# cd $RDK_DIR
# patch -s -p1 < $SCRIPTS_DIR/patches/rdkservices/HdmiCecSource.patch
# patch -s -p1 < $SCRIPTS_DIR/patches/rdkservices/HdmiCecSink.patch
# patch -s -p1 < $SCRIPTS_DIR/patches/rdkservices/FrontPanel.patch
# patch -s -p1 < $SCRIPTS_DIR/patches/rdkservices/Hdcp_Profile.patch

#included CmakeHelperFunctions.cmake instead to during in CMakeLists.txt
cp ${WORKSPACE}/install/usr/lib/cmake/WPEFramework/common/CmakeHelperFunctions.cmake $WORKSPACE/install/usr/include/WPEFramework/Modules
cp -r $RDK_DIR/Tests/L2HALMockTests/. $WORKSPACE/deps/rdk/flask
cp $RDK_DIR/L2HalMock/patches/rdkservices/files/rfcapi.h $RDK_DIR/helpers

#wdmp dependency 
cd /usr/local/include/
mkdir wdmp-c
cp $SCRIPTS_DIR/patches/rdkservices/files/wdmp-c.h /usr/local/include/wdmp-c

cd /usr/include/
mkdir rdk
cd /usr/include/rdk


sed -i 's/sendNotify/Notify/g' $RDK_DIR/HdmiCecSource/HdmiCecSource.cpp
sed -i 's/sendNotify/Notify/g' $RDK_DIR/HdmiCecSink/HdmiCecSink.cpp
sed -i 's/sendNotify/Notify/g' $RDK_DIR/HdcpProfile/HdcpProfile.cpp

cd $RDK_DIR;
cmake -S . -B build \
-DCMAKE_INSTALL_PREFIX="$WORKSPACE/install/usr" \
-DCMAKE_MODULE_PATH="$WORKSPACE/install/usr/include/WPEFramework/Modules" \
-DRDK_SERVICE_L2HALMOCK=ON \
-DUSE_THUNDER_R4=ON \
-DPLUGIN_HDMICECSOURCE=$HdmiCecSource \
-DPLUGIN_HDMICECSINK=$HdmiCecSink \
-DCOMCAST_CONFIG=OFF \
-DCEC_INCLUDE_DIRS="$SCRIPTS_DIR/workspace/deps/rdk/hdmicec/ccec/include" \
-DOSAL_INCLUDE_DIRS="$SCRIPTS_DIR/workspace/deps/rdk/hdmicec/osal/include" \
-DCEC_HOST_INCLUDE_DIRS="$SCRIPTS_DIR/workspace/deps/rdk/hdmicec/host/include" \
-DDS_LIBRARIES="$SCRIPTS_DIR/workspace/deps/rdk/devicesettings/install/lib/libds.so" \
-DDS_INCLUDE_DIRS="$SCRIPTS_DIR/workspace/deps/rdk/devicesettings/ds/include" \
-DDSHAL_INCLUDE_DIRS="$SCRIPTS_DIR/workspace/deps/rdk/devicesettings/hal/include" \
-DDSRPC_INCLUDE_DIRS="$SCRIPTS_DIR/workspace/deps/rdk/devicesettings/rpc/include" \
-DIARMBUS_INCLUDE_DIRS="$SCRIPTS_DIR/workspace/deps/rdk/iarmbus/core/include" \
-DIARMRECEIVER_INCLUDE_DIRS="$SCRIPTS_DIR/workspace/deps/rdk/iarmmgrs/receiver/include" \
-DIARMPWR_INCLUDE_DIRS="$SCRIPTS_DIR/workspace/deps/rdk/iarmmgrs/hal/include" \
-DCEC_LIBRARIES="$SCRIPTS_DIR/workspace/deps/rdk/hdmicec/install/lib/libRCEC.so" \
-DIARMBUS_LIBRARIES="$SCRIPTS_DIR/workspace/deps/rdk/iarmbus/install/libIARMBus.so" \
-DDSHAL_LIBRARIES="$SCRIPTS_DIR/workspace/deps/rdk/devicesettings/install/lib/libdshalcli.so" \
-DCEC_HAL_LIBRARIES="$SCRIPTS_DIR/workspace/deps/rdk/hdmicec/install/lib/libRCECHal.so" \
-DOSAL_LIBRARIES="$SCRIPTS_DIR/workspace/deps/rdk/hdmicec/install/lib/libRCECOSHal.so" \
-DCMAKE_CXX_FLAGS="-fprofile-arcs -ftest-coverage -Wall -Werror -Wno-error=format=-Wl,-wrap,system -Wl,-wrap,popen -Wl,-wrap,syslog"

(cd $RDK_DIR && cmake --build build --target install)

set +x #disable debugging mode
