set -x #enable debugging mode

source /root/.bashrc
#If Local compliation enable below
# source ~/.bashrc

export PLATFORM_SOC=intel

SCRIPT=$(readlink -f "$0")
#SCRIPTS_DIR=`dirname "$SCRIPT"`
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export BUILDS_DIR=$SCRIPT_DIR/workspace
export RDK_PROJECT_ROOT_PATH=$SCRIPT_DIR/workspace/deps/rdk
export COMBINED_ROOT=$BUILDS_DIR
export NUM_DIR=$BUILDS_DIR/96e2377
export DS_PATH=$BUILDS_DIR/deps/rdk/devicesettings
export USE_DBUS=y
export TOOLCHAIN_DIR=$COMBINED_ROOT/sdk/toolchain/staging_dir
export CROSS_TOOLCHAIN=/usr
export CROSS_COMPILE=$CROSS_TOOLCHAIN/bin/i686-cm-linux
export CC=gcc-9
export CXX=g++-9
export OPENSOURCE_BASE=/usr
#export DFB_ROOT=$TOOLCHAIN_DIR 
export DFB_LIB=$TOOLCHAIN_DIR/lib
export IARM_PATH=$BUILDS_DIR/deps/rdk/iarmbus
export FUSION_PATH=$BUILDS_DIR/deps/rdk/fusiondale
export SDK_FSROOT=$COMBINED_ROOT/sdk/fsroot/ramdisk
export FSROOT=$COMBINED_ROOT
export GLIB_INCLUDE_PATH=$CROSS_TOOLCHAIN/include/glib-2.0
export GLIB_LIBRARY_PATH=/usr/local/lib
# export GLIB_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu
export GLIB_CONFIG_INCLUDE_PATH=$GLIB_LIBRARY_PATH/glib-2.0
export GLIB_CONFIG_PATH=$GLIB_LIBRARY_PATH/glib-2.0
export GLIBS='-lglib-2.0 -lz'

export MFR_PATH=$COMBINED_ROOT/ri/mpe_os/platforms/intel/groveland/mfrlibs
export MFR_FPD_PATH=$COMBINED_ROOT/mfrlibs
export _ENABLE_WAKEUP_KEY=-D_ENABLE_WAKEUP_KEY
export RF4CE_PATH=$COMBINED_ROOT/rf4ce/
export USE_GREEN_PEAK_RF4CE_INTERFACE=-DUSE_GREEN_PEAK_RF4CE_INTERFACE

export IARM_LIB_PATH=$BUILDS_DIR/deps/rdk/iarmbus/install
THUNDER_LIB_PATH=$BUILDS_DIR/install/usr/lib
THUNDER_PATH=$BUILDS_DIR/install/usr/bin

export SAFEC_INCLUDE_PATH=$BUILDS_DIR/deps/third-party/safeclib/src/.libs/

#export SAFECLIB_PATH=$BUILDS_DIR/deps/third-party/safeclib/include/
export SAFEC_INCLUDE_PATH=/usr/local/include/libsafec

export DFB_INCLUDE_PATH=/usr/local/include/directfb

export HDMICEC_PATH=$BUILDS_DIR/deps/rdk/hdmicec
#export CCEC_HOST=${HDMICEC_PATH}/host/include/ccec/host
#export INCLUDE_DRI_IARMBUS=${HDMICEC_PATH}/ccec/drivers/include/ccec/drivers/iarmbus
#export CCEC_INCLUDE_CCEC=${HDMICEC_PATH}/ccec/include/ccec
#export DS_HAL_INCLUDE=${DS_PATH}/hal/include
#export DS_RPC_INCLUDE=${DS_PATH}/rpc/include
#export DS_DS_INCLUDE=${DS_PATH}/ds/include
#export IARM_CORE_INC=$BUILDS_DIR/deps/rdk/iarmbus/core/include
#export INCLUDE_OSAL=$BUILDS_DIR/deps/rdk/hdmicec/osal/include/osal
#export NUM_HAL_INCLUDE=$BUILDS_DIR/96e2377/hal/include
#export NUM_COMMON_INC=$NUM_DIR/mfr/common/include
#export NUM_IR_INC=$NUM_DIR/ir/include
#export NUM_POW_INC=$NUM_DIR/power/include
#export NUM_PWMGR_INC=$NUM_DIR/pwrmgr2/include
#export NUM_REC_INC=$NUM_DIR/receiver/include
HDMICEC_LIB_PATH=${HDMICEC_PATH}/install/lib

DS_LIB_PATH=${DS_PATH}/install/lib

export LD_LIBRARY_PATH=${THUNDER_LIB_PATH}:${IARM_LIB_PATH}:${HDMICEC_LIB_PATH}:${DS_LIB_PATH}

DS_INCLUDE_PATH=${DS_PATH}/ds/include
IARM_INCLUDE_PATH=${IARM_PATH}/core/include
IARM_LIB_PATH=${IARM_PATH}/install
export IARM_MGRS_PATH=$BUILDS_DIR/deps/rdk/iarmmgrs
export IARM_MGRS=$BUILDS_DIR/deps/rdk/iarmmgrs
export GLIB_HEADER_PATH=$CROSS_TOOLCHAIN/include/glib-2.0
export PLATFORM_SOC=L2HalMock
export UTILS_PATH=$IARM_MGRS/utils

# Temporary
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin

export PATH=${IARM_MGRS}:${HDMICEC_PATH}:${HDMICEC_LIB_PATH}:${DS_PATH}:${IARM_PATH}:${IARM_LIB_PATH}:${DS_INCLUDE_PATH}:${DS_LIB_PATH}:${THUNDER_PATH}:${PATH}

set +x #disable debugging mode

echo "Done!"
