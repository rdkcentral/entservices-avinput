#!/bin/bash

SCRIPT=$(readlink -f "$0")
SCRIPTS_DIR=`dirname "$SCRIPT"`
WORKSPACE=$SCRIPTS_DIR/../../../..

echo -e "${GREEN}========================================Run rdkservices===============================================${NC}"
cd $WORKSPACE
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH/usr/local/lib/:$WORKSPACE/deps/rdk/hdmicec/install/lib:$WORKSPACE/deps/rdk/hdmicec/ccec/drivers/test:$WORKSPACE/deps/rdk/iarmbus/install/:$WORKSPACE/install/usr/lib:$WORKSPACE/deps/rdk/devicesettings/install/lib
$WORKSPACE/install/usr/bin/WPEFramework -f -c $WORKSPACE/install/etc/WPEFramework/config.json &