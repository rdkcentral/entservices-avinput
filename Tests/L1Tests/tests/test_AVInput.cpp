/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2024 RDK Management
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

#include <gtest/gtest.h>
#include "COMLinkMock.h"
#include <gmock/gmock.h>

#include "AVInput.h"

#include "CompositeInputMock.h"
#include "FactoriesImplementation.h"
#include "HdmiInputMock.h"
#include "ManagerMock.h"
#include "HostMock.h"
#include "IarmBusMock.h"
#include "ServiceMock.h"
#include "ThunderPortability.h"

#include "AVInputImplementation.h"
#include "AVInputMock.h"
#include "WorkerPoolImplementation.h"

using namespace WPEFramework;

using ::testing::NiceMock;


class AVInputTest : public ::testing::Test {

    protected:

    Core::ProxyType<Plugin::AVInput> plugin;
    Core::JSONRPC::Handler& handler;
    DECL_CORE_JSONRPC_CONX connection;
    Core::ProxyType<Plugin::AVInputImplementation> AVInputImpl;

    NiceMock<ServiceMock> service;
    NiceMock<COMLinkMock> comLinkMock;

    Core::ProxyType<WorkerPoolImplementation> workerPool;
    string response;

    AVInputMock* p_avInputMock                          = nullptr;
    HdmiInputImplMock* p_hdmiInputImplMock              = nullptr;
    CompositeInputImplMock* p_compositeInputImplMock    = nullptr;
    HostImplMock* p_HostImplMock                        = nullptr;
    IarmBusImplMock* p_iarmBusImplMock                  = nullptr;
    ManagerImplMock* p_managerImplMock                  = nullptr;

    PLUGINHOST_DISPATCHER* dispatcher;

    AVInputTest()
        : plugin(Core::ProxyType<Plugin::AVInput>::Create())
        , handler(*(plugin))
        , INIT_CONX(1, 0)
        , workerPool(Core::ProxyType<WorkerPoolImplementation>::Create(2, Core::Thread::DefaultStackSize(), 16))
        , dispatcher(nullptr)
    {
        p_hdmiInputImplMock  = new NiceMock <HdmiInputImplMock>;
        device::HdmiInput::setImpl(p_hdmiInputImplMock);

        p_compositeInputImplMock = new NiceMock<CompositeInputImplMock>;
        device::CompositeInput::setImpl(p_compositeInputImplMock);

        p_HostImplMock = new NiceMock<HostImplMock>;
        device::Host::setImpl(p_HostImplMock);

        p_managerImplMock  = new NiceMock <ManagerImplMock>;
        device::Manager::setImpl(p_managerImplMock);

        EXPECT_CALL(*p_managerImplMock, Initialize())
            .Times(::testing::AnyNumber())
            .WillRepeatedly(::testing::Return());

        p_avInputMock  = new NiceMock<AVInputMock>;

        Core::IWorkerPool::Assign(&(*workerPool));
        workerPool->Run();

        dispatcher = static_cast<PLUGINHOST_DISPATCHER*>(
            plugin->QueryInterface(PLUGINHOST_DISPATCHER_ID));
        dispatcher->Activate(&service);

        #ifdef USE_THUNDER_R4
        ON_CALL(comLinkMock, Instantiate(::testing::_, ::testing::_, ::testing::_))
                .WillByDefault(::testing::Invoke(
                    [&](const RPC::Object& object, const uint32_t waitTime, uint32_t& connectionId) {
                        AVInputImpl = Core::ProxyType<Plugin::AVInputImplementation>::Create();
                        return &AVInputImpl;
                    }));
        #else
            ON_CALL(comLinkMock, Instantiate(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
                .WillByDefault(::testing::Return(AVInputImpl));
        #endif

        p_iarmBusImplMock  = new NiceMock <IarmBusImplMock>;
        IarmBus::setImpl(p_iarmBusImplMock);

        plugin->Initialize(&service);
    }

    virtual ~AVInputTest()
    {
        dispatcher->Deactivate();
        dispatcher->Release();

        plugin->Deinitialize(&service);

        Core::IWorkerPool::Assign(nullptr);
        workerPool.Release();

        IarmBus::setImpl(nullptr);
        if (p_iarmBusImplMock != nullptr) {
            delete p_iarmBusImplMock;
            p_iarmBusImplMock = nullptr;
        }

        if (p_avInputMock != nullptr) {
            delete p_avInputMock;
            p_avInputMock = nullptr;
        }

        device::Manager::setImpl(nullptr);
        if (p_managerImplMock != nullptr)
        {
            delete p_managerImplMock;
            p_managerImplMock = nullptr;
        }

        device::HdmiInput::setImpl(nullptr);
        if (p_hdmiInputImplMock != nullptr)
        {
            delete p_hdmiInputImplMock;
            p_hdmiInputImplMock = nullptr;
        }

        device::CompositeInput::setImpl(nullptr);
        if (p_compositeInputImplMock != nullptr) {
            delete p_compositeInputImplMock;
            p_compositeInputImplMock = nullptr;
        }

        device::Host::setImpl(nullptr);
        if (p_HostImplMock != nullptr) {
            delete p_HostImplMock;
            p_HostImplMock = nullptr;
        }
    }
};

TEST_F(AVInputTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("numberOfInputs")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("currentVideoMode")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("contentProtected")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setEdid2AllmSupport")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getEdid2AllmSupport")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setVRRSupport")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getVRRSupport")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getVRRFrameRate")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getInputDevices")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("writeEDID")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("readEDID")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getRawSPD")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getSPD")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setEdidVersion")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getEdidVersion")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getHdmiVersion")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setMixerLevels")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("startInput")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("stopInput")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setVideoRectangle")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getSupportedGameFeatures")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getGameFeatureStatus")));
}

TEST_F(AVInputTest, contentProtected)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("contentProtected"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"isContentProtected\":true,\"success\":true}"));
}

class AVInputDsTest : public AVInputTest {

    protected:

    AVInputDsTest() : AVInputTest() {}
    virtual ~AVInputDsTest() override {}
};

TEST_F(AVInputDsTest, numberOfInputs)
{
    ON_CALL(*p_hdmiInputImplMock, getNumberOfInputs())
        .WillByDefault(::testing::Return(1));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("numberOfInputs"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"numberOfInputs\":1,\"success\":true}"));
}

TEST_F(AVInputDsTest, currentVideoMode)
{
    ON_CALL(*p_hdmiInputImplMock, getCurrentVideoMode())
        .WillByDefault(::testing::Return(string("unknown")));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("currentVideoMode"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"currentVideoMode\":\"unknown\",\"success\":true}"));
}

TEST_F(AVInputDsTest, getEdid2AllmSupport)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getEdid2AllmSupport"), _T("{\"portId\": \"0\",\"allmSupport\":true}"), response));
    EXPECT_EQ(response, string("{\"allmSupport\":true,\"success\":true}"));
}

