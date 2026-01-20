/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2025 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

#include "HdmiCecSourceImplementation.h"


#include "ccec/Connection.hpp"
#include "ccec/CECFrame.hpp"
#include "ccec/MessageEncoder.hpp"
#include "host.hpp"

#include "dsDisplay.h"
#include "videoOutputPort.hpp"
#include "manager.hpp"
#include "websocket/URL.h"

#include "UtilsIarm.h"
#include "UtilsJsonRpc.h"
#include "UtilssyncPersistFile.h"
#include "UtilsSearchRDKProfile.h"

#define HDMICECSOURCE_METHOD_SET_ENABLED "SetEnabled"
#define HDMICECSOURCE_METHOD_GET_ENABLED "GetEnabled"
#define HDMICECSOURCE_METHOD_OTP_SET_ENABLED "SetOTPEnabled"
#define HDMICECSOURCE_METHOD_OTP_GET_ENABLED "GetOTPEnabled"
#define HDMICECSOURCE_METHOD_SET_OSD_NAME "SetOSDName"
#define HDMICECSOURCE_METHOD_GET_OSD_NAME "GetOSDName"
#define HDMICECSOURCE_METHOD_SET_VENDOR_ID "SetVendorId"
#define HDMICECSOURCE_METHOD_GET_VENDOR_ID "GetVendorId"
#define HDMICECSOURCE_METHOD_PERFORM_OTP_ACTION "PerformOTPAction"
#define HDMICECSOURCE_METHOD_SEND_STANDBY_MESSAGE "SendStandbyMessage"
#define HDMICECSOURCE_METHOD_GET_ACTIVE_SOURCE_STATUS "getActiveSourceStatus"
#define HDMICECSOURCE_METHOD_SEND_KEY_PRESS         "SendKeyPressEvent"
#define HDMICEC_EVENT_ON_DEVICES_CHANGED "onDevicesChanged"
#define HDMICEC_EVENT_ON_HDMI_HOT_PLUG "onHdmiHotPlug"
#define HDMICEC_EVENT_ON_STANDBY_MSG_RECEIVED "standbyMessageReceived"
#define HDMICEC_EVENT_ON_KEYPRESS_MSG_RECEIVED "onKeyPressEvent"
#define HDMICEC_EVENT_ON_KEYRELEASE_MSG_RECEIVED "onKeyReleaseEvent"
#define DEV_TYPE_TUNER 1
#define HDMI_HOT_PLUG_EVENT_CONNECTED 0
#define ABORT_REASON_ID 4

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 8

#define CEC_SETTING_ENABLED_FILE "/opt/persistent/ds/cecData_2.json"
#define CEC_SETTING_ENABLED "cecEnabled"
#define CEC_SETTING_OTP_ENABLED "cecOTPEnabled"
#define CEC_SETTING_OSD_NAME "cecOSDName"
#define CEC_SETTING_VENDOR_ID "cecVendorId"

static std::vector<uint8_t> defaultVendorId = {0x00,0x19,0xFB};
static VendorID appVendorId = {defaultVendorId.at(0),defaultVendorId.at(1),defaultVendorId.at(2)};
static VendorID lgVendorId = {0x00,0xE0,0x91};
static PhysicalAddress physical_addr = {0x0F,0x0F,0x0F,0x0F};
static LogicalAddress logicalAddress = 0xF;
static OSDName osdName = "TV Box";
static int32_t powerState = 1;
static PowerStatus tvPowerState(PowerStatus::POWER_STATUS_NOT_KNOWN);
static bool isDeviceActiveSource = false;
static bool isLGTvConnected = false;

#define KEY_UNSUPPORTED 0xFF

using namespace WPEFramework;


