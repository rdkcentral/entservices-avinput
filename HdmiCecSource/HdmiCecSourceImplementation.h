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

#pragma once

#include <stdint.h>
#include <mutex>
#include <condition_variable>

#include "ccec/FrameListener.hpp"
#include "ccec/Connection.hpp"
#include "libIARM.h"
#include "ccec/Assert.hpp"
#include "ccec/Messages.hpp"
#include "ccec/MessageDecoder.hpp"
#include "ccec/MessageProcessor.hpp"
#include <thread>

#undef Assert // this define from Connection.hpp conflicts with WPEFramework

#include "Module.h"

#include "UtilsBIT.h"
#include "UtilsThreadRAII.h"

#include <interfaces/IPowerManager.h>
#include "PowerManagerInterface.h"
#include <interfaces/IHdmiCecSource.h>
#include "host.hpp"


using namespace WPEFramework;
using PowerState = WPEFramework::Exchange::IPowerManager::PowerState;
using ThermalTemperature = WPEFramework::Exchange::IPowerManager::ThermalTemperature;

namespace WPEFramework {

    namespace Plugin {
        class HdmiCecSourceFrameListener : public FrameListener
        {
        public:
            HdmiCecSourceFrameListener(MessageProcessor &processor) : processor(processor) {}
            void notify(const CECFrame &in) const;
            ~HdmiCecSourceFrameListener() {}
        private:
            MessageProcessor &processor;
        };
        
        class HdmiCecSourceProcessor : public MessageProcessor
        {
        public:
            HdmiCecSourceProcessor(Connection &conn) : conn(conn) {}
            void process (const ActiveSource &msg, const Header &header);
	        void process (const RequestActiveSource &msg, const Header &header);
	        void process (const Standby &msg, const Header &header);
	        void process (const GetCECVersion &msg, const Header &header);
	        void process (const CECVersion &msg, const Header &header);
	        void process (const GiveOSDName &msg, const Header &header);
	        void process (const GivePhysicalAddress &msg, const Header &header);
	        void process (const GiveDeviceVendorID &msg, const Header &header);
	        void process (const SetOSDName &msg, const Header &header);
	        void process (const RoutingChange &msg, const Header &header);
	        void process (const RoutingInformation &msg, const Header &header);
	        void process (const SetStreamPath &msg, const Header &header);
	        void process (const ReportPhysicalAddress &msg, const Header &header);
            void process (const DeviceVendorID &msg, const Header &header);
	        void process (const GiveDevicePowerStatus &msg, const Header &header);
	        void process (const ReportPowerStatus &msg, const Header &header);
            void process (const UserControlPressed &msg, const Header &header);
	        void process (const UserControlReleased &msg, const Header &header);
	        void process (const FeatureAbort &msg, const Header &header);
	        void process (const Abort &msg, const Header &header);
	        void process (const Polling &msg, const Header &header);
        private:
            Connection conn;
        };

#define BIT_DEVICE_PRESENT    (0)

	class CECDeviceInfo_2 {
		public:

		LogicalAddress m_logicalAddress;
		VendorID m_vendorID;
		OSDName m_osdName;
		//<Bits 16 - 1: unused><Bit 0: DevicePresent>
		short m_deviceInfoStatus;
	    bool m_isOSDNameUpdated;
	    bool m_isVendorIDUpdated;
        std::mutex m_;
        std::condition_variable cv_;
		std::unique_lock<std::mutex> lk;

		CECDeviceInfo_2()
		: m_logicalAddress(0),m_vendorID(0,0,0),m_osdName("NA"), m_deviceInfoStatus(0), m_isOSDNameUpdated (false), m_isVendorIDUpdated (false)
		{
			BITMASK_CLEAR(m_deviceInfoStatus, 0xFFFF); //Clear all bits
		}

		void clear( )
		{
			m_logicalAddress = 0;
			m_vendorID = VendorID(0,0,0);
			m_osdName = "NA";
			BITMASK_CLEAR(m_deviceInfoStatus, 0xFFFF); //Clear all bits
			m_isOSDNameUpdated = false;
			m_isVendorIDUpdated = false;
		}

		bool update ( const VendorID &vendorId) {
			bool isVendorIdUpdated = false;
			if (!m_isVendorIDUpdated)
				isVendorIdUpdated = true; //First time no need to cross check the value. Since actual value can be default value
			else
				isVendorIdUpdated = (m_vendorID.toString().compare(vendorId.toString())==0)?false:true;

			m_isVendorIDUpdated = true;
			m_vendorID = vendorId;
			return isVendorIdUpdated;
		}

