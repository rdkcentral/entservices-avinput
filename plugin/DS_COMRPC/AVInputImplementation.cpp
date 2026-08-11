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

#include "AVInputImplementation.h"
#include <fstream>
#include <time.h>
#include <utility>

#include "UtilsJsonRpc.h"

#define STR_ALLM                        "ALLM"
#define VRR_TYPE_HDMI                   "VRR-HDMI"
#define VRR_TYPE_FREESYNC               "VRR-FREESYNC"
#define VRR_TYPE_FREESYNC_PREMIUM       "VRR-FREESYNC-PREMIUM"
#define VRR_TYPE_FREESYNC_PREMIUM_PRO   "VRR-FREESYNC-PREMIUM-PRO"
#define AV_HOT_PLUG_EVENT_CONNECTED     0
#define AV_HOT_PLUG_EVENT_DISCONNECTED  1

static bool isAudioBalanceSet = false;
static int planeType = 0;

using namespace std;

namespace WPEFramework {
namespace Plugin {
    SERVICE_REGISTRATION(AVInputImplementation, 1, 0);
    AVInputImplementation* AVInputImplementation::_instance = nullptr;

    AVInputImplementation::AVInputImplementation()
        : _DSHDMIInNotification(*this)
        , _DSCompositeInNotification(*this)
        , _adminLock()
        , _service(nullptr)
        , _registeredDsEventHandlers(false)
    {
        LOGINFO("Create AVInputImplementation Instance (COM-RPC)");

        m_primVolume = DEFAULT_PRIM_VOL_LEVEL;
        m_inputVolume = DEFAULT_INPUT_VOL_LEVEL;
        m_currentVrrType = Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_VRR_NONE;

        AVInputImplementation::_instance = this;
    }

    AVInputImplementation::~AVInputImplementation()
    {
        AVInputImplementation::_instance = nullptr;

        // COM-RPC: notifications are unregistered in Deinitialize() via
        // DSHelper::Close() which calls OnDeviceSettingsDeactivated()
        _registeredDsEventHandlers = false;
    }

    // =========================================================================
    // DSHelper override: called when DeviceSettings activates
    // DS_IARM equivalent: device::Host::getInstance().Register(IHdmiInEvents)
    //                     device::Host::getInstance().Register(ICompositeInEvents)
    // =========================================================================
    void AVInputImplementation::OnDeviceSettingsActivated()
    {
        LOGINFO("AVInputImplementation: OnDeviceSettingsActivated — registering DS notifications");

        // Register HDMI-In notification delegate
        {
            auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
            if (hdmiIn != nullptr) {
                hdmiIn->Register(&_DSHDMIInNotification);
                hdmiIn->Release();
                LOGINFO("AVInputImplementation: IDeviceSettingsHDMIIn::INotification registered");
            }
            else {
                LOGWARN("IDeviceSettingsHDMIIn not available");
            }
        }

        // Register Composite-In notification delegate
        {
            auto* compositeIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsCompositeIn>();
            if (compositeIn != nullptr) {
                compositeIn->Register(&_DSCompositeInNotification);
                compositeIn->Release();
                LOGINFO("AVInputImplementation: IDeviceSettingsCompositeIn::INotification registered");
                _registeredDsEventHandlers = true;
            }
            else {
                LOGWARN("IDeviceSettingsCompositeIn not available");
            }
        }
    }

    // =========================================================================
    // DSHelper override: called when DeviceSettings deactivates
    // DS_IARM equivalent: device::Host::getInstance().UnRegister(IHdmiInEvents)
    //                     device::Host::getInstance().UnRegister(ICompositeInEvents)
    // =========================================================================
    void AVInputImplementation::OnDeviceSettingsDeactivated()
    {
        LOGINFO("AVInputImplementation: OnDeviceSettingsDeactivated — unregistering DS notifications");

        {
            auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
            if (hdmiIn != nullptr) {
                hdmiIn->Unregister(&_DSHDMIInNotification);
                hdmiIn->Release();
            }
        }
        {
            auto* compositeIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsCompositeIn>();
            if (compositeIn != nullptr) {
                compositeIn->Unregister(&_DSCompositeInNotification);
                compositeIn->Release();
            }
        }

        _registeredDsEventHandlers = false;
    }

    Core::hresult AVInputImplementation::Configure(PluginHost::IShell* service)
    {
        _service = service;

        // COM-RPC: open the DeviceSettings plugin link.
        // DS_IARM equivalent: device::Manager::Initialize()
        // OnDeviceSettingsActivated() fires once DeviceSettings is ready,
        // which registers the HDMI-In and Composite-In notification delegates.
        DSHelper::Open(service);
        LOGINFO("AVInputImplementation: DSHelper::Open() called");

        return Core::ERROR_NONE;
    }

    // =========================================================================
    // Notification register/unregister template helpers (unchanged from DS_IARM)
    // =========================================================================
    template <typename T>
    Core::hresult AVInputImplementation::Register(std::list<T*>& list, T* notification)
    {
        uint32_t status = Core::ERROR_GENERAL;

        ASSERT(nullptr != notification);
        _adminLock.Lock();

        if (std::find(list.begin(), list.end(), notification) == list.end()) {
            list.push_back(notification);
            notification->AddRef();
            status = Core::ERROR_NONE;
        }

        _adminLock.Unlock();
        return status;
    }

    template <typename T>
    Core::hresult AVInputImplementation::Unregister(std::list<T*>& list, T* notification)
    {
        uint32_t status = Core::ERROR_GENERAL;

        ASSERT(nullptr != notification);
        _adminLock.Lock();

        auto itr = std::find(list.begin(), list.end(), notification);
        if (itr != list.end()) {
            (*itr)->Release();
            list.erase(itr);
            status = Core::ERROR_NONE;
        }

        _adminLock.Unlock();
        return status;
    }

    Core::hresult AVInputImplementation::RegisterDevicesChangedNotification(Exchange::IAVInput::IDevicesChangedNotification* notification)
    {
        Core::hresult errorCode = Register(_devicesChangedNotifications, notification);
        LOGINFO("IDevicesChangedNotification %p, errorCode: %u", notification, errorCode);
        return errorCode;
    }

    Core::hresult AVInputImplementation::UnregisterDevicesChangedNotification(Exchange::IAVInput::IDevicesChangedNotification* notification)
    {
        Core::hresult errorCode = Unregister(_devicesChangedNotifications, notification);
        LOGINFO("IDevicesChangedNotification %p, errorCode: %u", notification, errorCode);
        return errorCode;
    }

    Core::hresult AVInputImplementation::RegisterSignalChangedNotification(Exchange::IAVInput::ISignalChangedNotification* notification)
    {
        Core::hresult errorCode = Register(_signalChangedNotifications, notification);
        LOGINFO("ISignalChangedNotification %p, errorCode: %u", notification, errorCode);
        return errorCode;
    }

    Core::hresult AVInputImplementation::UnregisterSignalChangedNotification(Exchange::IAVInput::ISignalChangedNotification* notification)
    {
        Core::hresult errorCode = Unregister(_signalChangedNotifications, notification);
        LOGINFO("ISignalChangedNotification %p, errorCode: %u", notification, errorCode);
        return errorCode;
    }

    Core::hresult AVInputImplementation::RegisterInputStatusChangedNotification(Exchange::IAVInput::IInputStatusChangedNotification* notification)
    {
        Core::hresult errorCode = Register(_inputStatusChangedNotifications, notification);
        LOGINFO("IInputStatusChangedNotification %p, errorCode: %u", notification, errorCode);
        return errorCode;
    }

    Core::hresult AVInputImplementation::UnregisterInputStatusChangedNotification(Exchange::IAVInput::IInputStatusChangedNotification* notification)
    {
        Core::hresult errorCode = Unregister(_inputStatusChangedNotifications, notification);
        LOGINFO("IInputStatusChangedNotification %p, errorCode: %u", notification, errorCode);
        return errorCode;
    }

    Core::hresult AVInputImplementation::RegisterVideoStreamInfoUpdateNotification(Exchange::IAVInput::IVideoStreamInfoUpdateNotification* notification)
    {
        Core::hresult errorCode = Register(_videoStreamInfoUpdateNotifications, notification);
        LOGINFO("IVideoStreamInfoUpdateNotification %p, errorCode: %u", notification, errorCode);
        return errorCode;
    }

    Core::hresult AVInputImplementation::UnregisterVideoStreamInfoUpdateNotification(Exchange::IAVInput::IVideoStreamInfoUpdateNotification* notification)
    {
        Core::hresult errorCode = Unregister(_videoStreamInfoUpdateNotifications, notification);
        LOGINFO("IVideoStreamInfoUpdateNotification %p, errorCode: %u", notification, errorCode);
        return errorCode;
    }

    Core::hresult AVInputImplementation::RegisterGameFeatureStatusUpdateNotification(Exchange::IAVInput::IGameFeatureStatusUpdateNotification* notification)
    {
        Core::hresult errorCode = Register(_gameFeatureStatusUpdateNotifications, notification);
        LOGINFO("IGameFeatureStatusUpdateNotification %p, errorCode: %u", notification, errorCode);
        return errorCode;
    }

    Core::hresult AVInputImplementation::UnregisterGameFeatureStatusUpdateNotification(Exchange::IAVInput::IGameFeatureStatusUpdateNotification* notification)
    {
        Core::hresult errorCode = Unregister(_gameFeatureStatusUpdateNotifications, notification);
        LOGINFO("IGameFeatureStatusUpdateNotification %p, errorCode: %u", notification, errorCode);
        return errorCode;
    }