namespace WPEFramework
{
    namespace Plugin
    {
        SERVICE_REGISTRATION(HdmiCecSourceImplementation, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        HdmiCecSourceImplementation* HdmiCecSourceImplementation::_instance = nullptr;
        static int libcecInitStatus = 0;

//=========================================== HdmiCecSourceFrameListener =========================================
        void HdmiCecSourceFrameListener::notify(const CECFrame &in) const {
                const uint8_t *buf = NULL;
                char strBuffer[512] = {0}; 
                size_t len = 0;

                in.getBuffer(&buf, &len);
                for (unsigned int i = 0; i < len; i++) {
                   snprintf(strBuffer + (i*3) , sizeof(strBuffer) - (i*3), "%02X ",(uint8_t) *(buf + i));
                }
                LOGINFO("   >>>>>    Received CEC Frame: :%s \n",strBuffer);

                MessageDecoder(processor).decode(in);
       }

//=========================================== HdmiCecSourceProcessor =========================================
       void HdmiCecSourceProcessor::process (const ActiveSource &msg, const Header &header)
       {
             LOGINFO("Command: ActiveSource %s : %s  : %s \n",GetOpName(msg.opCode()),msg.physicalAddress.name().c_str(),msg.physicalAddress.toString().c_str());
             if(msg.physicalAddress.toString() == physical_addr.toString())
                 isDeviceActiveSource = true;
             else
                 isDeviceActiveSource = false;
             LOGINFO("ActiveSource isDeviceActiveSource status :%d \n", isDeviceActiveSource);
             HdmiCecSourceImplementation::_instance->sendActiveSourceEvent();
             HdmiCecSourceImplementation::_instance->addDevice(header.from.toInt());
       }

       void HdmiCecSourceProcessor::process (const RequestActiveSource &msg, const Header &header)
       {
             LOGINFO("Command: RequestActiveSource\n");
             if(isDeviceActiveSource)
             {
                  LOGINFO("sending  ActiveSource\n");
                  try
                  { 
                      conn.sendTo(LogicalAddress::BROADCAST, MessageEncoder().encode(ActiveSource(physical_addr)));
                  } 
                  catch(...)
                  {
                     LOGWARN("Exception while sending ActiveSource");
                  }
             }
       }
       void HdmiCecSourceProcessor::process (const Standby &msg, const Header &header)
       {
             LOGINFO("Command: Standby from %s\n", header.from.toString().c_str());
             HdmiCecSourceImplementation::_instance->SendStandbyMsgEvent(header.from.toInt());

       }
       void HdmiCecSourceProcessor::process (const GetCECVersion &msg, const Header &header)
       {
             LOGINFO("Command: GetCECVersion sending CECVersion response \n");
             try
             { 
                 conn.sendTo(header.from, MessageEncoder().encode(CECVersion(Version::V_1_4)));
             } 
             catch(...)
             {
                 LOGWARN("Exception while sending CECVersion ");
             }
       }
       void HdmiCecSourceProcessor::process (const CECVersion &msg, const Header &header)
       {
             LOGINFO("Command: CECVersion Version : %s \n",msg.version.toString().c_str());
             HdmiCecSourceImplementation::_instance->addDevice(header.from.toInt());
       }
      
       void HdmiCecSourceProcessor::process (const GiveOSDName &msg, const Header &header)
       {
             if (!(header.from == LogicalAddress(LogicalAddress::UNREGISTERED)))
             {
                 LOGINFO("Command: GiveOSDName sending SetOSDName : %s\n",osdName.toString().c_str());
                 try
                 { 
                     conn.sendTo(header.from, MessageEncoder().encode(SetOSDName(osdName)));
                 }
                 catch(...)
                 {
                     LOGWARN("Exception while sending SetOSDName");
                 }
             }
       }
       void HdmiCecSourceProcessor::process (const GivePhysicalAddress &msg, const Header &header)
       {
             LOGINFO("Command: GivePhysicalAddress\n");
             try
             { 
                 LOGINFO(" sending ReportPhysicalAddress response physical_addr :%s logicalAddress :%x \n",physical_addr.toString().c_str(), logicalAddress.toInt());
                 conn.sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(ReportPhysicalAddress(physical_addr,logicalAddress.toInt()))); 
             } 
             catch(...)
             {
                LOGWARN("Exception while sending ReportPhysicalAddress ");
             }
       }
       void HdmiCecSourceProcessor::process (const GiveDeviceVendorID &msg, const Header &header)
       {
             try
             {
                 LOGINFO("Command: GiveDeviceVendorID sending VendorID response :%s\n",(isLGTvConnected)?lgVendorId.toString().c_str():appVendorId.toString().c_str());
                 if(isLGTvConnected)
                     conn.sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(DeviceVendorID(lgVendorId)));
                 else 
                     conn.sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(DeviceVendorID(appVendorId)));
             }
             catch(...)
             {
                 LOGWARN("Exception while sending DeviceVendorID");
             }

       }
      
       void HdmiCecSourceProcessor::process (const SetOSDName &msg, const Header &header)
       {
             LOGINFO("Command: SetOSDName OSDName : %s\n",msg.osdName.toString().c_str());
             if (HdmiCecSourceImplementation::_instance) {
                 bool isOSDNameUpdated = HdmiCecSourceImplementation::_instance->deviceList[header.from.toInt()].update(msg.osdName);
                 if (isOSDNameUpdated)
                     HdmiCecSourceImplementation::_instance->sendDeviceUpdateInfo(header.from.toInt());
             } else {
                 LOGWARN("Exception HdmiCecSourceImplementation::_instance NULL");
             }
       }
       void HdmiCecSourceProcessor::process (const RoutingChange &msg, const Header &header)
       {
             LOGINFO("Command: RoutingChange From : %s To: %s \n",msg.from.toString().c_str(),msg.to.toString().c_str());
             if(msg.to.toString() == physical_addr.toString())
                 isDeviceActiveSource = true;
             else
                 isDeviceActiveSource = false;
             LOGINFO("physical_addr : %s isDeviceActiveSource :%d \n",physical_addr.toString().c_str(),isDeviceActiveSource);
             HdmiCecSourceImplementation::_instance->sendActiveSourceEvent();
       }
       void HdmiCecSourceProcessor::process (const RoutingInformation &msg, const Header &header)
       {
             LOGINFO("Command: RoutingInformation Routing Information to Sink : %s\n",msg.toSink.toString().c_str());
             if(msg.toSink.toString() == physical_addr.toString())
                 isDeviceActiveSource = true;
             else
                 isDeviceActiveSource = false;
             LOGINFO("physical_addr : %s isDeviceActiveSource :%d \n",physical_addr.toString().c_str(),isDeviceActiveSource);
             HdmiCecSourceImplementation::_instance->sendActiveSourceEvent();
       }
       void HdmiCecSourceProcessor::process (const SetStreamPath &msg, const Header &header)
       {
             LOGINFO("Command: SetStreamPath Set Stream Path to Sink : %s\n",msg.toSink.toString().c_str());
             if(msg.toSink.toString() == physical_addr.toString())
                 isDeviceActiveSource = true;
             else
                 isDeviceActiveSource = false;
             LOGINFO("physical_addr : %s isDeviceActiveSource :%d \n",physical_addr.toString().c_str(),isDeviceActiveSource);
             HdmiCecSourceImplementation::_instance->sendActiveSourceEvent();

       }
       
       void HdmiCecSourceProcessor::process (const ReportPhysicalAddress &msg, const Header &header)
       {
             LOGINFO("Command: ReportPhysicalAddress\n");
             HdmiCecSourceImplementation::_instance->addDevice(header.from.toInt());
       }
       void HdmiCecSourceProcessor::process (const DeviceVendorID &msg, const Header &header)
       {
             LOGINFO("Command: DeviceVendorID VendorID : %s\n",msg.vendorId.toString().c_str());
             if (HdmiCecSourceImplementation::_instance){
                 bool isVendorIdUpdated = HdmiCecSourceImplementation::_instance->deviceList[header.from.toInt()].update(msg.vendorId);
                 if (isVendorIdUpdated)
                     HdmiCecSourceImplementation::_instance->sendDeviceUpdateInfo(header.from.toInt());
             }
             else {
                 LOGWARN("Exception HdmiCecSourceImplementation::_instance NULL");
             }

       }
       void HdmiCecSourceProcessor::process (const GiveDevicePowerStatus &msg, const Header &header)
       {
             try
             { 
                 conn.sendTo(header.from, MessageEncoder().encode(ReportPowerStatus(PowerStatus(powerState))));
             } 
             catch(...)
             {
                 LOGWARN("Exception while sending ReportPowerStatus");
             }
       }
       void HdmiCecSourceProcessor::process (const ReportPowerStatus &msg, const Header &header)
       {
             if ((header.from == LogicalAddress(LogicalAddress::TV)))
                 tvPowerState = msg.status;
             HdmiCecSourceImplementation::_instance->addDevice(header.from.toInt());
       }
       void HdmiCecSourceProcessor::process (const UserControlPressed &msg, const Header &header)
       {
             LOGINFO("Command: UserControlPressed message received from:%s command : %d \n",header.from.toString().c_str(),msg.uiCommand.toInt());
             HdmiCecSourceImplementation::_instance->SendKeyPressMsgEvent(header.from.toInt(),msg.uiCommand.toInt());
       }
       void HdmiCecSourceProcessor::process (const UserControlReleased &msg, const Header &header)
       {
             LOGINFO("Command: UserControlReleased message received from:%s \n",header.from.toString().c_str());
             HdmiCecSourceImplementation::_instance->SendKeyReleaseMsgEvent(header.from.toInt());
       }
       void HdmiCecSourceProcessor::process (const FeatureAbort &msg, const Header &header)
       {
             LOGINFO("Command: FeatureAbort\n");
       }
       void HdmiCecSourceProcessor::process (const Abort &msg, const Header &header)
       {
             if (!(header.from == LogicalAddress(LogicalAddress::BROADCAST)))
             {
		 LOGINFO("Command: Abort, sending FeatureAbort");
		 try
		 { 
		     conn.sendTo(header.from, MessageEncoder().encode(FeatureAbort(OpCode(msg.opCode()),AbortReason(ABORT_REASON_ID))));
		 } 
		 catch(...)
		 {
		     LOGWARN("Exception while sending FeatureAbort command");
		 }

             }
             LOGINFO("Command: Abort\n");
       }
       void HdmiCecSourceProcessor::process (const Polling &msg, const Header &header)                                 {
             LOGINFO("Command: Polling\n");
       }


