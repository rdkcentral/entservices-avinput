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

#include "UtilsJsonRpc.h"

// COM-RPC path: DeviceSettingsInterface.h brings DSHelper
// and all DS sub-interface headers (IDeviceSettingsHDMIIn, IDeviceSettingsCompositeIn, ...).
#include "DeviceSettingsInterface.h"

#include <algorithm>
#include <vector>

#include <interfaces/IAVInput.h>
#include <interfaces/Ids.h>

#include <boost/variant.hpp>
#include <com/com.h>
#include <core/core.h>

#include "AVInputUtils.h"

#define INPUT_TYPE_ALL          "ALL"
#define INPUT_TYPE_HDMI         "HDMI"
#define INPUT_TYPE_COMPOSITE    "COMPOSITE"

#define DEFAULT_PRIM_VOL_LEVEL  25
#define MAX_PRIM_VOL_LEVEL      100
#define DEFAULT_INPUT_VOL_LEVEL 100

using ParamsType = boost::variant<
    WPEFramework::Exchange::IAVInput::IInputDeviceIterator* const,  // OnDevicesChanged
    std::tuple<int, string, string>,                                // OnSignalChanged
    std::tuple<int, string, string, int>,                           // OnInputStatusChanged
    std::tuple<int, string, int, int, bool, int, int>,              // VideoStreamInfoUpdate
    std::tuple<int, string, bool>,                                  // GameFeatureStatusUpdate
    std::tuple<int, int>                                            // AviContentTypeUpdate
>;

namespace WPEFramework {
namespace Plugin {

