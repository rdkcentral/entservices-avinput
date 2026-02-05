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

#include "AVInputImplementation.h"
#include <curl/curl.h>
#include <fstream>
#include <time.h>

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

    AVInputImplementation::AVInputImplementation() : _adminLock(), _registeredDsEventHandlers(false)
    {
        LOGINFO("Create AVInputImplementation Instance");

        m_primVolume = DEFAULT_PRIM_VOL_LEVEL;
        m_inputVolume = DEFAULT_INPUT_VOL_LEVEL;
        m_currentVrrType = dsVRR_NONE;
        
        AVInputImplementation::_instance = this;
    }

    AVInputImplementation::~AVInputImplementation()
    {
        AVInputImplementation::_instance = nullptr;

        device::Host::getInstance().UnRegister(baseInterface<device::Host::IHdmiInEvents>());
        device::Host::getInstance().UnRegister(baseInterface<device::Host::ICompositeInEvents>());
        
        _registeredDsEventHandlers = false;

        try {
            device::Manager::DeInitialize();
            LOGINFO("device::Manager::DeInitialize success");
        }
        catch(const device::Exception& err) {
            LOGINFO("device::Manager::DeInitialize failed due to device::Manager::DeInitialize()");
            LOG_DEVICE_EXCEPTION0();
        }
    }

    Core::hresult AVInputImplementation::Configure(PluginHost::IShell *service)
    {
        try {
            device::Manager::Initialize();
            LOGINFO("device::Manager::Initialize success");
            if (!_registeredDsEventHandlers) {
                _registeredDsEventHandlers = true;
                device::Host::getInstance().Register(baseInterface<device::Host::IHdmiInEvents>(), "WPE::AVInputHdmi");
                device::Host::getInstance().Register(baseInterface<device::Host::ICompositeInEvents>(), "WPE::AVInputComp");
            }
        }
        catch(const device::Exception& err) {
            LOGINFO("AVInput: Initialization failed due to device::manager::Initialize()");
            LOG_DEVICE_EXCEPTION0();
            return Core::ERROR_GENERAL;
        }

        return Core::ERROR_NONE;
    }

    template <typename T>
    Core::hresult AVInputImplementation::Register(std::list<T*>& list, T* notification)
    {
        uint32_t status = Core::ERROR_GENERAL;

        ASSERT(nullptr != notification);
        _adminLock.Lock();

        // Make sure we can't register the same notification callback multiple times
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

        // Make sure we can't unregister the same notification callback multiple times
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

    void AVInputImplementation::dispatchEvent(Event event, const ParamsType params)
    {
        Core::IWorkerPool::Instance().Submit(Job::Create(this, event, params));
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

                std::list<IAviContentTypeUpdateNotification*>::const_iterator index(_aviContentTypeUpdateNotifications.begin());

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

    // ==================================
    // Implementation of IAVInput methods
    // ==================================

    Core::hresult AVInputImplementation::ContentProtected(bool& isContentProtected, bool& success)
    {
        // "This is the way it's done in Service Manager"
        isContentProtected = true;
        success = true;
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::NumberOfInputs(uint32_t& numberOfInputs, bool& success)
    {
        try {
            numberOfInputs = device::HdmiInput::getInstance().getNumberOfInputs();
        } catch (...) {
            LOGERR("Exception caught");
            success = false;
            return Core::ERROR_GENERAL;
        }

        success = true;
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::CurrentVideoMode(string& currentVideoMode, bool& success)
    {
        try {
            currentVideoMode = device::HdmiInput::getInstance().getCurrentVideoMode();
        } catch (...) {
            LOGERR("Exception caught");
            success = false;
            return Core::ERROR_GENERAL;
        }

        success = true;
        return Core::ERROR_NONE;
    }
    
    Core::hresult AVInputImplementation::StartInput(const string& portId, const string& typeOfInput, const bool requestAudioMix, const int plane, const bool topMost, SuccessResult& successResult)
    {
        int id;

        try {
            id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("StartInput: Invalid paramater: portId: %s ", portId.c_str());
            successResult.success = false;
            return Core::ERROR_GENERAL;
        }

        if(plane != 0 && plane != 1 ){
            LOGERR("StartInput: Invalid paramater: plane: %d ", plane);
            successResult.success = false;
            return Core::ERROR_GENERAL;
        }

        try {
            switch(AVInputUtils::getTypeOfInput(typeOfInput)) {
                case INPUT_TYPE_INT_HDMI: {
                    device::HdmiInput::getInstance().selectPort(id, requestAudioMix, plane, topMost);
                    break;
                }
                case INPUT_TYPE_INT_COMPOSITE: {
                    device::CompositeInput::getInstance().selectPort(id);
                    break;
                }
                default: {
                    LOGWARN("Invalid input type passed to StartInput");
                    successResult.success = false;
                    return Core::ERROR_GENERAL;
                }
            }
            planeType = plane;
        } catch(...) {
            successResult.success = false;
            return Core::ERROR_GENERAL;
        }

        successResult.success = true;
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::StopInput(const string& typeOfInput, SuccessResult& successResult)
    {
        Core::hresult ret = Core::ERROR_NONE;
        successResult.success = true;

        try {
            planeType = -1;
            if (isAudioBalanceSet) {
                device::Host::getInstance().setAudioMixerLevels(dsAUDIO_INPUT_PRIMARY, MAX_PRIM_VOL_LEVEL);
                device::Host::getInstance().setAudioMixerLevels(dsAUDIO_INPUT_SYSTEM, DEFAULT_INPUT_VOL_LEVEL);
                isAudioBalanceSet = false;
            }

            switch(AVInputUtils::getTypeOfInput(typeOfInput)) {
                case INPUT_TYPE_INT_HDMI: {
                    device::HdmiInput::getInstance().selectPort(-1);
                    break;
                }
                case INPUT_TYPE_INT_COMPOSITE: {
                    device::CompositeInput::getInstance().selectPort(-1);
                    break;
                }
                default: {
                    LOGWARN("Invalid input type passed to StopInput");
                    successResult.success = false;
                    return Core::ERROR_GENERAL;
                }
            }
        } catch(...) {
            LOGWARN("AVInputImplementation::StopInput Failed");
            successResult.success = false;
            ret = Core::ERROR_GENERAL;
        }

        return ret;
    }

    Core::hresult AVInputImplementation::SetVideoRectangle(const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h, const string& typeOfInput, SuccessResult& successResult)
    {
        try {
            switch(AVInputUtils::getTypeOfInput(typeOfInput)) {
                case INPUT_TYPE_INT_HDMI: {
                    device::HdmiInput::getInstance().scaleVideo(x, y, w, h);
                    break;
                }
                case INPUT_TYPE_INT_COMPOSITE: {
                    device::CompositeInput::getInstance().scaleVideo(x, y, w, h);
                    break;
                }
                default: {
                    successResult.success = false;
                    return Core::ERROR_GENERAL;
                }
            }
        } catch(...) {
            successResult.success = false;
            return Core::ERROR_GENERAL;
        }

        successResult.success = true;
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::getInputDevices(const string& typeOfInput, std::list<WPEFramework::Exchange::IAVInput::InputDevice> &inputDeviceList)
    {
        int num = 0;
        bool isHdmi = true;

        try {
            switch(AVInputUtils::getTypeOfInput(typeOfInput)) {
                case INPUT_TYPE_INT_HDMI: {
                    num = device::HdmiInput::getInstance().getNumberOfInputs();
                    break;
                }
                case INPUT_TYPE_INT_COMPOSITE: {
                    num = device::CompositeInput::getInstance().getNumberOfInputs();
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
                for (i = 0; i < num; i++) {
                    // Input ID is aleays 0-indexed, continuous number starting 0
                    WPEFramework::Exchange::IAVInput::InputDevice inputDevice;

                    inputDevice.id = i;
                    std::stringstream locator;
                    if (isHdmi) {
                        locator << "hdmiin://localhost/deviceid/" << i;
                        inputDevice.connected = device::HdmiInput::getInstance().isPortConnected(i);
                    } else {
                        locator << "cvbsin://localhost/deviceid/" << i;
                        inputDevice.connected = device::CompositeInput::getInstance().isPortConnected(i);
                    }
                    inputDevice.locator = locator.str();
                    LOGINFO("getInputDevices id %d, locator=[%s], connected=[%d]", i, inputDevice.locator.c_str(), inputDevice.connected);
                    inputDeviceList.push_back(inputDevice);
                }
            }
        } catch (const std::exception& e) {
            LOGERR("AVInputService::getInputDevices Failed");
            return Core::ERROR_GENERAL;
        }

        return Core::ERROR_NONE;
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
            return Core::ERROR_GENERAL;
        }

        // TODO: This wasn't implemented in the original code, do we want to implement it?
        successResult.success = true;
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::ReadEDID(const string& portId, string& EDID, bool& success)
    {
        int id;

        try {
		    id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("ReadEDID: Invalid paramater: portId: %s ", portId.c_str());
            success = false;
            return Core::ERROR_GENERAL;
        }

        vector<uint8_t> edidVec({ 'u', 'n', 'k', 'n', 'o', 'w', 'n' });

        try {
            vector<uint8_t> edidVec2;
            device::HdmiInput::getInstance().getEDIDBytesInfo(id, edidVec2);
            edidVec = edidVec2; // edidVec must be "unknown" unless we successfully get to this line

            // convert to base64
            uint16_t size = min(edidVec.size(), (size_t)numeric_limits<uint16_t>::max());

            if(0 == size) {
                success = false;
                return Core::ERROR_GENERAL;
            }

            LOGWARN("AVInputImplementation::readEDID size:%d edidVec.size:%zu", size, edidVec.size());
            if (edidVec.size() > (size_t)numeric_limits<uint16_t>::max()) {
                LOGERR("Size too large to use ToString base64 wpe api");
                success = false;
                return Core::ERROR_GENERAL;
            }
            Core::ToString((uint8_t*)&edidVec[0], size, true, EDID);
        } catch (const device::Exception& err) {
            LOG_DEVICE_EXCEPTION1(std::to_string(id));
            success = false;
            return Core::ERROR_GENERAL;
        }

        success = true;
        return Core::ERROR_NONE;
    }

    /**
     * @brief This function is used to translate HDMI/COMPOSITE input hotplug to
     * deviceChanged event.
     *
     * @param[in] input Number of input port integer.
     * @param[in] connection status of input port integer.
     */
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
        case dsHDMI_IN_SIGNAL_STATUS_NOSIGNAL:
            signalStatusStr = "noSignal";
            break;

        case dsHDMI_IN_SIGNAL_STATUS_UNSTABLE:
            signalStatusStr = "unstableSignal";
            break;

        case dsHDMI_IN_SIGNAL_STATUS_NOTSUPPORTED:
            signalStatusStr = "notSupportedSignal";
            break;

        case dsHDMI_IN_SIGNAL_STATUS_STABLE:
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

    /**
     * @brief This function is used to translate HDMI input video mode change to
     * videoStreamInfoUpdate event.
     *
     * @param[in] port HDMI In port id.
     * @param[in] resolution resolution of HDMI In port.
     * @param[in] type HDMI/COMPOSITE In type.
     */
    void AVInputImplementation::AVInputVideoModeUpdate(int port, dsVideoPortResolution_t resolution, int type)
    {
        int width = 0;
        int height = 0;
        bool progressive = false;
        int frameRateN;
        int frameRateD;

        std::stringstream locator;

        LOGWARN("AVInputVideoModeUpdate [%d]", port);

        if (type == INPUT_TYPE_INT_HDMI) {
            locator << "hdmiin://localhost/deviceid/" << port;

            switch (resolution.pixelResolution) {

            case dsVIDEO_PIXELRES_720x480:
                width = 720;
                height = 480;
                break;

            case dsVIDEO_PIXELRES_720x576:
                width = 720;
                height = 576;
                break;

            case dsVIDEO_PIXELRES_1280x720:
                width = 1280;
                height = 720;
                break;

            case dsVIDEO_PIXELRES_1920x1080:
                width = 1920;
                height = 1080;
                break;

            case dsVIDEO_PIXELRES_3840x2160:
                width = 3840;
                height = 2160;
                break;

            case dsVIDEO_PIXELRES_4096x2160:
                width = 4096;
                height = 2160;
                break;

            default:
                width = 1920;
                height = 1080;
                break;
            }

            progressive = (!resolution.interlaced);

        } else if (type == INPUT_TYPE_INT_COMPOSITE) {
            locator << "cvbsin://localhost/deviceid/" << port;

            switch (resolution.pixelResolution) {
            case dsVIDEO_PIXELRES_720x480:
                width = 720;
                height = 480;
                break;

            case dsVIDEO_PIXELRES_720x576:
                width = 720;
                height = 576;
                break;

            default:
                width = 720;
                height = 576;
                break;
            }

            progressive = false;
        }

        switch (resolution.frameRate) {
        case dsVIDEO_FRAMERATE_24:
            frameRateN = 24000;
            frameRateD = 1000;
            break;

        case dsVIDEO_FRAMERATE_25:
            frameRateN = 25000;
            frameRateD = 1000;
            break;

        case dsVIDEO_FRAMERATE_30:
            frameRateN = 30000;
            frameRateD = 1000;
            break;

        case dsVIDEO_FRAMERATE_50:
            frameRateN = 50000;
            frameRateD = 1000;
            break;

        case dsVIDEO_FRAMERATE_60:
            frameRateN = 60000;
            frameRateD = 1000;
            break;

        case dsVIDEO_FRAMERATE_23dot98:
            frameRateN = 24000;
            frameRateD = 1001;
            break;

        case dsVIDEO_FRAMERATE_29dot97:
            frameRateN = 30000;
            frameRateD = 1001;
            break;

        case dsVIDEO_FRAMERATE_59dot94:
            frameRateN = 60000;
            frameRateD = 1001;
            break;

        case dsVIDEO_FRAMERATE_100:
            frameRateN = 100000;
            frameRateD = 1000;
            break;

        case dsVIDEO_FRAMERATE_119dot88:
            frameRateN = 120000;
            frameRateD = 1001;
            break;

        case dsVIDEO_FRAMERATE_120:
            frameRateN = 120000;
            frameRateD = 1000;
            break;

        case dsVIDEO_FRAMERATE_200:
            frameRateN = 200000;
            frameRateD = 1000;
            break;

        case dsVIDEO_FRAMERATE_239dot76:
            frameRateN = 240000;
            frameRateD = 1001;
            break;

        case dsVIDEO_FRAMERATE_240:
            frameRateN = 240000;
            frameRateD = 1000;
            break;

        default:
            frameRateN = 60000;
            frameRateD = 1000;
            break;
        }

        ParamsType params = std::make_tuple(port, locator.str(), width, height, progressive, frameRateN, frameRateD);
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

    void AVInputImplementation::AVInputVRRChange(int port, dsVRRType_t vrr_type, bool vrr_mode)
    {
        string gameFeature;

        switch (vrr_type) {
        case dsVRR_HDMI_VRR:
            gameFeature = VRR_TYPE_HDMI;
            break;
        case dsVRR_AMD_FREESYNC:
            gameFeature = VRR_TYPE_FREESYNC;
            break;
        case dsVRR_AMD_FREESYNC_PREMIUM:
            gameFeature = VRR_TYPE_FREESYNC_PREMIUM;
            break;
        case dsVRR_AMD_FREESYNC_PREMIUM_PRO:
            gameFeature = VRR_TYPE_FREESYNC_PREMIUM_PRO;
            break;
        default:
            break;
        }

        ParamsType params = std::make_tuple(port, gameFeature, vrr_mode);
        dispatchEvent(ON_AVINPUT_GAME_FEATURE_STATUS_UPDATE, params);
    }

    /* HDMIInEventsNotification*/

    void AVInputImplementation::OnHdmiInAVIContentType(dsHdmiInPort_t port, dsAviContentType_t aviContentType)
    {
        LOGINFO("Received OnHdmiInAVIContentType callback, port: %d, Content Type: %d", port, aviContentType);
        hdmiInputAviContentTypeChange(port, aviContentType);
    }

    void AVInputImplementation::OnHdmiInEventHotPlug(dsHdmiInPort_t port, bool isConnected)
    {
        LOGINFO("Received OnHdmiInEventHotPlug callback, port: %d, isConnected: %s", port, isConnected ? "true" : "false");
        AVInputImplementation::AVInputHotplug(port,isConnected ? AV_HOT_PLUG_EVENT_CONNECTED : AV_HOT_PLUG_EVENT_DISCONNECTED, INPUT_TYPE_INT_HDMI);
    }

    void AVInputImplementation::OnHdmiInEventSignalStatus(dsHdmiInPort_t port, dsHdmiInSignalStatus_t signalStatus)
    {
        LOGINFO("Received OnHdmiInEventSignalStatus callback, port: %d, signalStatus: %d",port, signalStatus);
        AVInputImplementation::AVInputSignalChange(port, signalStatus, INPUT_TYPE_INT_HDMI);
    }

    void AVInputImplementation::OnHdmiInEventStatus(dsHdmiInPort_t activePort, bool isPresented)
    {
        LOGINFO("Received OnHdmiInEventStatus callback, port: %d, isPresented: %s",activePort, isPresented ? "true" : "false");
        AVInputImplementation::AVInputStatusChange(activePort, isPresented, INPUT_TYPE_INT_HDMI);
    }

    void AVInputImplementation::OnHdmiInVideoModeUpdate(dsHdmiInPort_t port, const dsVideoPortResolution_t& videoPortResolution)
    {
        LOGINFO("Received OnHdmiInVideoModeUpdate callback, port: %d, pixelResolution: %d, interlaced: %d, frameRate: %d",
                port,
                videoPortResolution.pixelResolution,
                videoPortResolution.interlaced,
                videoPortResolution.frameRate);

        AVInputImplementation::AVInputVideoModeUpdate(port, videoPortResolution, INPUT_TYPE_INT_HDMI);
    }

    void AVInputImplementation::OnHdmiInAllmStatus(dsHdmiInPort_t port, bool allmStatus)
    {
        LOGINFO("Received OnHdmiInAllmStatus callback, port: %d, ALLM Mode: %s",
                port, allmStatus ? "true" : "false");

        AVInputImplementation::AVInputALLMChange(port, allmStatus);
    }

    void AVInputImplementation::OnHdmiInVRRStatus(dsHdmiInPort_t port, dsVRRType_t vrrType)
    {
        LOGINFO("Received OnHdmiInVRRStatus callback, port: %d, VRR Type: %d",
                port, vrrType);

        if (!AVInputImplementation::_instance)
            return;

        // Handle transitions
        if (dsVRR_NONE == vrrType) {
            if (m_currentVrrType != dsVRR_NONE) {
                AVInputImplementation::AVInputVRRChange(port, m_currentVrrType, false);
            }
        } else {
            if (m_currentVrrType != dsVRR_NONE) {
                AVInputImplementation::AVInputVRRChange(port, m_currentVrrType, false);
            }
            AVInputVRRChange(port, vrrType, true);
        }

        m_currentVrrType = vrrType;
    }


    /*CompositeInEventsNotification*/

    void AVInputImplementation::OnCompositeInHotPlug(dsCompositeInPort_t port, bool isConnected)
    {
        LOGINFO("Received OnCompositeInHotPlug callback, port: %d, isConnected: %s",port, isConnected ? "true" : "false");
        AVInputImplementation::AVInputHotplug(port,isConnected ? AV_HOT_PLUG_EVENT_CONNECTED : AV_HOT_PLUG_EVENT_DISCONNECTED, INPUT_TYPE_INT_COMPOSITE);
    }

    void AVInputImplementation::OnCompositeInSignalStatus(dsCompositeInPort_t port, dsCompInSignalStatus_t signalStatus)
    {
        LOGINFO("Received OnCompositeInSignalStatus callback, port: %d, signalStatus: %d",port, signalStatus);
        AVInputImplementation::AVInputSignalChange(port, signalStatus, INPUT_TYPE_INT_COMPOSITE);
    }

    void AVInputImplementation::OnCompositeInStatus(dsCompositeInPort_t activePort, bool isPresented)
    {
        LOGINFO("Received OnCompositeInStatus callback, port: %d, isPresented: %s",
                activePort, isPresented ? "true" : "false");

        AVInputImplementation::AVInputStatusChange(activePort, isPresented, INPUT_TYPE_INT_COMPOSITE);
    }

    void AVInputImplementation::OnCompositeInVideoModeUpdate(dsCompositeInPort_t activePort, dsVideoPortResolution_t videoResolution)
    {
        LOGINFO("Received OnCompositeInVideoModeUpdate callback, port: %d, pixelResolution: %d, interlaced: %d, frameRate: %d",
                activePort,
                videoResolution.pixelResolution,
                videoResolution.interlaced,
                videoResolution.frameRate);

        AVInputImplementation::AVInputVideoModeUpdate(activePort, videoResolution, INPUT_TYPE_INT_COMPOSITE);
    }

    Core::hresult AVInputImplementation::GetSupportedGameFeatures(IStringIterator*& features, bool& success)
    {
        Core::hresult result = Core::ERROR_NONE;
        success = true;
        features = nullptr;
        std::vector<std::string> supportedFeatures;
        try {
            device::HdmiInput::getInstance().getSupportedGameFeatures(supportedFeatures);
        } catch (const device::Exception& err) {
            LOG_DEVICE_EXCEPTION0();
            success = false;
            result = Core::ERROR_GENERAL;
        }

        if (!supportedFeatures.empty() && result == Core::ERROR_NONE) {
            features = Core::Service<RPC::IteratorType<IStringIterator>>::Create<IStringIterator>(supportedFeatures);
        } else {
            success = false;
            result = Core::ERROR_GENERAL;
        }

        return result;
    }

    Core::hresult AVInputImplementation::GetGameFeatureStatus(const string& portId, const string& gameFeature, bool& mode, bool& success)
    {
        int id;

        try {
		    id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("GetGameFeatureStatus: Invalid paramater: portId: %s ", portId.c_str());
            success = false;
            return Core::ERROR_GENERAL;
        }

        if (gameFeature == STR_ALLM) {
            mode = getALLMStatus(id);
        } else if (gameFeature == VRR_TYPE_HDMI) {
            dsHdmiInVrrStatus_t vrrStatus;
            getVRRStatus(id, &vrrStatus);
            mode = (vrrStatus.vrrType == dsVRR_HDMI_VRR);
        } else if (gameFeature == VRR_TYPE_FREESYNC) {
            dsHdmiInVrrStatus_t vrrStatus;
            getVRRStatus(id, &vrrStatus);
            mode = (vrrStatus.vrrType == dsVRR_AMD_FREESYNC);
        } else if (gameFeature == VRR_TYPE_FREESYNC_PREMIUM) {
            dsHdmiInVrrStatus_t vrrStatus;
            getVRRStatus(id, &vrrStatus);
            mode = (vrrStatus.vrrType == dsVRR_AMD_FREESYNC_PREMIUM);
        } else if (gameFeature == VRR_TYPE_FREESYNC_PREMIUM_PRO) {
            dsHdmiInVrrStatus_t vrrStatus;
            getVRRStatus(id, &vrrStatus);
            mode = (vrrStatus.vrrType == dsVRR_AMD_FREESYNC_PREMIUM_PRO);
        } else {
            LOGWARN("AVInputImplementation::GetGameFeatureStatus Unsupported feature: %s", gameFeature.c_str());
            success = false;
            return Core::ERROR_GENERAL;
        }

        success = true;
        return Core::ERROR_NONE;
    }

    bool AVInputImplementation::getALLMStatus(int iPort)
    {
        bool allm = false;

        try {
            device::HdmiInput::getInstance().getHdmiALLMStatus(iPort, &allm);
            LOGWARN("AVInputImplementation::getALLMStatus ALLM MODE: %d", allm);
        } catch (const device::Exception& err) {
            LOG_DEVICE_EXCEPTION1(std::to_string(iPort));
        }
        return allm;
    }

    bool AVInputImplementation::getVRRStatus(int iPort, dsHdmiInVrrStatus_t* vrrStatus)
    {
        bool ret = true;
        try {
            device::HdmiInput::getInstance().getVRRStatus(iPort, vrrStatus);
            LOGWARN("AVInputImplementation::getVRRStatus VRR TYPE: %d, VRR FRAMERATE: %f", vrrStatus->vrrType, vrrStatus->vrrAmdfreesyncFramerate_Hz);
        } catch (const device::Exception& err) {
            LOG_DEVICE_EXCEPTION1(std::to_string(iPort));
            ret = false;
        }
        return ret;
    }

    Core::hresult AVInputImplementation::GetVRRFrameRate(const string& portId, double& currentVRRVideoFrameRate, bool& success)
    {
        int id;

        try {
		    id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("GetVRRFrameRate: Invalid paramater: portId: %s ", portId.c_str());
            success = false;
            return Core::ERROR_GENERAL;
        }

        dsHdmiInVrrStatus_t vrrStatus;
        vrrStatus.vrrAmdfreesyncFramerate_Hz = 0;

        success = getVRRStatus(id, &vrrStatus);
        if(success == true)
        {
            currentVRRVideoFrameRate = vrrStatus.vrrAmdfreesyncFramerate_Hz;
        }

        return success ? Core::ERROR_NONE : Core::ERROR_GENERAL;
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
            return Core::ERROR_GENERAL;
        }

        vector<uint8_t> spdVect({ 'u', 'n', 'k', 'n', 'o', 'w', 'n' });
        HDMISPD.clear();
        try {
            LOGWARN("AVInputImplementation::getSPDInfo");
            vector<uint8_t> spdVect2;
            device::HdmiInput::getInstance().getHDMISPDInfo(id, spdVect2);
            spdVect = spdVect2; // spdVect must be "unknown" unless we successfully get to this line

            // convert to base64
            uint16_t size = min(spdVect.size(), (size_t)numeric_limits<uint16_t>::max());

            LOGWARN("AVInputImplementation::getSPD size:%d spdVec.size:%zu", size, spdVect.size());

            if (spdVect.size() > (size_t)numeric_limits<uint16_t>::max()) {
                LOGERR("Size too large to use ToString base64 wpe api");
                success = false;
                return Core::ERROR_GENERAL;
            }

            LOGINFO("------------getSPD: ");
            for (size_t itr = 0; itr < spdVect.size(); itr++) {
                LOGINFO("%02X ", spdVect[itr]);
            }
            Core::ToString((uint8_t*)&spdVect[0], size, false, HDMISPD);
        } catch (const device::Exception& err) {
            LOG_DEVICE_EXCEPTION1(std::to_string(id));
            success = false;
            return Core::ERROR_GENERAL;
        }

        success = true;
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
            return Core::ERROR_GENERAL;
        }

        vector<uint8_t> spdVect({ 'u', 'n', 'k', 'n', 'o', 'w', 'n' });

        LOGINFO("AVInputImplementation::GetSPD");

        try {
            vector<uint8_t> spdVect2;
            device::HdmiInput::getInstance().getHDMISPDInfo(id, spdVect2);
            spdVect = spdVect2; // edidVec must be "unknown" unless we successfully get to this line

            // convert to base64
            uint16_t size = min(spdVect.size(), (size_t)numeric_limits<uint16_t>::max());

            LOGWARN("AVInputImplementation::GetSPD size:%d spdVec.size:%zu", size, spdVect.size());

            if (spdVect.size() > (size_t)numeric_limits<uint16_t>::max()) {
                LOGERR("Size too large to use ToString base64 wpe api");
                success = false;
                return Core::ERROR_GENERAL;
            }

            LOGINFO("------------getSPD: ");
            for (size_t itr = 0; itr < spdVect.size(); itr++) {
                LOGINFO("%02X ", spdVect[itr]);
            }

            if (spdVect.size() > 0) {
                struct dsSpd_infoframe_st pre;
                memcpy(&pre, spdVect.data(), sizeof(struct dsSpd_infoframe_st));

                char str[200] = { 0 };
                snprintf(str, sizeof(str), "Packet Type:%02X,Version:%u,Length:%u,vendor name:%s,product des:%s,source info:%02X",
                    pre.pkttype, pre.version, pre.length, pre.vendor_name, pre.product_des, pre.source_info);
                HDMISPD = str;
            }
        } catch (const device::Exception& err) {
            LOG_DEVICE_EXCEPTION1(std::to_string(id));
            success = false;
            return Core::ERROR_GENERAL;
        }

        success = true;
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::SetMixerLevels(const int primaryVolume, const int inputVolume, SuccessResult& successResult)
    {
        if( (primaryVolume >=0) && (inputVolume >=0) ) {
                m_primVolume = primaryVolume;
                m_inputVolume = inputVolume;
        } else {
            LOGERR("Invalid params\n");
            successResult.success = false;
            return Core::ERROR_GENERAL;
        }

        if(m_primVolume > MAX_PRIM_VOL_LEVEL) {
       	     LOGWARN("Primary Volume greater than limit. Set to MAX_PRIM_VOL_LEVEL(100) !!!\n");
       	     m_primVolume = MAX_PRIM_VOL_LEVEL;
        }

        if(m_inputVolume > DEFAULT_INPUT_VOL_LEVEL) {
                LOGWARN("INPUT Volume greater than limit. Set to DEFAULT_INPUT_VOL_LEVEL(100) !!!\n");
                m_inputVolume = DEFAULT_INPUT_VOL_LEVEL;
        }

        try {
            device::Host::getInstance().setAudioMixerLevels(dsAUDIO_INPUT_PRIMARY, primaryVolume);
            device::Host::getInstance().setAudioMixerLevels(dsAUDIO_INPUT_SYSTEM, inputVolume);
        } catch (...) {
            LOGWARN("Not setting SoC volume !!!\n");
            successResult.success = false;
            return Core::ERROR_GENERAL;
        }

        isAudioBalanceSet = true;
        successResult.success = true;
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::SetEdid2AllmSupport(const string& portId, const bool allmSupport, SuccessResult& successResult)
    {
        int id;

        try {
		    id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("SetEdid2AllmSupport: Invalid paramater: portId: %s ", portId.c_str());
            successResult.success = false;
            return Core::ERROR_GENERAL;
        }

        try {
            device::HdmiInput::getInstance().setEdid2AllmSupport(id, allmSupport);
            LOGWARN("AVInput -  allmsupport:%d", allmSupport);
        } catch (const device::Exception& err) {
            LOG_DEVICE_EXCEPTION1(std::to_string(id));
            successResult.success = false;
            return Core::ERROR_GENERAL;
        }

        successResult.success = true;
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::GetEdid2AllmSupport(const string& portId, bool& allmSupport, bool& success)
    {
        int id;

        try {
		    id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("GetEdid2AllmSupport: Invalid paramater: portId: %s ", portId.c_str());
            success = false;
            return Core::ERROR_GENERAL;
        }

        allmSupport = true;

        try {
            device::HdmiInput::getInstance().getEdid2AllmSupport(id, &allmSupport);
            LOGINFO("AVInput - getEdid2AllmSupport:%d", allmSupport);
        } catch (const device::Exception& err) {
            LOG_DEVICE_EXCEPTION1(std::to_string(id));
            success = false;
            return Core::ERROR_GENERAL;
        }

        success = true;
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::GetVRRSupport(const string& portId, bool& vrrSupport, bool& success)
    {
        int id;

        try {
		    id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("GetVRRSupport: Invalid paramater: portId: %s ", portId.c_str());
            success = false;
            return Core::ERROR_GENERAL;
        }

        vrrSupport = true;

        try {
            device::HdmiInput::getInstance().getVRRSupport(id, &vrrSupport);
            LOGINFO("AVInput - getVRRSupport:%d", vrrSupport);
        } catch (const device::Exception& err) {
            LOG_DEVICE_EXCEPTION1(std::to_string(id));
            success = false;
            return Core::ERROR_GENERAL;
        }

        success = true;
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::SetVRRSupport(const string& portId, const bool vrrSupport, SuccessResult& successResult)
    {
        int id;

        try {
		    id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("SetVRRSupport: Invalid paramater: portId: %s ", portId.c_str());
            successResult.success = false;
            return Core::ERROR_GENERAL;
        }

        try {
            device::HdmiInput::getInstance().setVRRSupport(id, vrrSupport);
            LOGWARN("AVInput -  vrrSupport:%d", vrrSupport);
        } catch (const device::Exception& err) {
            LOG_DEVICE_EXCEPTION1(std::to_string(id));
            successResult.success = false;
            return Core::ERROR_GENERAL;
        }

        successResult.success = true;
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::GetHdmiVersion(const string& portId, string& HdmiCapabilityVersion, bool& success)
    {
        int id;

        try {
		    id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("GetHdmiVersion: Invalid paramater: portId: %s ", portId.c_str());
            success = false;
            return Core::ERROR_GENERAL;
        }

        dsHdmiMaxCapabilityVersion_t hdmiCapVersion = HDMI_COMPATIBILITY_VERSION_14;

        try {
            device::HdmiInput::getInstance().getHdmiVersion(id, &hdmiCapVersion);
            LOGWARN("AVInputImplementation::GetHdmiVersion Hdmi Version:%d", hdmiCapVersion);
        } catch (const device::Exception& err) {
            LOG_DEVICE_EXCEPTION1(std::to_string(id));
            success = false;
            return Core::ERROR_GENERAL;
        }

        switch ((int)hdmiCapVersion) {
        case HDMI_COMPATIBILITY_VERSION_14:
            HdmiCapabilityVersion = "1.4";
            success = true;
            break;
        case HDMI_COMPATIBILITY_VERSION_20:
            HdmiCapabilityVersion = "2.0";
            success = true;
            break;
        case HDMI_COMPATIBILITY_VERSION_21:
            HdmiCapabilityVersion = "2.1";
            success = true;
            break;
        default:
            success = false;
            return Core::ERROR_NONE;
        }

        if (hdmiCapVersion == HDMI_COMPATIBILITY_VERSION_MAX) {
            success = false;
            return Core::ERROR_GENERAL;
        }

        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::SetEdidVersion(const string& portId, const string& edidVersion, SuccessResult& successResult)
    {
        int id;

        try {
		    id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("SetEdidVersion: Invalid paramater: portId: %s ", portId.c_str());
            successResult.success = false;
            return Core::ERROR_GENERAL;
        }

        int edidVer = -1;

        if (strcmp(edidVersion.c_str(), "HDMI1.4") == 0) {
            edidVer = HDMI_EDID_VER_14;
        } else if (strcmp(edidVersion.c_str(), "HDMI2.0") == 0) {
            edidVer = HDMI_EDID_VER_20;
        } else {
            LOGERR("Invalid EDID Version: %s", edidVersion.c_str());
            successResult.success = false;
            return Core::ERROR_GENERAL;
        }

        try {
            device::HdmiInput::getInstance().setEdidVersion(id, edidVer);
            LOGWARN("AVInputImplementation::setEdidVersion EDID Version: %s", edidVersion.c_str());
        } catch (const device::Exception& err) {
            LOG_DEVICE_EXCEPTION1(std::to_string(id));
            successResult.success = false;
            return Core::ERROR_GENERAL;
        }

        successResult.success = true;
        return Core::ERROR_NONE;
    }

    Core::hresult AVInputImplementation::GetEdidVersion(const string& portId, string& edidVersion, bool& success)
    {
        int id;

        try {
		    id = stoi(portId);
        } catch (const std::exception& err) {
            LOGERR("GetEdidVersion: Invalid paramater: portId: %s ", portId.c_str());
            success = false;
            return Core::ERROR_GENERAL;
        }

        int version = -1;

        try {
            device::HdmiInput::getInstance().getEdidVersion(id, &version);
            LOGWARN("AVInputImplementation::getEdidVersion EDID Version:%d", version);
        } catch (const device::Exception& err) {
            LOG_DEVICE_EXCEPTION1(std::to_string(id));
            success = false;
            return Core::ERROR_GENERAL;
        }

        switch (version) {
        case HDMI_EDID_VER_14:
            edidVersion = "HDMI1.4";
            break;
        case HDMI_EDID_VER_20:
            edidVersion = "HDMI2.0";
            break;
        default:
            success = false;
            return Core::ERROR_GENERAL;
        }

        success = true;
        return Core::ERROR_NONE;
    }

} // namespace Plugin
} // namespace WPEFramework
