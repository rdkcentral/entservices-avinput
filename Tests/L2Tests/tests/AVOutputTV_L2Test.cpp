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
#include <gmock/gmock.h>
#include "L2Tests.h"
#include "L2TestsMock.h"
#include <mutex>
#include <condition_variable>
#include <fstream>

#define JSON_TIMEOUT   (1000)
#define TEST_LOG(x, ...) fprintf(stderr, "\033[1;32m[%s:%d](%s)<PID:%d><TID:%d>" x "\n\033[0m", __FILE__, __LINE__, __FUNCTION__, getpid(), gettid(), ##__VA_ARGS__); fflush(stderr);
#define AVOUTPUT_CALLSIGN  _T("org.rdk.AVOutput.1")
#define AVOUTPUT_CALLSIGNL2TEST_CALLSIGN _T("L2tests.1")

using ::testing::NiceMock;
using namespace WPEFramework;
using testing::StrictMock;

/**
* @brief Internal test mock class
*
* Note that this is for internal test use only and doesn't mock any actual
* concrete interface.
*/

/* AVOutput L2 test class declaration */
class AVOutput_L2test : public L2TestMocks {
protected:
    Core::JSONRPC::Message message;
    string response;
    IARM_EventHandler_t dsHdmiStatusEventHandler;
    IARM_EventHandler_t dsHdmiVideoModeEventHandler;

    virtual ~AVOutput_L2test() override;

    public:
        AVOutput_L2test();
};

/**
* @brief Constructor for AVOutput L2 test class
*/
AVOutput_L2test::AVOutput_L2test()
        : L2TestMocks()
{
        printf("AVOutput Constructor\n");
        uint32_t status = Core::ERROR_GENERAL;

		std::ofstream devicePropFileStream("/etc/device.properties");
		devicePropFileStream << "RDK_PROFILE=TV";
		devicePropFileStream << "\n";
		devicePropFileStream.close();

        EXPECT_CALL(*p_rfcApiImplMock, getRFCParameter(::testing::_, ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke(
            [](char* pcCallerID, const char* pcParameterName, RFC_ParamData_t* pstParamData) {
                EXPECT_EQ(string(pcCallerID), string("AVOutput"));
                EXPECT_EQ(string(pcParameterName), string("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AVOutput.DynamicAutoLatency"));
                strncpy(pstParamData->value, "true", sizeof(pstParamData->value));
                return WDMP_SUCCESS;
            }));

        ON_CALL(*p_iarmBusImplMock, IARM_Bus_RegisterEventHandler(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](const char* ownerName, IARM_EventId_t eventId, IARM_EventHandler_t handler) {
                   if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_HDMI_IN_STATUS)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsHdmiStatusEventHandler = handler;
                   }
                    if ((string(IARM_BUS_DSMGR_NAME) == string(ownerName)) && (eventId == IARM_BUS_DSMGR_EVENT_HDMI_IN_VIDEO_MODE_UPDATE)) {
                        EXPECT_TRUE(handler != nullptr);
                        dsHdmiVideoModeEventHandler = handler;
                    }
                    return IARM_RESULT_SUCCESS;
                }));

        ON_CALL(*p_hdmiInputImplMock, getCurrentVideoModeObj(::testing::_))
            .WillByDefault(::testing::Invoke(
                [&](dsVideoPortResolution_t& resolution) {
                    resolution.pixelResolution = dsVIDEO_PIXELRES_1920x1080;
                    return tvERROR_NONE;
                }));

        ON_CALL(*p_tvSettingsImplMock, TvInit())
        .WillByDefault(::testing::Return(tvERROR_NONE));

        ON_CALL(*p_tvSettingsImplMock, RegisterVideoFormatChangeCB(testing::_))
        .WillByDefault(testing::Return(tvERROR_NONE));
        ON_CALL(*p_tvSettingsImplMock, RegisterVideoContentChangeCB(testing::_))
        .WillByDefault(testing::Return(tvERROR_NONE));
        ON_CALL(*p_tvSettingsImplMock, RegisterVideoResolutionChangeCB(testing::_))
        .WillByDefault(testing::Return(tvERROR_NONE));
        ON_CALL(*p_tvSettingsImplMock, RegisterVideoFrameRateChangeCB(testing::_))
        .WillByDefault(testing::Return(tvERROR_NONE));

        EXPECT_CALL(*p_tr181ApiImplMock, getLocalParam(::testing::_, ::testing::_, ::testing::_))
        .Times(2)
        .WillOnce(::testing::Invoke(
        [&](char *pcCallerID, const char* pcParameterName, TR181_ParamData_t *pstParamData) {
            EXPECT_EQ(string(pcCallerID), string("AVOutput"));
            strncpy(pstParamData->value, "Normal", sizeof(pstParamData->value));

            return tr181Success;
            }))  
        .WillOnce(::testing::Invoke(
        [&](char *pcCallerID, const char* pcParameterName, TR181_ParamData_t *pstParamData) {
            EXPECT_EQ(string(pcCallerID), string("AVOutput"));
            strncpy(pstParamData->value, "Normal", sizeof(pstParamData->value));

            return tr181Success;
            }));

    ON_CALL(*p_tvSettingsImplMock, SetAspectRatio(testing::_))
    .WillByDefault(testing::Invoke(
        [&](tvDisplayMode_t dispMode) {
            EXPECT_EQ(dispMode, tvDisplayMode_NORMAL);
            return tvERROR_NONE;
            }));

    ON_CALL(*p_tvSettingsImplMock, GetCurrentSource(testing::_))
    .WillByDefault(testing::Invoke(
        [&](tvVideoSrcType_t *currentSource) {
            EXPECT_EQ(*currentSource, VIDEO_SOURCE_IP);
            return tvERROR_NONE;
            }));

    ON_CALL(*p_tvSettingsImplMock, GetCurrentVideoFormat(testing::_))
        .WillByDefault(testing::Invoke(
            [&](tvVideoFormatType_t* videoFormat) {
            EXPECT_EQ(*videoFormat, VIDEO_FORMAT_NONE);
            return tvERROR_NONE; // Return an appropriate error code
        }));

     ON_CALL(*p_tvSettingsImplMock, SetTVPictureMode(testing::_))
     .WillByDefault(testing::Invoke(
        [&](const char * pictureMode) {
            EXPECT_EQ(string(pictureMode), string("normal"));
            return tvERROR_NONE;
        }));

         /* Activate plugin in constructor */
         status = ActivateService("org.rdk.AVOutput");
         EXPECT_EQ(Core::ERROR_NONE, status);
}