		bool update ( const OSDName    &osdName ) {
			bool isOSDNameUpdated = false;
			if (!m_isOSDNameUpdated)
				isOSDNameUpdated = true; //First time no need to cross check the value. Since actual value can be default value
			else
				isOSDNameUpdated = (m_osdName.toString().compare(osdName.toString())==0)?false:true;

			m_isOSDNameUpdated = true;
			m_osdName = osdName;
			return isOSDNameUpdated;
		}

	};
		// This is a server for a JSONRPC communication channel. 
		// For a plugin to be capable to handle JSONRPC, inherit from PluginHost::JSONRPC.
		// By inheriting from this class, the plugin realizes the interface PluginHost::IDispatcher.
		// This realization of this interface implements, by default, the following methods on this plugin
		// - exists
		// - register
		// - unregister
		// Any other methood to be handled by this plugin  can be added can be added by using the
		// templated methods Register on the PluginHost::JSONRPC class.
		// As the registration/unregistration of notifications is realized by the class PluginHost::JSONRPC,
		// this class exposes a public method called, Notify(), using this methods, all subscribed clients
		// will receive a JSONRPC message as a notification, in case this method is called.
        class HdmiCecSourceImplementation : public Exchange::IHdmiCecSource, public device::Host::IDisplayDeviceEvents {
		enum {
				VOLUME_UP     = 0x41,
				VOLUME_DOWN   = 0x42,
				MUTE          = 0x43,
				UP            = 0x01,
				DOWN	      = 0x02,
				LEFT	      = 0x03,
				RIGHT	      = 0x04,
				SELECT	      = 0x00,
				HOME	      = 0x09,
				BACK	      = 0x0D,
				NUMBER_0      = 0x20,
				NUMBER_1      = 0x21,
				NUMBER_2      = 0x22,
				NUMBER_3      = 0x23,
				NUMBER_4      = 0x24,
				NUMBER_5      = 0x25,
				NUMBER_6      = 0x26,
				NUMBER_7      = 0x27,
				NUMBER_8      = 0x28,
				NUMBER_9      = 0x29
		      };
        public:
            HdmiCecSourceImplementation();
            virtual ~HdmiCecSourceImplementation();
            virtual void OnDisplayHDMIHotPlug(dsDisplayEvent_t displayEvent) override;
            void onPowerModeChanged(const PowerState currentState, const PowerState newState);
            void registerEventHandlers();
            static HdmiCecSourceImplementation* _instance;
            CECDeviceInfo_2 deviceList[16];
            pthread_cond_t m_condSig;
            pthread_mutex_t m_lock;
            pthread_cond_t m_condSigUpdate;
            pthread_mutex_t m_lockUpdate;
            bool cecEnableStatus;

            void SendStandbyMsgEvent(const int logicalAddress);
            void SendKeyPressMsgEvent(const int logicalAddress,const int keyCode);
            void SendKeyReleaseMsgEvent(const int logicalAddress);
            void sendActiveSourceEvent();
            void addDevice(const int logicalAddress);
            void removeDevice(const int logicalAddress);
            void sendUnencryptMsg(unsigned char* msg, int size);
            void sendDeviceUpdateInfo(const int logicalAddress);
			void sendKeyReleaseEvent(const int logicalAddress);
            Core::hresult setEnabledInternal(const bool enabled, const bool isPersist);
		    typedef struct sendKeyInfo
                {
                   int logicalAddr;
                   int keyCode;
                }SendKeyInfo;
            BEGIN_INTERFACE_MAP(HdmiCecSourceImplementation)
                INTERFACE_ENTRY(Exchange::IHdmiCecSource)
            END_INTERFACE_MAP


        private:
            template <typename T>
            T* baseInterface()
            {
                static_assert(std::is_base_of<T, HdmiCecSourceImplementation>(), "base type mismatch");
                return static_cast<T*>(this);
            }

            class PowerManagerNotification : public Exchange::IPowerManager::IModeChangedNotification {
            private:
                PowerManagerNotification(const PowerManagerNotification&) = delete;
                PowerManagerNotification& operator=(const PowerManagerNotification&) = delete;
            
            public:
                explicit PowerManagerNotification(HdmiCecSourceImplementation& parent)
                    : _parent(parent)
                {
                }
                ~PowerManagerNotification() override = default;
            
            public:
                void OnPowerModeChanged(const PowerState currentState, const PowerState newState) override
                {
                    _parent.onPowerModeChanged(currentState, newState);
                }

                template <typename T>
                T* baseInterface()
                {
                    static_assert(std::is_base_of<T, PowerManagerNotification>(), "base type mismatch");
                    return static_cast<T*>(this);
                }