TEST_F(AVInputDsTest, getEdid2AllmSupport_ErrorCase)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getEdid2AllmSupport"), _T("{\"portId\": \"test\",\"allmSupport\":true}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(AVInputDsTest, setEdid2AllmSupport)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEdid2AllmSupport"), _T("{\"portId\": \"0\",\"allmSupport\":true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(AVInputDsTest, setEdid2AllmSupport_ErrorCase)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setEdid2AllmSupport"), _T("{\"portId\": \"test\",\"allmSupport\":true}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(AVInputDsTest, getVRRSupport)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getVRRSupport"), _T("{\"portId\": \"0\",\"vrrSupport\":true}"), response));
    EXPECT_EQ(response, string("{\"vrrSupport\":true,\"success\":true}"));
}

TEST_F(AVInputDsTest, getVRRSupport_ErrorCase)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getVRRSupport"), _T("{\"portId\": \"test\",\"vrrSupport\":true}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(AVInputDsTest, setVRRSupport)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setVRRSupport"), _T("{\"portId\": \"0\",\"vrrSupport\":true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(AVInputDsTest, setVRRSupport_ErrorCase)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setVRRSupport"), _T("{\"portId\": \"test\",\"vrrSupport\":true}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(AVInputDsTest, getVRRFrameRate)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getVRRFrameRate"), _T("{\"portId\": \"0\"}"), response));
    EXPECT_EQ(response, string("{\"currentVRRVideoFrameRate\":0,\"success\":true}"));
}

TEST_F(AVInputDsTest, getVRRFrameRate_ErrorCase)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getVRRFrameRate"), _T("{\"portId\": \"test\"}"), response));
    EXPECT_EQ(response, string(""));
}

class AVInputInit : public AVInputDsTest {
protected:
    NiceMock<FactoriesImplementation> factoriesImplementation;
    Core::JSONRPC::Message message;

    AVInputInit()
        : AVInputDsTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);
    }

    virtual ~AVInputInit() override
    {
        PluginHost::IFactories::Assign(nullptr);
    }
};

TEST_F(AVInputInit, getInputDevices)
{
    EXPECT_CALL(*p_hdmiInputImplMock, getNumberOfInputs())
        .WillOnce(::testing::Return(1));
    EXPECT_CALL(*p_compositeInputImplMock, getNumberOfInputs())
        .WillOnce(::testing::Return(1));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getInputDevices"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"devices\":[{\"id\":0,\"connected\":false,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\"},{\"id\":0,\"connected\":false,\"locator\":\"cvbsin:\\/\\/localhost\\/deviceid\\/0\"}],\"success\":true}"));
}

TEST_F(AVInputInit, getInputDevices_HDMI)
{
    EXPECT_CALL(*p_hdmiInputImplMock, getNumberOfInputs())
        .WillOnce(::testing::Return(1));
    EXPECT_CALL(*p_hdmiInputImplMock, isPortConnected(::testing::_))
        .WillOnce(::testing::Return(true));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getInputDevices"), _T("{\"typeOfInput\": \"HDMI\"}"), response));
    EXPECT_EQ(response, string("{\"devices\":[{\"id\":0,\"connected\":true,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\"}],\"success\":true}"));
}

TEST_F(AVInputInit, getInputDevices_COMPOSITE)
{
    EXPECT_CALL(*p_compositeInputImplMock, getNumberOfInputs())
        .WillOnce(::testing::Return(1));
    EXPECT_CALL(*p_compositeInputImplMock, isPortConnected(::testing::_))
        .WillOnce(::testing::Return(true));
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getInputDevices"), _T("{\"typeOfInput\": \"COMPOSITE\"}"), response));
    EXPECT_EQ(response, string("{\"devices\":[{\"id\":0,\"connected\":true,\"locator\":\"cvbsin:\\/\\/localhost\\/deviceid\\/0\"}],\"success\":true}"));
}

