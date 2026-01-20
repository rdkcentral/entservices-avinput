#!/bin/bash

# Define ANSI color codes for green
GREEN='\033[0;32m'     # Green text
NC='\033[0m'           # No color (resets to default)

# Define ANSI color codes for green
GREEN='\033[0;32m'     # Green text
NC='\033[0m'           # No color (resets to default)

SCRIPT=$(readlink -f "$0")
SCRIPTS_DIR=`dirname "$SCRIPT"`
WORKSPACE=$SCRIPTS_DIR/workspace

echo -e "${GREEN}========================================Stop all existing services===============================================${NC}"

killall -9 IARMDaemonMain
killall -9 CecDaemonMain
killall -9 WPEFramework

pkill IARMDaemonMain
pkill CecDaemonMain
pkill WPEFramework

echo -e "${GREEN}========================================Run iarmbus===============================================${NC}"
cd $WORKSPACE
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/
deps/rdk/iarmbus/install/bin/IARMDaemonMain &

unset LD_LIBRARY_PATH

echo -e "${GREEN}========================================Run cec deamon===============================================${NC}"
cd $WORKSPACE
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/:$WORKSPACE/deps/rdk/hdmicec/install/lib:$WORKSPACE/deps/rdk/hdmicec/ccec/drivers/test:$WORKSPACE/deps/rdk/iarmbus/install/
deps/rdk/hdmicec/install/bin/CecDaemonMain &


echo -e "${GREEN}========================================Run rdkservices===============================================${NC}"
cd $WORKSPACE
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH/usr/local/lib/:$WORKSPACE/deps/rdk/hdmicec/install/lib:$WORKSPACE/deps/rdk/hdmicec/ccec/drivers/test:$WORKSPACE/deps/rdk/iarmbus/install/:$WORKSPACE/install/usr/lib:$WORKSPACE/deps/rdk/devicesettings/install/lib
$WORKSPACE/install/usr/bin/WPEFramework -f -c $WORKSPACE/install/etc/WPEFramework/config.json &