                BEGIN_INTERFACE_MAP(PowerManagerNotification)
                INTERFACE_ENTRY(Exchange::IPowerManager::IModeChangedNotification)
                END_INTERFACE_MAP

            private:
                HdmiCecSourceImplementation& _parent;
        
            };
            // We do not allow this plugin to be copied !!
            HdmiCecSourceImplementation(const HdmiCecSourceImplementation&) = delete;
            HdmiCecSourceImplementation& operator=(const HdmiCecSourceImplementation&) = delete;

            
		
            //End methods
            std::string logicalAddressDeviceType;
            bool cecSettingEnabled;
            bool cecOTPSettingEnabled;
            Connection *smConnection;
            int m_numberOfDevices;
            bool m_pollThreadExit;
            Utils::ThreadRAII m_pollThread;
            bool m_updateThreadExit;
            Utils::ThreadRAII m_UpdateThread;
            bool m_sendKeyEventThreadExit;
            bool m_sendKeyEventThreadRun;
            Utils::ThreadRAII m_sendKeyEventThread;
            std::mutex m_sendKeyEventMutex;
            std::queue<SendKeyInfo> m_SendKeyQueue;
            std::condition_variable m_sendKeyCV;
		
            HdmiCecSourceProcessor *msgProcessor;
            HdmiCecSourceFrameListener *msgFrameListener;
            void InitializePowerManager(PluginHost::IShell *service);
            const void InitializeIARM();
            void DeinitializeIARM();
            static void dsHdmiEventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
            void onHdmiHotPlug(int connectStatus);
            bool loadSettings();
            void persistSettings(bool enableStatus);
            void persistOTPSettings(bool enableStatus);
            void persistOSDName(const char *name);
            void persistVendorId(unsigned int vendorID);
            void CECEnable(void);
            void CECDisable(void);
            void getPhysicalAddress();
            void getLogicalAddress();
            void cecAddressesChanged(int changeStatus);
            bool pingDeviceUpdateList (int idev);
            void removeAllCecDevices();
            void requestVendorID(const int newDevlogicalAddress);
            void requestOsdName(const int newDevlogicalAddress);
            void requestCecDevDetails(const int logicalAddress);
            static void threadRun();
            static void threadUpdateCheck();
            static void  threadSendKeyEvent();
            static void threadHotPlugEventHandler(int data);
            static void threadCecDaemonInitHandler();
            static void threadCecStatusUpdateHandler(int data);
            uint32_t sendKeyPressEvent(const int logicalAddress, int keyCode);
            int getUIKeyCode(int keyCode);
            PowerManagerInterfaceRef _powerManagerPlugin;
            Core::Sink<PowerManagerNotification> _pwrMgrNotification;
            bool _registeredEventHandlers;
            private:
                mutable Core::CriticalSection _adminLock;
                std::list<Exchange::IHdmiCecSource::INotification*> _hdmiCecSourceNotifications;

        public:
            Core::hresult SetEnabled(const bool &enabled, HdmiCecSourceSuccess &success) override;
            Core::hresult GetEnabled(bool &enabled, bool &success) override;
            Core::hresult SetOTPEnabled(const bool &enabled, HdmiCecSourceSuccess &success) override;
            Core::hresult GetOTPEnabled(bool &enabled, bool &success) override;
            Core::hresult SetOSDName(const string &name, HdmiCecSourceSuccess &success) override;
            Core::hresult GetOSDName(string &name, bool &success) override;
            Core::hresult SetVendorId(const string &vendorid, HdmiCecSourceSuccess &success) override;
            Core::hresult GetVendorId(string &vendorid, bool &success) override;
            Core::hresult PerformOTPAction(HdmiCecSourceSuccess &success) override;
            Core::hresult SendStandbyMessage(HdmiCecSourceSuccess &success) override;
            Core::hresult SendKeyPressEvent(const uint32_t &logicalAddress,const uint32_t &keyCode, HdmiCecSourceSuccess &success) override;
            Core::hresult GetActiveSourceStatus(bool &isActiveSource, bool &success) override;
            Core::hresult GetDeviceList(uint32_t &numberofdevices, IHdmiCecSourceDeviceListIterator*& deviceList, bool &success) override;
            Core::hresult Configure(PluginHost::IShell* service) override;
            Core::hresult Register(Exchange::IHdmiCecSource::INotification *notification) override;
            Core::hresult Unregister(Exchange::IHdmiCecSource::INotification *notification) override;

        };
	} // namespace Plugin
} // namespace WPEFramework




