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

#include "Module.h"

#include <core/JSON.h>
#include <interfaces/IAVInput.h>
#include <interfaces/json/JAVInput.h>
#include <interfaces/json/JsonData_AVInput.h>
#include "AVInputImplementation.h"
#include "AVInputJsonData.h"
#include "AVInputUtils.h"

#include "UtilsLogging.h"
#include "tracing/Logging.h"

#include "host.hpp"
#include "compositeIn.hpp"
#include "hdmiIn.hpp"

namespace WPEFramework {
namespace Plugin {
    
    class AVInput: public PluginHost::IPlugin, 
                public PluginHost::JSONRPC {
    public:

        AVInput(const AVInput&) = delete;
        AVInput& operator=(const AVInput&) = delete;

        AVInput();
        virtual ~AVInput();

        BEGIN_INTERFACE_MAP(AVInput)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            INTERFACE_AGGREGATE(Exchange::IAVInput, _avInput)
        END_INTERFACE_MAP

        //  IPlugin methods
        // -------------------------------------------------------------------------------------------------------
        const string Initialize(PluginHost::IShell* service) override;
        void Deinitialize(PluginHost::IShell* service) override;
        string Information() const override;

    protected:

        void RegisterAll();
        void UnregisterAll();

    private:

        PluginHost::IShell* _service {};
        uint32_t _connectionId {};
        Exchange::IAVInput* _avInput {};

        class Notification : public RPC::IRemoteConnection::INotification,
                             public Exchange::IAVInput::IDevicesChangedNotification,
                             public Exchange::IAVInput::ISignalChangedNotification,
                             public Exchange::IAVInput::IInputStatusChangedNotification,
                             public Exchange::IAVInput::IVideoStreamInfoUpdateNotification,
                             public Exchange::IAVInput::IGameFeatureStatusUpdateNotification,
                             public Exchange::IAVInput::IAviContentTypeUpdateNotification {

        public:
        
            explicit Notification(AVInput* parent)
                : _parent(*parent)
            {
                ASSERT(parent != nullptr);
            }

            virtual ~Notification()
            {
            }

            template <typename T>
            T* baseInterface()
            {
                static_assert(std::is_base_of<T, Notification>(), "base type mismatch");
                return static_cast<T*>(this);
            }

            BEGIN_INTERFACE_MAP(Notification)
                INTERFACE_ENTRY(Exchange::IAVInput::IDevicesChangedNotification)
                INTERFACE_ENTRY(Exchange::IAVInput::ISignalChangedNotification)
                INTERFACE_ENTRY(Exchange::IAVInput::IInputStatusChangedNotification)
                INTERFACE_ENTRY(Exchange::IAVInput::IVideoStreamInfoUpdateNotification)
                INTERFACE_ENTRY(Exchange::IAVInput::IGameFeatureStatusUpdateNotification)
                INTERFACE_ENTRY(Exchange::IAVInput::IAviContentTypeUpdateNotification)
                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
            END_INTERFACE_MAP

            void Activated(RPC::IRemoteConnection*) override
            {
            }

            void Deactivated(RPC::IRemoteConnection* connection) override
            {
                _parent.Deactivated(connection);
            }

            void OnDevicesChanged(Exchange::IAVInput::IInputDeviceIterator* const devices) override;

            void OnSignalChanged(const int id, const string& locator, const string& signalStatus) override
            {
                LOGINFO("OnSignalChanged: id %d, locator %s, status %s\n", id, locator.c_str(), signalStatus.c_str());
                Exchange::JAVInput::Event::OnSignalChanged(_parent, id, locator, signalStatus);
            }

            void OnInputStatusChanged(const int id, const string& locator, const string& status, const int plane) override
            {
                LOGINFO("OnInputStatusChanged: id %d, locator %s, status %s, plane %d\n", id, locator.c_str(), status.c_str(), plane);
                Exchange::JAVInput::Event::OnInputStatusChanged(_parent, id, locator, status, plane);
            }

            void VideoStreamInfoUpdate(const int id, const string& locator, const int width, const int height, const bool progressive, const int frameRateN, const int frameRateD) override
            {
                LOGINFO("VideoStreamInfoUpdate: id %d, width %d, height %d, frameRateN %d, frameRateD %d, progressive %d, locator %s\n",
                    id, width, height, frameRateN, frameRateD, progressive, locator.c_str());
                Exchange::JAVInput::Event::VideoStreamInfoUpdate(_parent, id, locator, width, height, progressive, frameRateN, frameRateD);
            }

            void GameFeatureStatusUpdate(const int id, const string& gameFeature, const bool mode) override
            {
                LOGINFO("GameFeatureStatusUpdate: id %d, gameFeature %s, mode %d\n",
                    id, gameFeature.c_str(), static_cast<int>(mode));
                Exchange::JAVInput::Event::GameFeatureStatusUpdate(_parent, id, gameFeature, mode);
            }

            void AviContentTypeUpdate(const int id, const int aviContentType) override
            {
                LOGINFO("AviContentTypeUpdate: id %d, contentType %d\n", id, aviContentType);
                Exchange::JAVInput::Event::AviContentTypeUpdate(_parent, id, aviContentType);
            }

        private:
            AVInput& _parent;

            Notification() = delete;
            Notification(const Notification&) = delete;
            Notification& operator=(const Notification&) = delete;
        };

        Core::Sink<Notification> _avInputNotification;

        void Deactivated(RPC::IRemoteConnection* connection);

        JsonArray getInputDevices(int iType);
        uint32_t getInputDevicesWrapper(const JsonObject& parameters, JsonObject& response);

    }; // AVInput
} // namespace Plugin
} // namespace WPEFramework
