/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
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

#include "Module.h"
#include <interfaces/IHdcpProfile.h> 
#include <interfaces/json/JsonData_HdcpProfile.h>
#include <interfaces/json/JHdcpProfile.h>
#include <interfaces/IConfiguration.h>
#include "UtilsLogging.h"
#include "tracing/Logging.h"


namespace WPEFramework {

    namespace Plugin {
		
		class HdcpProfile : public PluginHost::IPlugin, public PluginHost::JSONRPC
		{
			private:
            	class Notification : public RPC::IRemoteConnection::INotification, public Exchange::IHdcpProfile::INotification
                {
					private:
			        	Notification() = delete;
			            Notification(const Notification&) = delete;
			            Notification& operator=(const Notification&) = delete;
						
					public:
						explicit Notification(HdcpProfile *parent)
							: _parent(*parent)
						{
							ASSERT(parent != nullptr);
						}
		
						virtual ~Notification()
						{
						}
					
						BEGIN_INTERFACE_MAP(Notification)
						INTERFACE_ENTRY(Exchange::IHdcpProfile::INotification)
						INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
						END_INTERFACE_MAP
							
						void Activated(RPC::IRemoteConnection *) override
						{
							LOGINFO("HdcpProfile Notification Activated");
						}
		
						void Deactivated(RPC::IRemoteConnection *connection) override
						{
							LOGINFO("HdcpProfile Notification Deactivated");
							_parent.Deactivated(connection);
						}
					
						void OnDisplayConnectionChanged(const Exchange::IHdcpProfile::HDCPStatus hdcpstatus) override
						{
							LOGINFO("OnDisplayConnectionChanged: isConnected: %d isHDCPCompliant: %d isHDCPEnabled: %d hdcpReason: %d supportedHDCPVersion: %s receiverHDCPVersion: %s currentHDCPVersion: %s", hdcpstatus.isConnected, hdcpstatus.isHDCPCompliant, hdcpstatus.isHDCPEnabled, hdcpstatus.hdcpReason, hdcpstatus.supportedHDCPVersion.c_str(), hdcpstatus.receiverHDCPVersion.c_str(), hdcpstatus.currentHDCPVersion.c_str());
							Exchange::JHdcpProfile::Event::OnDisplayConnectionChanged(_parent, hdcpstatus);
						}
		
						private:
							HdcpProfile &_parent;
				};
			
			public:
				HdcpProfile(const HdcpProfile &) = delete;
				HdcpProfile &operator=(const HdcpProfile &) = delete;
				
				HdcpProfile();
				virtual ~HdcpProfile();
			
				BEGIN_INTERFACE_MAP(HdcpProfile)
				INTERFACE_ENTRY(PluginHost::IPlugin)
				INTERFACE_ENTRY(PluginHost::IDispatcher)
				INTERFACE_AGGREGATE(Exchange::IHdcpProfile, _hdcpProfile)
				END_INTERFACE_MAP
				
				//  IPlugin methods
				// -------------------------------------------------------------------------------------------------------
				const string Initialize(PluginHost::IShell* service) override;
				void Deinitialize(PluginHost::IShell* service) override;
				string Information() const override;
				
				private:
                	void Deactivated(RPC::IRemoteConnection* connection);
			
				private:
					PluginHost::IShell *_service{};
					uint32_t _connectionId{};
					Exchange::IHdcpProfile *_hdcpProfile{};
					Core::Sink<Notification> _hdcpProfileNotification;
					Exchange::IConfiguration* configure{};
        };

    } // namespace Plugin
} // namespace WPEFramework