TEST_F(AVInputInit, getInputDevices_InvalidParameters)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getInputDevices"), _T("{\"typeOfInput\": \"Test\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(AVInputInit, writeEDID)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("writeEDID"), _T("{\"portId\": \"1\",\"message\":\"Test\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(AVInputInit, writeEDID_InvalidParameters)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("writeEDID"), _T("{}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(AVInputInit, readEDID)
{
    EXPECT_CALL(*p_hdmiInputImplMock, getEDIDBytesInfo(::testing::_, ::testing::_))
        .WillOnce([](int port, std::vector<uint8_t>& edid) {
            EXPECT_EQ(port, 1);
            edid = { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("readEDID"), _T("{\"portId\": \"1\"}"), response));
    EXPECT_EQ(response, string("{\"EDID\":\"AP\\/\\/\\/\\/\\/\\/\\/w==\",\"success\":true}"));
}

TEST_F(AVInputInit, readEDIDFailure)
{
    EXPECT_CALL(*p_hdmiInputImplMock, getEDIDBytesInfo(::testing::_, ::testing::_))
        .WillOnce([](int port, std::vector<uint8_t>& edid) {
            edid = {};
        });

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("readEDID"), _T("{\"portId\": \"1\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(AVInputInit, readEDID_InvalidParameters)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("readEDID"), _T("{\"portId\": \"test\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(AVInputInit, getRawSPD)
{
    EXPECT_CALL(*p_hdmiInputImplMock, getHDMISPDInfo(::testing::_, ::testing::_))
        .WillOnce([](int port, std::vector<uint8_t>& data) {
            data = { 0x53, 0x50, 0x44, 0x00 };
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getRawSPD"), _T("{\"portId\": \"1\"}"), response));
    EXPECT_EQ(response, string("{\"HDMISPD\":\"U1BEAA\",\"success\":true}"));
}

TEST_F(AVInputInit, getRawSPD_InvalidParameters)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getRawSPD"), _T("{\"portId\": \"test\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(AVInputInit, getSPD)
{
    EXPECT_CALL(*p_hdmiInputImplMock, getHDMISPDInfo(::testing::_, ::testing::_))
        .WillOnce([](int port, std::vector<uint8_t>& data) {
            data = { 
                0x53, 0x50, 0x44,                           // Type, Version, Length
                'w', 'n', ' ', ' ', ' ', ' ', ' ', ' ',     // Vendor name: "wn      " (8 bytes)
                ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',     // Product desc: "        " (16 bytes)
                ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                0x00                                         // Source info: 0x00
            };
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getSPD"), _T("{\"portId\": \"1\"}"), response));
    EXPECT_THAT(response, ::testing::HasSubstr("\"Packet Type:53"));
    EXPECT_THAT(response, ::testing::HasSubstr("Version:80"));
    EXPECT_THAT(response, ::testing::HasSubstr("Length:68"));
}

TEST_F(AVInputInit, getSPD_InvalidParameters)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getSPD"), _T("{\"portId\": \"test\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(AVInputInit, setEdidVersion)
{
    EXPECT_CALL(*p_hdmiInputImplMock, setEdidVersion(::testing::_, ::testing::_))
        .WillOnce([](int port, int edidVersion) {
            EXPECT_EQ(port, 1);
            EXPECT_EQ(edidVersion, 0);
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setEdidVersion"), _T("{\"portId\": \"1\", \"edidVersion\":\"HDMI1.4\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(AVInputInit, setEdidVersion_InvalidParameters)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setEdidVersion"), _T("{\"portId\": \"test\", \"edidVersion\":\"test\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(AVInputInit, getEdidVersion1)
{
    EXPECT_CALL(*p_hdmiInputImplMock, getEdidVersion(::testing::_, ::testing::_))
        .WillOnce([](int port, int* edidVersion) {
            EXPECT_EQ(port, 0);
            *edidVersion = 0;
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getEdidVersion"), _T("{\"portId\": \"0\"}"), response));
    EXPECT_EQ(response, string("{\"edidVersion\":\"HDMI1.4\",\"success\":true}"));
}

TEST_F(AVInputInit, getEdidVersion2)
{
    EXPECT_CALL(*p_hdmiInputImplMock, getEdidVersion(::testing::_, ::testing::_))
        .WillOnce([](int port, int* edidVersion) {
            EXPECT_EQ(port, 1);
            *edidVersion = 1;
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getEdidVersion"), _T("{\"portId\": \"1\"}"), response));
    EXPECT_EQ(response, string("{\"edidVersion\":\"HDMI2.0\",\"success\":true}"));
}

TEST_F(AVInputInit, getEdidVersion_InvalidParameters)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getEdidVersion"), _T("{\"portId\": \"test\"}}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(AVInputInit, getHdmiVersion)
{
    EXPECT_CALL(*p_hdmiInputImplMock, getHdmiVersion(::testing::_, ::testing::_))
        .WillOnce([](int port, dsHdmiMaxCapabilityVersion_t* capVersion) {
            if (capVersion) {
                *capVersion = HDMI_COMPATIBILITY_VERSION_21;
            }
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getHdmiVersion"), _T("{\"portId\": \"1\"}"), response));
    EXPECT_EQ(response, string("{\"HdmiCapabilityVersion\":\"2.1\",\"success\":true}"));
}

TEST_F(AVInputInit, getHdmiVersion_InvalidParameters)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getHdmiVersion"), _T("{\"portId\": \"test\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(AVInputInit, setMixerLevels)
{
    EXPECT_CALL(*p_HostImplMock, setAudioMixerLevels(dsAUDIO_INPUT_PRIMARY, ::testing::_))
        .WillOnce([](dsAudioInput_t input, int volume) {
            EXPECT_EQ(input, dsAUDIO_INPUT_PRIMARY);
            EXPECT_EQ(volume, 50);
        });

    EXPECT_CALL(*p_HostImplMock, setAudioMixerLevels(dsAUDIO_INPUT_SYSTEM, ::testing::_))
        .WillOnce([](dsAudioInput_t input, int volume) {
            EXPECT_EQ(input, dsAUDIO_INPUT_SYSTEM);
            EXPECT_EQ(volume, 30);
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setMixerLevels"), _T("{\"primaryVolume\": 50 ,\"inputVolume\":30}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(AVInputInit, setMixerLevelsErrorCase)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setMixerLevels"), _T("{\"primaryVolume\": 110 ,\"inputVolume\":110}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(AVInputInit, startInput_HDMI)
{
    EXPECT_CALL(*p_hdmiInputImplMock, selectPort(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce([](int8_t Port, bool audioMix, int videoPlane, bool topMost) {
            EXPECT_EQ(Port, 1);
            EXPECT_EQ(audioMix, true);
            EXPECT_EQ(videoPlane, 1);
            EXPECT_EQ(topMost, true);
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startInput"), _T("{\"portId\": 1 ,\"typeOfInput\":\"HDMI\", \"requestAudioMix\": true, \"plane\" : 1, \"topMost\" : true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(AVInputInit, startInput_COMPOSITE)
{
    EXPECT_CALL(*p_compositeInputImplMock, selectPort(::testing::_))
        .WillOnce([](int8_t Port) {
            EXPECT_EQ(Port, 2);
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startInput"), _T("{\"portId\": 2 ,\"typeOfInput\":\"COMPOSITE\", \"requestAudioMix\": true, \"plane\" : 1, \"topMost\" : true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(AVInputInit, startInput_InvalidParameters)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("startInput"), _T("{\"portId\": \"test\" ,\"typeOfInput\":\"HDMI\", \"requestAudioMix\": true, \"plane\" : 1, \"topMost\" : true}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(AVInputInit, stopInput_HDMI)
{
    EXPECT_CALL(*p_hdmiInputImplMock, selectPort(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce([](int8_t Port, bool audioMix, int videoPlane, bool topMost) {
            EXPECT_EQ(Port, -1);
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("stopInput"), _T("{\"typeOfInput\":\"HDMI\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(AVInputInit, stopInput_COMPOSITE)
{
    EXPECT_CALL(*p_compositeInputImplMock, selectPort(::testing::_))
        .WillOnce([](int8_t Port) {
            EXPECT_EQ(Port, -1);
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("stopInput"), _T("{\"typeOfInput\":\"COMPOSITE\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(AVInputInit, stopInput_COMPOSITE_InvalidParameters)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("stopInput"), _T("{\"typeOfInput\":\"DP\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(AVInputInit, setVideoRectangle_HDMI)
{
    EXPECT_CALL(*p_hdmiInputImplMock, scaleVideo(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce([](int32_t x, int32_t y, int32_t width, int32_t height) {
            EXPECT_EQ(x, 0);
            EXPECT_EQ(y, 0);
            EXPECT_EQ(width, 3840);
            EXPECT_EQ(height, 2160);
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setVideoRectangle"), _T("{\"x\" : 0, \"y\" : 0, \"w\" : 3840, \"h\" : 2160 ,\"typeOfInput\":\"HDMI\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(AVInputInit, setVideoRectangle_COMPOSITE)
{
    EXPECT_CALL(*p_compositeInputImplMock, scaleVideo(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce([](int32_t x, int32_t y, int32_t width, int32_t height) {
            EXPECT_EQ(x, 0);
            EXPECT_EQ(y, 0);
            EXPECT_EQ(width, 720);
            EXPECT_EQ(height, 480);
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setVideoRectangle"), _T("{\"x\" : 0, \"y\" : 0, \"w\" : 720, \"h\" : 480 ,\"typeOfInput\":\"COMPOSITE\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
}

TEST_F(AVInputInit, setVideoRectangle_InvalidParameters)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("setVideoRectangle"), _T("{\"x\" : 0, \"y\" : 0, \"w\" : 720, \"h\" : 480 ,\"typeOfInput\":\"DP\"}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(AVInputInit, getSupportedGameFeatures)
{
    EXPECT_CALL(*p_hdmiInputImplMock, getSupportedGameFeatures(::testing::_))
        .WillOnce([](std::vector<std::string>& featureList) {
            featureList = { "ALLM", "VRR", "QMS" };
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getSupportedGameFeatures"), _T("{}"), response));
    EXPECT_EQ(response, string("{\"supportedGameFeatures\":[\"ALLM\",\"VRR\",\"QMS\"],\"success\":true}"));
}

TEST_F(AVInputInit, getSupportedGameFeatures_ErrorCase)
{
    EXPECT_CALL(*p_hdmiInputImplMock, getSupportedGameFeatures(::testing::_))
        .WillOnce([](std::vector<std::string>& featureList) {
            featureList = {};
        });

    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getSupportedGameFeatures"), _T("{}"), response));
    EXPECT_EQ(response, string(""));
}

TEST_F(AVInputInit, getGameFeatureStatus_ALLM)
{
    EXPECT_CALL(*p_hdmiInputImplMock, getHdmiALLMStatus(::testing::_, ::testing::_))
        .WillOnce([](int iHdmiPort, bool* allmStatus) {
            EXPECT_EQ(iHdmiPort, 1);
            *allmStatus = true;
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getGameFeatureStatus"), _T("{\"portId\" : 1, \"gameFeature\" : \"ALLM\"}"), response));
    EXPECT_EQ(response, string("{\"mode\":true,\"success\":true}"));
}

TEST_F(AVInputInit, getGameFeatureStatus_VRR_HDMI)
{
    EXPECT_CALL(*p_hdmiInputImplMock, getVRRStatus(::testing::_, ::testing::_))
        .WillOnce([](int iHdmiPort, dsHdmiInVrrStatus_t* vrrStatus) {
            ASSERT_NE(vrrStatus, nullptr);
            vrrStatus->vrrType = dsVRR_HDMI_VRR;
            vrrStatus->vrrAmdfreesyncFramerate_Hz = 90.00;
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getGameFeatureStatus"), _T("{\"portId\" : 1, \"gameFeature\" : \"VRR-HDMI\"}"), response));
    EXPECT_EQ(response, string("{\"mode\":true,\"success\":true}"));
}

TEST_F(AVInputInit, getGameFeatureStatus_VRR_FREESYNC)
{
    EXPECT_CALL(*p_hdmiInputImplMock, getVRRStatus(::testing::_, ::testing::_))
        .WillOnce([](int iHdmiPort, dsHdmiInVrrStatus_t* vrrStatus) {
            ASSERT_NE(vrrStatus, nullptr);
            vrrStatus->vrrType = dsVRR_AMD_FREESYNC;
            vrrStatus->vrrAmdfreesyncFramerate_Hz = 100.00;
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getGameFeatureStatus"), _T("{\"portId\" : 1, \"gameFeature\" : \"VRR-FREESYNC\"}"), response));
    EXPECT_EQ(response, string("{\"mode\":true,\"success\":true}"));
}

TEST_F(AVInputInit, getGameFeatureStatus_VRR_FREESYNC_PREMIUM)
{
    EXPECT_CALL(*p_hdmiInputImplMock, getVRRStatus(::testing::_, ::testing::_))
        .WillOnce([](int iHdmiPort, dsHdmiInVrrStatus_t* vrrStatus) {
            ASSERT_NE(vrrStatus, nullptr);
            vrrStatus->vrrType = dsVRR_AMD_FREESYNC_PREMIUM;
            vrrStatus->vrrAmdfreesyncFramerate_Hz = 120.00;
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getGameFeatureStatus"), _T("{\"portId\" : 1, \"gameFeature\" : \"VRR-FREESYNC-PREMIUM\"}"), response));
    EXPECT_EQ(response, string("{\"mode\":true,\"success\":true}"));
}

TEST_F(AVInputInit, getGameFeatureStatus_VRR_FREESYNC_PREMIUM_PRO)
{
    EXPECT_CALL(*p_hdmiInputImplMock, getVRRStatus(::testing::_, ::testing::_))
        .WillOnce([](int iHdmiPort, dsHdmiInVrrStatus_t* vrrStatus) {
            ASSERT_NE(vrrStatus, nullptr);
            vrrStatus->vrrType = dsVRR_AMD_FREESYNC_PREMIUM_PRO;
            vrrStatus->vrrAmdfreesyncFramerate_Hz = 144.00;
        });

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("getGameFeatureStatus"), _T("{\"portId\" : 1, \"gameFeature\" : \"VRR-FREESYNC-PREMIUM-PRO\"}"), response));
    EXPECT_EQ(response, string("{\"mode\":true,\"success\":true}"));
}

TEST_F(AVInputInit, getGameFeatureStatus_InvalidParameters)
{
    EXPECT_EQ(Core::ERROR_GENERAL, handler.Invoke(connection, _T("getGameFeatureStatus"), _T("{\"portId\" : \"test\", \"gameFeature\" : \"VRR-FREESYNC-PREMIUM-PRO\"}"), response));
    EXPECT_EQ(response, string(""));
}

class AVInputEvents : public AVInputDsTest {
protected:
    NiceMock<FactoriesImplementation> factoriesImplementation;
    Core::JSONRPC::Message message;

    Exchange::IAVInput::IDevicesChangedNotification*            DevicesChangedNotification          = nullptr;
    Exchange::IAVInput::ISignalChangedNotification*             SignalChangedNotification           = nullptr;
    Exchange::IAVInput::IInputStatusChangedNotification*        InputStatusChangedNotification      = nullptr;
    Exchange::IAVInput::IVideoStreamInfoUpdateNotification*     VideoStreamInfoUpdateNotification   = nullptr;
    Exchange::IAVInput::IGameFeatureStatusUpdateNotification*   GameFeatureStatusUpdateNotification = nullptr;
    Exchange::IAVInput::IAviContentTypeUpdateNotification*      AviContentTypeUpdateNotification    = nullptr;

    AVInputEvents()
        : AVInputDsTest()
    {
        PluginHost::IFactories::Assign(&factoriesImplementation);

        ON_CALL(*p_avInputMock, RegisterDevicesChangedNotification(::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](Exchange::IAVInput::IDevicesChangedNotification* notification) {
                    DevicesChangedNotification = notification;
            return Core::ERROR_NONE;
                }));

        ON_CALL(*p_avInputMock, RegisterSignalChangedNotification(::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](Exchange::IAVInput::ISignalChangedNotification* notification) {
                    SignalChangedNotification = notification;
            return Core::ERROR_NONE;
                }));

        ON_CALL(*p_avInputMock, RegisterInputStatusChangedNotification(::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](Exchange::IAVInput::IInputStatusChangedNotification* notification) {
                    InputStatusChangedNotification = notification;
            return Core::ERROR_NONE;
                }));

        ON_CALL(*p_avInputMock, RegisterVideoStreamInfoUpdateNotification(::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](Exchange::IAVInput::IVideoStreamInfoUpdateNotification* notification) {
                    VideoStreamInfoUpdateNotification = notification;
            return Core::ERROR_NONE;
                }));

        ON_CALL(*p_avInputMock, RegisterGameFeatureStatusUpdateNotification(::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](Exchange::IAVInput::IGameFeatureStatusUpdateNotification* notification) {
                    GameFeatureStatusUpdateNotification = notification;
            return Core::ERROR_NONE;
                }));

        ON_CALL(*p_avInputMock, RegisterAviContentTypeUpdateNotification(::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](Exchange::IAVInput::IAviContentTypeUpdateNotification* notification) {
                    AviContentTypeUpdateNotification = notification;
            return Core::ERROR_NONE;
                }));
    }

    virtual ~AVInputEvents() override
    {
        PluginHost::IFactories::Assign(nullptr);
    }
};

TEST_F(AVInputEvents, onDevicesChangedHDMI)
{
    Core::Event onDevicesChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.onDevicesChanged\",\"params\":{\"devices\":[]}}");

                onDevicesChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("onDevicesChanged"), _T("org.rdk.AVInput"), message);

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInEventHotPlug(dsHDMI_IN_PORT_0, true);

    EXPECT_EQ(Core::ERROR_NONE, onDevicesChanged.Lock());

    EVENT_UNSUBSCRIBE(0, _T("onDevicesChanged"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, onDevicesChangedCOMPOSITE)
{
    Core::Event onDevicesChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.onDevicesChanged\",\"params\":{\"devices\":[]}}");

                onDevicesChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("onDevicesChanged"), _T("org.rdk.AVInput"), message);

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnCompositeInHotPlug(dsCOMPOSITE_IN_PORT_0, true);

    EXPECT_EQ(Core::ERROR_NONE, onDevicesChanged.Lock());

    EVENT_UNSUBSCRIBE(0, _T("onDevicesChanged"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, onSignalChangedStableHDMI)
{
    Core::Event onSignalChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text,
                    "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.onSignalChanged\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"signalStatus\":\"stableSignal\"}}");

                onSignalChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInEventSignalStatus(dsHDMI_IN_PORT_0, dsHDMI_IN_SIGNAL_STATUS_STABLE);

    EXPECT_EQ(Core::ERROR_NONE, onSignalChanged.Lock());

    EVENT_UNSUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, onSignalChangedNoSignalHDMI)
{
    Core::Event onSignalChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text,
                    "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.onSignalChanged\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"signalStatus\":\"noSignal\"}}");

                onSignalChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInEventSignalStatus(dsHDMI_IN_PORT_0, dsHDMI_IN_SIGNAL_STATUS_NOSIGNAL);

    EXPECT_EQ(Core::ERROR_NONE, onSignalChanged.Lock());

    EVENT_UNSUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, onSignalChangedUnstableHDMI)
{
    Core::Event onSignalChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text,
                    "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.onSignalChanged\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"signalStatus\":\"unstableSignal\"}}");

                onSignalChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInEventSignalStatus(dsHDMI_IN_PORT_0,dsHDMI_IN_SIGNAL_STATUS_UNSTABLE);

    EXPECT_EQ(Core::ERROR_NONE, onSignalChanged.Lock());

    EVENT_UNSUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, onSignalChangedNotSupportedHDMI)
{
    Core::Event onSignalChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.onSignalChanged\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"signalStatus\":\"notSupportedSignal\"}}");

                onSignalChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInEventSignalStatus(dsHDMI_IN_PORT_0, dsHDMI_IN_SIGNAL_STATUS_NOTSUPPORTED);

    EXPECT_EQ(Core::ERROR_NONE, onSignalChanged.Lock());

    EVENT_UNSUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, onSignalChangedDefaultHDMI)
{
    Core::Event onSignalChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.onSignalChanged\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"signalStatus\":\"none\"}}");

                onSignalChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);
    
    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInEventSignalStatus(dsHDMI_IN_PORT_0, dsHDMI_IN_SIGNAL_STATUS_MAX);

    EXPECT_EQ(Core::ERROR_NONE, onSignalChanged.Lock());

    EVENT_UNSUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, onSignalChangedStableCOMPOSITE)
{
    Core::Event onSignalChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.onSignalChanged\",\"params\":{\"id\":0,\"locator\":\"cvbsin:\\/\\/localhost\\/deviceid\\/0\",\"signalStatus\":\"stableSignal\"}}");

                onSignalChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);
    
    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnCompositeInSignalStatus(dsCOMPOSITE_IN_PORT_0, dsCOMP_IN_SIGNAL_STATUS_STABLE);

    EXPECT_EQ(Core::ERROR_NONE, onSignalChanged.Lock());

    EVENT_UNSUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, onSignalChangedNoSignalCOMPOSITE)
{
    Core::Event onSignalChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.onSignalChanged\",\"params\":{\"id\":0,\"locator\":\"cvbsin:\\/\\/localhost\\/deviceid\\/0\",\"signalStatus\":\"noSignal\"}}");

                onSignalChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnCompositeInSignalStatus(dsCOMPOSITE_IN_PORT_0, dsCOMP_IN_SIGNAL_STATUS_NOSIGNAL);

    EXPECT_EQ(Core::ERROR_NONE, onSignalChanged.Lock());

    EVENT_UNSUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, onSignalChangedUnstableCOMPOSITE)
{
    Core::Event onSignalChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.onSignalChanged\",\"params\":{\"id\":0,\"locator\":\"cvbsin:\\/\\/localhost\\/deviceid\\/0\",\"signalStatus\":\"unstableSignal\"}}");

                onSignalChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnCompositeInSignalStatus(dsCOMPOSITE_IN_PORT_0, dsCOMP_IN_SIGNAL_STATUS_UNSTABLE);

    EXPECT_EQ(Core::ERROR_NONE, onSignalChanged.Lock());

    EVENT_UNSUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, onSignalChangedNotSupportedCOMPOSITE)
{
    Core::Event onSignalChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.onSignalChanged\",\"params\":{\"id\":0,\"locator\":\"cvbsin:\\/\\/localhost\\/deviceid\\/0\",\"signalStatus\":\"notSupportedSignal\"}}");

                onSignalChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);
    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnCompositeInSignalStatus(dsCOMPOSITE_IN_PORT_0, dsCOMP_IN_SIGNAL_STATUS_NOTSUPPORTED);

    EXPECT_EQ(Core::ERROR_NONE, onSignalChanged.Lock());

    EVENT_UNSUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, onSignalChangedDefaultCOMPOSITE)
{
    Core::Event onSignalChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.onSignalChanged\",\"params\":{\"id\":0,\"locator\":\"cvbsin:\\/\\/localhost\\/deviceid\\/0\",\"signalStatus\":\"none\"}}");

                onSignalChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);
    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnCompositeInSignalStatus(dsCOMPOSITE_IN_PORT_0, dsCOMP_IN_SIGNAL_STATUS_MAX);

    EXPECT_EQ(Core::ERROR_NONE, onSignalChanged.Lock());

    EVENT_UNSUBSCRIBE(0, _T("onSignalChanged"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, onInputStatusChangeOn_HDMI)
{
    Core::Event onInputStatusChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.onInputStatusChanged\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"status\":\"started\",\"plane\":1}}");

                onInputStatusChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("onInputStatusChanged"), _T("org.rdk.AVInput"), message);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startInput"), _T("{\"portId\": \"0\" , \"typeOfInput\":\"HDMI\", \"requestAudioMix\": true, \"plane\" : 1, \"topMost\" : true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInEventStatus(dsHDMI_IN_PORT_0, true);

    EXPECT_EQ(Core::ERROR_NONE, onInputStatusChanged.Lock());

    EVENT_UNSUBSCRIBE(0, _T("onInputStatusChanged"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, onInputStatusChangeOff_HDMI)
{
    Core::Event onInputStatusChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.onInputStatusChanged\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"status\":\"stopped\",\"plane\":-1}}");

                onInputStatusChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("onInputStatusChanged"), _T("org.rdk.AVInput"), message);
   

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("stopInput"), _T("{\"typeOfInput\":\"HDMI\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInEventStatus(dsHDMI_IN_PORT_0, false);


    EXPECT_EQ(Core::ERROR_NONE, onInputStatusChanged.Lock());

    EVENT_UNSUBSCRIBE(0, _T("onInputStatusChanged"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, onInputStatusChangeOn_COMPOSITE)
{
    Core::Event onInputStatusChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.onInputStatusChanged\",\"params\":{\"id\":0,\"locator\":\"cvbsin:\\/\\/localhost\\/deviceid\\/0\",\"status\":\"started\",\"plane\":1}}");

                onInputStatusChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("onInputStatusChanged"), _T("org.rdk.AVInput"), message);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("startInput"), _T("{\"portId\": \"0\" , \"typeOfInput\":\"COMPOSITE\", \"requestAudioMix\": true, \"plane\" : 1, \"topMost\" : true}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnCompositeInStatus(dsCOMPOSITE_IN_PORT_0, true);

    EXPECT_EQ(Core::ERROR_NONE, onInputStatusChanged.Lock());

    EVENT_UNSUBSCRIBE(0, _T("onInputStatusChanged"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, onInputStatusChangeOff_COMPOSITE)
{
    Core::Event onInputStatusChanged(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.onInputStatusChanged\",\"params\":{\"id\":0,\"locator\":\"cvbsin:\\/\\/localhost\\/deviceid\\/0\",\"status\":\"stopped\",\"plane\":-1}}");

                onInputStatusChanged.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("onInputStatusChanged"), _T("org.rdk.AVInput"), message);

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("stopInput"), _T("{\"typeOfInput\":\"COMPOSITE\"}"), response));
    EXPECT_EQ(response, string("{\"success\":true}"));
    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnCompositeInStatus(dsCOMPOSITE_IN_PORT_0, false);

    EXPECT_EQ(Core::ERROR_NONE, onInputStatusChanged.Lock());

    EVENT_UNSUBSCRIBE(0, _T("onInputStatusChanged"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, hdmiGameFeatureStatusUpdate)
{
    Core::Event gameFeatureStatusUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.gameFeatureStatusUpdate\",\"params\":{\"id\":0,\"gameFeature\":\"ALLM\",\"mode\":true}}");

                gameFeatureStatusUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("gameFeatureStatusUpdate"), _T("org.rdk.AVInput"), message);

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInAllmStatus(dsHDMI_IN_PORT_0,true);

    EXPECT_EQ(Core::ERROR_NONE, gameFeatureStatusUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("gameFeatureStatusUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, hdmiGameFeatureStatusUpdate_HDMI_VRR)
{
    Core::Event gameFeatureStatusUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.gameFeatureStatusUpdate\",\"params\":{\"id\":0,\"gameFeature\":\"VRR-HDMI\",\"mode\":true}}");

                gameFeatureStatusUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("gameFeatureStatusUpdate"), _T("org.rdk.AVInput"), message);

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVRRStatus(dsHDMI_IN_PORT_0,dsVRR_HDMI_VRR);

    EXPECT_EQ(Core::ERROR_NONE, gameFeatureStatusUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("gameFeatureStatusUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, hdmiGameFeatureStatusUpdate_AMD_FREESYNC)
{
    Core::Event gameFeatureStatusUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.gameFeatureStatusUpdate\",\"params\":{\"id\":0,\"gameFeature\":\"VRR-FREESYNC\",\"mode\":true}}");

                gameFeatureStatusUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("gameFeatureStatusUpdate"), _T("org.rdk.AVInput"), message);

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVRRStatus(dsHDMI_IN_PORT_0, dsVRR_AMD_FREESYNC);

    EXPECT_EQ(Core::ERROR_NONE, gameFeatureStatusUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("gameFeatureStatusUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, hdmiGameFeatureStatusUpdate_AMD_FREESYNC_PREMIUM)
{
    Core::Event gameFeatureStatusUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.gameFeatureStatusUpdate\",\"params\":{\"id\":0,\"gameFeature\":\"VRR-FREESYNC-PREMIUM\",\"mode\":true}}");

                gameFeatureStatusUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("gameFeatureStatusUpdate"), _T("org.rdk.AVInput"), message);
    
    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVRRStatus(dsHDMI_IN_PORT_0, dsVRR_AMD_FREESYNC_PREMIUM);

    EXPECT_EQ(Core::ERROR_NONE, gameFeatureStatusUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("gameFeatureStatusUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, hdmiGameFeatureStatusUpdate_AMD_FREESYNC_PREMIUM_PRO)
{
    Core::Event gameFeatureStatusUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.gameFeatureStatusUpdate\",\"params\":{\"id\":0,\"gameFeature\":\"VRR-FREESYNC-PREMIUM-PRO\",\"mode\":true}}");

                gameFeatureStatusUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("gameFeatureStatusUpdate"), _T("org.rdk.AVInput"), message);

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVRRStatus(dsHDMI_IN_PORT_0, dsVRR_AMD_FREESYNC_PREMIUM_PRO);

    EXPECT_EQ(Core::ERROR_NONE, gameFeatureStatusUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("gameFeatureStatusUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, videoStreamInfoUpdate1_HDMI)
{
    Core::Event videoStreamInfoUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":1920,\"height\":1080,\"progressive\":false,\"frameRateN\":60000,\"frameRateD\":1001}}");

                videoStreamInfoUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);

    dsVideoPortResolution_t videoPortResolution;
    videoPortResolution.pixelResolution = dsVIDEO_PIXELRES_1920x1080;
    videoPortResolution.interlaced = true;
    videoPortResolution.frameRate = dsVIDEO_FRAMERATE_59dot94;

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVideoModeUpdate(dsHDMI_IN_PORT_0,videoPortResolution);

    EXPECT_EQ(Core::ERROR_NONE, videoStreamInfoUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, videoStreamInfoUpdate2_HDMI)
{
    Core::Event videoStreamInfoUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":720,\"height\":480,\"progressive\":false,\"frameRateN\":24000,\"frameRateD\":1000}}");

                videoStreamInfoUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);

    dsVideoPortResolution_t videoPortResolution;
    videoPortResolution.pixelResolution = dsVIDEO_PIXELRES_720x480;
    videoPortResolution.interlaced = true;
    videoPortResolution.frameRate = dsVIDEO_FRAMERATE_24;
    
    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVideoModeUpdate(dsHDMI_IN_PORT_0, videoPortResolution);

    EXPECT_EQ(Core::ERROR_NONE, videoStreamInfoUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, videoStreamInfoUpdate3_HDMI)
{
    Core::Event videoStreamInfoUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":720,\"height\":576,\"progressive\":false,\"frameRateN\":25000,\"frameRateD\":1000}}");

                videoStreamInfoUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);

    dsVideoPortResolution_t videoPortResolution;
    videoPortResolution.pixelResolution = dsVIDEO_PIXELRES_720x576;
    videoPortResolution.interlaced = true;
    videoPortResolution.frameRate = dsVIDEO_FRAMERATE_25;

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVideoModeUpdate(dsHDMI_IN_PORT_0, videoPortResolution);

    EXPECT_EQ(Core::ERROR_NONE, videoStreamInfoUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, videoStreamInfoUpdate4_HDMI)
{
    Core::Event videoStreamInfoUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":3840,\"height\":2160,\"progressive\":false,\"frameRateN\":30000,\"frameRateD\":1000}}");

                videoStreamInfoUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);

    dsVideoPortResolution_t videoPortResolution;
    
    videoPortResolution.pixelResolution = dsVIDEO_PIXELRES_3840x2160;
    videoPortResolution.interlaced = true;
    videoPortResolution.frameRate = dsVIDEO_FRAMERATE_30;

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVideoModeUpdate(dsHDMI_IN_PORT_0, videoPortResolution);

    EXPECT_EQ(Core::ERROR_NONE, videoStreamInfoUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, videoStreamInfoUpdate5_HDMI)
{
    Core::Event videoStreamInfoUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":4096,\"height\":2160,\"progressive\":false,\"frameRateN\":50000,\"frameRateD\":1000}}");

                videoStreamInfoUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);

    dsVideoPortResolution_t videoPortResolution;
    
    videoPortResolution.pixelResolution = dsVIDEO_PIXELRES_4096x2160;
    videoPortResolution.interlaced = true;
    videoPortResolution.frameRate = dsVIDEO_FRAMERATE_50;
    
    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVideoModeUpdate(dsHDMI_IN_PORT_0, videoPortResolution);
    EXPECT_EQ(Core::ERROR_NONE, videoStreamInfoUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, videoStreamInfoUpdate6_HDMI)
{
    Core::Event videoStreamInfoUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":4096,\"height\":2160,\"progressive\":false,\"frameRateN\":60000,\"frameRateD\":1000}}");

                videoStreamInfoUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
    
    dsVideoPortResolution_t videoPortResolution;
    
    videoPortResolution.pixelResolution = dsVIDEO_PIXELRES_4096x2160;
    videoPortResolution.interlaced = true;
    videoPortResolution.frameRate = dsVIDEO_FRAMERATE_60;

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVideoModeUpdate(dsHDMI_IN_PORT_0, videoPortResolution);

    EXPECT_EQ(Core::ERROR_NONE, videoStreamInfoUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, videoStreamInfoUpdate7_HDMI)
{
    Core::Event videoStreamInfoUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text,"{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":4096,\"height\":2160,\"progressive\":false,\"frameRateN\":24000,\"frameRateD\":1001}}");

                videoStreamInfoUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);

    dsVideoPortResolution_t videoPortResolution;
    
    videoPortResolution.pixelResolution = dsVIDEO_PIXELRES_4096x2160;
    videoPortResolution.interlaced = true;
    videoPortResolution.frameRate = dsVIDEO_FRAMERATE_23dot98;

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVideoModeUpdate(dsHDMI_IN_PORT_0, videoPortResolution);

    EXPECT_EQ(Core::ERROR_NONE, videoStreamInfoUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, videoStreamInfoUpdate8_HDMI)
{
    Core::Event videoStreamInfoUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text,"{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":4096,\"height\":2160,\"progressive\":false,\"frameRateN\":30000,\"frameRateD\":1001}}");

                videoStreamInfoUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);

    dsVideoPortResolution_t videoPortResolution;
    
    videoPortResolution.pixelResolution = dsVIDEO_PIXELRES_4096x2160;
    videoPortResolution.interlaced = true;
    videoPortResolution.frameRate = dsVIDEO_FRAMERATE_29dot97;

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVideoModeUpdate(dsHDMI_IN_PORT_0, videoPortResolution);

    EXPECT_EQ(Core::ERROR_NONE, videoStreamInfoUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, videoStreamInfoUpdate9_HDMI)
{
    Core::Event videoStreamInfoUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text,"{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":1280,\"height\":720,\"progressive\":false,\"frameRateN\":30000,\"frameRateD\":1001}}");

                videoStreamInfoUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);

    dsVideoPortResolution_t videoPortResolution;
    
    videoPortResolution.pixelResolution = dsVIDEO_PIXELRES_1280x720;
    videoPortResolution.interlaced = true;
    videoPortResolution.frameRate = dsVIDEO_FRAMERATE_29dot97;

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVideoModeUpdate(dsHDMI_IN_PORT_0, videoPortResolution);
    
    EXPECT_EQ(Core::ERROR_NONE, videoStreamInfoUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, videoStreamInfoUpdate10_HDMI)
{
    Core::Event videoStreamInfoUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text,"{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":1280,\"height\":720,\"progressive\":false,\"frameRateN\":100000,\"frameRateD\":1000}}");

                videoStreamInfoUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);

    dsVideoPortResolution_t videoPortResolution;
    
    videoPortResolution.pixelResolution = dsVIDEO_PIXELRES_1280x720;
    videoPortResolution.interlaced = true;
    videoPortResolution.frameRate = dsVIDEO_FRAMERATE_100;

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVideoModeUpdate(dsHDMI_IN_PORT_0, videoPortResolution);

    EXPECT_EQ(Core::ERROR_NONE, videoStreamInfoUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, videoStreamInfoUpdate11_HDMI)
{
    Core::Event videoStreamInfoUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text,"{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":1280,\"height\":720,\"progressive\":false,\"frameRateN\":120000,\"frameRateD\":1001}}");

                videoStreamInfoUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);

    dsVideoPortResolution_t videoPortResolution;
    
    videoPortResolution.pixelResolution = dsVIDEO_PIXELRES_1280x720;
    videoPortResolution.interlaced = true;
    videoPortResolution.frameRate = dsVIDEO_FRAMERATE_119dot88;

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVideoModeUpdate(dsHDMI_IN_PORT_0, videoPortResolution);

    EXPECT_EQ(Core::ERROR_NONE, videoStreamInfoUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, videoStreamInfoUpdate12_HDMI)
{
    Core::Event videoStreamInfoUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text,"{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":1280,\"height\":720,\"progressive\":false,\"frameRateN\":120000,\"frameRateD\":1000}}");

                videoStreamInfoUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);

    dsVideoPortResolution_t videoPortResolution;
    
    videoPortResolution.pixelResolution = dsVIDEO_PIXELRES_1280x720;
    videoPortResolution.interlaced = true;
    videoPortResolution.frameRate = dsVIDEO_FRAMERATE_120;

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVideoModeUpdate(dsHDMI_IN_PORT_0, videoPortResolution);

    EXPECT_EQ(Core::ERROR_NONE, videoStreamInfoUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, videoStreamInfoUpdate13_HDMI)
{
    Core::Event videoStreamInfoUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text,"{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":1280,\"height\":720,\"progressive\":false,\"frameRateN\":200000,\"frameRateD\":1000}}");

                videoStreamInfoUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);

    dsVideoPortResolution_t videoPortResolution;
    
    videoPortResolution.pixelResolution = dsVIDEO_PIXELRES_1280x720;
    videoPortResolution.interlaced = true;
    videoPortResolution.frameRate = dsVIDEO_FRAMERATE_200;

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVideoModeUpdate(dsHDMI_IN_PORT_0, videoPortResolution);

    EXPECT_EQ(Core::ERROR_NONE, videoStreamInfoUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, videoStreamInfoUpdate14_HDMI)
{
    Core::Event videoStreamInfoUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text,"{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":1280,\"height\":720,\"progressive\":false,\"frameRateN\":240000,\"frameRateD\":1001}}");

                videoStreamInfoUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);

    dsVideoPortResolution_t videoPortResolution;
    
    videoPortResolution.pixelResolution = dsVIDEO_PIXELRES_1280x720;
    videoPortResolution.interlaced = true;
    videoPortResolution.frameRate = dsVIDEO_FRAMERATE_239dot76;

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVideoModeUpdate(dsHDMI_IN_PORT_0, videoPortResolution);

    EXPECT_EQ(Core::ERROR_NONE, videoStreamInfoUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, videoStreamInfoUpdate15_HDMI)
{
    Core::Event videoStreamInfoUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text,"{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":1280,\"height\":720,\"progressive\":false,\"frameRateN\":240000,\"frameRateD\":1000}}");

                videoStreamInfoUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);

    dsVideoPortResolution_t videoPortResolution;
    
    videoPortResolution.pixelResolution = dsVIDEO_PIXELRES_1280x720;
    videoPortResolution.interlaced = true;
    videoPortResolution.frameRate = dsVIDEO_FRAMERATE_240;

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVideoModeUpdate(dsHDMI_IN_PORT_0, videoPortResolution);

    EXPECT_EQ(Core::ERROR_NONE, videoStreamInfoUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, videoStreamInfoUpdateDefault_HDMI)
{
    Core::Event videoStreamInfoUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text,"{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"hdmiin:\\/\\/localhost\\/deviceid\\/0\",\"width\":1920,\"height\":1080,\"progressive\":false,\"frameRateN\":60000,\"frameRateD\":1000}}");

                videoStreamInfoUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);

    dsVideoPortResolution_t videoPortResolution;
    
    videoPortResolution.pixelResolution = dsVIDEO_PIXELRES_MAX;
    videoPortResolution.interlaced = true;
    videoPortResolution.frameRate = dsVIDEO_FRAMERATE_MAX;

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInVideoModeUpdate(dsHDMI_IN_PORT_0, videoPortResolution);

    EXPECT_EQ(Core::ERROR_NONE, videoStreamInfoUpdate.Lock());
    
    EVENT_UNSUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, videoStreamInfoUpdate1_COMPOSITE)
{
    Core::Event videoStreamInfoUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text,"{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"cvbsin:\\/\\/localhost\\/deviceid\\/0\",\"width\":720,\"height\":480,\"progressive\":false,\"frameRateN\":24000,\"frameRateD\":1000}}");

                videoStreamInfoUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);

    dsVideoPortResolution_t videoPortResolution;
    videoPortResolution.pixelResolution = dsVIDEO_PIXELRES_720x480;
    videoPortResolution.frameRate = dsVIDEO_FRAMERATE_24;

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnCompositeInVideoModeUpdate(dsCOMPOSITE_IN_PORT_0, videoPortResolution);

    EXPECT_EQ(Core::ERROR_NONE, videoStreamInfoUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
}


TEST_F(AVInputEvents, videoStreamInfoUpdate2_COMPOSITE)
{
    Core::Event videoStreamInfoUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text, "{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"cvbsin:\\/\\/localhost\\/deviceid\\/0\",\"width\":720,\"height\":576,\"progressive\":false,\"frameRateN\":25000,\"frameRateD\":1000}}");

                videoStreamInfoUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);

    dsVideoPortResolution_t videoPortResolution;
    videoPortResolution.pixelResolution = dsVIDEO_PIXELRES_720x576;
    videoPortResolution.frameRate = dsVIDEO_FRAMERATE_25;

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnCompositeInVideoModeUpdate(dsCOMPOSITE_IN_PORT_0, videoPortResolution);

    EXPECT_EQ(Core::ERROR_NONE, videoStreamInfoUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, videoStreamInfoUpdateDefault_COMPOSITE)
{
    Core::Event videoStreamInfoUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text,"{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.videoStreamInfoUpdate\",\"params\":{\"id\":0,\"locator\":\"cvbsin:\\/\\/localhost\\/deviceid\\/0\",\"width\":720,\"height\":576,\"progressive\":false,\"frameRateN\":60000,\"frameRateD\":1000}}");

                videoStreamInfoUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);

    dsVideoPortResolution_t videoPortResolution;
    videoPortResolution.pixelResolution = dsVIDEO_PIXELRES_MAX;
    videoPortResolution.frameRate = dsVIDEO_FRAMERATE_MAX;

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnCompositeInVideoModeUpdate(dsCOMPOSITE_IN_PORT_0, videoPortResolution);

    EXPECT_EQ(Core::ERROR_NONE, videoStreamInfoUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("videoStreamInfoUpdate"), _T("org.rdk.AVInput"), message);
}

TEST_F(AVInputEvents, aviContentTypeUpdate_HDMI)
{
    Core::Event aviContentTypeUpdate(false, true);

    EXPECT_CALL(service, Submit(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [&](const uint32_t, const Core::ProxyType<Core::JSON::IElement>& json) {
                string text;
                EXPECT_TRUE(json->ToString(text));
                EXPECT_EQ(text,"{\"jsonrpc\":\"2.0\",\"method\":\"org.rdk.AVInput.aviContentTypeUpdate\",\"params\":{\"id\":0,\"aviContentType\":0}}");

                aviContentTypeUpdate.SetEvent();

                return Core::ERROR_NONE;
            }));

    EVENT_SUBSCRIBE(0, _T("aviContentTypeUpdate"), _T("org.rdk.AVInput"), message);    

    ASSERT_NE(Plugin::AVInputImplementation::_instance, nullptr);
    Plugin::AVInputImplementation::_instance->OnHdmiInAVIContentType(dsHDMI_IN_PORT_0, dsAVICONTENT_TYPE_GRAPHICS);

    EXPECT_EQ(Core::ERROR_NONE, aviContentTypeUpdate.Lock());

    EVENT_UNSUBSCRIBE(0, _T("aviContentTypeUpdate"), _T("org.rdk.AVInput"), message);
}
