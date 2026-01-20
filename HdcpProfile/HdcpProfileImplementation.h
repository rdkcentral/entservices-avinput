/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
 */

 #pragma once

 #include "Module.h"
 #include <interfaces/Ids.h>
 #include <interfaces/IHdcpProfile.h>
 #include <interfaces/IPowerManager.h>
 #include<interfaces/IConfiguration.h>
 
 #include <com/com.h>
 #include <core/core.h>
 #include <mutex>
 #include <vector>

#include "host.hpp"

 #include "PowerManagerInterface.h"
 
 namespace WPEFramework
 {
     namespace Plugin
     {
         
         class HdcpProfileImplementation : public Exchange::IHdcpProfile, public Exchange::IConfiguration, public device::Host::IVideoOutputPortEvents, public device::Host::IDisplayDeviceEvents
         // , public Exchange::IConfiguration
         {
         public:
             // We do not allow this plugin to be copied !!
             HdcpProfileImplementation();
             ~HdcpProfileImplementation() override;

             virtual void OnDisplayHDMIHotPlug(dsDisplayEvent_t displayEvent) override;
             virtual void OnHDCPStatusChange(dsHdcpStatus_t hdcpStatus) override;

             static HdcpProfileImplementation *instance(HdcpProfileImplementation *HdcpProfileImpl = nullptr);
 
             // We do not allow this plugin to be copied !!
             HdcpProfileImplementation(const HdcpProfileImplementation &) = delete;
             HdcpProfileImplementation &operator=(const HdcpProfileImplementation &) = delete;
            
             
             BEGIN_INTERFACE_MAP(HdcpProfileImplementation)
             INTERFACE_ENTRY(Exchange::IHdcpProfile)
             INTERFACE_ENTRY(Exchange::IConfiguration)
             END_INTERFACE_MAP
 
         public:
             enum Event
             {
                 HDCPPROFILE_EVENT_DISPLAYCONNECTIONCHANGED
             };
             class EXTERNAL Job : public Core::IDispatch
             {
             protected:
                 Job(HdcpProfileImplementation *HdcpProfileImplementation, Event event, HDCPStatus &params)
                     : _hdcpProfileImplementation(HdcpProfileImplementation), _event(event), _params(params)
                 {
                     if (_hdcpProfileImplementation != nullptr)
                     {
                         _hdcpProfileImplementation->AddRef();
                     }
                 }
 
             public:
                 Job() = delete;
                 Job(const Job &) = delete;
                 Job &operator=(const Job &) = delete;
                 ~Job()
                 {
                     if (_hdcpProfileImplementation != nullptr)
                     {
                         _hdcpProfileImplementation->Release();
                     }
                 }
 
             public:
                 static Core::ProxyType<Core::IDispatch> Create(HdcpProfileImplementation *hdcpProfileImplementation, Event event, HDCPStatus params)
                 {
 #ifndef USE_THUNDER_R4
                     return (Core::proxy_cast<Core::IDispatch>(Core::ProxyType<Job>::Create(hdcpProfileImplementation, event, params)));
 #else
                     return (Core::ProxyType<Core::IDispatch>(Core::ProxyType<Job>::Create(hdcpProfileImplementation, event, params)));
 #endif
                 }
                 virtual void Dispatch()
                 {
                     _hdcpProfileImplementation->Dispatch(_event, _params);
                 }
 
             private:
                 HdcpProfileImplementation *_hdcpProfileImplementation;
                 const Event _event;
                 HDCPStatus _params;
             };
             
 
         public:
            template <typename T>
            T* baseInterface()
            {
                static_assert(std::is_base_of<T, HdcpProfileImplementation>(), "base type mismatch");
                return static_cast<T*>(this);
            }

             Core::hresult Register(Exchange::IHdcpProfile::INotification *notification) override;
             Core::hresult Unregister(Exchange::IHdcpProfile::INotification *notification) override;
 
             Core::hresult GetHDCPStatus(HDCPStatus& hdcpstatus,bool& success) override;
             Core::hresult GetSettopHDCPSupport(string& supportedHDCPVersion,bool& isHDCPSupported,bool& success) override;
             bool GetHDCPStatusInternal(HDCPStatus& hdcpstatus);
             void InitializePowerManager(PluginHost::IShell *service);
             void onHdmiOutputHotPlug(dsDisplayEvent_t connectStatus);
             void onHdmiOutputHDCPStatusEvent(dsHdcpStatus_t);
             void logHdcpStatus (const char *trigger, HDCPStatus& status);   
             void onHdcpProfileDisplayConnectionChanged();  
             static PowerManagerInterfaceRef _powerManagerPlugin;      
             uint32_t Configure(PluginHost::IShell* service) override;
 
         private:
             mutable Core::CriticalSection _adminLock;
             PluginHost::IShell *mShell;
             std::list<Exchange::IHdcpProfile::INotification *> _hdcpProfileNotification; // List of registered notifications
             PluginHost::IShell* _service;
             void dispatchEvent(Event, const HDCPStatus &params);
             void Dispatch(Event event, const HDCPStatus &params);
 
         public:
            static HdcpProfileImplementation *_instance;
            //  friend class Job;
         };
 
     } // namespace Plugin
 } // namespace WPEFramework