/**
* @brief Destructor for AVInput L2 test class
*/
AVOutput_L2test::~AVOutput_L2test()
{
    printf("AVOutput Destructor\n");
    uint32_t status = Core::ERROR_GENERAL;

    ON_CALL(*p_tvSettingsImplMock, TvTerm())
    .WillByDefault(::testing::Return(tvERROR_NONE));

    status = DeactivateService("org.rdk.AVOutput");
    EXPECT_EQ(Core::ERROR_NONE, status);
}

TEST_F(AVOutput_L2test, AVOUTPUT_GETINPUTDEVICE_Test)
{
    JSONRPC::LinkType<Core::JSON::IElement> jsonrpc(AVOUTPUT_CALLSIGN, AVOUTPUT_CALLSIGNL2TEST_CALLSIGN);
    uint32_t status = Core::ERROR_GENERAL;
    JsonObject result, params;

    ON_CALL(*p_tvSettingsImplMock, ReadCapablitiesFromConfODM(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillByDefault([](std::string& rangeInfo, std::string& pqmodeInfo, std::string& formatInfo, std::string& sourceInfo, std::string param, std::string& platformsupport, std::string& index) {
            printf("ReadCapablitiesFromConfODM\n");
            rangeInfo = "\"Standard\",\"Vivid\",\"EnergySaving\",\"Custom\",\"Theater\",\"Game\"";
            pqmodeInfo = "";
            formatInfo = "\"SDR\"";
            sourceInfo = "\"HDMI\",\"HDMI2\"";
            platformsupport = "";
            index = "0";

            return tvERROR_NONE;
        });

    status = InvokeServiceMethod("org.rdk.AVOutput.1", "getPictureModeCaps", params, result);

}