//=========================================== HdmiCecSourceImplementation =========================================

    HdmiCecSourceImplementation::HdmiCecSourceImplementation()
    : cecEnableStatus(false),smConnection(nullptr), m_sendKeyEventThreadRun(false)
    , _pwrMgrNotification(*this)
    , _registeredEventHandlers(false)
    {
        LOGWARN("ctor");
        HdmiCecSourceImplementation::_instance = this;
    }

    HdmiCecSourceImplementation::~HdmiCecSourceImplementation()
    {
         LOGWARN("dtor");

         if(cecEnableStatus)
         {
            setEnabledInternal(false, false);
         }

         HdmiCecSourceImplementation::_instance = nullptr;

           if(_powerManagerPlugin)
           {
               _powerManagerPlugin->Unregister(_pwrMgrNotification.baseInterface<Exchange::IPowerManager::IModeChangedNotification>());
               _powerManagerPlugin.Reset();
           }
           _registeredEventHandlers = false;
           device::Host::getInstance().UnRegister(baseInterface<device::Host::IDisplayDeviceEvents>());
    }

    Core::hresult HdmiCecSourceImplementation::Configure(PluginHost::IShell* service)
    {
        LOGINFO("Configure");
        ASSERT(service != nullptr);
        PowerState pwrStateCur = WPEFramework::Exchange::IPowerManager::POWER_STATE_UNKNOWN;
        PowerState pwrStatePrev = WPEFramework::Exchange::IPowerManager::POWER_STATE_UNKNOWN;
        Core::hresult res = Core::ERROR_GENERAL;
        string msg;
        if (Utils::IARM::init()) {
            //Initialize cecEnableStatus to false in ctor
            cecEnableStatus = false;

            logicalAddressDeviceType = "None";
            logicalAddress = 0xFF;

            //CEC plugin functionalities will only work if CECmgr is available. If plugin Initialize failure upper layer will call dtor directly.
            InitializePowerManager(service);

            // load persistence setting
            loadSettings();
            try
            {
                //TODO(MROLLINS) this is probably per process so we either need to be running in our own process or be carefull no other plugin is calling it
                device::Manager::Initialize();
                device::Host::getInstance().Register(baseInterface<device::Host::IDisplayDeviceEvents>(), "WPE::CecSource");

                std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
                if (vPort.isDisplayConnected())
                {
                    std::vector<uint8_t> edidVec;
                    vPort.getDisplay().getEDIDBytes(edidVec);
                    //Set LG vendor id if connected with LG TV
                    if(edidVec.at(8) == 0x1E && edidVec.at(9) == 0x6D)
                    {
                        isLGTvConnected = true;
                    }
                    LOGINFO("manufacturer byte from edid :%x: %x  isLGTvConnected :%d",edidVec.at(8),edidVec.at(9),isLGTvConnected);
                }
             }
             catch(...)
             {
                 LOGWARN("Exception in getting edid info .\r\n");
             }

             // get power state:
             ASSERT (_powerManagerPlugin);
             if (_powerManagerPlugin){
                 res = _powerManagerPlugin->GetPowerState(pwrStateCur, pwrStatePrev);
                 if (Core::ERROR_NONE == res)
                 {
                     powerState = (pwrStateCur == WPEFramework::Exchange::IPowerManager::POWER_STATE_ON)?0:1 ;
                     LOGINFO("Current state is PowerManagerPlugin: (%d) powerState :%d \n",pwrStateCur,powerState);
                 }
             }

             if (cecSettingEnabled)
             {
                try
                {
                    CECEnable();
                }
                catch(...)
                {
                    LOGWARN("Exception while enabling CEC settings .\r\n");
                }
             }
        } else {
            msg = "IARM bus is not available";
            LOGERR("IARM bus is not available. Failed to activate HdmiCecSource Plugin");
        }
        ASSERT(_powerManagerPlugin);
        registerEventHandlers();
        return Core::ERROR_NONE;
    }

    void HdmiCecSourceImplementation::registerEventHandlers()
    {
        ASSERT (_powerManagerPlugin);
    
        if(!_registeredEventHandlers && _powerManagerPlugin) {
            _registeredEventHandlers = true;
            _powerManagerPlugin->Register(_pwrMgrNotification.baseInterface<Exchange::IPowerManager::IModeChangedNotification>());
        }


    }

    Core::hresult HdmiCecSourceImplementation::Register(Exchange::IHdmiCecSource::INotification* notification)
    {

        LOGINFO("Register");
        if(notification != nullptr){
            _adminLock.Lock();
            if(std::find(_hdmiCecSourceNotifications.begin(), _hdmiCecSourceNotifications.end(), notification) == _hdmiCecSourceNotifications.end())
            {
                _hdmiCecSourceNotifications.push_back(notification);
                notification->AddRef();
            }
            else
            {
                LOGERR("Same notification is registered already");
            }
            _adminLock.Unlock();
        }

        return Core::ERROR_NONE;
    }


    Core::hresult HdmiCecSourceImplementation::Unregister(Exchange::IHdmiCecSource::INotification* notification)
    {
        LOGINFO("Unregister");
        if(notification != nullptr){
            _adminLock.Lock();
            std::list<Exchange::IHdmiCecSource::INotification*>::iterator index = std::find(_hdmiCecSourceNotifications.begin(), _hdmiCecSourceNotifications.end(), notification);
            if(index != _hdmiCecSourceNotifications.end())
            {
                (*index)->Release();
                _hdmiCecSourceNotifications.erase(index);
            }
            else
            {
                LOGERR("Notification is not registered");
            }
            _adminLock.Unlock();
        }

        return Core::ERROR_NONE;
    }

    void HdmiCecSourceImplementation::addDevice(const int logicalAddress) {

		if(!HdmiCecSourceImplementation::_instance)
			return;

		if ( logicalAddress >= LogicalAddress::UNREGISTERED){
			LOGERR("Logical Address NOT Allocated Or its not valid");
			return;
		}

		if ( !(BIT_CHECK(HdmiCecSourceImplementation::_instance->deviceList[logicalAddress].m_deviceInfoStatus, BIT_DEVICE_PRESENT)) )
		{
			BIT_SET(HdmiCecSourceImplementation::_instance->deviceList[logicalAddress].m_deviceInfoStatus, BIT_DEVICE_PRESENT);
			HdmiCecSourceImplementation::_instance->deviceList[logicalAddress].m_logicalAddress = LogicalAddress(logicalAddress);
			HdmiCecSourceImplementation::_instance->m_numberOfDevices++;
			LOGINFO("New cec logical address add notification send:  \r\n");
            std::list<Exchange::IHdmiCecSource::INotification*>::const_iterator index(_hdmiCecSourceNotifications.begin());
            while (index != _hdmiCecSourceNotifications.end()) {
                (*index)->OnDeviceAdded(logicalAddress);
                index++;
            }
		}
		//Two source devices can have same logical address.
		requestCecDevDetails(logicalAddress);
	}

    void HdmiCecSourceImplementation::removeDevice(const int logicalAddress) {
		if(!HdmiCecSourceImplementation::_instance)
			return;

		if ( logicalAddress >= LogicalAddress::UNREGISTERED ){
			LOGERR("Logical Address NOT Allocated Or its not valid");
			return;
		}

		if (BIT_CHECK(HdmiCecSourceImplementation::_instance->deviceList[logicalAddress].m_deviceInfoStatus, BIT_DEVICE_PRESENT))
		{
			_instance->m_numberOfDevices--;
			_instance->deviceList[logicalAddress].clear();
			LOGINFO("Cec ligical address remove notification send:  \r\n");
            std::list<Exchange::IHdmiCecSource::INotification*>::const_iterator index(_hdmiCecSourceNotifications.begin());
            while (index != _hdmiCecSourceNotifications.end()) {
                (*index)->OnDeviceRemoved(logicalAddress);
                index++;
            }

		}
	}


        Core::hresult HdmiCecSourceImplementation::GetActiveSourceStatus(bool &isActiveSource, bool &success)
       {
            isActiveSource = isDeviceActiveSource;
            success = true;
            return Core::ERROR_NONE;
       }

	    uint32_t HdmiCecSourceImplementation::sendKeyPressEvent(const int logicalAddress, int keyCode)
		{
			if(!(_instance->smConnection))
            {
                 return Core::ERROR_GENERAL;
            }

            
		    LOGINFO(" SendKeyPressEvent logicalAddress 0x%x keycode 0x%x\n",logicalAddress,keyCode);
            _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlPressed(static_cast<UICommand>(keyCode))),100);
			
            return Core::ERROR_NONE;
		}

        int HdmiCecSourceImplementation::getUIKeyCode(int keyCode)
        {
            switch (keyCode)
            {
                case VOLUME_UP:   return UICommand::UI_COMMAND_VOLUME_UP;
                case VOLUME_DOWN: return UICommand::UI_COMMAND_VOLUME_DOWN;
                case MUTE:        return UICommand::UI_COMMAND_MUTE;
                case UP:          return UICommand::UI_COMMAND_UP;
                case DOWN:        return UICommand::UI_COMMAND_DOWN;
                case LEFT:        return UICommand::UI_COMMAND_LEFT;
                case RIGHT:       return UICommand::UI_COMMAND_RIGHT;
                case SELECT:      return UICommand::UI_COMMAND_SELECT;
                case HOME:        return UICommand::UI_COMMAND_HOME;
                case BACK:        return UICommand::UI_COMMAND_BACK;
                case NUMBER_0:    return UICommand::UI_COMMAND_NUM_0;
                case NUMBER_1:    return UICommand::UI_COMMAND_NUM_1;
                case NUMBER_2:    return UICommand::UI_COMMAND_NUM_2;
                case NUMBER_3:    return UICommand::UI_COMMAND_NUM_3;
                case NUMBER_4:    return UICommand::UI_COMMAND_NUM_4;
                case NUMBER_5:    return UICommand::UI_COMMAND_NUM_5;
                case NUMBER_6:    return UICommand::UI_COMMAND_NUM_6;
                case NUMBER_7:    return UICommand::UI_COMMAND_NUM_7;
                case NUMBER_8:    return UICommand::UI_COMMAND_NUM_8;
                case NUMBER_9:    return UICommand::UI_COMMAND_NUM_9;
                default:
                    return KEY_UNSUPPORTED; // Unsupported key
            }
        }


        Core::hresult HdmiCecSourceImplementation::SendKeyPressEvent(const uint32_t &logicalAddress,const uint32_t &keyCode, HdmiCecSourceSuccess &success)
		{
            //Input params validation
            if(logicalAddress > LogicalAddress::UNREGISTERED)
            {
                LOGERR("Invalid Logical Address 0x%x",logicalAddress);
                success.success = false;
                return Core::ERROR_GENERAL;
            }

            if(getUIKeyCode(keyCode) == KEY_UNSUPPORTED)
            {
                LOGERR("Invalid Key Code 0x%x",keyCode);
                success.success = false;
                return Core::ERROR_NOT_SUPPORTED;
            }

			SendKeyInfo keyInfo;
			try {
               keyInfo.logicalAddr = logicalAddress;
               keyInfo.keyCode     = keyCode;
            } catch (const std::invalid_argument& e) {
               std::cerr << "Invalid input: " << e.what() << std::endl;
               success.success = false;
               return Core::ERROR_GENERAL;
            }
			std::unique_lock<std::mutex> lk(m_sendKeyEventMutex);
			m_SendKeyQueue.push(keyInfo);
            m_sendKeyEventThreadRun = true;
			m_sendKeyCV.notify_one();
			LOGINFO("Post send key press event to queue size:%d \n",(int)m_SendKeyQueue.size());
            success.success = true;
			return Core::ERROR_NONE;
		}

		void HdmiCecSourceImplementation::sendKeyReleaseEvent(const int logicalAddress)
		 {
	            LOGINFO(" sendKeyReleaseEvent logicalAddress 0x%x \n",logicalAddress);
                    if(!(_instance->smConnection))
                    {
                        return;
                    }
		        _instance->smConnection->sendTo(LogicalAddress(logicalAddress), MessageEncoder().encode(UserControlReleased()), 100);

         }

        Core::hresult HdmiCecSourceImplementation::SendStandbyMessage(HdmiCecSourceSuccess &success)
       {
            bool ret = false;

            if(true == cecEnableStatus)
            {
                if (smConnection){
                   try
                   {
                       smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(Standby()));
		               ret = true;
                   }
                   catch(...)
                   {
                       LOGWARN("Exception while sending CEC StandBy Message");
                   }
                }
                else {
                    LOGWARN("smConnection is NULL");
                }
            }
            else
                LOGWARN("cecEnableStatus=false");

            if(ret)
            {
                success.success = true;
                return Core::ERROR_NONE;
            }
            else{
                success.success = false;
                return Core::ERROR_GENERAL;
            }
       }

        void HdmiCecSourceImplementation::InitializePowerManager(PluginHost::IShell *service)
        {
            LOGINFO("Connect the COM-RPC socket\n");
            _powerManagerPlugin = PowerManagerInterfaceBuilder(_T("org.rdk.PowerManager"))
                                    .withIShell(service)
                                    .withRetryIntervalMS(200)
                                    .withRetryCount(25)
                                    .createInterface();
            registerEventHandlers();
        }

        void HdmiCecSourceImplementation::threadHotPlugEventHandler(int data)
        {
            LOGINFO("entry threadHotPlugEventHandler \r\n");
            if(!HdmiCecSourceImplementation::_instance)
                return;

            LOGINFO("Pocessing IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG  event status:%d \r\n",data);
            HdmiCecSourceImplementation::_instance->onHdmiHotPlug(data);
            //Trigger CEC device poll here
            pthread_mutex_lock(&(_instance->m_lock));
            pthread_cond_signal(&(_instance->m_condSig));
            pthread_mutex_unlock(&(_instance->m_lock));

            LOGINFO("Exit threadHotPlugEventHandler \r\n");
        }

       void HdmiCecSourceImplementation::OnDisplayHDMIHotPlug(dsDisplayEvent_t displayEvent)
       {
           LOGINFO("HdmiCecSourceImplementation::OnDisplayHDMIHotPlug : displayEvent = %d ", displayEvent);

           if(!HdmiCecSourceImplementation::_instance  || !_instance->cecEnableStatus)
           {
			   bool cecEnableStatus = _instance ? _instance->cecEnableStatus : false;
               LOGINFO("HdmiCecSourceImplementation::OnDisplayHDMIHotPlug failed _instance:%p cecEnableStatus:%d  \r\n", HdmiCecSourceImplementation::_instance, cecEnableStatus);
               return;
           }

           int hdmi_hotplug_event = (int) displayEvent;
           LOGINFO("Received IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG  event data:%d \r\n", hdmi_hotplug_event);
           std::thread worker(threadHotPlugEventHandler,hdmi_hotplug_event);
           worker.detach();

       }

       void HdmiCecSourceImplementation::onPowerModeChanged(const PowerState currentState, const PowerState newState)
       {
            if(!HdmiCecSourceImplementation::_instance)
                return;

            LOGINFO("Event IARM_BUS_PWRMGR_EVENT_MODECHANGED: State Changed %d -- > %d\r",
                    currentState, newState);
            if (WPEFramework::Exchange::IPowerManager::POWER_STATE_ON == newState)
            {
                powerState = 0;
                HdmiCecSourceImplementation::_instance->getLogicalAddress(); // get the updated LA after wakeup
            }
            else
                powerState = 1;
       }

       void HdmiCecSourceImplementation::onHdmiHotPlug(int connectStatus)
       {
            if (HDMI_HOT_PLUG_EVENT_CONNECTED == connectStatus)
            {
                LOGINFO ("onHdmiHotPlug Status : %d ", connectStatus);
                getPhysicalAddress();
                getLogicalAddress();
                try
                {
                   std::string strVideoPort = device::Host::getInstance().getDefaultVideoPortName();
                   device::VideoOutputPort vPort = device::Host::getInstance().getVideoOutputPort(strVideoPort.c_str());
                   if (vPort.isDisplayConnected())
                   {
                     std::vector<uint8_t> edidVec;
                     vPort.getDisplay().getEDIDBytes(edidVec);
                     //Set LG vendor id if connected with LG TV
                     if(edidVec.at(8) == 0x1E && edidVec.at(9) == 0x6D)
                     {
                         isLGTvConnected = true;
                     }
                     LOGINFO("manufacturer byte from edid :%x: %x  isLGTvConnected :%d",edidVec.at(8),edidVec.at(9),isLGTvConnected);
                   }
                 }
                 catch(...)
                 {
                    LOGWARN("Exception in getting edid info .\r\n");
                 }
                 if(smConnection)
                 {
                     try
                     {
                         LOGINFO(" sending ReportPhysicalAddress response physical_addr :%s logicalAddress :%x \n",physical_addr.toString().c_str(), logicalAddress.toInt());
                         smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(ReportPhysicalAddress(physical_addr,logicalAddress.toInt()))); 

                         LOGINFO("Command: GiveDeviceVendorID sending VendorID response :%s\n", \
                             (isLGTvConnected)?lgVendorId.toString().c_str():appVendorId.toString().c_str());
                         if(isLGTvConnected)
                             smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(DeviceVendorID(lgVendorId)));
                         else 
                             smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(DeviceVendorID(appVendorId)));
                     } 
                     catch(...)
                     {
                         LOGWARN("Exception while sending Messages onHdmiHotPlug\n");
                     }
                 }
            }
            return;
       }

        bool HdmiCecSourceImplementation::loadSettings()
        {
            Core::File file;
            file = CEC_SETTING_ENABLED_FILE;

            if( file.Open())
            {
                JsonObject parameters;
                parameters.IElement::FromFile(file);
                bool isConfigAdded = false;

                if( parameters.HasLabel(CEC_SETTING_ENABLED))
                {
                     getBoolParameter(CEC_SETTING_ENABLED, cecSettingEnabled);
                     LOGINFO("CEC_SETTING_ENABLED present value:%d",cecSettingEnabled);
                }
                else
                {
                    parameters[CEC_SETTING_ENABLED] = true;
                    cecSettingEnabled = true;
                    isConfigAdded = true;
                    LOGINFO("CEC_SETTING_ENABLED not present set dafult true:\n ");
                }

                if( parameters.HasLabel(CEC_SETTING_OTP_ENABLED))
                {
                    getBoolParameter(CEC_SETTING_OTP_ENABLED, cecOTPSettingEnabled);
                    LOGINFO("CEC_SETTING_OTP_ENABLED present value :%d",cecOTPSettingEnabled);
                }
                else
                {
                    parameters[CEC_SETTING_OTP_ENABLED] = true;
                    cecOTPSettingEnabled = true;
                    isConfigAdded = true;
                    LOGINFO("CEC_SETTING_OTP_ENABLED not present set dafult true:\n ");
                }
                if( parameters.HasLabel(CEC_SETTING_OSD_NAME))
                {
                    std::string osd_name;
                    getStringParameter(CEC_SETTING_OSD_NAME, osd_name);
                    osdName = osd_name.c_str();
                    LOGINFO("CEC_SETTING_OTP_ENABLED present osd_name :%s",osdName.toString().c_str());
                }
                else
                {
                    parameters[CEC_SETTING_OSD_NAME] = osdName.toString();
                    LOGINFO("CEC_SETTING_OSD_NMAE not  present set dafult value :%s\n ",osdName.toString().c_str());
                    isConfigAdded = true;
                }
                unsigned int  vendorId = (defaultVendorId.at(0) <<16) | ( defaultVendorId.at(1) << 8 ) | defaultVendorId.at(2);
                if( parameters.HasLabel(CEC_SETTING_VENDOR_ID))
                {
                    getNumberParameter(CEC_SETTING_VENDOR_ID, vendorId);
                    LOGINFO("CEC_SETTING_VENDOR_ID present :%x ",vendorId);
                }
                else
                {
                    LOGINFO("CEC_SETTING_VENDOR_ID not  present set dafult value :%x \n ",vendorId);
                    parameters[CEC_SETTING_VENDOR_ID] = vendorId;
                    isConfigAdded = true;
                }

                appVendorId = {(uint8_t)(vendorId >> 16 & 0xff),(uint8_t)(vendorId >> 8 & 0xff),(uint8_t) (vendorId & 0xff)}; 
                LOGINFO("appVendorId : %s  vendorId :%x \n",appVendorId.toString().c_str(), vendorId );

                if(isConfigAdded)
                {
                    LOGINFO("isConfigAdded true so update file:\n ");
                    file.Destroy();
                    file.Create();
                    parameters.IElement::ToFile(file);

                }

                file.Close();
            }
            else
            {
                LOGINFO("CEC_SETTING_ENABLED_FILE file not present create with default settings ");
                file.Open(false);
                if (!file.IsOpen())
                    file.Create();

                JsonObject parameters;
                unsigned int  vendorId = (defaultVendorId.at(0) <<16) | ( defaultVendorId.at(1) << 8 ) | defaultVendorId.at(2);
                parameters[CEC_SETTING_ENABLED] = true;
                parameters[CEC_SETTING_OTP_ENABLED] = true;
                parameters[CEC_SETTING_OSD_NAME] = osdName.toString();
                parameters[CEC_SETTING_VENDOR_ID] = vendorId;

                cecSettingEnabled = true;
                cecOTPSettingEnabled = true;
                parameters.IElement::ToFile(file);

                file.Close();

            }

            return cecSettingEnabled;
        }

        Core::hresult HdmiCecSourceImplementation::SetEnabled(const bool &enabled, HdmiCecSourceSuccess &success)
        {
           LOGINFO("Entered SetEnabled ");

           Core:: hresult ret = setEnabledInternal(enabled, true);

           if(ret == Core::ERROR_NONE)
           {
               success.success = true;
           }
           else
           {
               success.success = false;
           }
           return ret;
        }

        Core::hresult HdmiCecSourceImplementation::setEnabledInternal(const bool enabled, const bool isPersist)
        {
            LOGINFO("Entered setEnabledInternal enabled:%d isPersist:%d ",enabled,isPersist);
           if (cecSettingEnabled != enabled && isPersist)
           {
               Utils::persistJsonSettings (CEC_SETTING_ENABLED_FILE, CEC_SETTING_ENABLED, JsonValue(enabled));
               cecSettingEnabled = enabled;
           }
           if(true == enabled)
           {
               CECEnable();
           }
           else
           {
               CECDisable();
           }
           return Core::ERROR_NONE;

        }

        Core::hresult HdmiCecSourceImplementation::SetOTPEnabled(const bool &enabled, HdmiCecSourceSuccess &success)
        {
           if (cecOTPSettingEnabled != enabled)
           {
               LOGINFO("persist SetOTPEnabled ");
               Utils::persistJsonSettings (CEC_SETTING_ENABLED_FILE, CEC_SETTING_OTP_ENABLED, JsonValue(enabled));
               cecOTPSettingEnabled = enabled;
           }
           success.success = true;
           return Core::ERROR_NONE;
        }

        void HdmiCecSourceImplementation::CECEnable(void)
        {
            LOGINFO("Entered CECEnable");

            if (cecEnableStatus)
            {
                LOGWARN("CEC Already Enabled");
                return;
            }

            if(0 == libcecInitStatus)
            {
                try
                {
                    LibCCEC::getInstance().init("HdmiCecSource");
                }
                catch (const std::exception& e)
                {
                    LOGWARN("CEC exception caught from LibCCEC::getInstance().init()");
                }
            }
            libcecInitStatus++;

            m_sendKeyEventThreadExit = false;
            try {
               if (m_sendKeyEventThread.get().joinable()) {
                   m_sendKeyEventThread.get().join();
	       }
               m_sendKeyEventThread = Utils::ThreadRAII(std::thread(threadSendKeyEvent));
            } catch(const std::system_error& e) {
                LOGERR("exception in creating threadSendKeyEvent %s", e.what());
	    }


            //Acquire CEC Addresses
            getPhysicalAddress();
            getLogicalAddress();

            smConnection = new Connection(logicalAddress.toInt(),false,"ServiceManager::Connection::");
            smConnection->open();
            msgProcessor = new HdmiCecSourceProcessor(*smConnection);
            msgFrameListener = new HdmiCecSourceFrameListener(*msgProcessor);
            smConnection->addFrameListener(msgFrameListener);

            cecEnableStatus = true;

            if(smConnection)
            {
                LOGINFO("Command: sending GiveDevicePowerStatus \r\n");
                smConnection->sendTo(LogicalAddress::TV, MessageEncoder().encode(GiveDevicePowerStatus()));
                LOGINFO("Command: sending request active Source isDeviceActiveSource is set to false\r\n");
                smConnection->sendTo(LogicalAddress::BROADCAST, MessageEncoder().encode(RequestActiveSource()));
                isDeviceActiveSource = false;
                LOGINFO("Command: GiveDeviceVendorID sending VendorID response :%s\n", \
                                                 (isLGTvConnected)?lgVendorId.toString().c_str():appVendorId.toString().c_str());
                if(isLGTvConnected)
                    smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(DeviceVendorID(lgVendorId)));
                else 
                    smConnection->sendTo(LogicalAddress(LogicalAddress::BROADCAST), MessageEncoder().encode(DeviceVendorID(appVendorId)));

                LOGWARN("Start Update thread %p", smConnection );
                m_updateThreadExit = false;
                _instance->m_lockUpdate = PTHREAD_MUTEX_INITIALIZER;
                _instance->m_condSigUpdate = PTHREAD_COND_INITIALIZER;
                try {
                    if (m_UpdateThread.get().joinable()) {
                       m_UpdateThread.get().join();
	            }
                    m_UpdateThread = Utils::ThreadRAII(std::thread(threadUpdateCheck));
                } catch(const std::system_error& e) {
                    LOGERR("exception in creating threadUpdateCheck %s", e.what());
	        }

                LOGWARN("Start Thread %p", smConnection );
                m_pollThreadExit = false;
                _instance->m_numberOfDevices = 0;
                _instance->m_lock = PTHREAD_MUTEX_INITIALIZER;
                _instance->m_condSig = PTHREAD_COND_INITIALIZER;
                try {
                    if (m_pollThread.get().joinable()) {
                       m_pollThread.get().join();
	            }
                    m_pollThread = Utils::ThreadRAII(std::thread(threadRun));
                } catch(const std::system_error& e) {
                    LOGERR("exception in creating threadRun %s", e.what());
	        }

            }
            return;
        }

        void HdmiCecSourceImplementation::CECDisable(void)
        {
            LOGINFO("Entered CECDisable ");

            if(!cecEnableStatus)
            {
                LOGWARN("CEC Already Disabled ");
                return;
            }

            {
                m_sendKeyEventThreadExit = true;
                std::unique_lock<std::mutex> lk(m_sendKeyEventMutex);
                m_sendKeyEventThreadRun = true;
                m_sendKeyCV.notify_one();
            }
            try
	        {
                if (m_sendKeyEventThread.get().joinable())
                    m_sendKeyEventThread.get().join();
	        }
	        catch(const std::system_error& e)
	        {
		        LOGERR("system_error exception in thread join %s", e.what());
	        }
	        catch(const std::exception& e)
	        {
		        LOGERR("exception in thread join %s", e.what());
	        }

            if (smConnection != NULL)
            {
                LOGWARN("Stop Thread %p", smConnection );

                m_updateThreadExit = true;
                //Trigger codition to exit poll loop
                pthread_mutex_lock(&(_instance->m_lockUpdate)); //Join mutex lock to wait until thread is in its wait condition
                pthread_cond_signal(&(_instance->m_condSigUpdate));
                pthread_mutex_unlock(&(_instance->m_lockUpdate));
                if (m_UpdateThread.get().joinable()) {//Join thread to make sure it's deleted before moving on.
                    m_UpdateThread.get().join();
                }
                LOGWARN("Deleted update Thread %p", smConnection );

                m_pollThreadExit = true;
                //Trigger codition to exit poll loop
                pthread_mutex_lock(&(_instance->m_lock)); //Join mutex lock to wait until thread is in its wait condition
                pthread_cond_signal(&(_instance->m_condSig));
                pthread_mutex_unlock(&(_instance->m_lock));
                if (m_pollThread.get().joinable()) {//Join thread to make sure it's deleted before moving on.
                    m_pollThread.get().join();
                }
                LOGWARN("Deleted Thread %p", smConnection );
                //Clear cec device cache.
                removeAllCecDevices();

                smConnection->close();
                delete smConnection;
                delete msgProcessor;
                delete msgFrameListener;
                msgProcessor = NULL;
                msgFrameListener = NULL;
                smConnection = NULL;
            }
            cecEnableStatus = false;

            if(1 == libcecInitStatus)
            {
                try
                {
                   LibCCEC::getInstance().term();
                }
                catch (const std::exception& e)
                {
                    LOGWARN("CEC exception caught from LibCCEC::getInstance().term() ");
                }
            }

            libcecInitStatus--;

            return;
        }


        void HdmiCecSourceImplementation::getPhysicalAddress()
        {
            LOGINFO("Entered getPhysicalAddress ");

            uint32_t physAddress = 0x0F0F0F0F;
            try {
                    LibCCEC::getInstance().getPhysicalAddress(&physAddress);
                    physical_addr = {(uint8_t)((physAddress >> 24) & 0xFF),(uint8_t)((physAddress >> 16) & 0xFF),(uint8_t) ((physAddress >> 8)  & 0xFF),(uint8_t)((physAddress) & 0xFF)};
                    LOGINFO("getPhysicalAddress: physicalAddress: %s ", physical_addr.toString().c_str());
            }
            catch (const std::exception& e)
            {
                LOGWARN("exception caught from getPhysicalAddress");
            }
            return;
        }

        void HdmiCecSourceImplementation::getLogicalAddress()
        {
            LOGINFO("Entered getLogicalAddress ");

            try{
                LogicalAddress addr = LibCCEC::getInstance().getLogicalAddress(DEV_TYPE_TUNER);

                std::string logicalAddrDeviceType = DeviceType(LogicalAddress(addr).getType()).toString().c_str();

                LOGINFO("logical address obtained is %d , saved logical address is %d ", addr.toInt(), logicalAddress.toInt());

                if (logicalAddress.toInt() != addr.toInt() || logicalAddressDeviceType != logicalAddrDeviceType)
                {
                    logicalAddress = addr;
                    logicalAddressDeviceType = logicalAddrDeviceType;
                    if(smConnection)
                        smConnection->setSource(logicalAddress); //update initiator LA
                }
            }
            catch (const std::exception& e)
            {
                LOGWARN("CEC exception caught from getLogicalAddress ");
            }
            return;
        }

        Core::hresult HdmiCecSourceImplementation::GetEnabled(bool &enabled, bool &success)
        {
            LOGINFO("GetEnabled :%d ",cecEnableStatus);
            enabled = cecEnableStatus;
            success = true;
            return Core::ERROR_NONE;
        }

        Core::hresult HdmiCecSourceImplementation::GetOTPEnabled(bool &enabled, bool &success)
        {
            enabled = cecOTPSettingEnabled;
            LOGINFO("GetOTPEnabled :%d ",cecOTPSettingEnabled);
            success = true;
            return Core::ERROR_NONE;
        }

        Core::hresult HdmiCecSourceImplementation::GetOSDName(std::string &name, bool &success)
        {
            name = osdName.toString();
            LOGINFO("GetOSDName :%s ",name.c_str());
            success = true;
            return Core::ERROR_NONE;
        }

        Core::hresult HdmiCecSourceImplementation::SetOSDName(const std::string &name, HdmiCecSourceSuccess &success)
        {
            LOGINFO("SetOSDName :%s ",name.c_str());
            osdName = name.c_str();
            Utils::persistJsonSettings (CEC_SETTING_ENABLED_FILE, CEC_SETTING_OSD_NAME, JsonValue(name.c_str()));
            success.success = true;
            return Core::ERROR_NONE;
        }

        Core::hresult HdmiCecSourceImplementation::GetVendorId(std::string &vendorid, bool &success)
        {
            vendorid = appVendorId.toString();
            LOGINFO("GetVendorId :%s ",vendorid.c_str());
            success = true;
            return Core::ERROR_NONE;
        }

        Core::hresult HdmiCecSourceImplementation::SetVendorId(const string &vendorid, HdmiCecSourceSuccess &success)
        {
            LOGINFO("SetVendorId :%s ",vendorid.c_str());
            if (vendorid.empty()) {
                LOGERR("SetVendorId failed: vendorid is not given");
                success.success = false;
                return Core::ERROR_GENERAL;
            }
            unsigned int vendorIdInt = 0;
            try
            {
                vendorIdInt = stoi(vendorid,NULL,16);
            }
            catch (const std::invalid_argument& e)
            {
                LOGWARN("Invalid vendorId format: %s, using default\n", e.what());
                vendorIdInt = 0x0019FB;
            }
            catch (const std::out_of_range& e)
            {
                LOGWARN("VendorId out of range: %s, using default\n", e.what());
                vendorIdInt = 0x0019FB;
            }
            catch (...)
            {
                LOGWARN("Exception in setVendorIdWrapper set default value\n");
                vendorIdInt = 0x0019FB;
            }
            appVendorId = {(uint8_t)(vendorIdInt >> 16 & 0xff),(uint8_t)(vendorIdInt >> 8 & 0xff),(uint8_t) (vendorIdInt & 0xff)};
            Utils::persistJsonSettings (CEC_SETTING_ENABLED_FILE, CEC_SETTING_VENDOR_ID, JsonValue(vendorIdInt));
            LOGINFO("SetVendorId :%s ",appVendorId.toString().c_str());
            success.success = true;
            return Core::ERROR_NONE;
        }

        Core::hresult HdmiCecSourceImplementation::PerformOTPAction(HdmiCecSourceSuccess &success)
        {
            LOGINFO("PerformOTPAction ");
            bool ret = false; 

            if((true == cecEnableStatus) && (cecOTPSettingEnabled == true))
            {
                if (smConnection)  {
                    try
                    {
                        LOGINFO("Command: sending ImageViewOn TV \r\n");
                        smConnection->sendTo(LogicalAddress::TV, MessageEncoder().encode(ImageViewOn()));
                        usleep(10000);
                        LOGINFO("Command: sending ActiveSource  physical_addr :%s \r\n",physical_addr.toString().c_str());
                        smConnection->sendTo(LogicalAddress::BROADCAST, MessageEncoder().encode(ActiveSource(physical_addr)));
                        usleep(10000);
                        isDeviceActiveSource = true;
                        LOGINFO("Command: sending GiveDevicePowerStatus \r\n");
                        smConnection->sendTo(LogicalAddress::TV, MessageEncoder().encode(GiveDevicePowerStatus()));
                        ret = true;
                    }
                    catch(...)
                    {
                        LOGWARN("Exception while processing PerformOTPAction");
                    }
                }
                else {
                    LOGWARN("smConnection is NULL");
                }
            }
            else
                LOGWARN("cecEnableStatus=false");

            if (ret){
                success.success = true;
                return Core::ERROR_NONE;
            } else {
                success.success = false;
                return Core::ERROR_GENERAL;
            }
        }

        Core::hresult HdmiCecSourceImplementation::GetDeviceList(uint32_t &numberofdevices, IHdmiCecSourceDeviceListIterator*& deviceList, bool &success)
        {   //sample servicemanager response:
            std::vector<Exchange::IHdmiCecSource::HdmiCecSourceDevices> localDevices;
            Exchange::IHdmiCecSource::HdmiCecSourceDevices actual_hdmicecdevices = {0};

		    //Trigger CEC device poll here
		    pthread_mutex_lock(&(_instance->m_lock));
		    pthread_cond_signal(&(_instance->m_condSig));
		    pthread_mutex_unlock(&(_instance->m_lock));

		    success = true;
		    LOGINFO("getDeviceListWrapper  m_numberOfDevices :%d \n", HdmiCecSourceImplementation::_instance->m_numberOfDevices);
            numberofdevices = HdmiCecSourceImplementation::_instance->m_numberOfDevices;
		    try
		    {
		    	int i = 0;
		    	for(i=0; i< LogicalAddress::UNREGISTERED; i++ ) {
		    		if (BIT_CHECK(HdmiCecSourceImplementation::_instance->deviceList[i].m_deviceInfoStatus, BIT_DEVICE_PRESENT)) {
                        actual_hdmicecdevices.logicalAddress = HdmiCecSourceImplementation::_instance->deviceList[i].m_logicalAddress.toInt();
                        actual_hdmicecdevices.osdName = HdmiCecSourceImplementation::_instance->deviceList[i].m_osdName.toString();
                        actual_hdmicecdevices.vendorID = HdmiCecSourceImplementation::_instance->deviceList[i].m_vendorID.toString();
                        localDevices.push_back(actual_hdmicecdevices);
                    }
		    	}
		    }
		    catch (...)
		    {
		    	LOGERR("Exception in api");
		    	success = false;
		    }
            deviceList = (Core::Service<RPC::IteratorType<Exchange::IHdmiCecSource::IHdmiCecSourceDeviceListIterator>>::Create<Exchange::IHdmiCecSource::IHdmiCecSourceDeviceListIterator>(localDevices));
            return Core::ERROR_NONE;
	    }

	bool HdmiCecSourceImplementation::pingDeviceUpdateList (int idev)
	{
		bool isConnected = false;
		//self ping is not required
		if (idev == logicalAddress.toInt()){
		        return isConnected;
		}
		if(!HdmiCecSourceImplementation::_instance)
		{
			LOGERR("HdmiCecSourceImplementation::_instance not existing");
			return isConnected;
		}
		if ( !(_instance->smConnection) || logicalAddress.toInt() == LogicalAddress::UNREGISTERED || (false==cecEnableStatus)){
			LOGERR("Exiting from pingDeviceUpdateList _instance->smConnection:%p, logicalAddress:%d, cecEnableStatus=%d",
					_instance->smConnection, logicalAddress.toInt(), cecEnableStatus);
			return isConnected;
		}

		LOGWARN("PING for  0x%x \r\n",idev);
		try {
			_instance->smConnection->ping(logicalAddress, LogicalAddress(idev), Throw_e());
		}
		catch(CECNoAckException &e)
		{
			if (BIT_CHECK(_instance->deviceList[idev].m_deviceInfoStatus, BIT_DEVICE_PRESENT)) {
				LOGINFO("Device disconnected: %d \r\n",idev);
				removeDevice (idev);
			} else {
				LOGINFO("Device is not connected: %d. Ping caught %s\r\n",idev, e.what());
			}
			isConnected = false;
			return isConnected;;
		}
		catch(IOException &e)
		{
			LOGINFO("Device is not reachable: %d. Ping caught %s\r\n",idev, e.what());
			removeDevice (idev);
			isConnected = false;
			return isConnected;;
		}
		catch(Exception &e)
		{
			LOGINFO("Ping caught %s \r\n",e.what());
		}

		/* If we get ACK, then the device is present in the network*/
		isConnected = true;
		if ( !(BIT_CHECK(_instance->deviceList[idev].m_deviceInfoStatus, BIT_DEVICE_PRESENT)) )
		{
			LOGINFO("Device connected: %d \r\n",idev);
			addDevice (idev);
		}
		return isConnected;
	}

	void HdmiCecSourceImplementation::removeAllCecDevices() {
		int i = 0;
		for(i=0; i< LogicalAddress::UNREGISTERED; i++ ) {
			removeDevice (i);
		}
	}

	void HdmiCecSourceImplementation::sendUnencryptMsg(unsigned char* msg, int size)
	{
		LOGINFO("sendMessage ");

		if(true == cecEnableStatus)
		{
			std::vector <unsigned char> buf;
			buf.resize(size);

			int itr = 0;
			for (itr= 0; itr<size; itr++)
				buf [itr] = msg [itr];

			CECFrame frame = CECFrame((const uint8_t *)buf.data(), size);
			//      SVCLOG_WARN("Frame to be sent from servicemanager in %s \n",__FUNCTION__);
			//      frame.hexDump();
			smConnection->sendAsync(frame);
		}
		else
			LOGWARN("cecEnableStatus=false");
		return;
	}

	void HdmiCecSourceImplementation::requestVendorID(const int newDevlogicalAddress)
	{
		//Get OSD name and vendor ID only from connected devices. Since devices are identified using polling
		//Once OSD name and Vendor ID is updated. We have to poll again in next iteration also. Just to check
		//a new device is reconnected with same logical address
		unsigned char msg [2];
		unsigned int logicalAddr = logicalAddress.toInt();
		unsigned char sender = (unsigned char)(logicalAddr & 0x0f);
		unsigned char receiver = (unsigned char) (newDevlogicalAddress & 0x0f);

		msg [0] = (sender<<4)|receiver;
		//Request vendor id
		msg [1] = 0x8c;
		LOGINFO("Sending msg request vendor id %x %x", msg [0], msg [1]);
		_instance->sendUnencryptMsg (msg, sizeof(msg));

	}

	void HdmiCecSourceImplementation::requestOsdName(const int newDevlogicalAddress)
	{
		//Get OSD name and vendor ID only from connected devices. Since devices are identified using polling
		//Once OSD name and Vendor ID is updated. We have to poll again in next iteration also. Just to check
		//a new device is reconnected with same logical address
		unsigned char msg [2];
		unsigned int logicalAddr = logicalAddress.toInt();
		unsigned char sender = (unsigned char)(logicalAddr & 0x0f);
		unsigned char receiver = (unsigned char) (newDevlogicalAddress & 0x0f);

		msg [0] = (sender<<4)|receiver;
		//Request OSD  name
		msg [1] = 0x46;
		LOGINFO("Sending msg request osd name %x %x", msg [0], msg [1]);
		_instance->sendUnencryptMsg (msg, sizeof(msg));

	}

	void HdmiCecSourceImplementation::requestCecDevDetails(const int newDevlogicalAddress)
	{
		//Get OSD name and vendor ID only from connected devices. Since devices are identified using polling
		//Once OSD name and Vendor ID is updated. We have to poll again in next iteration also. Just to check
		//a new device is reconnected with same logical address
		requestVendorID (newDevlogicalAddress);
		requestOsdName (newDevlogicalAddress);
	}

	void HdmiCecSourceImplementation::threadRun()
	{
		if(!HdmiCecSourceImplementation::_instance)
			return;
		if(!(_instance->smConnection))
			return;
		LOGINFO("Entering ThreadRun: _instance->m_pollThreadExit %d",_instance->m_pollThreadExit);
		int i = 0;
		pthread_mutex_lock(&(_instance->m_lock));//pthread_cond_wait should be mutex protected. //pthread_cond_wait will unlock the mutex and perfoms wait for the condition.
		while (!_instance->m_pollThreadExit) {
			bool isActivateUpdateThread = false;
			LOGINFO("Starting cec device polling");
			for(i=0; i< LogicalAddress::UNREGISTERED; i++ ) {
				bool isConnected = _instance->pingDeviceUpdateList(i);
				if (isConnected){
					isActivateUpdateThread = isConnected;
				}

			}
			if (isActivateUpdateThread){
				//i any of devices is connected activate thread update check
				pthread_cond_signal(&(_instance->m_condSigUpdate));
			}
			//Wait for mutex signal here to continue the worker thread again.
			pthread_cond_wait(&(_instance->m_condSig), &(_instance->m_lock));

		}
		pthread_mutex_unlock(&(_instance->m_lock));
	        LOGINFO("%s: Thread exited", __FUNCTION__);
	}
	void HdmiCecSourceImplementation::threadSendKeyEvent()
        {
            if(!HdmiCecSourceImplementation::_instance)
                return;

	    SendKeyInfo keyInfo = {-1,-1};

            while(!_instance->m_sendKeyEventThreadExit)
            {
                keyInfo.logicalAddr = -1;
                keyInfo.keyCode = -1;
                {
                    // Wait for a message to be added to the queue
                    std::unique_lock<std::mutex> lk(_instance->m_sendKeyEventMutex);
                    _instance->m_sendKeyCV.wait(lk, []{return (_instance->m_sendKeyEventThreadRun == true);});
                }

                if (_instance->m_sendKeyEventThreadExit == true)
                {
                    LOGINFO(" threadSendKeyEvent Exiting");
                    _instance->m_sendKeyEventThreadRun = false;
                    break;
                }

                if (_instance->m_SendKeyQueue.empty()) {
                    _instance->m_sendKeyEventThreadRun = false;
                    continue;
                }

                keyInfo = _instance->m_SendKeyQueue.front();
                _instance->m_SendKeyQueue.pop();
                
                LOGINFO("sendRemoteKeyThread : logical addr:0x%x keyCode: 0x%x  queue size :%d \n",keyInfo.logicalAddr,keyInfo.keyCode,(int)_instance->m_SendKeyQueue.size());
    	        _instance->sendKeyPressEvent(keyInfo.logicalAddr,_instance->getUIKeyCode(keyInfo.keyCode));
	            _instance->sendKeyReleaseEvent(keyInfo.logicalAddr);

            }
	    LOGINFO("%s: Thread exited", __FUNCTION__);
        }
	void HdmiCecSourceImplementation::threadUpdateCheck()
	{
		if(!HdmiCecSourceImplementation::_instance)
			return;
		if(!(_instance->smConnection))
			return;
		LOGINFO("Entering ThreadUpdate: _instance->m_updateThreadExit %d",_instance->m_updateThreadExit);
		int i = 0;
		pthread_mutex_lock(&(_instance->m_lockUpdate));//pthread_cond_wait should be mutex protected. //pthread_cond_wait will unlock the mutex and perfoms wait for the condition.
		while (!_instance->m_updateThreadExit) {
			//Wait for mutex signal here to continue the worker thread again.
			pthread_cond_wait(&(_instance->m_condSigUpdate), &(_instance->m_lockUpdate));

			LOGINFO("Starting cec device update check");
			for(i=0; ((i< LogicalAddress::UNREGISTERED)&&(!_instance->m_updateThreadExit)); i++ ) {
				//If details are not updated. update now.
				if (BIT_CHECK(HdmiCecSourceImplementation::_instance->deviceList[i].m_deviceInfoStatus, BIT_DEVICE_PRESENT))
				{
					int itr = 0;
					bool retry = true;
					int iCounter = 0;
					for (itr = 0; ((itr<5)&&(retry)); itr++){

						if (!HdmiCecSourceImplementation::_instance->deviceList[i].m_isOSDNameUpdated){
							iCounter = 0;
							while ((!_instance->m_updateThreadExit) && (iCounter < (2*10))) { //sleep for 2sec.
								usleep (100 * 1000); //sleep for 100 milli sec
								iCounter ++;
							}

							HdmiCecSourceImplementation::_instance->requestOsdName (i);
							retry = true;
						}
						else {
							retry = false;
						}

						if (!HdmiCecSourceImplementation::_instance->deviceList[i].m_isVendorIDUpdated){
							iCounter = 0;
							while ((!_instance->m_updateThreadExit) && (iCounter < (2*10))) { //sleep for 1sec.
								usleep (100 * 1000); //sleep for 100 milli sec
								iCounter ++;
							}

							HdmiCecSourceImplementation::_instance->requestVendorID (i);
							retry = true;
						}
					}
					if (retry){
						LOGINFO("cec device: %d update time out", i);
					}
				}
			}

		}
		pthread_mutex_unlock(&(_instance->m_lockUpdate));
	        LOGINFO("%s: Thread exited", __FUNCTION__);
	}


    void  HdmiCecSourceImplementation::sendDeviceUpdateInfo(const int logicalAddress)
	{
		LOGINFO("Device info updated notification send: for logical address:%d\r\n", logicalAddress);
        std::list<Exchange::IHdmiCecSource::INotification*>::const_iterator index(_hdmiCecSourceNotifications.begin());
        while (index != _hdmiCecSourceNotifications.end()) {
            (*index)->OnDeviceInfoUpdated(logicalAddress);
            index++;
        }
	}

    void HdmiCecSourceImplementation::sendActiveSourceEvent()
       {
           LOGWARN(" sendActiveSourceEvent isDeviceActiveSource: %d ",isDeviceActiveSource);
           std::list<Exchange::IHdmiCecSource::INotification*>::const_iterator index(_hdmiCecSourceNotifications.begin());
           while (index != _hdmiCecSourceNotifications.end()) {
             (*index)->OnActiveSourceStatusUpdated(isDeviceActiveSource);
             index++;
           }
       }

    void HdmiCecSourceImplementation::SendStandbyMsgEvent(const int logicalAddress)
       {
           std::list<Exchange::IHdmiCecSource::INotification*>::const_iterator index(_hdmiCecSourceNotifications.begin());
           while (index != _hdmiCecSourceNotifications.end()) {
              (*index)->StandbyMessageReceived(logicalAddress);
              index++;
           }
       }

    void HdmiCecSourceImplementation::SendKeyReleaseMsgEvent(const int logicalAddress)
       {
           std::list<Exchange::IHdmiCecSource::INotification*>::const_iterator index(_hdmiCecSourceNotifications.begin());
           while (index != _hdmiCecSourceNotifications.end()) {
               (*index)->OnKeyReleaseEvent(logicalAddress);
               index++;
           }
       }

    void HdmiCecSourceImplementation::SendKeyPressMsgEvent(const int logicalAddress,const int keyCode)
       {
           std::list<Exchange::IHdmiCecSource::INotification*>::const_iterator index(_hdmiCecSourceNotifications.begin());
              while (index != _hdmiCecSourceNotifications.end()) {
                (*index)->OnKeyPressEvent(logicalAddress,keyCode);
                index++;
              }
       }

    } // namespace Plugin
} // namespace WPEFramework