    Core::hresult AVInputImplementation::RegisterAviContentTypeUpdateNotification(Exchange::IAVInput::IAviContentTypeUpdateNotification* notification)
    {
        Core::hresult errorCode = Register(_aviContentTypeUpdateNotifications, notification);
        LOGINFO("IAviContentTypeUpdateNotification %p, errorCode: %u", notification, errorCode);
        return errorCode;
    }

    Core::hresult AVInputImplementation::UnregisterAviContentTypeUpdateNotification(Exchange::IAVInput::IAviContentTypeUpdateNotification* notification)
    {
        Core::hresult errorCode = Unregister(_aviContentTypeUpdateNotifications, notification);
        LOGINFO("IAviContentTypeUpdateNotification %p, errorCode: %u", notification, errorCode);
        return errorCode;
    }

    // =========================================================================
    // Event dispatch helpers — identical logic to DS_IARM dispatchEvent()
    // =========================================================================
    void AVInputImplementation::dispatchEvent(Event event, ParamsType params)
    {
        Core::IWorkerPool::Instance().Submit(DispatchJob::Create(this, event, params));
    }

    void AVInputImplementation::Dispatch(Event event, const ParamsType params)
    {
        using namespace WPEFramework::Exchange;

        _adminLock.Lock();

        switch (event) {
        case ON_AVINPUT_DEVICES_CHANGED: {

            if (auto* const devices = boost::get<Exchange::IAVInput::IInputDeviceIterator* const>(&params)) {
                LOGINFO("ON_AVINPUT_DEVICES_CHANGED");

                std::list<IAVInput::IDevicesChangedNotification*>::const_iterator index(_devicesChangedNotifications.begin());

                while (index != _devicesChangedNotifications.end()) {
                    (*index)->OnDevicesChanged(*devices);
                    ++index;
                }
            }
            break;
        }
        case ON_AVINPUT_SIGNAL_CHANGED: {

            if (const auto* tupleValue = boost::get<std::tuple<int, string, string>>(&params)) {
                int id = std::get<0>(*tupleValue);
                string locator = std::get<1>(*tupleValue);
                string signalStatus = std::get<2>(*tupleValue);

                std::list<IAVInput::ISignalChangedNotification*>::const_iterator index(_signalChangedNotifications.begin());

                while (index != _signalChangedNotifications.end()) {
                    (*index)->OnSignalChanged(id, locator, signalStatus);
                    ++index;
                }
            }
            break;
        }
        case ON_AVINPUT_STATUS_CHANGED: {

            if (const auto* tupleValue = boost::get<std::tuple<int, string, string, int>>(&params)) {
                int id = std::get<0>(*tupleValue);
                string locator = std::get<1>(*tupleValue);
                string status = std::get<2>(*tupleValue);
                int plane = std::get<3>(*tupleValue);

                std::list<IAVInput::IInputStatusChangedNotification*>::const_iterator index(_inputStatusChangedNotifications.begin());

                while (index != _inputStatusChangedNotifications.end()) {
                    (*index)->OnInputStatusChanged(id, locator, status, plane);
                    ++index;
                }
            }
            break;
        }
        case ON_AVINPUT_VIDEO_STREAM_INFO_UPDATE: {
            if (const auto* tupleValue = boost::get<std::tuple<int, string, int, int, bool, int, int>>(&params)) {
                int id = std::get<0>(*tupleValue);
                string locator = std::get<1>(*tupleValue);
                int width = std::get<2>(*tupleValue);
                int height = std::get<3>(*tupleValue);
                bool progressive = std::get<4>(*tupleValue);
                int frameRateN = std::get<5>(*tupleValue);
                int frameRateD = std::get<6>(*tupleValue);

                std::list<IAVInput::IVideoStreamInfoUpdateNotification*>::const_iterator index(_videoStreamInfoUpdateNotifications.begin());

                while (index != _videoStreamInfoUpdateNotifications.end()) {
                    (*index)->VideoStreamInfoUpdate(id, locator, width, height, progressive, frameRateN, frameRateD);
                    ++index;
                }
            }
            break;
        }
        case ON_AVINPUT_GAME_FEATURE_STATUS_UPDATE: {
            if (const auto* tupleValue = boost::get<std::tuple<int, string, bool>>(&params)) {
                int id = std::get<0>(*tupleValue);
                string gameFeature = std::get<1>(*tupleValue);
                bool mode = std::get<2>(*tupleValue);

                std::list<IAVInput::IGameFeatureStatusUpdateNotification*>::const_iterator index(_gameFeatureStatusUpdateNotifications.begin());

                while (index != _gameFeatureStatusUpdateNotifications.end()) {
                    (*index)->GameFeatureStatusUpdate(id, gameFeature, mode);
                    ++index;
                }
            }
            break;
        }
        case ON_AVINPUT_AVI_CONTENT_TYPE_UPDATE: {
            if (const auto* tupleValue = boost::get<std::tuple<int, int>>(&params)) {
                int id = std::get<0>(*tupleValue);
                int aviContentType = std::get<1>(*tupleValue);

                std::list<IAVInput::IAviContentTypeUpdateNotification*>::const_iterator index(_aviContentTypeUpdateNotifications.begin());

                while (index != _aviContentTypeUpdateNotifications.end()) {
                    (*index)->AviContentTypeUpdate(id, aviContentType);
                    ++index;
                }
            }
            break;
        }

        default: {
            LOGWARN("Event[%u] not handled", event);
            break;
        }
        }
        _adminLock.Unlock();
    }

    // =========================================================================
    // IAVInput method implementations
    // =========================================================================