    // =========================================================================
    // COM-RPC implementation of IAVInput.
    //
    // Replaces the DS_IARM version that inherited device::Host::IHdmiInEvents
    // and device::Host::ICompositeInEvents (libds callbacks) and called
    // device::HdmiInput / device::CompositeInput directly.
    //
    // Inherits DSHelper to:
    //   - Connect to the entservices-devicesettings plugin via COM-RPC
    //   - Acquire IDeviceSettingsHDMIIn / IDeviceSettingsCompositeIn sub-interfaces
    //   - Register inner notification delegate classes for events
    // =========================================================================
    class AVInputImplementation :   public Exchange::IAVInput,
                                    public DSHelper {

    public:

        static AVInputImplementation* _instance;

        // COM-RPC VRR type tracking (mirrors m_currentVrrType in DS_IARM)
        Exchange::IDeviceSettingsHDMIIn::HDMIInVRRType m_currentVrrType
            { Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_VRR_NONE };

        friend class DispatchJob;

        AVInputImplementation();
        ~AVInputImplementation() override;

        AVInputImplementation(const AVInputImplementation&) = delete;
        AVInputImplementation& operator=(const AVInputImplementation&) = delete;

        BEGIN_INTERFACE_MAP(AVInputImplementation)
        INTERFACE_ENTRY(Exchange::IAVInput)
        END_INTERFACE_MAP

        enum Event {
            ON_AVINPUT_DEVICES_CHANGED,
            ON_AVINPUT_SIGNAL_CHANGED,
            ON_AVINPUT_STATUS_CHANGED,
            ON_AVINPUT_VIDEO_STREAM_INFO_UPDATE,
            ON_AVINPUT_GAME_FEATURE_STATUS_UPDATE,
            ON_AVINPUT_AVI_CONTENT_TYPE_UPDATE
        };

        class EXTERNAL DispatchJob : public Core::IDispatch {

        public:

            DispatchJob() = delete;
            DispatchJob(const DispatchJob&) = delete;
            DispatchJob& operator=(const DispatchJob&) = delete;
            ~DispatchJob()
            {
                if (_avInputImplementation != nullptr) {
                    _avInputImplementation->Release();
                }
            }

            static Core::ProxyType<Core::IDispatch> Create(AVInputImplementation* avInputImplementation, Event event, ParamsType params)
            {
#ifndef USE_THUNDER_R4
                return (Core::proxy_cast<Core::IDispatch>(Core::ProxyType<DispatchJob>::Create(avInputImplementation, event, params)));
#else
                return (Core::ProxyType<Core::IDispatch>(Core::ProxyType<DispatchJob>::Create(avInputImplementation, event, params)));
#endif
            }

            virtual void Dispatch()
            {
                _avInputImplementation->Dispatch(_event, _params);
            }

        protected:

            DispatchJob(AVInputImplementation* avInputImplementation, Event event, ParamsType params)
                : _avInputImplementation(avInputImplementation)
                , _event(event)
                , _params(params)
            {
                if (_avInputImplementation != nullptr) {
                    _avInputImplementation->AddRef();
                }
            }

        private:

            AVInputImplementation*  _avInputImplementation;
            const Event             _event;
            const ParamsType        _params;
        };

        void Dispatch(Event event, const ParamsType params);

        // =========================================================================
        // COM-RPC notification delegate: IDeviceSettingsHDMIIn::INotification
        // Bridges all HDMI-In events from DeviceSettings COM-RPC to AVInput event
        // dispatchers (mirrors DS_IARM's device::Host::IHdmiInEvents callbacks).
        // =========================================================================
        class DSHDMIInNotification
            : public Exchange::IDeviceSettingsHDMIIn::INotification {
        public:
            explicit DSHDMIInNotification(AVInputImplementation& parent) : _parent(parent) {}
            DSHDMIInNotification(const DSHDMIInNotification&) = delete;
            DSHDMIInNotification& operator=(const DSHDMIInNotification&) = delete;

            void OnHDMIInEventHotPlug(const Exchange::IDeviceSettingsHDMIIn::HDMIInPort port,
                                      const bool isConnected) override {
                _parent.onHdmiInEventHotPlug(static_cast<int>(port), isConnected);
            }
            void OnHDMIInEventSignalStatus(const Exchange::IDeviceSettingsHDMIIn::HDMIInPort port,
                                           const Exchange::IDeviceSettingsHDMIIn::HDMIInSignalStatus signalStatus) override {
                _parent.onHdmiInEventSignalStatus(static_cast<int>(port), static_cast<int>(signalStatus));
            }
            void OnHDMIInEventStatus(const Exchange::IDeviceSettingsHDMIIn::HDMIInPort activePort,
                                     const bool isPresented) override {
                _parent.onHdmiInEventStatus(static_cast<int>(activePort), isPresented);
            }
            void OnHDMIInVideoModeUpdate(const Exchange::IDeviceSettingsHDMIIn::HDMIInPort port,
                                         const Exchange::IDeviceSettingsHDMIIn::HDMIVideoPortResolution& videoPortResolution) override {
                _parent.onHdmiInVideoModeUpdate(static_cast<int>(port), videoPortResolution);
            }
            void OnHDMIInAllmStatus(const Exchange::IDeviceSettingsHDMIIn::HDMIInPort port,
                                    const bool allmStatus) override {
                _parent.onHdmiInAllmStatus(static_cast<int>(port), allmStatus);
            }
            void OnHDMIInAVIContentType(const Exchange::IDeviceSettingsHDMIIn::HDMIInPort port,
                                        const Exchange::IDeviceSettingsHDMIIn::HDMIInAviContentType aviContentType) override {
                _parent.onHdmiInAVIContentType(static_cast<int>(port), static_cast<int>(aviContentType));
            }
            void OnHDMIInVRRStatus(const Exchange::IDeviceSettingsHDMIIn::HDMIInPort port,
                                   const Exchange::IDeviceSettingsHDMIIn::HDMIInVRRType vrrType) override {
                _parent.onHdmiInVRRStatus(static_cast<int>(port), vrrType);
            }

            BEGIN_INTERFACE_MAP(DSHDMIInNotification)
                INTERFACE_ENTRY(Exchange::IDeviceSettingsHDMIIn::INotification)
            END_INTERFACE_MAP
        private:
            AVInputImplementation& _parent;
        };

        // =========================================================================
        // COM-RPC notification delegate: IDeviceSettingsCompositeIn::INotification
        // Bridges all Composite-In events from DeviceSettings COM-RPC to AVInput
        // event dispatchers (mirrors DS_IARM's ICompositeInEvents callbacks).
        // =========================================================================
        class DSCompositeInNotification
            : public Exchange::IDeviceSettingsCompositeIn::INotification {
        public:
            explicit DSCompositeInNotification(AVInputImplementation& parent) : _parent(parent) {}
            DSCompositeInNotification(const DSCompositeInNotification&) = delete;
            DSCompositeInNotification& operator=(const DSCompositeInNotification&) = delete;

            void OnCompositeInHotPlug(const Exchange::IDeviceSettingsCompositeIn::CompositeInPort port,
                                      const bool isConnected) override {
                _parent.onCompositeInHotPlug(static_cast<int>(port), isConnected);
            }
            void OnCompositeInSignalStatus(const Exchange::IDeviceSettingsCompositeIn::CompositeInPort port,
                                           const Exchange::IDeviceSettingsCompositeIn::CompositeInSignalStatus signalStatus) override {
                _parent.onCompositeInSignalStatus(static_cast<int>(port), static_cast<int>(signalStatus));
            }
            void OnCompositeInStatus(const Exchange::IDeviceSettingsCompositeIn::CompositeInPort activePort,
                                     const bool isPresented) override {
                _parent.onCompositeInStatus(static_cast<int>(activePort), isPresented);
            }
            void OnCompositeInVideoModeUpdate(const Exchange::IDeviceSettingsCompositeIn::CompositeInPort activePort,
                                              const Exchange::IDeviceSettingsCompositeIn::DisplayVideoPortResolution& videoResolution) override {
                _parent.onCompositeInVideoModeUpdate(static_cast<int>(activePort), videoResolution);
            }

            BEGIN_INTERFACE_MAP(DSCompositeInNotification)
                INTERFACE_ENTRY(Exchange::IDeviceSettingsCompositeIn::INotification)
            END_INTERFACE_MAP
        private:
            AVInputImplementation& _parent;
        };

        // Notification delegate instances (initialized with *this in ctor)
        Core::Sink<DSHDMIInNotification>        _DSHDMIInNotification;
        Core::Sink<DSCompositeInNotification>   _DSCompositeInNotification;

        // =========================================================================
        // DSHelper overrides
        // =========================================================================
        void OnDeviceSettingsActivated() override;
        void OnDeviceSettingsDeactivated() override;

        // =========================================================================
        // IAVInput methods
        // =========================================================================
        Core::hresult Configure(PluginHost::IShell* service) override;

        Core::hresult RegisterDevicesChangedNotification(Exchange::IAVInput::IDevicesChangedNotification* notification) override;
        Core::hresult UnregisterDevicesChangedNotification(Exchange::IAVInput::IDevicesChangedNotification* notification) override;

        Core::hresult RegisterSignalChangedNotification(Exchange::IAVInput::ISignalChangedNotification* notification) override;
        Core::hresult UnregisterSignalChangedNotification(Exchange::IAVInput::ISignalChangedNotification* notification) override;

        Core::hresult RegisterInputStatusChangedNotification(Exchange::IAVInput::IInputStatusChangedNotification* notification) override;
        Core::hresult UnregisterInputStatusChangedNotification(Exchange::IAVInput::IInputStatusChangedNotification* notification) override;

        Core::hresult RegisterVideoStreamInfoUpdateNotification(Exchange::IAVInput::IVideoStreamInfoUpdateNotification* notification) override;
        Core::hresult UnregisterVideoStreamInfoUpdateNotification(Exchange::IAVInput::IVideoStreamInfoUpdateNotification* notification) override;

        Core::hresult RegisterGameFeatureStatusUpdateNotification(Exchange::IAVInput::IGameFeatureStatusUpdateNotification* notification) override;
        Core::hresult UnregisterGameFeatureStatusUpdateNotification(Exchange::IAVInput::IGameFeatureStatusUpdateNotification* notification) override;

        Core::hresult RegisterAviContentTypeUpdateNotification(Exchange::IAVInput::IAviContentTypeUpdateNotification* notification) override;
        Core::hresult UnregisterAviContentTypeUpdateNotification(Exchange::IAVInput::IAviContentTypeUpdateNotification* notification) override;

        Core::hresult ContentProtected(bool& isContentProtected, bool& success) override;
        Core::hresult NumberOfInputs(uint32_t& numberOfInputs, bool& success) override;
        Core::hresult CurrentVideoMode(string& currentVideoMode, bool& success) override;
        Core::hresult WriteEDID(const string& portId, const string& message, SuccessResult& successResult) override;
        Core::hresult ReadEDID(const string& portId, string& EDID, bool& success) override;
        Core::hresult GetRawSPD(const string& portId, string& HDMISPD, bool& success) override;
        Core::hresult GetSPD(const string& portId, string& HDMISPD, bool& success) override;
        Core::hresult StartInput(const string& portId, const string& typeOfInput, const bool requestAudioMix, const int plane, const bool topMost, SuccessResult& successResult) override;
        Core::hresult StopInput(const string& typeOfInput, SuccessResult& successResult) override;
        Core::hresult SetVideoRectangle(const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h, const string& typeOfInput, SuccessResult& successResult) override;
        Core::hresult GetSupportedGameFeatures(Exchange::IAVInput::IStringIterator*& features, bool& success) override;
        Core::hresult GetGameFeatureStatus(const string& portId, const string& gameFeature, bool& mode, bool& success) override;
        Core::hresult GetVRRFrameRate(const string& portId, double& currentVRRVideoFrameRate, bool& success) override;
        Core::hresult GetEdid2AllmSupport(const string& portId, bool& allmSupport, bool& success) override;
        Core::hresult SetEdid2AllmSupport(const string& portId, const bool allmSupport, SuccessResult& successResult) override;
        Core::hresult GetVRRSupport(const string& portId, bool& vrrSupport, bool& success) override;
        Core::hresult SetVRRSupport(const string& portId, const bool vrrSupport, SuccessResult& successResult) override;
        Core::hresult GetARCPortId(string& portId, bool& success) override;
        Core::hresult GetHdmiVersion(const string& portId, string& HdmiCapabilityVersion, bool& success) override;
        Core::hresult SetMixerLevels(const int primaryVolume, const int inputVolume, SuccessResult& successResult) override;
        Core::hresult SetEdidVersion(const string& portId, const string& edidVersion, SuccessResult& successResult) override;
        Core::hresult GetEdidVersion(const string& portId, string& edidVersion, bool& success) override;

    private:

        mutable Core::CriticalSection _adminLock;
        PluginHost::IShell* _service { nullptr };
        bool _registeredDsEventHandlers { false };
        int m_primVolume  { DEFAULT_PRIM_VOL_LEVEL };
        int m_inputVolume { DEFAULT_INPUT_VOL_LEVEL };

        // Notification subscriber lists
        std::list<Exchange::IAVInput::IDevicesChangedNotification*>          _devicesChangedNotifications;
        std::list<Exchange::IAVInput::ISignalChangedNotification*>            _signalChangedNotifications;
        std::list<Exchange::IAVInput::IInputStatusChangedNotification*>       _inputStatusChangedNotifications;
        std::list<Exchange::IAVInput::IVideoStreamInfoUpdateNotification*>    _videoStreamInfoUpdateNotifications;
        std::list<Exchange::IAVInput::IGameFeatureStatusUpdateNotification*>  _gameFeatureStatusUpdateNotifications;
        std::list<Exchange::IAVInput::IAviContentTypeUpdateNotification*>     _aviContentTypeUpdateNotifications;

        // Generic notification register/unregister helpers
        template <typename T>
        Core::hresult Register(std::list<T*>& list, T* notification);

        template <typename T>
        Core::hresult Unregister(std::list<T*>& list, T* notification);

        void dispatchEvent(Event event, ParamsType params);

        // Internal event forwarders called from notification delegates
        // (bridge COM-RPC INotification callbacks → shared dispatch helpers)
        void onHdmiInEventHotPlug(int port, bool isConnected);
        void onHdmiInEventSignalStatus(int port, int signalStatus);
        void onHdmiInEventStatus(int activePort, bool isPresented);
        void onHdmiInVideoModeUpdate(int port, const Exchange::IDeviceSettingsHDMIIn::HDMIVideoPortResolution& videoPortResolution);
        void onHdmiInAllmStatus(int port, bool allmStatus);
        void onHdmiInAVIContentType(int port, int aviContentType);
        void onHdmiInVRRStatus(int port, Exchange::IDeviceSettingsHDMIIn::HDMIInVRRType vrrType);

        void onCompositeInHotPlug(int port, bool isConnected);
        void onCompositeInSignalStatus(int port, int signalStatus);
        void onCompositeInStatus(int activePort, bool isPresented);
        void onCompositeInVideoModeUpdate(int activePort, const Exchange::IDeviceSettingsCompositeIn::DisplayVideoPortResolution& videoResolution);

        // Device-list helpers (getInputDevices replaces libds; GetInputDevices identical to DS_IARM)
        Core::hresult getInputDevices(const string& typeOfInput, std::list<WPEFramework::Exchange::IAVInput::InputDevice>& inputDeviceList);
        Core::hresult GetInputDevices(const string& typeOfInput, Exchange::IAVInput::IInputDeviceIterator*& devices, bool& success);

        // Shared event-to-notification dispatch helpers
        // (identical logic to DS_IARM AVInputSignalChange / AVInputHotplug etc.)
        void AVInputHotplug(int port, int isConnected, int type);
        void AVInputSignalChange(int port, int signalStatus, int type);
        void AVInputStatusChange(int port, bool isPresented, int type);
        void AVInputVideoModeUpdate(int port, const Exchange::IDeviceSettingsHDMIIn::HDMIVideoPortResolution& resolution, int type);
        void AVInputVideoModeUpdate(int port, const Exchange::IDeviceSettingsCompositeIn::DisplayVideoPortResolution& resolution, int type);
        void hdmiInputAviContentTypeChange(int port, int content_type);
        void AVInputALLMChange(int port, bool allm_mode);
        void AVInputVRRChange(int port, Exchange::IDeviceSettingsHDMIIn::HDMIInVRRType vrr_type, bool vrr_mode);

        // COM-RPC equivalents of DS_IARM getALLMStatus() / getVRRStatus() helpers
        bool getALLMStatus(int iPort);
        bool getVRRStatus(int iPort, Exchange::IDeviceSettingsHDMIIn::HDMIInVRRStatus& vrrStatus);

    }; // AVInputImplementation

} // namespace Plugin
} // namespace WPEFramework
