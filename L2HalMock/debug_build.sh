#!/bin/bash

# Define ANSI color codes for green
GREEN='\033[0;32m'     # Green text
YELLOW='\033[0;33m'     # Yellow text
NC='\033[0m'           # No color (resets to default)

SCRIPT=$(readlink -f "$0")
SCRIPTS_DIR=`dirname "$SCRIPT"`
RDK_DIR=$SCRIPTS_DIR/../
WORKSPACE=$SCRIPTS_DIR/workspace

PS3='Please enter your choice: '
options=("Build IARM" "Build DeviceSettings" "Build HdmiCec" "Build rdkservices" "Build All" "Quit")
# set -x #enable debugging mode
select opt in "${options[@]}" 
do
    case $opt in
        "Build IARM")
            echo "you chose choice $opt"
            # Build IARM
            echo -e "${GREEN}========================================Build IARM===============================================${NC}"
            (cd $WORKSPACE/deps/rdk/iarmbus/ && ./build.sh)
            ;;
        "Build DeviceSettings")
            echo "you chose choice $opt"
            echo -e "${GREEN}========================================Build DeviceSettings===============================================${NC}"
            # Build DeviceSettings
            (cd $WORKSPACE/deps/rdk/devicesettings/ && ./build.sh)
            ;;

        "Build HdmiCec")
            echo "you chose choice $opt"
            echo -e "${GREEN}========================================Build HdmiCec===============================================${NC}"
            # Build HdmiCec
            (cd $WORKSPACE/deps/rdk/hdmicec/ && ./build.sh)
            ;;
        
        "Build rdkservices")
            echo "you chose choice $opt"
            echo -e "${GREEN}========================================Build rdkservices===============================================${NC}"
            cd $RDK_DIR;

            cmake -S . -B build \
            -DCMAKE_INSTALL_PREFIX="$WORKSPACE/install/usr" \
            -DCMAKE_MODULE_PATH="$WORKSPACE/install/usr/include/WPEFramework/Modules" \
            -DPLUGIN_HDMICECSOURCE=ON \
            -DPLUGIN_HDMICECSINK=ON \
            -DUSE_THUNDER_R4=ON \
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
            ;;

        "Build All")
            echo "you chose choice $opt"
            echo -e "${GREEN}========================================Building All===============================================${NC}"
            ./build.sh
            ;;

        "Quit")
            break
            ;;
        *) echo "invalid option $REPLY";;
    esac
    echo -e "${YELLOW}1) Build IARM 2) Build DeviceSettings 3) Build HdmiCec 4) Build rdkservices 5) Build All 6) Quit${NC}"
done

set +x #disable debugging mode
