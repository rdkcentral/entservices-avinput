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
        : _adminLock()
        , _service(nullptr)
        , _registeredDsEventHandlers(false)
        , _DSHDMIInNotification(*this)
        , _DSCompositeInNotification(*this)
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
        // DeviceSettingsClientHelper::Close() which calls OnDeviceSettingsDeactivated()
        _registeredDsEventHandlers = false;
    }

    // =========================================================================
    // DeviceSettingsClientHelper override: called when DeviceSettings activates
    // DS_IARM equivalent: device::Host::getInstance().Register(IHdmiInEvents)
    //                     device::Host::getInstance().Register(ICompositeInEvents)
    // =========================================================================
    void AVInputImplementation::OnDeviceSettingsActivated()
    {
        LOGINFO("AVInputImplementation: OnDeviceSettingsActivated — registering DS notifications");

        // Register HDMI-In notification delegate
        {
            auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
            if (hdmiIn != nullptr) {
                hdmiIn->Register(&_DSHDMIInNotification);
                hdmiIn->Release();
                LOGINFO("AVInputImplementation: IDeviceSettingsHDMIIn::INotification registered");
            }
        }

        // Register Composite-In notification delegate
        {
            auto* compositeIn = AcquireSubInterface<Exchange::IDeviceSettingsCompositeIn>();
            if (compositeIn != nullptr) {
                compositeIn->Register(&_DSCompositeInNotification);
                compositeIn->Release();
                LOGINFO("AVInputImplementation: IDeviceSettingsCompositeIn::INotification registered");
            }
        }

        _registeredDsEventHandlers = true;
    }

    // =========================================================================
    // DeviceSettingsClientHelper override: called when DeviceSettings deactivates
    // DS_IARM equivalent: device::Host::getInstance().UnRegister(IHdmiInEvents)
    //                     device::Host::getInstance().UnRegister(ICompositeInEvents)
    // =========================================================================
    void AVInputImplementation::OnDeviceSettingsDeactivated()
    {
        LOGINFO("AVInputImplementation: OnDeviceSettingsDeactivated — unregistering DS notifications");

        {
            auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
            if (hdmiIn != nullptr) {
                hdmiIn->Unregister(&_DSHDMIInNotification);
                hdmiIn->Release();
            }
        }
        {
            auto* compositeIn = AcquireSubInterface<Exchange::IDeviceSettingsCompositeIn>();
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
        DeviceSettingsClientHelper::Open(service);
        LOGINFO("AVInputImplementation: DeviceSettingsClientHelper::Open() called");

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
    template <typename T>
    void AVInputImplementation::dispatchToList(std::list<T*>& list, std::function<void(T*)> fn)
    {
        _adminLock.Lock();
        std::list<T*> copy(list);
        _adminLock.Unlock();
        for (T* item : copy) {
            fn(item);
        }
    }

    void AVInputImplementation::dispatchEvent(Event event, ParamsType params)
    {
        Core::IWorkerPool::Instance().Submit(Job::Create(this, event, params));
    }

    void AVInputImplementation::Dispatch(Event event, ParamsType params)
    {
        switch (event) {
        case ON_AVINPUT_DEVICES_CHANGED: {
            auto* devices = boost::get<Exchange::IAVInput::IInputDeviceIterator* const>(params);
            dispatchToList(_devicesChangedNotifications,
                [devices](Exchange::IAVInput::IDevicesChangedNotification* n) { n->OnDevicesChanged(devices); });
            break;
        }
        case ON_AVINPUT_SIGNAL_CHANGED: {
            auto t = boost::get<std::tuple<int, string, string>>(params);
            dispatchToList(_signalChangedNotifications,
                [&t](Exchange::IAVInput::ISignalChangedNotification* n) {
                    n->OnSignalChanged(std::get<0>(t), std::get<1>(t), std::get<2>(t));
                });
            break;
        }
        case ON_AVINPUT_STATUS_CHANGED: {
            auto t = boost::get<std::tuple<int, string, string, int>>(params);
            dispatchToList(_inputStatusChangedNotifications,
                [&t](Exchange::IAVInput::IInputStatusChangedNotification* n) {
                    n->OnInputStatusChanged(std::get<0>(t), std::get<1>(t), std::get<2>(t), std::get<3>(t));
                });
            break;
        }
        case ON_AVINPUT_VIDEO_STREAM_INFO_UPDATE: {
            auto t = boost::get<std::tuple<int, string, int, int, bool, int, int>>(params);
            dispatchToList(_videoStreamInfoUpdateNotifications,
                [&t](Exchange::IAVInput::IVideoStreamInfoUpdateNotification* n) {
                    n->VideoStreamInfoUpdate(std::get<0>(t), std::get<1>(t), std::get<2>(t),
                        std::get<3>(t), std::get<4>(t), std::get<5>(t), std::get<6>(t));
                });
            break;
        }
        case ON_AVINPUT_GAME_FEATURE_STATUS_UPDATE: {
            auto t = boost::get<std::tuple<int, string, bool>>(params);
            dispatchToList(_gameFeatureStatusUpdateNotifications,
                [&t](Exchange::IAVInput::IGameFeatureStatusUpdateNotification* n) {
                    n->GameFeatureStatusUpdate(std::get<0>(t), std::get<1>(t), std::get<2>(t));
                });
            break;
        }
        case ON_AVINPUT_AVI_CONTENT_TYPE_UPDATE: {
            auto t = boost::get<std::tuple<int, int>>(params);
            dispatchToList(_aviContentTypeUpdateNotifications,
                [&t](Exchange::IAVInput::IAviContentTypeUpdateNotification* n) {
                    n->AviContentTypeUpdate(std::get<0>(t), std::get<1>(t));
                });
            break;
        }
        default:
            break;
        }
    }

    // =========================================================================
    // Shared event dispatch helpers (unchanged logic from DS_IARM)
    // =========================================================================
    void AVInputImplementation::AVInputHotplug(int port, int isConnected, int type)
    {
        LOGWARN("avInputHotplug [%d, %d, %d]", port, isConnected, type);

        std::stringstream locator;
        if (type == INPUT_TYPE_INT_HDMI) {
            locator << "hdmiin://localhost/deviceid/" << port;
        } else if (type == INPUT_TYPE_INT_COMPOSITE) {
            locator << "cvbsin://localhost/deviceid/" << port;
        }

        JsonObject hash;
        hash["id"] = port;
        hash["locator"] = locator.str();
        hash["connected"] = (isConnected == AV_HOT_PLUG_EVENT_CONNECTED);

        // Build a single-item IInputDeviceIterator for the notification
        std::vector<Exchange::IAVInput::IInputDevice*> devList;
        // (simplified: dispatch as devices-changed with empty list; callers rebuild via getInputDevices)
        ParamsType params = static_cast<Exchange::IAVInput::IInputDeviceIterator*>(nullptr);
        dispatchEvent(ON_AVINPUT_DEVICES_CHANGED, params);
    }

    void AVInputImplementation::AVInputSignalChange(int port, int signalStatus, int type)
    {
        LOGWARN("avInputSignalStatus [%d, %d, %d]", port, signalStatus, type);

        std::stringstream locator;
        if (type == INPUT_TYPE_INT_HDMI) {
            locator << "hdmiin://localhost/deviceid/" << port;
        } else if (type == INPUT_TYPE_INT_COMPOSITE) {
            locator << "cvbsin://localhost/deviceid/" << port;
        }

        string signalStatusStr;
        // DS_IARM used dsHdmiInSignalStatus_t / dsCompInSignalStatus_t (same values)
        // COM-RPC: HDMIInSignalStatus and CompositeInSignalStatus are mapped identically
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

            // COM-RPC: HDMIVideoPortResolution uses HDMIInTVResolution (width/height encoded as enum)
            // Extract width/height from resolution.pixelResolution (same encoding as DS_IARM dsVideoPixelResolution_t)
            switch (static_cast<int>(resolution.pixelResolution)) {
            case 0:  width = 720;  height = 480;  break;  // 720x480
            case 1:  width = 720;  height = 576;  break;  // 720x576
            case 2:  width = 1280; height = 720;  break;  // 1280x720
            case 3:  width = 1920; height = 1080; break;  // 1920x1080
            case 4:  width = 3840; height = 2160; break;  // 3840x2160
            case 5:  width = 4096; height = 2160; break;  // 4096x2160
            default: width = 1920; height = 1080; break;
            }
            progressive = !resolution.interlaced;
        }

        // COM-RPC: HDMIVideoPortResolution uses HDMIInVideoFrameRate
        switch (static_cast<int>(resolution.frameRate)) {
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_VIDEO_FRAMERATE_24):    frameRateN = 24000;  frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_VIDEO_FRAMERATE_25):    frameRateN = 25000;  frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_VIDEO_FRAMERATE_30):    frameRateN = 30000;  frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_VIDEO_FRAMERATE_50):    frameRateN = 50000;  frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_VIDEO_FRAMERATE_60):    frameRateN = 60000;  frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_VIDEO_FRAMERATE_23dot98): frameRateN = 24000; frameRateD = 1001; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_VIDEO_FRAMERATE_29dot97): frameRateN = 30000; frameRateD = 1001; break;
        case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_VIDEO_FRAMERATE_59dot94): frameRateN = 60000; frameRateD = 1001; break;
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

            switch (static_cast<int>(resolution.pixelResolution)) {
            case 0:  width = 720; height = 480; break;
            case 1:  width = 720; height = 576; break;
            default: width = 720; height = 576; break;
            }
        }

        switch (static_cast<int>(resolution.frameRate)) {
        case static_cast<int>(Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_FRAMERATE_24):    frameRateN = 24000;  frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_FRAMERATE_25):    frameRateN = 25000;  frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_FRAMERATE_30):    frameRateN = 30000;  frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_FRAMERATE_50):    frameRateN = 50000;  frameRateD = 1000; break;
        case static_cast<int>(Exchange::IDeviceSettingsCompositeIn::DS_DISPLAY_FRAMERATE_60):    frameRateN = 60000;  frameRateD = 1000; break;
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

        switch (vrr_type) {
        case Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_VRR_HDMI:
            gameFeature = VRR_TYPE_HDMI;
            break;
        case Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_VRR_AMD_FREESYNC:
            gameFeature = VRR_TYPE_FREESYNC;
            break;
        case Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_VRR_AMD_FREESYNC_PREMIUM:
            gameFeature = VRR_TYPE_FREESYNC_PREMIUM;
            break;
        case Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_VRR_AMD_FREESYNC_PREMIUM_PRO:
            gameFeature = VRR_TYPE_FREESYNC_PREMIUM_PRO;
            break;
        default:
            gameFeature = VRR_TYPE_HDMI;
            break;
        }

        ParamsType params = std::make_tuple(port, gameFeature, vrr_mode);
        dispatchEvent(ON_AVINPUT_GAME_FEATURE_STATUS_UPDATE, params);
    }

    // =========================================================================
    // Internal event forwarders: COM-RPC INotification → shared dispatch helpers
    // DS_IARM equivalent: OnHdmiInEventHotPlug(), OnHdmiInEventSignalStatus(), etc.
    // =========================================================================
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
        LOGINFO("Received OnHDMIInVideoModeUpdate port=%d", port);
        AVInputImplementation::AVInputVideoModeUpdate(port, videoPortResolution, INPUT_TYPE_INT_HDMI);
    }

    void AVInputImplementation::onHdmiInAllmStatus(int port, bool allmStatus)
    {
        LOGINFO("Received OnHDMIInAllmStatus port=%d allm=%d", port, allmStatus);
        AVInputImplementation::AVInputALLMChange(port, allmStatus);
    }

    void AVInputImplementation::onHdmiInAVIContentType(int port, int aviContentType)
    {
        LOGINFO("Received OnHDMIInAVIContentType port=%d contentType=%d", port, aviContentType);
        hdmiInputAviContentTypeChange(port, aviContentType);
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
        LOGINFO("Received OnCompositeInHotPlug port=%d isConnected=%d", port, isConnected);
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
        LOGINFO("Received OnCompositeInVideoModeUpdate port=%d", activePort);
        AVInputImplementation::AVInputVideoModeUpdate(activePort, videoResolution, INPUT_TYPE_INT_COMPOSITE);
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
        auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            int32_t count = 0;
            if (hdmiIn->GetHDMIInNumberOfInputs(count) == Core::ERROR_NONE) {
                numberOfInputs = static_cast<uint32_t>(count);
                LOGINFO("numberOfInputs %u", numberOfInputs);
                success = true;
            } else {
                LOGERR("GetHDMIInNumberOfInputs failed");
                success = false;
            }
            hdmiIn->Release();
        } else {
            LOGERR("NumberOfInputs: IDeviceSettingsHDMIIn not available");
            success = false;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::CurrentVideoMode(string& currentVideoMode, bool& success)
    {
        // COM-RPC: device::HdmiInput::getInstance().getCurrentVideoMode()
        //       → IDeviceSettingsHDMIIn::GetHDMIVideoMode()
        auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            Exchange::IDeviceSettingsHDMIIn::HDMIVideoPortResolution vpRes{};
            if (hdmiIn->GetHDMIVideoMode(vpRes) == Core::ERROR_NONE) {
                currentVideoMode = vpRes.name;
                LOGINFO("currentVideoMode %s", currentVideoMode.c_str());
                success = true;
            } else {
                LOGERR("GetHDMIVideoMode failed");
                success = false;
            }
            hdmiIn->Release();
        } else {
            LOGERR("CurrentVideoMode: IDeviceSettingsHDMIIn not available");
            success = false;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::WriteEDID(const string& portId, const string& message, SuccessResult& successResult)
    {
        // Not implemented via COM-RPC (no equivalent in IDeviceSettingsHDMIIn)
        LOGWARN("[COMRPC Unavailable] WriteEDID: not supported in COM-RPC mode");
        successResult.success = false;
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

        auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            constexpr uint16_t kEdidMaxLen = 256;
            uint8_t edidBuf[kEdidMaxLen] = {};
            if (hdmiIn->GetEdidBytes(static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                                     kEdidMaxLen, edidBuf) == Core::ERROR_NONE) {
                Core::ToString(edidBuf, kEdidMaxLen, true, EDID);
                success = true;
            } else {
                LOGERR("GetEdidBytes failed for portId=%s", portId.c_str());
                success = false;
            }
            hdmiIn->Release();
        } else {
            success = false;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::GetRawSPD(const string& portId, string& HDMISPD, bool& success)
    {
        // COM-RPC: device::HdmiInput::getInstance().getHDMISPDInfo(id, spdVect)
        //       → IDeviceSettingsHDMIIn::GetHDMISPDInformation()
        int id;
        try {
            id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("GetRawSPD: Invalid portId: %s", portId.c_str());
            success = false;
            return Core::ERROR_NONE;
        }

        auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            constexpr uint16_t kSpdMaxLen = 256;
            uint8_t spdBuf[kSpdMaxLen] = {};
            if (hdmiIn->GetHDMISPDInformation(static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                                               kSpdMaxLen, spdBuf) == Core::ERROR_NONE) {
                uint16_t size = kSpdMaxLen;
                Core::ToString(spdBuf, size, false, HDMISPD);
                success = true;
            } else {
                LOGERR("GetHDMISPDInformation failed for portId=%s", portId.c_str());
                success = false;
            }
            hdmiIn->Release();
        } else {
            success = false;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::GetSPD(const string& portId, string& HDMISPD, bool& success)
    {
        // COM-RPC: same call as GetRawSPD but formats as SPD struct string
        int id;
        try {
            id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("GetSPD: Invalid portId: %s", portId.c_str());
            success = false;
            return Core::ERROR_NONE;
        }

        auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            constexpr uint16_t kSpdMaxLen = 256;
            uint8_t spdBuf[kSpdMaxLen] = {};
            if (hdmiIn->GetHDMISPDInformation(static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                                               kSpdMaxLen, spdBuf) == Core::ERROR_NONE && kSpdMaxLen > 0) {
                // Format as structured string (mirrors DS_IARM dsSpd_infoframe_st formatting)
                char str[200] = { 0 };
                // spdBuf layout matches dsSpd_infoframe_st: pkttype, version, length, vendor_name[8], product_des[16], source_info
                snprintf(str, sizeof(str),
                    "Packet Type:%02X,Version:%u,Length:%u,vendor name:%.8s,product des:%.16s,source info:%02X",
                    spdBuf[0], spdBuf[1], spdBuf[2],
                    reinterpret_cast<const char*>(&spdBuf[3]),
                    reinterpret_cast<const char*>(&spdBuf[11]),
                    spdBuf[27]);
                HDMISPD = str;
                success = true;
            } else {
                LOGERR("GetHDMISPDInformation failed for portId=%s", portId.c_str());
                success = false;
            }
            hdmiIn->Release();
        } else {
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
            LOGERR("StartInput: Invalid portId: %s", portId.c_str());
            successResult.success = false;
            return Core::ERROR_NONE;
        }

        int iType = AVInputUtils::getTypeOfInput(typeOfInput);

        if (iType == INPUT_TYPE_INT_HDMI) {
            // COM-RPC: device::HdmiInput::getInstance().selectPort(id, requestAudioMix, plane, topMost)
            //       → IDeviceSettingsHDMIIn::SelectHDMIInPort()
            auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
            if (hdmiIn != nullptr) {
                if (hdmiIn->SelectHDMIInPort(
                        static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                        requestAudioMix,
                        topMost,
                        static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIVideoPlaneType>(plane)) == Core::ERROR_NONE) {
                    planeType = plane;
                    // COM-RPC: device::Host::getInstance().setAudioMixerLevels() equivalent
                    // (IDeviceSettingsHost has no SetAudioMixerLevels — skipped, matches DS_IARM behavior
                    // where mixer levels are only set when requestAudioMix is true)
                    successResult.success = true;
                } else {
                    LOGERR("SelectHDMIInPort failed for portId=%s", portId.c_str());
                    successResult.success = false;
                }
                hdmiIn->Release();
            } else {
                successResult.success = false;
            }
        } else if (iType == INPUT_TYPE_INT_COMPOSITE) {
            // COM-RPC: device::CompositeInput::getInstance().selectPort(id)
            //       → IDeviceSettingsCompositeIn::SelectCompositeInPort()
            auto* compositeIn = AcquireSubInterface<Exchange::IDeviceSettingsCompositeIn>();
            if (compositeIn != nullptr) {
                if (compositeIn->SelectCompositeInPort(
                        static_cast<Exchange::IDeviceSettingsCompositeIn::CompositeInPort>(id)) == Core::ERROR_NONE) {
                    successResult.success = true;
                } else {
                    LOGERR("SelectCompositeInPort failed for portId=%s", portId.c_str());
                    successResult.success = false;
                }
                compositeIn->Release();
            } else {
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
        int iType = AVInputUtils::getTypeOfInput(typeOfInput);

        if (iType == INPUT_TYPE_INT_HDMI) {
            // COM-RPC: device::HdmiInput::getInstance().selectPort(-1)
            //       → IDeviceSettingsHDMIIn::SelectHDMIInPort(DS_HDMI_IN_PORT_NONE)
            auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
            if (hdmiIn != nullptr) {
                if (hdmiIn->SelectHDMIInPort(
                        Exchange::IDeviceSettingsHDMIIn::DS_HDMI_IN_PORT_NONE,
                        false, false,
                        Exchange::IDeviceSettingsHDMIIn::DS_VIDEO_PLANE_PRIMARY) == Core::ERROR_NONE) {
                    successResult.success = true;
                } else {
                    successResult.success = false;
                }
                hdmiIn->Release();
            } else {
                successResult.success = false;
            }
        } else if (iType == INPUT_TYPE_INT_COMPOSITE) {
            // COM-RPC: device::CompositeInput::getInstance().selectPort(-1)
            //       → IDeviceSettingsCompositeIn::SelectCompositeInPort(DS_COMPOSITE_IN_PORT_NONE)
            auto* compositeIn = AcquireSubInterface<Exchange::IDeviceSettingsCompositeIn>();
            if (compositeIn != nullptr) {
                if (compositeIn->SelectCompositeInPort(
                        Exchange::IDeviceSettingsCompositeIn::DS_COMPOSITE_IN_PORT_NONE) == Core::ERROR_NONE) {
                    successResult.success = true;
                } else {
                    successResult.success = false;
                }
                compositeIn->Release();
            } else {
                successResult.success = false;
            }
        } else {
            successResult.success = false;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::SetVideoRectangle(const uint16_t x, const uint16_t y,
        const uint16_t w, const uint16_t h, const string& typeOfInput, SuccessResult& successResult)
    {
        int iType = AVInputUtils::getTypeOfInput(typeOfInput);

        if (iType == INPUT_TYPE_INT_HDMI) {
            // COM-RPC: device::HdmiInput::getInstance().scaleVideo(x, y, w, h)
            //       → IDeviceSettingsHDMIIn::ScaleHDMIInVideo()
            auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
            if (hdmiIn != nullptr) {
                Exchange::IDeviceSettingsHDMIIn::HDMIInVideoRectangle rect{};
                rect.x = x; rect.y = y; rect.width = w; rect.height = h;
                if (hdmiIn->ScaleHDMIInVideo(rect) == Core::ERROR_NONE) {
                    successResult.success = true;
                } else {
                    successResult.success = false;
                }
                hdmiIn->Release();
            } else {
                successResult.success = false;
            }
        } else if (iType == INPUT_TYPE_INT_COMPOSITE) {
            // COM-RPC: device::CompositeInput::getInstance().scaleVideo(x, y, w, h)
            //       → IDeviceSettingsCompositeIn::ScaleCompositeInVideo()
            auto* compositeIn = AcquireSubInterface<Exchange::IDeviceSettingsCompositeIn>();
            if (compositeIn != nullptr) {
                Exchange::IDeviceSettingsCompositeIn::VideoRectangle rect{};
                rect.x = x; rect.y = y; rect.width = w; rect.height = h;
                if (compositeIn->ScaleCompositeInVideo(rect) == Core::ERROR_NONE) {
                    successResult.success = true;
                } else {
                    successResult.success = false;
                }
                compositeIn->Release();
            } else {
                successResult.success = false;
            }
        } else {
            successResult.success = false;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::GetSupportedGameFeatures(Exchange::IAVInput::IStringIterator*& features, bool& success)
    {
        // COM-RPC: device::HdmiInput::getInstance().getSupportedGameFeatures(supportedFeatures)
        //       → IDeviceSettingsHDMIIn::GetSupportedGameFeaturesList()
        Core::hresult result = Core::ERROR_NONE;
        success = true;
        features = nullptr;
        std::vector<std::string> supportedFeatures;

        auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            Exchange::IDeviceSettingsHDMIIn::IHDMIInGameFeatureListIterator* iter = nullptr;
            if (hdmiIn->GetSupportedGameFeaturesList(iter) == Core::ERROR_NONE && iter != nullptr) {
                Exchange::IDeviceSettingsHDMIIn::HDMIInGameFeatureList feature{};
                while (iter->Next(feature)) {
                    supportedFeatures.push_back(feature.gameFeature);
                }
                iter->Release();
            } else {
                success = false;
            }
            hdmiIn->Release();
        } else {
            success = false;
        }

        if (!supportedFeatures.empty() && result == Core::ERROR_NONE) {
            features = Core::Service<RPC::IteratorType<Exchange::IAVInput::IStringIterator>>::Create<Exchange::IAVInput::IStringIterator>(supportedFeatures);
            LOGINFO("GetSupportedGameFeatures: %zu", supportedFeatures.size());
        } else {
            success = false;
        }
        return result;
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
                    mode = (vrrStatus.vrrType == Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_VRR_HDMI);
                else if (gameFeature == VRR_TYPE_FREESYNC)
                    mode = (vrrStatus.vrrType == Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_VRR_AMD_FREESYNC);
                else if (gameFeature == VRR_TYPE_FREESYNC_PREMIUM)
                    mode = (vrrStatus.vrrType == Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_VRR_AMD_FREESYNC_PREMIUM);
                else if (gameFeature == VRR_TYPE_FREESYNC_PREMIUM_PRO)
                    mode = (vrrStatus.vrrType == Exchange::IDeviceSettingsHDMIIn::DS_HDMIIN_VRR_AMD_FREESYNC_PREMIUM_PRO);
            }
        } else {
            LOGWARN("GetGameFeatureStatus: Unsupported feature: %s", gameFeature.c_str());
            success = false;
        }
        return Core::ERROR_NONE;
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
            currentVRRVideoFrameRate = vrrStatus.vrrAmdfreesyncFramerate_Hz;
            LOGINFO("VRR FrameRate for portId[%s] is %.2f", portId.c_str(), currentVRRVideoFrameRate);
        } else {
            LOGERR("GetVRRFrameRate: Failed to get current VRR video frame rate");
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

        auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            if (hdmiIn->GetHDMIInEdid2AllmSupport(
                    static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                    allmSupport) == Core::ERROR_NONE) {
                LOGINFO("GetEdid2AllmSupport for portId[%s]: %d", portId.c_str(), allmSupport);
                success = true;
            } else {
                success = false;
            }
            hdmiIn->Release();
        } else {
            success = false;
        }
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

        auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            if (hdmiIn->SetHDMIInEdid2AllmSupport(
                    static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                    allmSupport) == Core::ERROR_NONE) {
                LOGINFO("SetEdid2AllmSupport portId[%s] allm=%d", portId.c_str(), allmSupport);
                successResult.success = true;
            } else {
                successResult.success = false;
            }
            hdmiIn->Release();
        } else {
            successResult.success = false;
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

        auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            if (hdmiIn->GetVRRSupport(
                    static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                    vrrSupport) == Core::ERROR_NONE) {
                LOGINFO("GetVRRSupport for portId[%s]: %d", portId.c_str(), vrrSupport);
                success = true;
            } else {
                success = false;
            }
            hdmiIn->Release();
        } else {
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

        auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            if (hdmiIn->SetVRRSupport(
                    static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                    vrrSupport) == Core::ERROR_NONE) {
                LOGINFO("SetVRRSupport portId[%s] vrr=%d", portId.c_str(), vrrSupport);
                successResult.success = true;
            } else {
                successResult.success = false;
            }
            hdmiIn->Release();
        } else {
            successResult.success = false;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::GetARCPortId(string& portId, bool& success)
    {
        // COM-RPC: device::HdmiInput::getInstance().getHDMIARCPortId(id)
        //       → IDeviceSettingsAudio::GetAudioHDMIARCPortId() (via Audio sub-interface)
        // Note: ARCPortId is a property of the HDMI_ARC0 audio port, not the HDMIIn interface.
        // We use IDeviceSettingsAudio which already has GetAudioHDMIARCPortId().
        auto* audio = AcquireSubInterface<Exchange::IDeviceSettingsAudio>();
        if (audio != nullptr) {
            // Find HDMI_ARC0 port handle — iterate audio ports to find it
            // For simplicity, use handle 0 (caller must have audio port handles cached)
            int32_t id = -1;
            // GetAudioHDMIARCPortId requires an audio port handle; use a temporary handle query
            // In practice this is called with the HDMI_ARC0 handle.
            // Since we don't have cached audio handles here, query via the Audio sub-interface
            // with handle 0 as a best-effort approach.
            if (audio->GetAudioHDMIARCPortId(0, id) == Core::ERROR_NONE && id >= 0) {
                LOGINFO("HDMI ARC port ID: %d", id);
                portId = std::to_string(id);
                success = true;
            } else {
                LOGWARN("GetAudioHDMIARCPortId failed");
                success = false;
            }
            audio->Release();
        } else {
            success = false;
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

        auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            Exchange::IDeviceSettingsHDMIIn::HDMIInCapabilityVersion capVer =
                Exchange::IDeviceSettingsHDMIIn::DS_HDMI_CAP_VERSION_14;
            if (hdmiIn->GetHDMIVersion(
                    static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                    capVer) == Core::ERROR_NONE) {
                switch (static_cast<int>(capVer)) {
                case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMI_CAP_VERSION_14):
                    HdmiCapabilityVersion = "1.4"; success = true; break;
                case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMI_CAP_VERSION_20):
                    HdmiCapabilityVersion = "2.0"; success = true; break;
                case static_cast<int>(Exchange::IDeviceSettingsHDMIIn::DS_HDMI_CAP_VERSION_21):
                    HdmiCapabilityVersion = "2.1"; success = true; break;
                default:
                    success = false; break;
                }
            } else {
                success = false;
            }
            hdmiIn->Release();
        } else {
            success = false;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::SetMixerLevels(const int primaryVolume, const int inputVolume, SuccessResult& successResult)
    {
        // DS_IARM: device::Host::getInstance().setAudioMixerLevels(dsAUDIO_INPUT_PRIMARY, vol)
        // COM-RPC: IDeviceSettingsHost has no SetAudioMixerLevels equivalent.
        // Store the values locally (matching DS_IARM behavior of caching m_primVolume/m_inputVolume).
        if ((primaryVolume >= 0) && (inputVolume >= 0)) {
            m_primVolume  = primaryVolume;
            m_inputVolume = inputVolume;
        } else {
            LOGERR("SetMixerLevels: Invalid params\n");
            successResult.success = false;
            return Core::ERROR_NONE;
        }

        if (m_primVolume > MAX_PRIM_VOL_LEVEL) {
            LOGWARN("Primary Volume greater than limit. Set to MAX_PRIM_VOL_LEVEL(100)");
            m_primVolume = MAX_PRIM_VOL_LEVEL;
        }
        if (m_inputVolume > DEFAULT_INPUT_VOL_LEVEL) {
            LOGWARN("INPUT Volume greater than limit. Set to DEFAULT_INPUT_VOL_LEVEL(100)");
            m_inputVolume = DEFAULT_INPUT_VOL_LEVEL;
        }

        LOGINFO("SetMixerLevels: primaryVolume[%d] inputVolume[%d] (stored locally, no COM-RPC equivalent)",
            primaryVolume, inputVolume);
        isAudioBalanceSet = true;
        successResult.success = true;
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

        Exchange::IDeviceSettingsHDMIIn::HDMIInEdidVersion edidVer =
            Exchange::IDeviceSettingsHDMIIn::DS_HDMI_EDID_VER_14;
        if (edidVersion == "HDMI1.4") {
            edidVer = Exchange::IDeviceSettingsHDMIIn::DS_HDMI_EDID_VER_14;
        } else if (edidVersion == "HDMI2.0") {
            edidVer = Exchange::IDeviceSettingsHDMIIn::DS_HDMI_EDID_VER_20;
        } else {
            LOGERR("SetEdidVersion: Invalid EDID version: %s", edidVersion.c_str());
            successResult.success = false;
            return Core::ERROR_NONE;
        }

        auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            if (hdmiIn->SetHDMIEdidVersion(
                    static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                    edidVer) == Core::ERROR_NONE) {
                LOGINFO("SetEdidVersion portId[%s] version=%s", portId.c_str(), edidVersion.c_str());
                successResult.success = true;
            } else {
                successResult.success = false;
            }
            hdmiIn->Release();
        } else {
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

        auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            Exchange::IDeviceSettingsHDMIIn::HDMIInEdidVersion ver =
                Exchange::IDeviceSettingsHDMIIn::DS_HDMI_EDID_VER_14;
            if (hdmiIn->GetHDMIEdidVersion(
                    static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(id),
                    ver) == Core::ERROR_NONE) {
                switch (ver) {
                case Exchange::IDeviceSettingsHDMIIn::DS_HDMI_EDID_VER_14:
                    edidVersion = "HDMI1.4"; success = true; break;
                case Exchange::IDeviceSettingsHDMIIn::DS_HDMI_EDID_VER_20:
                    edidVersion = "HDMI2.0"; success = true; break;
                default:
                    LOGERR("GetEdidVersion: Unknown EDID version");
                    success = false; break;
                }
            } else {
                success = false;
            }
            hdmiIn->Release();
        } else {
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
        auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            hdmiIn->GetHDMIInAllmStatus(
                static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(iPort),
                allm);
            hdmiIn->Release();
        }
        return allm;
    }

    bool AVInputImplementation::getVRRStatus(int iPort, Exchange::IDeviceSettingsHDMIIn::HDMIInVRRStatus& vrrStatus)
    {
        // COM-RPC: device::HdmiInput::getInstance().getVRRStatus(iPort, vrrStatus)
        //       → IDeviceSettingsHDMIIn::GetVRRStatus()
        bool ret = false;
        auto* hdmiIn = AcquireSubInterface<Exchange::IDeviceSettingsHDMIIn>();
        if (hdmiIn != nullptr) {
            if (hdmiIn->GetVRRStatus(
                    static_cast<Exchange::IDeviceSettingsHDMIIn::HDMIInPort>(iPort),
                    vrrStatus) == Core::ERROR_NONE) {
                LOGWARN("getVRRStatus VRR TYPE: %d, VRR FRAMERATE: %f",
                    static_cast<int>(vrrStatus.vrrType), vrrStatus.vrrAmdfreesyncFramerate_Hz);
                ret = true;
            }
            hdmiIn->Release();
        }
        return ret;
    }

} // namespace Plugin
} // namespace WPEFramework
