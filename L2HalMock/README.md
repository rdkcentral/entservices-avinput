
README contains all the dependencies based on the peru yml file and steps to build the emulator binaries.All the run time instructions ,VM setup instructions and package dependency instructions are captured in this repo 



# Utils dependencis for ubuntu 22.04 machine 
------------------------------------------------




## Generic dependencies 
---------------------
sudo apt install -y git

pip install flake8

sudo pip install peru

sudo apt-get install -y libtool

suod apt install -y autoconf

sudo add-apt-repository ppa:ubuntu-toolchain-r/ppa -y

sudo apt update

sudo apt install -y g++-9 gcc-9

sudo apt-get install -y libglib2.0-dev

sudo apt-get install -y libdbus-1-dev

sudo apt-get install -y curl


# Install system-level dependencies
RUN apt-get update && \
    apt-get install -y \
    git \
    wget \
    vim \
    build-essential \
    libtool \
    autoconf \
    g++-9 \
    gcc-9 \
    libglib2.0-dev \
    libdbus-1-dev \
    curl \
    cmake \
    ninja-build \
    net-tools \
    netcat \
    psmisc \
    libusb-1.0-0-dev \
    zlib1g-dev \
    libssl-dev \
    python3-pip \
    libjsoncpp-dev \
    libjansson4 \
    libjansson-dev \
    libcurl4-openssl-dev \
    libwebsocketpp-dev \
    libwebsockets-dev \
    libboost-all-dev

# Create a directory in the image where you want to copy the files
RUN mkdir -p /usr/lib/aarch64-linux-gnu/

# Copy files from the host machine to the image
RUN cp -r /usr/lib/x86_64-linux-gnu/dbus-1.0 /usr/lib/aarch64-linux-gnu/

# Copy the custom config files to the appropriate location in the image
COPY dbus/system.conf /usr/share/dbus-1/system.conf
COPY dbus/session.conf /usr/share/dbus-1/session.conf


## Thuder dependencies 
--------------------
sudo apt install -y build-essential cmake ninja-build libusb-1.0-0-dev zlib1g-dev libssl-dev

sudo apt install -y python3-pip

pip install jsonref

## Hdmicec emulator hal dependencies 
----------------------------------
sudo apt-get install -y libjsoncpp-dev 

sudo apt-get install -y libjansson4 libjansson-dev

sudo apt-get install -y libcurl4-openssl-dev

sudo apt-get install -y libwebsocketpp-dev

sudo apt-get install -y libwebsockets-dev

pip install websockets

sudo apt-get install -y libboost-all-dev

## flask server dependencies 
----------------------------
sudo pip install pandas

sudo pip install beautifulsoup4

sudo pip install flask

sudo pip install colorama

--------------------------------------------------
# pre-setup
# clone rdk-e/rdkservices repository
# After cloning rdkservices.git, clone the repository https://github.com/rdk-e/FLASK-FOR-HAL-MOCK
# switch to branch peru, copy the peru.yaml file present insde the repo to L2HalMock folder inside rdkservices

# Build step 
-----------------------------------------------
./build.sh

# Using Debug Build we can build individual modules
-----------------------------------------------
./debug_build.sh

# Execute the framework
-----------------------------------------------
./run.sh

# For executing test scripts 
-----------------------------------------------
cd workspace/deps/rdk/flask/Test_Framework

update details in Config.py

python3 TestManager.py


