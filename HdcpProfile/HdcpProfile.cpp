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

#include "HdcpProfile.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 9

namespace WPEFramework
{

    namespace {

        static Plugin::Metadata<Plugin::HdcpProfile> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

    namespace Plugin
    {
        SERVICE_REGISTRATION(HdcpProfile, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

		HdcpProfile::HdcpProfile()
            : _service(nullptr)
            , _connectionId(0)
            , _hdcpProfile(nullptr)
            , _hdcpProfileNotification(this)
        {
            SYSLOG(Logging::Startup, (_T("HdcpProfile Constructor")));
        }

        HdcpProfile::~HdcpProfile()
        {
            SYSLOG(Logging::Shutdown, (string(_T("HdcpProfile Destructor"))));
        }
    
	const string HdcpProfile::Initialize(PluginHost::IShell *service)
        {
            string message = "";

            ASSERT(nullptr != service);
            ASSERT(nullptr == _service);
            ASSERT(nullptr == _hdcpProfile);
            ASSERT(0 == _connectionId);

            SYSLOG(Logging::Startup, (_T("HdcpProfile::Initialize: PID=%u"), getpid()));

            _service = service;
            _service->AddRef();
            _service->Register(&_hdcpProfileNotification);
            _hdcpProfile = _service->Root<Exchange::IHdcpProfile>(_connectionId, 5000, _T("HdcpProfileImplementation"));

            if (nullptr != _hdcpProfile)
            {
                configure = _hdcpProfile->QueryInterface<Exchange::IConfiguration>();
                if (configure != nullptr)
                {
                    uint32_t result = configure->Configure(_service);
                    if(result != Core::ERROR_NONE)
                    {
                        message = _T("HdcpProfile could not be configured");
                    }
			configure->Release();
                }
                else
                {
                    message = _T("HdcpProfile implementation did not provide a configuration interface");
                }
                // Register for notifications
                _hdcpProfile->Register(&_hdcpProfileNotification);
                
                // Invoking Plugin API register to wpeframework
                Exchange::JHdcpProfile::Register(*this, _hdcpProfile);
            }
            else
            {
                SYSLOG(Logging::Startup, (_T("HdcpProfile::Initialize: Failed to initialise HdcpProfile plugin")));
                message = _T("HdcpProfile plugin could not be initialised");
            }

            if (0 != message.length())
            {
                printf("HdcpProfile::Initialize: Failed to initialise HdcpProfile plugin");
                Deinitialize(service);
            }

            return message;
        }

        void HdcpProfile::Deinitialize(PluginHost::IShell *service)
        {
            ASSERT(_service == service);
            printf("HdcpProfile::Deinitialize: service = %p", service);
            SYSLOG(Logging::Shutdown, (string(_T("HdcpProfile::Deinitialize"))));

            // Make sure the Activated and Deactivated are no longer called before we start cleaning up..
            if (_service != nullptr)
            {
                _service->Unregister(&_hdcpProfileNotification);
            }
            if (nullptr != _hdcpProfile)
            {
                
                _hdcpProfile->Unregister(&_hdcpProfileNotification);
                Exchange::JHdcpProfile::Unregister(*this);
                // Stop processing:
                RPC::IRemoteConnection *connection = service->RemoteConnection(_connectionId);
                VARIABLE_IS_NOT_USED uint32_t result = _hdcpProfile->Release();

                _hdcpProfile = nullptr;

                // It should have been the last reference we are releasing,
                // so it should endup in a DESTRUCTION_SUCCEEDED, if not we
                // are leaking...
                ASSERT(result == Core::ERROR_DESTRUCTION_SUCCEEDED);
                if (result != Core::ERROR_DESTRUCTION_SUCCEEDED) {
                    LOGWARN("HdcpProfile Release did not return DESTRUCTION_SUCCEEDED, potential leak detected (result=%u)", result);
                }

                // If this was running in a (container) process...
                if (nullptr != connection)
                {
                    // Lets trigger the cleanup sequence for
                    // out-of-process code. Which will guard
                    // that unwilling processes, get shot if
                    // not stopped friendly :-)
                    connection->Terminate();
                    connection->Release();
                }
            }
			_connectionId = 0;

            if (_service != nullptr)
            {
                _service->Release();
                _service = nullptr;
            }
            SYSLOG(Logging::Shutdown, (string(_T("HdcpProfile de-initialised"))));
        }
	    string HdcpProfile::Information() const
        {
            return ("This HdcpProfile Plugin facilitates to persist event data for monitoring applications");
        }

        void HdcpProfile::Deactivated(RPC::IRemoteConnection *connection)
        {
            if (connection->Id() == _connectionId)
            {
                ASSERT(nullptr != _service);
                Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
            }
        }
    } // namespace Plugin
} // namespace WPEFramework
