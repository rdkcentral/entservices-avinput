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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.Fv
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "Module.h"

#include "UtilsIarm.h"
#include "UtilsJsonRpc.h"
#include "dsMgr.h"

#include "compositeIn.hpp"
#include "exception.hpp"
#include "hdmiIn.hpp"
#include "host.hpp"
#include "manager.hpp"

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

    class AVInputImplementation :   public Exchange::IAVInput,
                                    public device::Host::IHdmiInEvents, 
                                    public device::Host::ICompositeInEvents {

    public:

        static AVInputImplementation* _instance;
        dsVRRType_t m_currentVrrType;
        friend class Job;

        // We do not allow this plugin to be copied !!
        AVInputImplementation();
        ~AVInputImplementation() override;

        // We do not allow this plugin to be copied !!
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

        class EXTERNAL Job : public Core::IDispatch {

        public:

            Job() = delete;
            Job(const Job&) = delete;
            Job& operator=(const Job&) = delete;
            ~Job()
            {
                if (_avInputImplementation != nullptr) {
                    _avInputImplementation->Release();
                }
            }

            static Core::ProxyType<Core::IDispatch> Create(AVInputImplementation* avInputImplementation, Event event, ParamsType params)
            {
#ifndef USE_THUNDER_R4
                return (Core::proxy_cast<Core::IDispatch>(Core::ProxyType<Job>::Create(avInputImplementation, event, params)));
#else
                return (Core::ProxyType<Core::IDispatch>(Core::ProxyType<Job>::Create(avInputImplementation, event, params)));
#endif
            }

            virtual void Dispatch()
            {
                _avInputImplementation->Dispatch(_event, _params);
            }

        protected:

            Job(AVInputImplementation* avInputImplementation, Event event, ParamsType& params)
                : _avInputImplementation(avInputImplementation)
                , _event(event)
                , _params(params)
            {
                if (_avInputImplementation != nullptr) {
                    _avInputImplementation->AddRef();
                }
            }

        private:
        
            AVInputImplementation* _avInputImplementation;
            const Event _event;
            ParamsType _params;
        };

        /* HdmiInEventNotification*/

        void OnHdmiInEventHotPlug(dsHdmiInPort_t port, bool isConnected) override;
        void OnHdmiInEventSignalStatus(dsHdmiInPort_t port, dsHdmiInSignalStatus_t signalStatus) override;   
        void OnHdmiInEventStatus(dsHdmiInPort_t activePort, bool isPresented) override;
        void OnHdmiInVideoModeUpdate(dsHdmiInPort_t port, const dsVideoPortResolution_t& videoPortResolution) override;
        void OnHdmiInAllmStatus(dsHdmiInPort_t port, bool allmStatus) override;
        void OnHdmiInAVIContentType(dsHdmiInPort_t port, dsAviContentType_t aviContentType) override;
        void OnHdmiInVRRStatus(dsHdmiInPort_t port, dsVRRType_t vrrType) override;

        /* CompositeInEventNotification */

        void OnCompositeInHotPlug(dsCompositeInPort_t port, bool isConnected) override;
        void OnCompositeInSignalStatus(dsCompositeInPort_t port, dsCompInSignalStatus_t signalStatus) override;
        void OnCompositeInStatus(dsCompositeInPort_t activePort, bool isPresented) override;
        void OnCompositeInVideoModeUpdate(dsCompositeInPort_t activePort, dsVideoPortResolution_t videoResolution) override;

        virtual Core::hresult RegisterDevicesChangedNotification(Exchange::IAVInput::IDevicesChangedNotification* notification) override;
        virtual Core::hresult UnregisterDevicesChangedNotification(Exchange::IAVInput::IDevicesChangedNotification* notification) override;
        virtual Core::hresult RegisterSignalChangedNotification(Exchange::IAVInput::ISignalChangedNotification* notification) override;
        virtual Core::hresult UnregisterSignalChangedNotification(Exchange::IAVInput::ISignalChangedNotification* notification) override;
        virtual Core::hresult RegisterInputStatusChangedNotification(Exchange::IAVInput::IInputStatusChangedNotification* notification) override;
        virtual Core::hresult UnregisterInputStatusChangedNotification(Exchange::IAVInput::IInputStatusChangedNotification* notification) override;
        virtual Core::hresult RegisterVideoStreamInfoUpdateNotification(Exchange::IAVInput::IVideoStreamInfoUpdateNotification* notification) override;
        virtual Core::hresult UnregisterVideoStreamInfoUpdateNotification(Exchange::IAVInput::IVideoStreamInfoUpdateNotification* notification) override;
        virtual Core::hresult RegisterGameFeatureStatusUpdateNotification(Exchange::IAVInput::IGameFeatureStatusUpdateNotification* notification) override;
        virtual Core::hresult UnregisterGameFeatureStatusUpdateNotification(Exchange::IAVInput::IGameFeatureStatusUpdateNotification* notification) override;
        virtual Core::hresult RegisterAviContentTypeUpdateNotification(Exchange::IAVInput::IAviContentTypeUpdateNotification* notification) override;
        virtual Core::hresult UnregisterAviContentTypeUpdateNotification(Exchange::IAVInput::IAviContentTypeUpdateNotification* notification) override;

        Core::hresult Configure(PluginHost::IShell* service) override;

        Core::hresult NumberOfInputs(uint32_t& numberOfInputs, bool& success) override;
        Core::hresult GetInputDevices(const string& typeOfInput, Exchange::IAVInput::IInputDeviceIterator*& devices, bool& success);
        Core::hresult WriteEDID(const string& portId, const string& message, SuccessResult& successResult) override;
        Core::hresult ReadEDID(const string& portId, string& EDID, bool& success) override;
        Core::hresult GetRawSPD(const string& portId, string& HDMISPD, bool& success) override;
        Core::hresult GetSPD(const string& portId, string& HDMISPD, bool& success) override;
        Core::hresult SetEdidVersion(const string& portId, const string& edidVersion, SuccessResult& successResult) override;
        Core::hresult GetEdidVersion(const string& portId, string& edidVersion, bool& success) override;
        Core::hresult SetEdid2AllmSupport(const string& portId, const bool allmSupport, SuccessResult& successResult) override;
        Core::hresult GetEdid2AllmSupport(const string& portId, bool& allmSupport, bool& success) override;
        Core::hresult SetVRRSupport(const string& portId, const bool vrrSupport, SuccessResult& successResult) override;
        Core::hresult GetVRRSupport(const string& portId, bool& vrrSupport, bool& success) override;
        Core::hresult GetHdmiVersion(const string& portId, string& HdmiCapabilityVersion, bool& success) override;
        Core::hresult SetMixerLevels(const int primaryVolume, const int inputVolume, SuccessResult& successResult) override;
        Core::hresult StartInput(const string& portId, const string& typeOfInput, const bool requestAudioMix, const int plane, const bool topMost, SuccessResult& successResult) override;
        Core::hresult StopInput(const string& typeOfInput, SuccessResult& successResult) override;
        Core::hresult SetVideoRectangle(const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h, const string& typeOfInput, SuccessResult& successResult) override;
        Core::hresult CurrentVideoMode(string& currentVideoMode, bool& success) override;
        Core::hresult ContentProtected(bool& isContentProtected, bool& success) override;
        Core::hresult GetSupportedGameFeatures(IStringIterator*& features, bool& success) override;
        Core::hresult GetGameFeatureStatus(const string& portId, const string& gameFeature, bool& mode, bool& success) override;
        Core::hresult GetVRRFrameRate(const string& portId, double& currentVRRVideoFrameRate, bool& success) override;
        Core::hresult getInputDevices(const string& typeOfInput, std::list<WPEFramework::Exchange::IAVInput::InputDevice>& inputDeviceList);

    private:

        mutable Core::CriticalSection _adminLock;
        PluginHost::IShell* _service;
        bool _registeredDsEventHandlers;

        template <typename T>
        T* baseInterface()
        {
            static_assert(std::is_base_of<T, AVInputImplementation>(), "base type mismatch");
            return static_cast<T*>(this);
        }

        template <typename T>
        uint32_t Register(std::list<T*>& list, T* notification);
        template <typename T>
        uint32_t Unregister(std::list<T*>& list, T* notification);

        std::list<Exchange::IAVInput::IDevicesChangedNotification*> _devicesChangedNotifications;
        std::list<Exchange::IAVInput::ISignalChangedNotification*> _signalChangedNotifications;
        std::list<Exchange::IAVInput::IInputStatusChangedNotification*> _inputStatusChangedNotifications;
        std::list<Exchange::IAVInput::IVideoStreamInfoUpdateNotification*> _videoStreamInfoUpdateNotifications;
        std::list<Exchange::IAVInput::IGameFeatureStatusUpdateNotification*> _gameFeatureStatusUpdateNotifications;
        std::list<Exchange::IAVInput::IAviContentTypeUpdateNotification*> _aviContentTypeUpdateNotifications;

        int m_primVolume;
        int m_inputVolume; // Player Volume

        void dispatchEvent(Event, const ParamsType params);
        void Dispatch(Event event, const ParamsType params);

        static string currentVideoMode(bool& success);

        bool getALLMStatus(int iPort);
        bool getVRRStatus(int iPort, dsHdmiInVrrStatus_t* vrrStatus);
        std::string GetRawSPD(int iPort);

        static void dsAVEventHandler(const char* owner, IARM_EventId_t eventId, void* data, size_t len);
        static void dsAVSignalStatusEventHandler(const char* owner, IARM_EventId_t eventId, void* data, size_t len);
        static void dsAVStatusEventHandler(const char* owner, IARM_EventId_t eventId, void* data, size_t len);
        static void dsAVVideoModeEventHandler(const char* owner, IARM_EventId_t eventId, void* data, size_t len);
        static void dsAVGameFeatureStatusEventHandler(const char* owner, IARM_EventId_t eventId, void* data, size_t len);
        static void dsAviContentTypeEventHandler(const char* owner, IARM_EventId_t eventId, void* data, size_t len);

        /* Notification utility methods */
        
        void AVInputHotplug(int input, int connect, int type);
        void AVInputSignalChange(int port, int signalStatus, int type);
        void AVInputStatusChange(int port, bool isPresented, int type);
        void AVInputVideoModeUpdate(int port, dsVideoPortResolution_t resolution, int type);
        void hdmiInputAviContentTypeChange(int port, int content_type);
        void AVInputALLMChange(int port, bool allm_mode);
        void AVInputVRRChange(int port, dsVRRType_t vrr_type, bool vrr_mode);
    };

} // namespace Plugin
} // namespace WPEFramework
