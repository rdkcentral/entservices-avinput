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

#include <stdint.h>
#include <mutex>
#include <condition_variable>

#include <thread>

#undef Assert // this define from Connection.hpp conflicts with WPEFramework

#include "Module.h"

#include "UtilsBIT.h"
#include "UtilsThreadRAII.h"

#include <interfaces/IHdmiCecSource.h>
#include <interfaces/json/JHdmiCecSource.h>
#include <interfaces/json/JsonData_HdmiCecSource.h>

using namespace WPEFramework;

namespace WPEFramework {

    namespace Plugin {
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
        class HdmiCecSource : public PluginHost::IPlugin, public PluginHost::JSONRPC {

            private:
                class Notification : public RPC::IRemoteConnection::INotification,
                                     public Exchange::IHdmiCecSource::INotification
                    {
                       private:
                           Notification() = delete;
                           Notification(const Notification&) = delete;
                           Notification& operator=(const Notification&) = delete;

                        public:
                            explicit Notification(HdmiCecSource* parent)
                                : _parent(*parent)
                                {
                                    ASSERT(parent != nullptr);
                                }

                                virtual ~Notification()
                                {
                                }

                                BEGIN_INTERFACE_MAP(Notification) 
                                INTERFACE_ENTRY(Exchange::IHdmiCecSource::INotification)
                                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
                                END_INTERFACE_MAP

                                void Activated(RPC::IRemoteConnection*) override
                                {
                                }
                            
                                void Deactivated(RPC::IRemoteConnection *connection) override
                                {
                                   _parent.Deactivated(connection);
                                }

                                void OnDeviceAdded(const int logicalAddress) override
                                {
                                    LOGINFO("OnDeviceAdded");
                                    LOGINFO("logicalAddress: %d", logicalAddress);
                                    Exchange::JHdmiCecSource::Event::OnDeviceAdded(_parent, logicalAddress);
                                }
                                void OnDeviceRemoved(const int logicalAddress) override
                                {
                                    LOGINFO("OnDeviceRemoved");
                                    LOGINFO("logicalAddress: %d", logicalAddress);
                                    Exchange::JHdmiCecSource::Event::OnDeviceRemoved(_parent, logicalAddress);
                                }
                                void OnDeviceInfoUpdated(const int logicalAddress) override
                                {
                                    LOGINFO("OnDeviceInfoUpdated");
                                    LOGINFO("logicalAddress: %d", logicalAddress);
                                    Exchange::JHdmiCecSource::Event::OnDeviceInfoUpdated(_parent, logicalAddress);
                                }
                                void OnActiveSourceStatusUpdated(const bool status) override
                                {
                                    LOGINFO("OnActiveSourceStatusUpdated");
                                    LOGINFO("status: %d", status);
                                    Exchange::JHdmiCecSource::Event::OnActiveSourceStatusUpdated(_parent, status);
                                }
                                void StandbyMessageReceived(const int logicalAddress) override
                                {
                                    LOGINFO("StandbyMessageReceived");
                                    LOGINFO("logicalAddress: %d", logicalAddress);
                                    Exchange::JHdmiCecSource::Event::StandbyMessageReceived(_parent, logicalAddress);
                                }
                                void OnKeyReleaseEvent(const int logicalAddress) override
                                {
                                    LOGINFO("OnKeyReleaseEvent");
                                    LOGINFO("logicalAddress: %d", logicalAddress);
                                    Exchange::JHdmiCecSource::Event::OnKeyReleaseEvent(_parent, logicalAddress);
                                }
                                void OnKeyPressEvent(const int logicalAddress, const int keyCode) override
                                {
                                    LOGINFO("OnKeyPressEvent");
                                    LOGINFO("logicalAddress: %d, keyCode: %d", logicalAddress, keyCode);
                                    Exchange::JHdmiCecSource::Event::OnKeyPressEvent(_parent, logicalAddress, keyCode);
                                }

                            private:
                                HdmiCecSource &_parent;

                    };

            public:
                // We do not allow this plugin to be copied !!
                HdmiCecSource(const HdmiCecSource&) = delete;
                HdmiCecSource& operator=(const HdmiCecSource&) = delete;

                HdmiCecSource()
                : PluginHost::IPlugin()
                , PluginHost::JSONRPC()
                , _service(nullptr)
                , _notification(this)
                , _hdmiCecSource(nullptr)
                , _connectionId(0)
                {

                }
                virtual ~HdmiCecSource()
                {

                }

                BEGIN_INTERFACE_MAP(HdmiCecSource)
                INTERFACE_ENTRY(PluginHost::IPlugin)
                INTERFACE_ENTRY(PluginHost::IDispatcher)
                INTERFACE_AGGREGATE(Exchange::IHdmiCecSource, _hdmiCecSource)
                END_INTERFACE_MAP

                //  IPlugin methods
                // -------------------------------------------------------------------------------------------------------
		        const string Initialize(PluginHost::IShell* service) override;
                void Deinitialize(PluginHost::IShell* service) override;
                string Information() const override;
                //Begin methods

            private:
                void Deactivated(RPC::IRemoteConnection* connection);

            private:
                PluginHost::IShell* _service{};
                Core::Sink<Notification> _notification;
                Exchange::IHdmiCecSource* _hdmiCecSource;
                uint32_t _connectionId;
        };
	} // namespace Plugin
} // namespace WPEFramework




