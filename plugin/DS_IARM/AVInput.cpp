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

#include "AVInput.h"

#include "compositeIn.hpp"
#include "hdmiIn.hpp"

#include "UtilsJsonRpc.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 7
#define API_VERSION_NUMBER_PATCH 1

// Explicitly implementing getInputDevices method instead of autogenerating via IAVInput.h
// because it requires optional parameters which are not supported in Thunder 4.x. This can 
// be refactored after migrating to 5.x.
#define AVINPUT_METHOD_GET_INPUT_DEVICES "getInputDevices"

namespace WPEFramework {
namespace {

    static Plugin::Metadata<Plugin::AVInput> metadata(
        // Version (Major, Minor, Patch)
        API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
        // Preconditions
        {},
        // Terminations
        {},
        // Controls
        {});
}

namespace Plugin {

    SERVICE_REGISTRATION(AVInput, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

    AVInput::AVInput()
        : _service(nullptr)
        , _connectionId(0)
        , _avInput(nullptr)
        , _avInputNotification(this)
    {
        Register<JsonObject, JsonObject>(_T(AVINPUT_METHOD_GET_INPUT_DEVICES), &AVInput::getInputDevicesWrapper, this);
        SYSLOG(Logging::Startup, (_T("AVInput Constructor")));
    }

    AVInput::~AVInput()
    {
        Unregister(_T(AVINPUT_METHOD_GET_INPUT_DEVICES));
        SYSLOG(Logging::Shutdown, (string(_T("AVInput Destructor"))));
    }

    const string AVInput::Initialize(PluginHost::IShell* service)
    {
        string message = "";

        ASSERT(nullptr != service);
        ASSERT(nullptr == _service);
        ASSERT(nullptr == _avInput);
        ASSERT(0 == _connectionId);

        SYSLOG(Logging::Startup, (_T("AVInput::Initialize: PID=%u"), getpid()));

        _service = service;
        _service->AddRef();
        _service->Register(&_avInputNotification);

        _avInput = service->Root<Exchange::IAVInput>(_connectionId, 5000, _T("AVInputImplementation"));

        if (nullptr != _avInput) {
            _avInput->RegisterDevicesChangedNotification(_avInputNotification.baseInterface<Exchange::IAVInput::IDevicesChangedNotification>());
            _avInput->RegisterSignalChangedNotification(_avInputNotification.baseInterface<Exchange::IAVInput::ISignalChangedNotification>());
            _avInput->RegisterInputStatusChangedNotification(_avInputNotification.baseInterface<Exchange::IAVInput::IInputStatusChangedNotification>());
            _avInput->RegisterVideoStreamInfoUpdateNotification(_avInputNotification.baseInterface<Exchange::IAVInput::IVideoStreamInfoUpdateNotification>());
            _avInput->RegisterGameFeatureStatusUpdateNotification(_avInputNotification.baseInterface<Exchange::IAVInput::IGameFeatureStatusUpdateNotification>());
            _avInput->RegisterAviContentTypeUpdateNotification(_avInputNotification.baseInterface<Exchange::IAVInput::IAviContentTypeUpdateNotification>());

            _avInput->Configure(service);

            // Invoking Plugin API register to wpeframework
            Exchange::JAVInput::Register(*this, _avInput);
        } else {
            SYSLOG(Logging::Startup, (_T("AVInput::Initialize: Failed to initialize AVInput plugin")));
            message = _T("AVInput plugin could not be initialized");
        }

        return message;
    }

    void AVInput::Deinitialize(PluginHost::IShell* service)
    {
        ASSERT(_service == service);

        SYSLOG(Logging::Shutdown, (string(_T("AVInput::Deinitialize"))));

        // Make sure the Activated and Deactivated are no longer called before we start cleaning up.
        _service->Unregister(&_avInputNotification);

        if (nullptr != _avInput) {

            _avInput->UnregisterDevicesChangedNotification(_avInputNotification.baseInterface<Exchange::IAVInput::IDevicesChangedNotification>());
            _avInput->UnregisterSignalChangedNotification(_avInputNotification.baseInterface<Exchange::IAVInput::ISignalChangedNotification>());
            _avInput->UnregisterInputStatusChangedNotification(_avInputNotification.baseInterface<Exchange::IAVInput::IInputStatusChangedNotification>());
            _avInput->UnregisterVideoStreamInfoUpdateNotification(_avInputNotification.baseInterface<Exchange::IAVInput::IVideoStreamInfoUpdateNotification>());
            _avInput->UnregisterGameFeatureStatusUpdateNotification(_avInputNotification.baseInterface<Exchange::IAVInput::IGameFeatureStatusUpdateNotification>());
            _avInput->UnregisterAviContentTypeUpdateNotification(_avInputNotification.baseInterface<Exchange::IAVInput::IAviContentTypeUpdateNotification>());

            Exchange::JAVInput::Unregister(*this);

            // Stop processing:
            RPC::IRemoteConnection* connection = service->RemoteConnection(_connectionId);
            VARIABLE_IS_NOT_USED uint32_t result = _avInput->Release();

            _avInput = nullptr;

            // It should have been the last reference we are releasing,
            // so it should endup in a DESTRUCTION_SUCCEEDED, if not we
            // are leaking...
            ASSERT(result == Core::ERROR_DESTRUCTION_SUCCEEDED);

            // If this was running in a (container) process...
            if (nullptr != connection) {
                // Lets trigger the cleanup sequence for
                // out-of-process code. Which will guard
                // that unwilling processes, get shot if
                // not stopped friendly :-)
                try {
                    connection->Terminate();
                    // Log success if needed
                    LOGWARN("Connection terminated successfully.");
                } catch (const std::exception& e) {
                    std::string errorMessage = "Failed to terminate connection: ";
                    errorMessage += e.what();
                    LOGWARN("%s", errorMessage.c_str());
                }

                connection->Release();
            }
        }

        _connectionId = 0;
        _service->Release();
        _service = nullptr;
        SYSLOG(Logging::Shutdown, (string(_T("AVInput de-initialised"))));
    }

    JsonArray AVInput::getInputDevices(int iType)
    {
        JsonArray list;
        try
        {
            int num = 0;
            if (iType == INPUT_TYPE_INT_HDMI) {
                num = device::HdmiInput::getInstance().getNumberOfInputs();
            }
            else if (iType == INPUT_TYPE_INT_COMPOSITE) {
                num = device::CompositeInput::getInstance().getNumberOfInputs();
            }
            if (num > 0) {
                int i = 0;
                for (i = 0; i < num; i++) {
                    //Input ID is aleays 0-indexed, continuous number starting 0
                    JsonObject hash;
                    hash["id"] = i;
                    std::stringstream locator;
                    if (iType == INPUT_TYPE_INT_HDMI) {
                        locator << "hdmiin://localhost/deviceid/" << i;
                        hash["connected"] = device::HdmiInput::getInstance().isPortConnected(i);
                    }
                    else if (iType == INPUT_TYPE_INT_COMPOSITE) {
                        locator << "cvbsin://localhost/deviceid/" << i;
                        hash["connected"] = device::CompositeInput::getInstance().isPortConnected(i);
                    }
                    hash["locator"] = locator.str();
                    LOGWARN("AVInputService::getInputDevices id %d, locator=[%s], connected=[%d]", i, hash["locator"].String().c_str(), hash["connected"].Boolean());
                    list.Add(hash);
                }
            }
        }
        catch (const std::exception &e) {
            LOGWARN("AVInputService::getInputDevices Failed");
        }
        return list;
    }

    uint32_t AVInput::getInputDevicesWrapper(const JsonObject& parameters, JsonObject& response)
    {
        LOGINFOMETHOD();

        if (parameters.HasLabel("typeOfInput")) {
            string sType = parameters["typeOfInput"].String();
            int iType = 0;
            try {
                iType = AVInputUtils::getTypeOfInput(sType);
            }catch (...) {
                LOGWARN("Invalid Arguments");
                returnResponse(false);
            }
            response["devices"] = getInputDevices(iType);
        }
        else {
            JsonArray listHdmi = getInputDevices(INPUT_TYPE_INT_HDMI);
            JsonArray listComposite = getInputDevices(INPUT_TYPE_INT_COMPOSITE);
            for (int i = 0; i < listComposite.Length(); i++) {
                listHdmi.Add(listComposite.Get(i));
            }
            response["devices"] = listHdmi;
        }
        returnResponse(true);
    }

    string AVInput::Information() const
    {
        return (string());
    }

    void AVInput::Deactivated(RPC::IRemoteConnection* connection)
    {
        if (connection->Id() == _connectionId) {
            ASSERT(nullptr != _service);
            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service, PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
        }
    }

    void AVInput::Notification::OnDevicesChanged(Exchange::IAVInput::IInputDeviceIterator* const devices)
    {
        if (devices != nullptr)
        {
            Exchange::IAVInput::InputDevice resultItem{};
            Core::JSON::Container eventPayload;

            if(devices->Count() == 0) {
                JsonObject params;
                JsonArray emptyArray;
                params["devices"] = emptyArray;
                _parent.Notify(_T("onDevicesChanged"), params);
                return;
            }

            Core::JSON::ArrayType<InputDeviceJson> deviceArray;
            while (devices->Next(resultItem) == true) { deviceArray.Add() = resultItem; }
            eventPayload.Add(_T("devices"), &deviceArray);
            _parent.Notify(_T("onDevicesChanged"), eventPayload);
        }
    }
} // namespace Plugin
} // namespace WPEFramework