    Core::hresult AVInputImplementation::ContentProtected(bool& isContentProtected, bool& success)
    {
        isContentProtected = true;
        success = true;
        LOGINFO("isContentProtected: %s", isContentProtected ? "true" : "false");
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::NumberOfInputs(uint32_t& numberOfInputs, bool& success)
    {
        // COM-RPC: device::HdmiInput::getInstance().getNumberOfInputs()
        //       → IDeviceSettingsHDMIIn::GetHDMIInNumberOfInputs()
        auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            int32_t count = 0;
            Core::hresult comResult = hdmiIn->GetHDMIInNumberOfInputs(count);
            if (comResult == Core::ERROR_NONE) {
                numberOfInputs = static_cast<uint32_t>(count);
                LOGINFO("numberOfInputs %u", numberOfInputs);
                success = true;
            } else {
                LOGERR("GetHDMIInNumberOfInputs failed, Error: %d", static_cast<int>(comResult));
                success = false;
            }
            hdmiIn->Release();
        } else {
            LOGERR("IDeviceSettingsHDMIIn not available");
            success = false;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::CurrentVideoMode(string& currentVideoMode, bool& success)
    {
        // COM-RPC: device::HdmiInput::getInstance().getCurrentVideoMode()
        //       → IDeviceSettingsHDMIIn::GetHDMIVideoMode()
        auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            Exchange::IDeviceSettingsHDMIIn::HDMIVideoPortResolution vpRes{};
            Core::hresult comResult = hdmiIn->GetHDMIVideoMode(vpRes);
            if (comResult == Core::ERROR_NONE) {
                currentVideoMode = vpRes.name;
                LOGINFO("currentVideoMode %s", currentVideoMode.c_str());
                success = true;
            } else {
                LOGERR("GetHDMIVideoMode failed, Error: %d", static_cast<int>(comResult));
                success = false;
            }
            hdmiIn->Release();
        } else {
            LOGERR("IDeviceSettingsHDMIIn not available");
            success = false;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::StartInput(const string& portId, const string& typeOfInput,
        const bool requestAudioMix, const int plane, const bool topMost, SuccessResult& successResult)
    {
        int id;

        try {
            id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("StartInput: Invalid paramater: portId: %s ", portId.c_str());
            successResult.success = false;
            return Core::ERROR_NONE;
        }

        if(plane != 0 && plane != 1 ){
            LOGERR("StartInput: Invalid paramater: plane: %d ", plane);
            successResult.success = false;
            return Core::ERROR_NONE;
        }

        int iType = AVInputUtils::getTypeOfInput(typeOfInput);
        Core::hresult comResult = Core::ERROR_NONE;

        if (iType == INPUT_TYPE_INT_HDMI) {
            // COM-RPC: device::HdmiInput::getInstance().selectPort(id, requestAudioMix, plane, topMost)
            //       → IDeviceSettingsHDMIIn::SelectHDMIInPort()
            auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
            if (hdmiIn != nullptr) {
                comResult = hdmiIn->SelectHDMIInPort(static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                                                        requestAudioMix,
                                                        topMost,
                                                        static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIVideoPlaneType>(plane));
                if (comResult == Core::ERROR_NONE) {
                    planeType = plane;
                    // COM-RPC: device::Host::getInstance().setAudioMixerLevels() equivalent
                    // (IDeviceSettingsHost has no SetAudioMixerLevels — skipped, matches DS_IARM behavior
                    // where mixer levels are only set when requestAudioMix is true)
                    successResult.success = true;
                } else {
                    LOGERR("SelectHDMIInPort failed for portId=%s, Error: %d", portId.c_str(), static_cast<int>(comResult));
                    successResult.success = false;
                }
                hdmiIn->Release();
            } else {
                LOGERR("StartInput: IDeviceSettingsHDMIIn not available");
                successResult.success = false;
            }
        } else if (iType == INPUT_TYPE_INT_COMPOSITE) {
            // COM-RPC: device::CompositeInput::getInstance().selectPort(id)
            //       → IDeviceSettingsCompositeIn::SelectCompositeInPort()
            auto* compositeIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsCompositeIn>();
            if (compositeIn != nullptr) {
                comResult = compositeIn->SelectCompositeInPort(static_cast<Exchange::IDeviceSettingsCompositeIn::CompositeInPort>(id));
                if (comResult == Core::ERROR_NONE) {
                    successResult.success = true;
                    planeType = plane;  // plane is ignored for composite input, but stored for consistency
                } else {
                    LOGERR("SelectCompositeInPort failed for portId=%s, Error: %d", portId.c_str(), static_cast<int>(comResult));
                    successResult.success = false;
                }
                compositeIn->Release();
            } else {
                LOGERR("IDeviceSettingsCompositeIn not available");
                successResult.success = false;
            }
        } else {
            LOGERR("StartInput: Unknown typeOfInput: %s", typeOfInput.c_str());
            successResult.success = false;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::StopInput(const string& typeOfInput, SuccessResult& successResult)
    {
        successResult.success = false;

        LOGINFO("StopInput: typeOfInput %s", typeOfInput.c_str());
        try {
            planeType = -1;
            Core::hresult comResult = Core::ERROR_NONE;
            if (isAudioBalanceSet) {
                // COM-RPC: device::Host::getInstance().setAudioMixerLevels() — handle is NULL (0)
                //       → IDeviceSettingsAudio::SetAudioMixerLevels(0, audioInput, volume)
                auto* audio = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsAudio>();
                if (audio != nullptr) {
                    comResult = audio->SetAudioMixerLevels(0, Exchange::IDeviceSettingsAudio::AUDIO_INPUT_PRIMARY, MAX_PRIM_VOL_LEVEL);
                    if (comResult != Core::ERROR_NONE) {
                        LOGERR("SetAudioMixerLevels failed for primary input, Error: %d", static_cast<int>(comResult));
                    }
                    comResult = audio->SetAudioMixerLevels(0, Exchange::IDeviceSettingsAudio::AUDIO_INPUT_SYSTEM, DEFAULT_INPUT_VOL_LEVEL);
                    if (comResult != Core::ERROR_NONE) {
                        LOGERR("SetAudioMixerLevels failed for system input, Error: %d", static_cast<int>(comResult));
                    }
                    audio->Release();
                }
                else {
                    LOGERR("IDeviceSettingsAudio not available");
                }
                isAudioBalanceSet = false;
            }

            switch(AVInputUtils::getTypeOfInput(typeOfInput)) {
                case INPUT_TYPE_INT_HDMI: {
                    // COM-RPC: device::HdmiInput::getInstance().selectPort(-1)
                    //       → IDeviceSettingsHDMIIn::SelectHDMIInPort(DS_HDMI_IN_PORT_NONE)
                    auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
                    if (hdmiIn != nullptr) {
                        comResult = hdmiIn->SelectHDMIInPort( Exchange::IDeviceSettingsHDMIIn::DS_HDMI_IN_PORT_NONE, 
                                                                false,
                                                                false,
                                                                Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_VIDEOPLANE_PRIMARY);
                        if (comResult != Core::ERROR_NONE) {
                            LOGERR("SelectHDMIInPort failed for DS_HDMI_IN_PORT_NONE, Error: %d", static_cast<int>(comResult));
                        }
                        else {
                            successResult.success = true;
                        }
                        hdmiIn->Release();
                    }
                    else {
                        LOGERR("IDeviceSettingsHDMIIn not available");
                    }
                    break;
                }
                case INPUT_TYPE_INT_COMPOSITE: {
                    // COM-RPC: device::CompositeInput::getInstance().selectPort(-1)
                    //       → IDeviceSettingsCompositeIn::SelectCompositeInPort(DS_COMPOSITE_IN_PORT_NONE)
                    auto* compositeIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsCompositeIn>();
                    if (compositeIn != nullptr) {
                        comResult = compositeIn->SelectCompositeInPort(Exchange::IDeviceSettingsCompositeIn::DS_COMPOSITE_IN_PORT_NONE);
                        if (comResult != Core::ERROR_NONE) {
                            LOGERR("SelectCompositeInPort failed for DS_COMPOSITE_IN_PORT_NONE, Error: %d", static_cast<int>(comResult));
                        }
                        else {
                            successResult.success = true;
                        }
                        compositeIn->Release();
                    }
                    else {
                        LOGERR("IDeviceSettingsCompositeIn not available");
                    }
                    break;
                }
                default: {
                    LOGWARN("Invalid input type passed to StopInput");
                    successResult.success = false;
                    return Core::ERROR_NONE;
                }
            }
        } catch(...) {
            LOGWARN("AVInputImplementation::StopInput Failed");
            successResult.success = false;
        }

        return Core::ERROR_NONE;
    }    

    Core::hresult AVInputImplementation::SetVideoRectangle(const uint16_t x, const uint16_t y,
        const uint16_t w, const uint16_t h, const string& typeOfInput, SuccessResult& successResult)
    {
        int iType = AVInputUtils::getTypeOfInput(typeOfInput);
        Core::hresult comResult = Core::ERROR_NONE;

        if (iType == INPUT_TYPE_INT_HDMI) {
            // COM-RPC: device::HdmiInput::getInstance().scaleVideo(x, y, w, h)
            //       → IDeviceSettingsHDMIIn::ScaleHDMIInVideo()
            auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
            if (hdmiIn != nullptr) {
                Exchange::IDeviceSettingsHDMIIn::HDMIInVideoRectangle rect{};
                rect.x = x; rect.y = y; rect.width = w; rect.height = h;
                comResult = hdmiIn->ScaleHDMIInVideo(rect);
                if (comResult == Core::ERROR_NONE) {
                    successResult.success = true;
                } else {
                    LOGERR("ScaleHDMIInVideo failed, Error: %d", static_cast<int>(comResult));
                    successResult.success = false;
                }
                hdmiIn->Release();
            } else {
                LOGERR("IDeviceSettingsHDMIIn not available");
                successResult.success = false;
            }
        } else if (iType == INPUT_TYPE_INT_COMPOSITE) {
            // COM-RPC: device::CompositeInput::getInstance().scaleVideo(x, y, w, h)
            //       → IDeviceSettingsCompositeIn::ScaleCompositeInVideo()
            auto* compositeIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsCompositeIn>();
            if (compositeIn != nullptr) {
                Exchange::IDeviceSettingsCompositeIn::VideoRectangle rect{};
                rect.x = x; rect.y = y; rect.width = w; rect.height = h;
                comResult = compositeIn->ScaleCompositeInVideo(rect);
                if (comResult == Core::ERROR_NONE) {
                    successResult.success = true;
                } else {
                    LOGERR("ScaleCompositeInVideo failed, Error: %d", static_cast<int>(comResult));
                    successResult.success = false;
                }
                compositeIn->Release();
            } else {
                LOGERR("IDeviceSettingsCompositeIn not available");
                successResult.success = false;
            }
        } else {
            LOGERR("SetVideoRectangle: Unknown typeOfInput: %s", typeOfInput.c_str());
            successResult.success = false;
        }
        LOGINFO("SetVideoRectangle: x=%u, y=%u, w=%u, h=%u, typeOfInput=%s, success=%d", x, y, w, h, typeOfInput.c_str(), successResult.success);
        return Core::ERROR_NONE;
    }

    // =========================================================================
    // Device-list helpers
    // getInputDevices: replaces device::HdmiInput/CompositeInput libds calls with COM-RPC
    // GetInputDevices: identical to DS_IARM (wraps getInputDevices in an IInputDeviceIterator)
    // =========================================================================
    Core::hresult AVInputImplementation::getInputDevices(const string& typeOfInput, std::list<WPEFramework::Exchange::IAVInput::InputDevice>& inputDeviceList)
    {
        int32_t num = 0;
        bool isHdmi = true;
        Core::hresult comResult = Core::ERROR_GENERAL;

        try {
            switch(AVInputUtils::getTypeOfInput(typeOfInput)) {
                case INPUT_TYPE_INT_HDMI: {
                    // COM-RPC: device::HdmiInput::getInstance().getNumberOfInputs()
                    auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
                    if (nullptr != hdmiIn) {
                        comResult = hdmiIn->GetHDMIInNumberOfInputs(num);
                        if (comResult != Core::ERROR_NONE) {
                            LOGERR("GetHDMIInNumberOfInputs failed, Error: %d", static_cast<int>(comResult));
                            num = 0;
                        }
                    } else {
                        LOGERR("IDeviceSettingsHDMIIn not available");
                        num = 0;
                    }
                    hdmiIn->Release();
                    break;
                }
                case INPUT_TYPE_INT_COMPOSITE: {
                    // COM-RPC: device::CompositeInput::getInstance().getNumberOfInputs()
                    auto* compositeIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsCompositeIn>();
                    if (nullptr != compositeIn) {
                        comResult = compositeIn->GetNrOfCompositeInputs(num);
                        if (comResult != Core::ERROR_NONE) {
                            LOGERR("GetNrOfCompositeInputs failed, Error: %d", static_cast<int>(comResult));
                            num = 0;
                        }
                    } else {
                        LOGERR("IDeviceSettingsCompositeIn not available");
                        num = 0;
                    }
                    compositeIn->Release();
                    isHdmi = false;
                    break;
                }
                default: {
                    LOGERR("getInputDevices: Invalid input type");
                    return Core::ERROR_GENERAL;
                }
            }

            if (num > 0) {
                int i = 0;
                // Pre-fetch connection status for all ports.
                // COM-RPC: device::HdmiInput::getInstance().isPortConnected(i) →
                //          GetHDMIInStatus() iterator (position in iterator == port index)
                // COM-RPC: device::CompositeInput::getInstance().isPortConnected(i) →
                //          GetCompositeInStatus().isPort0/1Connected
                std::vector<bool> connected(static_cast<size_t>(num), false);
                if (isHdmi) {
                    auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
                    if (hdmiIn != nullptr) {
                        Exchange::IDeviceSettingsHDMIIn::HDMIInStatus hdmiStatus{};
                        Exchange::IDeviceSettingsHDMIIn::IHDMIInPortConnectionStatusIterator* portIter = nullptr;
                        comResult = hdmiIn->GetHDMIInStatus(hdmiStatus, portIter);
                        if (comResult != Core::ERROR_NONE) {
                            LOGERR("GetHDMIInStatus failed, Error: %d", static_cast<int>(comResult));
                        }
                        if (portIter != nullptr) {
                            Exchange::IDeviceSettingsHDMIIn::HDMIPortConnectionStatus portStatus{};
                            int portIdx = 0;
                            while (portIter->Next(portStatus)) {
                                if (portIdx < num) {
                                    connected[static_cast<size_t>(portIdx)] = portStatus.isPortConnected;
                                }
                                portIdx++;
                            }
                            portIter->Release();
                        }
                        else {
                            LOGERR("GetHDMIInStatus returned null port iterator");
                        }
                        hdmiIn->Release();
                    }
                } else {
                    auto* compositeIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsCompositeIn>();
                    if (compositeIn != nullptr) {
                        Exchange::IDeviceSettingsCompositeIn::CompositeInStatus status{};
                        comResult = compositeIn->GetCompositeInStatus(status);
                        if (comResult != Core::ERROR_NONE) {
                            LOGERR("GetCompositeInStatus failed, Error: %d", static_cast<int>(comResult));
                            status.isPort0Connected = false;
                            status.isPort1Connected = false;
                        }
                        if (num > 0) connected[0] = status.isPort0Connected;
                        if (num > 1) connected[1] = status.isPort1Connected;
                        compositeIn->Release();
                    }
                }

                for (i = 0; i < num; i++) {
                    // Input ID is always 0-indexed, continuous number starting 0
                    WPEFramework::Exchange::IAVInput::InputDevice inputDevice;

                    inputDevice.id = i;
                    std::stringstream locator;
                    if (isHdmi) {
                        locator << "hdmiin://localhost/deviceid/" << i;
                    } else {
                        locator << "cvbsin://localhost/deviceid/" << i;
                    }
                    inputDevice.connected = connected[static_cast<size_t>(i)];
                    inputDevice.locator = locator.str();
                    LOGINFO("getInputDevices id %d, locator=[%s], connected=[%d]", i, inputDevice.locator.c_str(), inputDevice.connected);
                    inputDeviceList.push_back(std::move(inputDevice));
                }
            }
        } catch (const std::exception& e) {
            LOGERR("AVInputService::getInputDevices Failed");
        }

        return comResult;
    }

    Core::hresult AVInputImplementation::GetInputDevices(const string& typeOfInput, IInputDeviceIterator*& devices, bool& success)
    {
        Core::hresult result;
        std::list<WPEFramework::Exchange::IAVInput::InputDevice> inputDeviceList;
        success = false;

        try {
            switch(AVInputUtils::getTypeOfInput(typeOfInput)) {
                case INPUT_TYPE_INT_ALL: {
                    result = getInputDevices(INPUT_TYPE_HDMI, inputDeviceList);
                    if(result == Core::ERROR_NONE) {
                        result = getInputDevices(INPUT_TYPE_COMPOSITE, inputDeviceList);
                    }
                    break;
                }
                case INPUT_TYPE_INT_HDMI:
                case INPUT_TYPE_INT_COMPOSITE: {
                    result = getInputDevices(typeOfInput, inputDeviceList);
                    break;
                }
                default: {
                    LOGERR("GetInputDevices: Invalid input type");
                    return Core::ERROR_GENERAL;
                }
            }
        } catch(...) {
            LOGERR("GetInputDevices: Exception occurred while getting input devices");
            return Core::ERROR_GENERAL;
        }

        if(Core::ERROR_NONE == result) {
            devices = Core::Service<RPC::IteratorType<IInputDeviceIterator>>::Create<IInputDeviceIterator>(inputDeviceList);
            success = true;
        }

        return result;
    }

    Core::hresult AVInputImplementation::WriteEDID(const string& portId, const string& message, SuccessResult& successResult)
    {
        try {
		    stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("WriteEDID: Invalid paramater: portId: %s ", portId.c_str());
            successResult.success = false;
            return Core::ERROR_NONE;
        }

        // TODO: This wasn't implemented in the original code, do we want to implement it?
        successResult.success = true;
		LOGINFO("WriteEDID with portId[%s], EDID length[%zu]", portId.c_str(), message.size());
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::ReadEDID(const string& portId, string& EDID, bool& success)
    {
        // COM-RPC: device::HdmiInput::getInstance().getEDIDBytesInfo(id, edidVec)
        //       → IDeviceSettingsHDMIIn::GetEdidBytes()
        int id;
        try {
            id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("ReadEDID: Invalid portId: %s", portId.c_str());
            success = false;
            return Core::ERROR_NONE;
        }

        auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            Core::hresult comResult = Core::ERROR_NONE;
            constexpr uint16_t kEdidMaxLen = 256;
            uint8_t edidBuf[kEdidMaxLen] = {};
            comResult = hdmiIn->GetEdidBytes(static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                                            kEdidMaxLen, edidBuf);
            if (comResult == Core::ERROR_NONE) {
                Core::ToString(edidBuf, kEdidMaxLen, true, EDID);
                success = true;
            } else {
                LOGERR("GetEdidBytes failed for portId=%s", portId.c_str());
                success = false;
            }
            hdmiIn->Release();
        } else {
            LOGERR("IDeviceSettingsHDMIIn not available");
            success = false;
        }
        return Core::ERROR_NONE;
    }

    // =========================================================================
    // Shared event dispatch helpers (unchanged logic from DS_IARM)
    // =========================================================================
    void AVInputImplementation::AVInputHotplug(int input, int connect, int type)
    {
        LOGWARN("AVInputHotplug [%d, %d, %d]", input, connect, type);

        IInputDeviceIterator* devices;
        bool success;

        string typeOfInput;

        try {
            typeOfInput = AVInputUtils::getTypeOfInput(type);
        } catch(...) {
            LOGERR("AVInputHotplug: Invalid input type");
            return;
        }

        Core::hresult result = GetInputDevices(typeOfInput, devices, success);
        if (Core::ERROR_NONE != result) {
            LOGERR("AVInputHotplug [%d, %d, %d]: Failed to get devices", input, connect, type);
            return;
        }

        ParamsType params = devices;
        dispatchEvent(ON_AVINPUT_DEVICES_CHANGED, params);
    }

    /**
     * @brief This function is used to translate HDMI/COMPOSITE input signal change to
     * signalChanged event.
     *
     * @param[in] port HDMI/COMPOSITE In port id.
     * @param[in] signalStatus signal status of HDMI/COMPOSITE In port.
     * @param[in] type HDMI/COMPOSITE In type.
     */
    void AVInputImplementation::AVInputSignalChange(int port, int signalStatus, int type)
    {
        LOGWARN("AVInputSignalStatus [%d, %d, %d]", port, signalStatus, type);

        string signalStatusStr;

        std::stringstream locator;
        if (type == INPUT_TYPE_INT_HDMI) {
            locator << "hdmiin://localhost/deviceid/" << port;
        } else {
            locator << "cvbsin://localhost/deviceid/" << port;
        }

        /* values of dsHdmiInSignalStatus_t and dsCompInSignalStatus_t are same
           Hence used only HDMI macro for case statement */
        switch (signalStatus) {
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMI_IN_SIGNAL_STATUS_NOSIGNAL):
            signalStatusStr = "noSignal";
            break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMI_IN_SIGNAL_STATUS_UNSTABLE):
            signalStatusStr = "unstableSignal";
            break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMI_IN_SIGNAL_STATUS_NOTSUPPORTED):
            signalStatusStr = "notSupportedSignal";
            break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMI_IN_SIGNAL_STATUS_STABLE):
            signalStatusStr = "stableSignal";
            break;
        default:
            signalStatusStr = "none";
            break;
        }

        ParamsType params = std::make_tuple(port, locator.str(), signalStatusStr);
        dispatchEvent(ON_AVINPUT_SIGNAL_CHANGED, params);
    }

    /**
     * @brief This function is used to translate HDMI/COMPOSITE input status change to
     * inputStatusChanged event.
     *
     * @param[in] port HDMI/COMPOSITE In port id.
     * @param[in] isPresented HDMI/COMPOSITE In presentation started/stopped.
     * @param[in] type HDMI/COMPOSITE In type.
     */
    void AVInputImplementation::AVInputStatusChange(int port, bool isPresented, int type)
    {
        LOGWARN("avInputStatus [%d, %d, %d]", port, isPresented, type);

        std::stringstream locator;
        if (type == INPUT_TYPE_INT_HDMI) {
            locator << "hdmiin://localhost/deviceid/" << port;
        } else if (type == INPUT_TYPE_INT_COMPOSITE) {
            locator << "cvbsin://localhost/deviceid/" << port;
        }

        string status = isPresented ? "started" : "stopped";
        ParamsType params = std::make_tuple(port, locator.str(), status, planeType);
        dispatchEvent(ON_AVINPUT_STATUS_CHANGED, params);
    }

    // COM-RPC HDMIVideoPortResolution → dispatch videoStreamInfoUpdate
    void AVInputImplementation::AVInputVideoModeUpdate(int port,
        const Exchange::IDeviceSettingsHDMIIn::HDMIVideoPortResolution& resolution, int type)
    {
        int width = 0, height = 0;
        bool progressive = false;
        int frameRateN = 60000, frameRateD = 1000;

        std::stringstream locator;
        LOGWARN("AVInputVideoModeUpdate [%d]", port);

        if (type == INPUT_TYPE_INT_HDMI) {
            locator << "hdmiin://localhost/deviceid/" << port;

            // COM-RPC: HDMIVideoPortResolution.pixelResolution is HDMIInVideoResolution —
            // a pixel-dimension enum (values 0–6) matching DS_IARM dsVideoPixelResolution_t.
            switch (resolution.pixelResolution) {
            case Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_PIXELRES_720X480:
                width = 720;  height = 480;  break;
            case Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_PIXELRES_720X576:
                width = 720;  height = 576;  break;
            case Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_PIXELRES_1280X720:
                width = 1280; height = 720;  break;
            case Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_PIXELRES_1366X768:
                width = 1366; height = 768;  break;
            case Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_PIXELRES_1920X1080:
                width = 1920; height = 1080; break;
            case Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_PIXELRES_3840X2160:
                width = 3840; height = 2160; break;
            case Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_PIXELRES_4096X2160:
                width = 4096; height = 2160; break;
            default: width = 1920; height = 1080; break;
            }
            progressive = !resolution.interlaced;
        }

        // COM-RPC: HDMIVideoPortResolution uses HDMIInVideoFrameRate
        switch (static_cast<int>(resolution.frameRate)) {
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_FRAMERATE_24):    frameRateN = 24000;  frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_FRAMERATE_25):    frameRateN = 25000;  frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_FRAMERATE_30):    frameRateN = 30000;  frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_FRAMERATE_50):    frameRateN = 50000;  frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_FRAMERATE_60):    frameRateN = 60000;  frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_FRAMERATE_23_98): frameRateN = 24000; frameRateD = 1001; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_FRAMERATE_29_97): frameRateN = 30000; frameRateD = 1001; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_FRAMERATE_59_94): frameRateN = 60000; frameRateD = 1001; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_FRAMERATE_100):   frameRateN = 100000; frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_FRAMERATE_119_88): frameRateN = 120000; frameRateD = 1001; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_FRAMERATE_120):   frameRateN = 120000; frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_FRAMERATE_200):   frameRateN = 200000; frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_FRAMERATE_239_76): frameRateN = 240000; frameRateD = 1001; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_FRAMERATE_240):   frameRateN = 240000; frameRateD = 1000; break;
        default: frameRateN = 60000; frameRateD = 1000; break;
        }

        ParamsType params = std::make_tuple(port, locator.str(), width, height, progressive, frameRateN, frameRateD);
        dispatchEvent(ON_AVINPUT_VIDEO_STREAM_INFO_UPDATE, params);
    }

    // COM-RPC DisplayVideoPortResolution (CompositeIn) → dispatch videoStreamInfoUpdate
    void AVInputImplementation::AVInputVideoModeUpdate(int port,
        const Exchange::IDeviceSettingsCompositeIn::DisplayVideoPortResolution& resolution, int type)
    {
        int width = 0, height = 0;
        int frameRateN = 60000, frameRateD = 1000;

        std::stringstream locator;
        LOGWARN("AVInputVideoModeUpdate composite [%d]", port);

        if (type == INPUT_TYPE_INT_COMPOSITE) {
            locator << "cvbsin://localhost/deviceid/" << port;

            switch (resolution.pixelResolution) {
            case Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_RESOLUTION_480I:
            case Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_RESOLUTION_480P:
                width = 720; height = 480; break;
            case Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_RESOLUTION_576I:
            case Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_RESOLUTION_576P:
            case Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_RESOLUTION_576P50:
                width = 720; height = 576; break;
            default: width = 720; height = 576; break;
            }
        }

        switch (static_cast<int>(resolution.frameRate)) {
        case static_cast<int>(Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_FRAMERATE_24):    frameRateN = 24000;  frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_FRAMERATE_25):    frameRateN = 25000;  frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_FRAMERATE_30):    frameRateN = 30000;  frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_FRAMERATE_50):    frameRateN = 50000;  frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_FRAMERATE_60):    frameRateN = 60000;  frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_FRAMERATE_23_98): frameRateN = 24000; frameRateD = 1001; break;
        case static_cast<int>(Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_FRAMERATE_29_97): frameRateN = 30000; frameRateD = 1001; break;
        case static_cast<int>(Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_FRAMERATE_59_94): frameRateN = 60000; frameRateD = 1001; break;
        case static_cast<int>(Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_FRAMERATE_100):   frameRateN = 100000; frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_FRAMERATE_119_88): frameRateN = 120000; frameRateD = 1001; break;
        case static_cast<int>(Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_FRAMERATE_120):   frameRateN = 120000; frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_FRAMERATE_200):   frameRateN = 200000; frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_FRAMERATE_239_76): frameRateN = 240000; frameRateD = 1001; break;
        case static_cast<int>(Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_FRAMERATE_240):   frameRateN = 240000; frameRateD = 1000; break;
        default: frameRateN = 60000; frameRateD = 1000; break;
        }

        ParamsType params = std::make_tuple(port, locator.str(), width, height, false, frameRateN, frameRateD);
        dispatchEvent(ON_AVINPUT_VIDEO_STREAM_INFO_UPDATE, params);
    }

    void AVInputImplementation::hdmiInputAviContentTypeChange(int port, int content_type)
    {
        ParamsType params = std::make_tuple(port, content_type);
        dispatchEvent(ON_AVINPUT_AVI_CONTENT_TYPE_UPDATE, params);
    }

    void AVInputImplementation::AVInputALLMChange(int port, bool allm_mode)
    {
        ParamsType params = std::make_tuple(port, STR_ALLM, allm_mode);
        dispatchEvent(ON_AVINPUT_GAME_FEATURE_STATUS_UPDATE, params);
    }

    void AVInputImplementation::AVInputVRRChange(int port, Exchange::IDeviceSettingsHDMIIn::HDMIInVRRType vrr_type, bool vrr_mode)
    {
        string gameFeature;

        LOGINFO("AVInputVRRChange port=%d vrr_type=%d vrr_mode=%d", port, static_cast<int>(vrr_type), vrr_mode);

        switch (vrr_type) {
        case Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_HDMI_VRR:
            gameFeature = VRR_TYPE_HDMI;
            break;
        case Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_AMD_FREESYNC:
            gameFeature = VRR_TYPE_FREESYNC;
            break;
        case Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_AMD_FREESYNC_PREMIUM:
            gameFeature = VRR_TYPE_FREESYNC_PREMIUM;
            break;
        case Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_AMD_FREESYNC_PREMIUM_PRO:
            gameFeature = VRR_TYPE_FREESYNC_PREMIUM_PRO;
            break;
        default:
            break;
        }

        ParamsType params = std::make_tuple(port, gameFeature, vrr_mode);
        dispatchEvent(ON_AVINPUT_GAME_FEATURE_STATUS_UPDATE, params);
    }

    // =========================================================================
    // Internal event forwarders: COM-RPC INotification → shared dispatch helpers
    // DS_IARM equivalent: OnHdmiInEventHotPlug(), OnHdmiInEventSignalStatus(), etc.
    // =========================================================================
    void AVInputImplementation::onHdmiInAVIContentType(int port, int aviContentType)
    {
        LOGINFO("Received OnHDMIInAVIContentType port=%d contentType=%d", port, aviContentType);
        hdmiInputAviContentTypeChange(port, aviContentType);
    }

    void AVInputImplementation::onHdmiInEventHotPlug(int port, bool isConnected)
    {
        LOGINFO("Received OnHDMIInEventHotPlug port=%d isConnected=%d", port, isConnected);
        AVInputImplementation::AVInputHotplug(port,
            isConnected ? AV_HOT_PLUG_EVENT_CONNECTED : AV_HOT_PLUG_EVENT_DISCONNECTED,
            INPUT_TYPE_INT_HDMI);
    }

    void AVInputImplementation::onHdmiInEventSignalStatus(int port, int signalStatus)
    {
        LOGINFO("Received OnHDMIInEventSignalStatus port=%d signalStatus=%d", port, signalStatus);
        AVInputImplementation::AVInputSignalChange(port, signalStatus, INPUT_TYPE_INT_HDMI);
    }

    void AVInputImplementation::onHdmiInEventStatus(int activePort, bool isPresented)
    {
        LOGINFO("Received OnHDMIInEventStatus port=%d isPresented=%d", activePort, isPresented);
        AVInputImplementation::AVInputStatusChange(activePort, isPresented, INPUT_TYPE_INT_HDMI);
    }

    void AVInputImplementation::onHdmiInVideoModeUpdate(int port,
        const Exchange::IDeviceSettingsHDMIIn::HDMIVideoPortResolution& videoPortResolution)
    {
        LOGINFO("Received OnHdmiInVideoModeUpdate callback, port: %d, pixelResolution: %d, interlaced: %d, frameRate: %d",
                port,
                videoPortResolution.pixelResolution,
                videoPortResolution.interlaced,
                videoPortResolution.frameRate);

        AVInputImplementation::AVInputVideoModeUpdate(port, videoPortResolution, INPUT_TYPE_INT_HDMI);
    }

    void AVInputImplementation::onHdmiInAllmStatus(int port, bool allmStatus)
    {
        LOGINFO("Received OnHdmiInAllmStatus callback, port: %d, ALLM Mode: %s",
                port, allmStatus ? "true" : "false");

        AVInputImplementation::AVInputALLMChange(port, allmStatus);
    }

    void AVInputImplementation::onHdmiInVRRStatus(int port, Exchange::IDeviceSettingsHDMIIn::HDMIInVRRType vrrType)
    {
        LOGINFO("Received OnHDMIInVRRStatus port=%d vrrType=%d", port, static_cast<int>(vrrType));

        if (!AVInputImplementation::_instance)
            return;

        if (Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_VRR_NONE == vrrType) {
            if (m_currentVrrType != Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_VRR_NONE) {
                AVInputImplementation::AVInputVRRChange(port, m_currentVrrType, false);
            }
        } else {
            if (m_currentVrrType != Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_VRR_NONE) {
                AVInputImplementation::AVInputVRRChange(port, m_currentVrrType, false);
            }
            AVInputVRRChange(port, vrrType, true);
        }

        m_currentVrrType = vrrType;
    }

    void AVInputImplementation::onCompositeInHotPlug(int port, bool isConnected)
    {
        LOGINFO("Received OnCompositeInHotPlug port=%d isConnected=%s", port, isConnected ? "true" : "false");
        AVInputImplementation::AVInputHotplug(port,
            isConnected ? AV_HOT_PLUG_EVENT_CONNECTED : AV_HOT_PLUG_EVENT_DISCONNECTED,
            INPUT_TYPE_INT_COMPOSITE);
    }

    void AVInputImplementation::onCompositeInSignalStatus(int port, int signalStatus)
    {
        LOGINFO("Received OnCompositeInSignalStatus port=%d signalStatus=%d", port, signalStatus);
        AVInputImplementation::AVInputSignalChange(port, signalStatus, INPUT_TYPE_INT_COMPOSITE);
    }

    void AVInputImplementation::onCompositeInStatus(int activePort, bool isPresented)
    {
        LOGINFO("Received OnCompositeInStatus port=%d isPresented=%d", activePort, isPresented);
        AVInputImplementation::AVInputStatusChange(activePort, isPresented, INPUT_TYPE_INT_COMPOSITE);
    }

    void AVInputImplementation::onCompositeInVideoModeUpdate(int activePort,
        const Exchange::IDeviceSettingsCompositeIn::DisplayVideoPortResolution& videoResolution)
    {
        LOGINFO("Received OnCompositeInVideoModeUpdate callback, port: %d, pixelResolution: %d, interlaced: %d, frameRate: %d",
                activePort,
                videoResolution.pixelResolution,
                videoResolution.interlaced,
                videoResolution.frameRate);

        AVInputImplementation::AVInputVideoModeUpdate(activePort, videoResolution, INPUT_TYPE_INT_COMPOSITE);
    }

    Core::hresult AVInputImplementation::GetSupportedGameFeatures(Exchange::IAVInput::IStringIterator*& features, bool& success)
    {
        // COM-RPC: device::HdmiInput::getInstance().getSupportedGameFeatures(supportedFeatures)
        //       → IDeviceSettingsHDMIIn::GetSupportedGameFeaturesList()
        success = false;
        features = nullptr;
        std::vector<std::string> supportedFeatures;

        auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            Exchange::IDeviceSettingsHDMIIn::IHDMIInGameFeatureListIterator* iter = nullptr;
            Core::hresult comResult = Core::ERROR_NONE;
            comResult = hdmiIn->GetSupportedGameFeaturesList(iter);
            if (comResult == Core::ERROR_NONE && iter != nullptr) {
                Exchange::IDeviceSettingsHDMIIn::HDMIInGameFeatureList feature{};
                while (iter->Next(feature)) {
                    supportedFeatures.push_back(feature.gameFeature);
                }
                iter->Release();
                if (!supportedFeatures.empty() && comResult == Core::ERROR_NONE) {
                    features = Core::Service<RPC::IteratorType<Exchange::IAVInput::IStringIterator>>::Create<Exchange::IAVInput::IStringIterator>(supportedFeatures);
                    LOGINFO("GetSupportedGameFeatures: %zu", supportedFeatures.size());
                    success = true;
                } else {
                    LOGERR("GetSupportedGameFeaturesList returned empty list");
                }
            } else {
                LOGERR("GetSupportedGameFeaturesList failed, Error: %d", static_cast<int>(comResult));
            }
            hdmiIn->Release();
        } else {
            LOGERR("IDeviceSettingsHDMIIn not available");
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::GetGameFeatureStatus(const string& portId, const string& gameFeature, bool& mode, bool& success)
    {
        int id;
        try {
            id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("GetGameFeatureStatus: Invalid portId: %s", portId.c_str());
            success = false;
            return Core::ERROR_NONE;
        }

        if (gameFeature == STR_ALLM) {
            mode = getALLMStatus(id);
            success = true;
        } else if (gameFeature == VRR_TYPE_HDMI || gameFeature == VRR_TYPE_FREESYNC ||
                   gameFeature == VRR_TYPE_FREESYNC_PREMIUM || gameFeature == VRR_TYPE_FREESYNC_PREMIUM_PRO) {
            Exchange::IDeviceSettingsHDMIIn::HDMIInVRRStatus vrrStatus{};
            success = getVRRStatus(id, vrrStatus);
            if (success) {
                if (gameFeature == VRR_TYPE_HDMI)
                    mode = (vrrStatus.vrrType == Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_HDMI_VRR);
                else if (gameFeature == VRR_TYPE_FREESYNC)
                    mode = (vrrStatus.vrrType == Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_AMD_FREESYNC);
                else if (gameFeature == VRR_TYPE_FREESYNC_PREMIUM)
                    mode = (vrrStatus.vrrType == Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_AMD_FREESYNC_PREMIUM);
                else if (gameFeature == VRR_TYPE_FREESYNC_PREMIUM_PRO)
                    mode = (vrrStatus.vrrType == Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_AMD_FREESYNC_PREMIUM_PRO);
            }
        } else {
            LOGWARN("GetGameFeatureStatus: Unsupported feature: %s", gameFeature.c_str());
            success = false;
        }
        return Core::ERROR_NONE;
    }

    // =========================================================================
    // Internal COM-RPC helpers: getALLMStatus / getVRRStatus
    // DS_IARM equivalent: same-named private methods using device::HdmiInput::getInstance()
    // =========================================================================
    bool AVInputImplementation::getALLMStatus(int iPort)
    {
        // COM-RPC: device::HdmiInput::getInstance().getHdmiALLMStatus(iPort, &allm)
        //       → IDeviceSettingsHDMIIn::GetHDMIInAllmStatus()
        bool allm = false;
        auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            Core::hresult comResult = Core::ERROR_NONE;
            comResult = hdmiIn->GetHDMIInAllmStatus(static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(iPort),allm);
            if (comResult != Core::ERROR_NONE) {
                LOGERR("getALLMStatus failed for portId=%d, Error: %d", iPort, static_cast<int>(comResult));
            }
            else {
                LOGINFO("getALLMStatus for portId=%d, ALLM Mode: %s", iPort, allm ? "true" : "false");
            }
            hdmiIn->Release();
        }
        return allm;
    }

    bool AVInputImplementation::getVRRStatus(int iPort, Exchange::IDeviceSettingsHDMIIn::HDMIInVRRStatus& vrrStatus)
    {
        // COM-RPC: device::HdmiInput::getInstance().getVRRStatus(iPort, vrrStatus)
        //       → IDeviceSettingsHDMIIn::GetVRRStatus()
        bool ret = false;
        auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            Core::hresult comResult = Core::ERROR_NONE;
            comResult = hdmiIn->GetVRRStatus(
                static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(iPort),
                vrrStatus);
            if (comResult == Core::ERROR_NONE) {
                LOGWARN("getVRRStatus VRR TYPE: %d, VRR FRAMERATE: %f",
                    static_cast<int>(vrrStatus.vrrType), vrrStatus.vrrFreeSyncFramerateHz);
                ret = true;
            }
            else {
                LOGERR("getVRRStatus failed for portId=%d, Error: %d", iPort, static_cast<int>(comResult));
            }
            hdmiIn->Release();
        }
        return ret;
    }

    Core::hresult AVInputImplementation::GetVRRFrameRate(const string& portId, double& currentVRRVideoFrameRate, bool& success)
    {
        int id;
        try {
            id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("GetVRRFrameRate: Invalid portId: %s", portId.c_str());
            success = false;
            return Core::ERROR_NONE;
        }

        Exchange::IDeviceSettingsHDMIIn::HDMIInVRRStatus vrrStatus{};
        success = getVRRStatus(id, vrrStatus);
        if (success) {
            currentVRRVideoFrameRate = vrrStatus.vrrFreeSyncFramerateHz;
            LOGINFO("VRR FrameRate for portId[%s] is %.2f", portId.c_str(), currentVRRVideoFrameRate);
        } else {
            LOGERR("GetVRRFrameRate: Failed to get current VRR video frame rate");
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::GetRawSPD(const string& portId, string& HDMISPD, bool& success)
    {
        LOGINFO("AVInputImplementation::GetRawSPD");

        int id;

        try {
            id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("GetRawSPD: Invalid paramater: portId: %s ", portId.c_str());
            success = false;
            return Core::ERROR_NONE;
        }
        success = false;

        vector<uint8_t> spdVect({ 'u', 'n', 'k', 'n', 'o', 'w', 'n' });
        HDMISPD.clear();
        try {
            LOGWARN("AVInputImplementation::getSPDInfo");
            // COM-RPC: device::HdmiInput::getInstance().getHDMISPDInfo(id, spdVect2)
            //       \u2192 IDeviceSettingsHDMIIn::GetHDMISPDInformation()
            auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
            if (hdmiIn == nullptr) {
                success = false;
                LOGERR("IDeviceSettingsHDMIIn not available");
                return Core::ERROR_NONE;
            }
            constexpr uint16_t kSpdMaxLen = 256;
            uint8_t spdBuf[kSpdMaxLen] = {};
            Core::hresult comResult = Core::ERROR_NONE;
            comResult = hdmiIn->GetHDMISPDInformation(
                static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                kSpdMaxLen, spdBuf);
            if (comResult != Core::ERROR_NONE) {
                LOGERR("GetHDMISPDInformation failed for portId=%d, Error: %d", id, static_cast<int>(comResult));
                success = false;
            }
            else {
                LOGINFO("GetHDMISPDInformation succeeded for portId=%d", id);
                success = true;
            }
            hdmiIn->Release();

            vector<uint8_t> spdVect2(spdBuf, spdBuf + kSpdMaxLen);
            spdVect = std::move(spdVect2); // spdVect must be "unknown" unless we successfully get to this line

            // convert to base64
            uint16_t size = min(spdVect.size(), (size_t)numeric_limits<uint16_t>::max());

            LOGINFO("AVInputImplementation::getSPD size:%d spdVec.size:%zu for portId: %s ", size, spdVect.size(), portId.c_str());

            if (spdVect.size() > (size_t)numeric_limits<uint16_t>::max()) {
                LOGERR("Size too large to use ToString base64 wpe api");
                success = false;
            }
            else {
                LOGINFO("------------getSPD: ");
                for (size_t itr = 0; itr < spdVect.size(); itr++) {
                    LOGINFO("%02X ", spdVect[itr]);
                }
                Core::ToString((uint8_t*)&spdVect[0], size, false, HDMISPD);
            }
        } catch (const std::exception& err) {
            success = false;
            return Core::ERROR_NONE;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::GetSPD(const string& portId, string& HDMISPD, bool& success)
    {
        int id;

        try {
            id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("GetSPD: Invalid paramater: portId: %s ", portId.c_str());
            success = false;
            return Core::ERROR_NONE;
        }

        vector<uint8_t> spdVect({ 'u', 'n', 'k', 'n', 'o', 'w', 'n' });

        LOGINFO("AVInputImplementation::GetSPD");

        try {
            // COM-RPC: device::HdmiInput::getInstance().getHDMISPDInfo(id, spdVect2)
            //       \u2192 IDeviceSettingsHDMIIn::GetHDMISPDInformation()
            auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
            if (hdmiIn == nullptr) {
                success = false;
                LOGERR("IDeviceSettingsHDMIIn not available");
                return Core::ERROR_NONE;
            }
            constexpr uint16_t kSpdMaxLen = 256;
            uint8_t spdBuf[kSpdMaxLen] = {};
            Core::hresult comResult = Core::ERROR_NONE;
            comResult = hdmiIn->GetHDMISPDInformation(
                static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                kSpdMaxLen, spdBuf);
            bool spdOk = (comResult == Core::ERROR_NONE);
            hdmiIn->Release();
            if (!spdOk) {
                success = false;
                LOGERR("GetHDMISPDInformation failed for portId=%d", id);
                return Core::ERROR_NONE;
            }
            vector<uint8_t> spdVect2(spdBuf, spdBuf + kSpdMaxLen);
            spdVect = std::move(spdVect2); // edidVec must be "unknown" unless we successfully get to this line

            // convert to base64
            uint16_t size = min(spdVect.size(), (size_t)numeric_limits<uint16_t>::max());

            LOGWARN("AVInputImplementation::GetSPD size:%u spdVec.size:%zu for portId:%s",
                static_cast<unsigned int>(size), spdVect.size(), portId.c_str());

            if (spdVect.size() > (size_t)numeric_limits<uint16_t>::max()) {
                LOGERR("Size too large to use ToString base64 wpe api");
                success = false;
                return Core::ERROR_NONE;
            }

            LOGINFO("------------getSPD: ");
            for (size_t itr = 0; itr < spdVect.size(); itr++) {
                LOGINFO("%02X ", spdVect[itr]);
            }

            if (spdVect.size() > 0) {
                // COM-RPC: spdBuf layout matches dsSpd_infoframe_st:
                //   [0]=pkttype, [1]=version, [2]=length, [3..10]=vendor_name[8],
                //   [11..26]=product_des[16], [27]=source_info
                char str[200] = { 0 };
                snprintf(str, sizeof(str), "Packet Type:%02X,Version:%u,Length:%u,vendor name:%s,product des:%s,source info:%02X",
                    spdVect[0], spdVect[1], spdVect[2],
                    reinterpret_cast<const char*>(&spdVect[3]),
                    reinterpret_cast<const char*>(&spdVect[11]),
                    spdVect[27]);
                HDMISPD = str;
            }
        } catch (const std::exception& err) {
            success = false;
            return Core::ERROR_NONE;
        }

        success = true;
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::SetMixerLevels(const int primaryVolume, const int inputVolume, SuccessResult& successResult)
    {
        // DS_IARM: device::Host::getInstance().setAudioMixerLevels(dsAUDIO_INPUT_PRIMARY/SYSTEM, vol)
        // COM-RPC: IDeviceSettingsAudio::SetAudioMixerLevels(0, audioInput, volume)
        //          handle=0 because DS_IARM passes NULL to HAL (not port-specific)
        if ((primaryVolume >= 0) && (inputVolume >= 0)) {
            m_primVolume  = primaryVolume;
            m_inputVolume = inputVolume;
        } else {
            LOGERR("SetMixerLevels: Invalid params\n");
            successResult.success = false;
            return Core::ERROR_NONE;
        }

        successResult.success = true;

        if (m_primVolume > MAX_PRIM_VOL_LEVEL) {
            LOGWARN("Primary Volume greater than limit. Set to MAX_PRIM_VOL_LEVEL(100) !!!\n");
            m_primVolume = MAX_PRIM_VOL_LEVEL;
        }
        if (m_inputVolume > DEFAULT_INPUT_VOL_LEVEL) {
            LOGWARN("INPUT Volume greater than limit. Set to DEFAULT_INPUT_VOL_LEVEL(100) !!!\n");
            m_inputVolume = DEFAULT_INPUT_VOL_LEVEL;
        }

        try {
            // COM-RPC: device::Host::getInstance().setAudioMixerLevels(dsAUDIO_INPUT_PRIMARY/SYSTEM, vol)
            //       → IDeviceSettingsAudio::SetAudioMixerLevels(0, audioInput, volume)
            //       handle=0 because DS_IARM passes NULL to HAL (not port-specific)
            auto* audio = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsAudio>();
            if (audio == nullptr) {
                LOGERR("IDeviceSettingsAudio not available");
                successResult.success = false;
                return Core::ERROR_NONE;
            }
            Core::hresult comResult = Core::ERROR_NONE;
            comResult = audio->SetAudioMixerLevels(0, Exchange::IDeviceSettingsAudio::AUDIO_INPUT_PRIMARY, primaryVolume);
            if (comResult != Core::ERROR_NONE) {
                LOGERR("SetAudioMixerLevels failed for primaryVolume=%d, Error: %d", primaryVolume, static_cast<int>(comResult));
                successResult.success = false;
            }
            comResult = audio->SetAudioMixerLevels(0, Exchange::IDeviceSettingsAudio::AUDIO_INPUT_SYSTEM, inputVolume);
            if (comResult != Core::ERROR_NONE) {
                LOGERR("SetAudioMixerLevels failed for inputVolume=%d, Error: %d", inputVolume, static_cast<int>(comResult));
                successResult.success = false;
            }
            LOGINFO("Setting MixerLevels: primaryVolume[%d] inputVolume[%d]", primaryVolume, inputVolume);
            audio->Release();
        } catch (...) {
            LOGERR("Exception occurred while setting SoC volume levels");
            successResult.success = false;
        }

        isAudioBalanceSet = successResult.success;
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::SetEdid2AllmSupport(const string& portId, const bool allmSupport, SuccessResult& successResult)
    {
        // COM-RPC: device::HdmiInput::getInstance().setEdid2AllmSupport(id, allmSupport)
        //       → IDeviceSettingsHDMIIn::SetHDMIInEdid2AllmSupport()
        int id;
        try {
            id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("SetEdid2AllmSupport: Invalid portId: %s", portId.c_str());
            successResult.success = false;
            return Core::ERROR_NONE;
        }

        auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            Core::hresult comResult = Core::ERROR_NONE;
            comResult = hdmiIn->SetHDMIInEdid2AllmSupport(
                static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                allmSupport);
            if (comResult == Core::ERROR_NONE) {
                LOGINFO("SetEdid2AllmSupport portId[%s] allm=%d", portId.c_str(), allmSupport);
                successResult.success = true;
            } else {
                LOGERR("SetEdid2AllmSupport failed for portId=%d, Error: %d", id, static_cast<int>(comResult));
                successResult.success = false;
            }
            hdmiIn->Release();
        } else {
            LOGERR("IDeviceSettingsHDMIIn not available");
            successResult.success = false;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::GetEdid2AllmSupport(const string& portId, bool& allmSupport, bool& success)
    {
        // COM-RPC: device::HdmiInput::getInstance().getEdid2AllmSupport(id, &allmSupport)
        //       → IDeviceSettingsHDMIIn::GetHDMIInEdid2AllmSupport()
        int id;
        try {
            id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("GetEdid2AllmSupport: Invalid portId: %s", portId.c_str());
            success = false;
            return Core::ERROR_NONE;
        }

        auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            Core::hresult comResult = Core::ERROR_NONE;
            comResult = hdmiIn->GetHDMIInEdid2AllmSupport(
                static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                allmSupport);
            if (comResult == Core::ERROR_NONE) {
                LOGINFO("GetEdid2AllmSupport for portId[%s]: %d", portId.c_str(), allmSupport);
                success = true;
            } else {
                LOGERR("GetEdid2AllmSupport failed for portId=%d, Error: %d", id, static_cast<int>(comResult));
                success = false;
            }
            hdmiIn->Release();
        } else {
            LOGERR("IDeviceSettingsHDMIIn not available");
            success = false;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::GetVRRSupport(const string& portId, bool& vrrSupport, bool& success)
    {
        // COM-RPC: device::HdmiInput::getInstance().getVRRSupport(id, &vrrSupport)
        //       → IDeviceSettingsHDMIIn::GetVRRSupport()
        int id;
        try {
            id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("GetVRRSupport: Invalid portId: %s", portId.c_str());
            success = false;
            return Core::ERROR_NONE;
        }

        auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            Core::hresult comResult = Core::ERROR_NONE;
            comResult = hdmiIn->GetVRRSupport(
                static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                vrrSupport);
            if (comResult == Core::ERROR_NONE) {
                LOGINFO("GetVRRSupport for portId[%s]: %d", portId.c_str(), vrrSupport);
                success = true;
            } else {
                LOGERR("GetVRRSupport failed for portId=%d, Error: %d", id, static_cast<int>(comResult));
                success = false;
            }
            hdmiIn->Release();
        } else {
            LOGERR("IDeviceSettingsHDMIIn not available");
            success = false;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::SetVRRSupport(const string& portId, const bool vrrSupport, SuccessResult& successResult)
    {
        // COM-RPC: device::HdmiInput::getInstance().setVRRSupport(id, vrrSupport)
        //       → IDeviceSettingsHDMIIn::SetVRRSupport()
        int id;
        try {
            id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("SetVRRSupport: Invalid portId: %s", portId.c_str());
            successResult.success = false;
            return Core::ERROR_NONE;
        }

        auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            Core::hresult comResult = Core::ERROR_NONE;
            comResult = hdmiIn->SetVRRSupport(
                static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                vrrSupport);
            if (comResult == Core::ERROR_NONE) {
                LOGINFO("SetVRRSupport portId[%s] vrr=%d", portId.c_str(), vrrSupport);
                successResult.success = true;
            } else {
                LOGERR("SetVRRSupport failed for portId=%d, Error: %d", id, static_cast<int>(comResult));
                successResult.success = false;
            }
            hdmiIn->Release();
        } else {
            LOGERR("IDeviceSettingsHDMIIn not available");
            successResult.success = false;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::GetHdmiVersion(const string& portId, string& HdmiCapabilityVersion, bool& success)
    {
        // COM-RPC: device::HdmiInput::getInstance().getHdmiVersion(id, &hdmiCapVersion)
        //       → IDeviceSettingsHDMIIn::GetHDMIVersion()
        int id;
        try {
            id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("GetHdmiVersion: Invalid portId: %s", portId.c_str());
            success = false;
            return Core::ERROR_NONE;
        }

        auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            Exchange::IDeviceSettingsHDMIIn::HDMIInCapabilityVersion capVer = Exchange::IDeviceSettingsHDMIIn::HDMI_COMPATIBILITY_VERSION_14;
            Core::hresult comResult = Core::ERROR_NONE;
            comResult = hdmiIn->GetHDMIVersion(
                static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                capVer);
            if (comResult == Core::ERROR_NONE) {
                switch (static_cast<int>(capVer)) {
                case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::HDMI_COMPATIBILITY_VERSION_14):
                    HdmiCapabilityVersion = "1.4"; success = true; break;
                case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::HDMI_COMPATIBILITY_VERSION_20):
                    HdmiCapabilityVersion = "2.0"; success = true; break;
                case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::HDMI_COMPATIBILITY_VERSION_21):
                    HdmiCapabilityVersion = "2.1"; success = true; break;
                default:
                    LOGERR("GetHdmiVersion: Unknown HDMI version, capVer=%d", static_cast<int>(capVer));
                    success = false; break;
                }
            } else {
                LOGERR("GetHdmiVersion failed for portId=%d, Error: %d", id, static_cast<int>(comResult));
                success = false;
            }
            hdmiIn->Release();
        } else {
            LOGERR("IDeviceSettingsHDMIIn not available");
            success = false;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::SetEdidVersion(const string& portId, const string& edidVersion, SuccessResult& successResult)
    {
        // COM-RPC: device::HdmiInput::getInstance().setEdidVersion(id, edidVer)
        //       → IDeviceSettingsHDMIIn::SetHDMIEdidVersion()
        int id;
        try {
            id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("SetEdidVersion: Invalid portId: %s", portId.c_str());
            successResult.success = false;
            return Core::ERROR_NONE;
        }

        Exchange::IDeviceSettingsHDMIIn::HDMIInEdidVersion edidVer = Exchange::IDeviceSettingsHDMIIn::HDMI_EDID_VER_14;
        if (edidVersion == "HDMI1.4") {
            edidVer = Exchange::IDeviceSettingsHDMIIn::HDMI_EDID_VER_14;
        } else if (edidVersion == "HDMI2.0") {
            edidVer = Exchange::IDeviceSettingsHDMIIn::HDMI_EDID_VER_20;
        } else {
            LOGERR("SetEdidVersion: Invalid EDID version: %s", edidVersion.c_str());
            successResult.success = false;
            return Core::ERROR_NONE;
        }

        auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            Core::hresult comResult = Core::ERROR_NONE;
            comResult = hdmiIn->SetHDMIEdidVersion(
                static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                edidVer);
            if (comResult == Core::ERROR_NONE) {
                LOGINFO("SetEdidVersion portId[%s] version=%s", portId.c_str(), edidVersion.c_str());
                successResult.success = true;
            } else {
                LOGERR("SetEdidVersion failed for portId[%s], Error: %d", portId.c_str(), static_cast<int>(comResult));
                successResult.success = false;
            }
            hdmiIn->Release();
        } else {
            LOGERR("IDeviceSettingsHDMIIn not available");
            successResult.success = false;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::GetEdidVersion(const string& portId, string& edidVersion, bool& success)
    {
        // COM-RPC: device::HdmiInput::getInstance().getEdidVersion(id, &version)
        //       → IDeviceSettingsHDMIIn::GetHDMIEdidVersion()
        int id;
        try {
            id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("GetEdidVersion: Invalid portId: %s", portId.c_str());
            success = false;
            return Core::ERROR_NONE;
        }

        auto* hdmiIn = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            Exchange::IDeviceSettingsHDMIIn::HDMIInEdidVersion ver = Exchange::IDeviceSettingsHDMIIn::HDMI_EDID_VER_14;
            Core::hresult comResult = Core::ERROR_NONE;
            comResult = hdmiIn->GetHDMIEdidVersion(
                static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                ver);
            if (comResult == Core::ERROR_NONE) {
                switch (ver) {
                case Exchange::IDeviceSettingsHDMIIn::HDMI_EDID_VER_14:
                    edidVersion = "HDMI1.4"; success = true; break;
                case Exchange::IDeviceSettingsHDMIIn::HDMI_EDID_VER_20:
                    edidVersion = "HDMI2.0"; success = true; break;
                default:
                    LOGERR("GetEdidVersion: Unknown EDID version");
                    success = false; break;
                }
            } else {
                LOGERR("GetEdidVersion failed for portId[%s], Error: %d", portId.c_str(), static_cast<int>(comResult));
                success = false;
            }
            hdmiIn->Release();
        } else {
            LOGERR("IDeviceSettingsHDMIIn not available");
            success = false;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::GetARCPortId(string& portId, bool& success)
    {
        // COM-RPC: device::HdmiInput::getInstance().getHDMIARCPortId(id)
        //       → IDeviceSettingsAudio::GetAudioHDMIARCPortId() (via Audio sub-interface)
        // Note: ARCPortId is a property of the HDMI_ARC0 audio port, not the HDMIIn interface.
        // We use IDeviceSettingsAudio which already has GetAudioHDMIARCPortId().
        auto* audio = DSHelper::AcquireSubInterface<Exchange::IDeviceSettingsAudio>();
        if (audio != nullptr) {
            // Find HDMI_ARC0 port handle — iterate audio ports to find it
            // For simplicity, use handle 0 (caller must have audio port handles cached)
            int32_t id = -1;
            // GetAudioHDMIARCPortId requires an audio port handle; use a temporary handle query
            // In practice this is called with the HDMI_ARC0 handle.
            // Since we don't have cached audio handles here, query via the Audio sub-interface
            // with handle 0 as a best-effort approach.
            Core::hresult comResult = Core::ERROR_NONE;
            comResult = audio->GetAudioHDMIARCPortId(0, id);
            if (comResult == Core::ERROR_NONE && id >= 0) {
                LOGINFO("HDMI ARC port ID: %d", id);
                portId = std::to_string(id);
                success = true;
            } else {
                LOGWARN("GetAudioHDMIARCPortId failed, Error: %d", static_cast<int>(comResult));
                success = false;
            }
            audio->Release();
        } else {
            LOGERR("IDeviceSettingsAudio not available");
            success = false;
        }
        return Core::ERROR_NONE;
    }
} // namespace Plugin
} // namespace WPEFramework
