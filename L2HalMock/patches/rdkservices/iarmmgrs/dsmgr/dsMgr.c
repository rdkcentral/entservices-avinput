/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
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



/**
* @defgroup iarmmgrs
* @{
* @defgroup dsmgr
* @{
**/


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "libIARM.h"
#include "libIBusDaemon.h"
#include "dsMgrInternal.h"
#include "libIBus.h"
#include "iarmUtil.h"

#include "sysMgr.h"

#include "dsMgr.h"
#include "dsUtl.h"
#include "dsError.h"
#include "dsTypes.h"
#include "dsRpc.h"
#include "dsVideoPort.h"
#include "dsVideoResolutionSettings.h"
#include "dsDisplay.h"
#include "dsAudioSettings.h"
#include "dsAudio.h"
#include "safec_lib.h"
#include "rfcapi.h"
#include "dsMgrPwrEventListener.h"

extern IARM_Result_t _dsSetResolution(void *arg);
extern IARM_Result_t _dsGetResolution(void *arg);
extern IARM_Result_t _dsInitResolution(void *arg);
extern IARM_Result_t _dsGetAudioPort(void *arg);
extern IARM_Result_t _dsGetStereoMode(void *arg);
extern IARM_Result_t _dsSetStereoMode(void *arg);
extern IARM_Result_t _dsGetEDID(void *arg);
extern IARM_Result_t _dsGetEDIDBytes(void *arg);
extern IARM_Result_t _dsGetVideoPort(void *arg);
extern IARM_Result_t _dsIsDisplayConnected(void *arg);
extern IARM_Result_t _dsGetStereoAuto(void *arg);
extern IARM_Result_t _dsIsDisplaySurround(void *arg);
extern IARM_Result_t _dsGetForceDisable4K(void *arg);
extern IARM_Result_t _dsSetBackgroundColor(void *arg);
extern IARM_Result_t _dsGetIgnoreEDIDStatus(void *arg);
extern bool isComponentPortPresent();

extern bool dsGetHDMIDDCLineStatus(void);
static int _SetVideoPortResolution();
static int  _SetResolution(intptr_t* handle,dsVideoPortType_t PortType);
static void _EventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len);
static IARM_Result_t _SysModeChange(void *arg);
static void dumpHdmiEdidInfo(dsDisplayEDID_t* pedidData);
static int iTuneReady = 0;
static dsDisplayEvent_t edisplayEventStatus = dsDISPLAY_EVENT_MAX;
static pthread_t edsHDMIHPDThreadID; // HDMI HPD - HDMI Hot Plug detect events
static pthread_mutex_t tdsMutexLock;
static pthread_cond_t  tdsMutexCond;
static void* _DSMgrResnThreadFunc(void *arg);
static void _setAudioMode();
void _setEASAudioMode();
static int iResnCount = 5;
static int iInitResnFlag = 0;
static bool bHDCPAuthenticated = false;
static bool bPwrMgeRFCEnabled = false;
IARM_Bus_Daemon_SysMode_t isEAS = IARM_BUS_SYS_MODE_NORMAL; // Default is Normal Mode

#define RFC_PWRMGR2 "Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Power.PwrMgr2.Enable"

/*DSMgr Glib variables */
/* For glib APIs*/
#include <glib.h>
GMainLoop *dsMgr_Gloop = NULL;
static gboolean heartbeatMsg(gpointer data);
static gboolean _SetResolutionHandler(gpointer data);
static guint hotplug_event_src = 0;
static gboolean dumpEdidOnChecksumDiff(gpointer data);
static bool IsIgnoreEdid_gs = false;

static intptr_t getVideoPortHandle(_dsVideoPortType_t port)
{
    /* Get the HDMI Video Port Parameter */
    dsVideoPortGetHandleParam_t vidPortParam;
    memset(&vidPortParam, 0, sizeof(vidPortParam));
    vidPortParam.type = port;
    vidPortParam.index = 0;
    _dsGetVideoPort(&vidPortParam);
    return vidPortParam.handle;
}

static bool isHDMIConnected()
{
    /* Get the Display connection status */
    dsVideoPortIsDisplayConnectedParam_t ConParam;
    memset(&ConParam, 0, sizeof(ConParam));
    ConParam.handle = getVideoPortHandle(dsVIDEOPORT_TYPE_HDMI);
    _dsIsDisplayConnected(&ConParam);
    return ConParam.connected; 
}

IARM_Result_t DSMgr_Start()
{
	FILE *fDSCtrptr = NULL;
	IARM_Bus_SYSMgr_GetSystemStates_Param_t tuneReadyParam;


	setvbuf(stdout, NULL, _IOLBF, 0);
    INT_INFO("Entering [%s] - [%s] - disabling io redirect buf \r\n", __FUNCTION__, IARM_BUS_DSMGR_NAME);
	
	/* Register with IARM Libs and Connect */
	IARM_Bus_Init(IARM_BUS_DSMGR_NAME);
    IARM_Bus_Connect();
	IARM_Bus_RegisterEvent(IARM_BUS_DSMGR_EVENT_MAX);

	/*Initialize the DS Manager - DS Srv and DS HAL */
	dsMgr_init();
	  
	iInitResnFlag = 1;
        dsEdidIgnoreParam_t ignoreEdidParam;
        memset(&ignoreEdidParam,0,sizeof(ignoreEdidParam));
        ignoreEdidParam.handle = dsVIDEOPORT_TYPE_HDMI;
        _dsGetIgnoreEDIDStatus(&ignoreEdidParam);
	IsIgnoreEdid_gs = ignoreEdidParam.ignoreEDID;
	__TIMESTAMP();printf("ResOverride DSMgr_Start IsIgnoreEdid_gs: %d\n", IsIgnoreEdid_gs);
	/*Register the Events */
	IARM_Bus_RegisterEventHandler(IARM_BUS_SYSMGR_NAME,IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE,_EventHandler);
	IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG,_EventHandler);
	IARM_Bus_RegisterEventHandler(IARM_BUS_DSMGR_NAME,IARM_BUS_DSMGR_EVENT_HDCP_STATUS,_EventHandler);

	/*Register EAS handler so that we can ensure audio settings for EAS */
	IARM_Bus_RegisterCall(IARM_BUS_COMMON_API_SysModeChange, _SysModeChange);

    RFC_ParamData_t rfcParam;
    // WDMP_STATUS status = getRFCParameter("dsMgr", RFC_PWRMGR2, &rfcParam);
	WDMP_STATUS status = WDMP_SUCCESS;
    if(strncmp(rfcParam.value, "true",4) == 0)
    {
        bPwrMgeRFCEnabled= true;
        __TIMESTAMP(); printf("dsMgr:RFC PwrMgr2 feature enabled \r\n");
    }

    if(bPwrMgeRFCEnabled)
	{
        /*Refactored dsMGR code*/
        initPwrEventListner();   
	}
	/* Create  Thread for listening Hot Plug events */
	pthread_mutex_init (&tdsMutexLock, NULL);
	pthread_cond_init (&tdsMutexCond, NULL);
	pthread_create (&edsHDMIHPDThreadID, NULL, _DSMgrResnThreadFunc, NULL);
			
	/* Read the HDMI DDC Line delay to be introduced 
	 * for setting  the resolution
	 * The DDC line is used for EDID and HDCP Negotiation
	 */
	fDSCtrptr = fopen("/opt/ddcDelay","r");
	if (NULL != fDSCtrptr)
	{
		if(0 > fscanf(fDSCtrptr,"%d",&iResnCount))
		{
			INT_INFO("Error: fscanf on ddcDelay failed");
		}
                fclose (fDSCtrptr);
	}
	__TIMESTAMP(); printf("Retry DS manager Resolution count is iResnCount = %d \r\n",iResnCount);


	IARM_Bus_Call(IARM_BUS_SYSMGR_NAME, IARM_BUS_SYSMGR_API_GetSystemStates, &tuneReadyParam, sizeof(tuneReadyParam));
	__TIMESTAMP(); printf("Tune Ready Status on start up is %d \r\n",tuneReadyParam.TuneReadyStatus.state);
    
	/* Read the Tune Ready status on startup  and update the tune ready flag */
	if(1 == tuneReadyParam.TuneReadyStatus.state)
	{
		iTuneReady = 1;
	}

	/* Create Main loop for DS Manager */
    dsMgr_Gloop = g_main_loop_new ( NULL , FALSE );
    if(dsMgr_Gloop != NULL){
        g_timeout_add_seconds (300 , heartbeatMsg , dsMgr_Gloop); 
    }
    else {
        INT_INFO("Fails to Create a main Loop for [%s] \r\n",IARM_BUS_DSMGR_NAME);
    }

    if(!isHDMIConnected())
    {
        __TIMESTAMP();printf("HDMI not connected at bootup -Schedule a handler to set the resolution .. \r\n");
        _SetVideoPortResolution(); 
    }
	return IARM_RESULT_SUCCESS;
}

IARM_Result_t DSMgr_Loop()
{
   
   /* DS Mgr loop */
    if(dsMgr_Gloop)
    { 
        g_main_loop_run (dsMgr_Gloop);
        g_main_loop_unref(dsMgr_Gloop);
    }
	return IARM_RESULT_SUCCESS;
}

static gboolean heartbeatMsg(gpointer data)
{
    time_t curr = 0;
    time(&curr);
    INT_INFO("I-ARM BUS DS Mgr: HeartBeat at %s\r\n", ctime(&curr));
    return TRUE;
}

IARM_Result_t DSMgr_Stop()
{
    
	if(dsMgr_Gloop)
    { 
        g_main_loop_quit(dsMgr_Gloop);
    }
    
    IARM_Bus_Disconnect();
    IARM_Bus_Term();
	pthread_mutex_destroy (&tdsMutexLock);
	pthread_cond_destroy  (&tdsMutexCond);
	
    return IARM_RESULT_SUCCESS;
}

/**
 * @fn static IARM_Result_t _SysModeChange(void *arg){
 * @brief This function is a event handler which returns current system
 *  mode using IARM. It returns mode as  "NORMAL", "WAREHOUSE","EAS" or "UNKNOWN".
 *
 * @param[in] void pointer to void, containing IARM_Bus_CommonAPI_SysModeChange_Param_t data.
 *
 * @return variable of IARM_Result_t type.
 * @retval IARM_RESULT_SUCCESS On function completion.
 */
static IARM_Result_t _SysModeChange(void *arg)
{
    IARM_Bus_CommonAPI_SysModeChange_Param_t *param = (IARM_Bus_CommonAPI_SysModeChange_Param_t *)arg;
    IARM_Bus_Daemon_SysMode_t isNextEAS = IARM_BUS_SYS_MODE_NORMAL;

    __TIMESTAMP();printf("[DSMgr] Recvd Sysmode Change::New mode --> %d,Old mode --> %d",param->newMode,param->oldMode);
         
     if ((param->newMode == IARM_BUS_SYS_MODE_EAS) ||  
     	 (param->newMode == IARM_BUS_SYS_MODE_NORMAL)) 
     {
        isNextEAS = param->newMode;
     }
     else
     {
     	/* Do not process any other mode change as of now for DS Manager.. */
     	return IARM_RESULT_SUCCESS;
     }

    if ((isEAS == IARM_BUS_SYS_MODE_EAS) && (isNextEAS == IARM_BUS_SYS_MODE_NORMAL) ) {
        isEAS = IARM_BUS_SYS_MODE_NORMAL;
        _setAudioMode();
    }
    else if ((isEAS == IARM_BUS_SYS_MODE_NORMAL) && (isNextEAS == IARM_BUS_SYS_MODE_EAS) ) {
    	/* 
    		* Change the Audio Mode to Stereo if Current Audio Setting is Paasthrough
    	*/
        isEAS = IARM_BUS_SYS_MODE_EAS;
        _setEASAudioMode();
        
    }
    else {
        /* no op for no mode change */
    }
    return IARM_RESULT_SUCCESS;
}

static void setBGColor(dsVideoBackgroundColor_t color)
{
    /* Get the HDMI Video Port Parameter */
    dsVideoPortGetHandleParam_t vidPortParam;
    memset(&vidPortParam, 0, sizeof(vidPortParam));
    vidPortParam.type = dsVIDEOPORT_TYPE_HDMI;
    vidPortParam.index = 0;
    _dsGetVideoPort(&vidPortParam);
    vidPortParam.handle;

    if(vidPortParam.handle != NULL)
    {
        dsSetBackgroundColorParam_t setBGColorParam;
        memset(&setBGColorParam, 0, sizeof(setBGColorParam));
        setBGColorParam.color = color;
        setBGColorParam.handle= vidPortParam.handle;
        _dsSetBackgroundColor(&setBGColorParam);  
    }
}


/*Event Handler for DS Manager And Sys Manager Events */
static void _EventHandler(const char *owner, IARM_EventId_t eventId, void *data, size_t len)
{
    /*Handle only Sys Manager Events */
	if (strcmp(owner, IARM_BUS_SYSMGR_NAME)  == 0) 
	{
		/* Only handle state events */
	    if (eventId != IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE) return;
		//	__TIMESTAMP();printf("_sysMgrEventHandler invoked in DS Manager\r\n");
		IARM_Bus_SYSMgr_EventData_t *sysEventData = (IARM_Bus_SYSMgr_EventData_t*)data;
		IARM_Bus_SYSMgr_SystemState_t stateId = sysEventData->data.systemStates.stateId;
		int state = sysEventData->data.systemStates.state;
		__TIMESTAMP();printf("_sysEventHandler invoked for stateid %d of state %d \r\n", stateId, state);
		switch(stateId) {
			case IARM_BUS_SYSMGR_SYSSTATE_TUNEREADY:
					__TIMESTAMP();printf("Tune Ready Evenets in DS Manager \r\n");
				
					if (0 == iTuneReady)
					{
						iTuneReady = 1;

						/* Set audio mode from persistent */
						_setAudioMode();
						
						/* Un-block the Resolution Settings Thread */
						pthread_mutex_lock(&tdsMutexLock);
						pthread_cond_signal(&tdsMutexCond);
						pthread_mutex_unlock(&tdsMutexLock);
					}
  			break;
			default:
				break;
		}
	}else if (strcmp(owner,IARM_BUS_DSMGR_NAME) == 0)
    {
        switch (eventId) {
			case IARM_BUS_DSMGR_EVENT_HDMI_HOTPLUG:
				{
					
					IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
					
					__TIMESTAMP();printf("[DsMgr] Got HDMI %s Event  \r\n",(eventData->data.hdmi_hpd.event == dsDISPLAY_EVENT_CONNECTED ? "Connect" : "Disconnect"));

                                        setBGColor(dsVIDEO_BGCOLOR_NONE);

					/* Un-Block the Resolution Settings Thread */
					pthread_mutex_lock(&tdsMutexLock);
					edisplayEventStatus = ((eventData->data.hdmi_hpd.event == dsDISPLAY_EVENT_CONNECTED) ? dsDISPLAY_EVENT_CONNECTED : dsDISPLAY_EVENT_DISCONNECTED);
					pthread_cond_signal(&tdsMutexCond);
					pthread_mutex_unlock(&tdsMutexLock);
										
				}
                break;
			case IARM_BUS_DSMGR_EVENT_HDCP_STATUS:
				{
					IARM_Bus_DSMgr_EventData_t *eventData = (IARM_Bus_DSMgr_EventData_t *)data;
					IARM_Bus_SYSMgr_EventData_t HDCPeventData;
					int status = eventData->data.hdmi_hdcp.hdcpStatus;
					//__TIMESTAMP();printf("%s: IARM_BUS_DSMGR_EVENT_HDCP_STATUS  event status :%d \r\n",__FUNCTION__, status);
					
					/* HDCP is enabled */
					HDCPeventData.data.systemStates.stateId = IARM_BUS_SYSMGR_SYSSTATE_HDCP_ENABLED;
					HDCPeventData.data.systemStates.state = 1;
					if (status == dsHDCP_STATUS_AUTHENTICATED )
					{
						__TIMESTAMP();printf("Changed status to HDCP Authentication Pass  !!!!!!!! ..\r\n");
						HDCPeventData.data.systemStates.state =  1;
                                                bHDCPAuthenticated = true;
                                                __TIMESTAMP();printf("HDCP success - Removed hotplug_event_src Time source %d and set resolution immediately \r\n",hotplug_event_src);                                          
                                                if(hotplug_event_src)
                                                {
                                                    g_source_remove(hotplug_event_src);
                                                    hotplug_event_src = 0;
                                                }
                                                setBGColor(dsVIDEO_BGCOLOR_NONE);
                                                if (!IsIgnoreEdid_gs) {
                                                    _SetVideoPortResolution();
                                                }
                                                g_timeout_add_seconds((guint)1,dumpEdidOnChecksumDiff,NULL);
					} 
					else if (status == dsHDCP_STATUS_AUTHENTICATIONFAILURE )
					{
						__TIMESTAMP();printf("Changed status to HDCP Authentication Fail   !!!!!!!! ..\r\n");
						HDCPeventData.data.systemStates.state =  0;
                                                setBGColor(dsVIDEO_BGCOLOR_BLUE);
                                                bHDCPAuthenticated = false;
                                                if (!IsIgnoreEdid_gs) {
                                                    _SetVideoPortResolution();
                                                }
                                                g_timeout_add_seconds((guint)1,dumpEdidOnChecksumDiff,NULL);
					}

					IARM_Bus_BroadcastEvent(IARM_BUS_SYSMGR_NAME, (IARM_EventId_t) IARM_BUS_SYSMGR_EVENT_SYSTEMSTATE, (void *)&HDCPeventData, sizeof(HDCPeventData));
					
				}
                break;	
			default:
				break;
			}
	}
}

/* Set Video resolution on HDMI Hot Plug or Tune Ready events  */
static int _SetVideoPortResolution()
{
	intptr_t _hdmihandle = 0;
	intptr_t _comphandle = 0;
	bool connected=false;
	int iCount = 0;
	

	__TIMESTAMP(); printf("%s:Enter \r\n",__FUNCTION__);
	 
        _hdmihandle = getVideoPortHandle(dsVIDEOPORT_TYPE_HDMI);	
	if(_hdmihandle != NULL)
	{		

		usleep(100*1000);	//wait for 100 milli seconds


		/* 	
			* Check for HDMI DDC Line  when HDMI is connected.
		*/
                connected = isHDMIConnected();
		if(iInitResnFlag && connected)
		{

			#ifdef _INIT_RESN_SETTINGS  	
				/*Wait for iResnCount*/
				while(iCount < iResnCount)
				{
					sleep(1);	//wait for 1 sec
					if (dsGetHDMIDDCLineStatus())
					{
						break;
					}
					__TIMESTAMP(); printf ("Waiting for HDMI DDC Line to be ready for resolution Change...\r\n");
					iCount++;
				}
			#endif
		}
		
		/*Set HDMI Resolution if Connected else COomponent or Composite Resolution */	
		if(connected){

			__TIMESTAMP(); printf("Setting HDMI resolution.......... \r\n");
			_SetResolution(&_hdmihandle,dsVIDEOPORT_TYPE_HDMI);
		}
		else {
			_comphandle = getVideoPortHandle(dsVIDEOPORT_TYPE_COMPONENT);
			
			if (NULL != _comphandle)
			{
				__TIMESTAMP();printf("Setting Component/Composite Resolution.......... \r\n");
				_SetResolution(&_comphandle,dsVIDEOPORT_TYPE_COMPONENT);
			}
			else
			{
			    __TIMESTAMP();printf("%s: NULL Handle for component\r\n",__FUNCTION__);

                            intptr_t _compositehandle = getVideoPortHandle(dsVIDEOPORT_TYPE_BB);

                            if (NULL != _compositehandle)
                            {
                                __TIMESTAMP();printf("Setting BB Composite Resolution.......... \r\n");
                                _SetResolution(&_compositehandle,dsVIDEOPORT_TYPE_BB);
                            }
                            else
                            {
                                 __TIMESTAMP();printf("%s: NULL Handle for Composite \r\n",__FUNCTION__);
                                 intptr_t _rfhandle = getVideoPortHandle(dsVIDEOPORT_TYPE_RF);
                                 if (NULL != _rfhandle)
                                 {
                                     __TIMESTAMP();printf("Setting RF Resolution.......... \r\n");
                                     _SetResolution(&_rfhandle,dsVIDEOPORT_TYPE_RF);
                                 }
                                 else
                                 {
                                     __TIMESTAMP();printf("%s: NULL Handle for RF \r\n",__FUNCTION__);
                                 }
                            }
			}
		
		}
	}
	else
	{
		__TIMESTAMP();printf("%s: NULL Handle for HDMI \r\n",__FUNCTION__);
	}
	__TIMESTAMP();printf("%s:Exit \r\n",__FUNCTION__);
	return 0;
}

/**
 * @brief This Function does following :
 *   Read Persisted resolution 
 *   Verify Persisted resolution with Platform and EDID resolution 
 *   If fails set best EDID resolution supported by platform 
 *   If fails Default to 720P
 * 	 If 720p is not supported by TV , Default to 480p
 *  @param void pointer Device Handle
 * * @param Connection Status 
 ** @return void pointer (NULL)
 */
static int  _SetResolution(intptr_t* handle,dsVideoPortType_t PortType)
{
	errno_t rc = -1;
	intptr_t _displayHandle = 0;
	int numResolutions = 0,i=0;
	intptr_t _handle = *handle;
	bool IsValidResolution = false;
	dsVideoPortSetResolutionParam_t Setparam;
	dsVideoPortGetResolutionParam_t Getparam;
	dsVideoPortResolution_t *setResn = NULL;
	dsDisplayEDID_t edidData;
	dsDisplayGetEDIDParam_t Edidparam;
	/*
		* Default Resolution Compatible check is false - Do not Force compatible resolution on startup
	*/
	Setparam.forceCompatible = false;

	/*Initialize the struct*/
	memset(&edidData, 0, sizeof(edidData));
	
	/* Return if Handle is NULL */
	if (_handle == NULL)
	{
		__TIMESTAMP();printf("_SetResolution - Got NULL Handle ..\r\n");
		return 0;
	}
	
	/*Get the User Persisted Resolution Based on Handle */
	memset(&Getparam,0,sizeof(Getparam));
	Getparam.handle = _handle;
	Getparam.toPersist = true;
	_dsGetResolution(&Getparam);
	dsVideoPortResolution_t *presolution = &Getparam.resolution;	
	__TIMESTAMP();printf("Got User Persisted Resolution - %s..\r\n",presolution->name);

		
	if (PortType == dsVIDEOPORT_TYPE_HDMI)	{
		/*Get The Display Handle */
		dsGetDisplay(dsVIDEOPORT_TYPE_HDMI, 0, &_displayHandle);
		if (_displayHandle)
		{
			/* Get the EDID Display Handle */
			 memset(&Edidparam,0,sizeof(Edidparam));
    			Edidparam.handle = _displayHandle;
			_dsGetEDID(&Edidparam);
			rc = memcpy_s(&edidData,sizeof(edidData), &Edidparam.edid, sizeof(Edidparam.edid));
			if(rc!=EOK)
			{
				ERR_CHK(rc);
			}
			dumpHdmiEdidInfo(&edidData);
			numResolutions = edidData.numOfSupportedResolution;
			__TIMESTAMP();printf("numResolutions is %d \r\n",numResolutions);
			
			/*  If HDMI is connected and Low power Mode. 
				The TV might not Transmit the EDID information 
				Change the Resolution in Next Hot plug
				DO not set the Resolution if TV is in DVI mode.
			*/
			if ((0 == numResolutions) || (!(edidData.hdmiDeviceType)))
			{

				__TIMESTAMP();printf("Do not Set Resolution..The HDMI is not Ready  !! \r\n");
				__TIMESTAMP();printf("numResolutions  = %d edidData.hdmiDeviceType = %d !! \r\n",numResolutions,edidData.hdmiDeviceType);
				return 0;
			}
			
			/* 
				* Check if Persisted Resolution matches with 
				* TV Resolution  list	
			*/
			for (i = 0; i < numResolutions; i++)
			{
				setResn = &(edidData.suppResolutionList[i]);
				printf("presolution->name : %s, resolution->name : %s\r\n",presolution->name,setResn->name);
				if ((strcmp(presolution->name,setResn->name) == 0 ))
				{
					__TIMESTAMP();printf("Breaking..Got Platform Resolution - %s..\r\n",setResn->name);
					IsValidResolution = true;
					Setparam.forceCompatible = true;
					break;
				}
			}
                        /*
                        * The Persisted Resolution Does not matches with TV resolution list 
			*  Set  the Best Resolution Supported by TV and Platform
			*/
                        if (false == IsValidResolution)
			{
                        	/* Set  the Best Resolution Supported by TV and Platform*/
                		for (i = numResolutions-1; i >= 0; i--)
                		{
                        		setResn = &(edidData.suppResolutionList[i]);
                        		int pNumResolutions = dsUTL_DIM(kResolutions);
                        		for (int j = pNumResolutions-1; j >=0; j--)
		        		{
		                		dsVideoPortResolution_t *pfResolution = &kResolutions[j];
		                		if (0 == (strcmp(pfResolution->name,setResn->name)))
		                		{
		                			__TIMESTAMP();printf("[DsMgr] Set Best TV Supported Resolution %s \r\n",pfResolution->name);
		                			IsValidResolution = true;
		                    			break;
		                		}
                                        }
                                	if (IsValidResolution)
                                	{
                                		break;
                                        }
                                }
                        }
			/* 
				* The Persisted Resolution Does not matches with TV and Platform 
				* Resolution List
				* Force PLatform Default Resolution 	
			*/
			if (false == IsValidResolution)
			{
				/* Check if the Default platform resolution is supported by Platfrom resolution List i.e 720p */
				dsVideoPortResolution_t *defaultResn; 
				defaultResn = &kResolutions[kDefaultResIndex];
				for (i = 0; i < numResolutions; i++)
				{
					setResn = &(edidData.suppResolutionList[i]);
					//printf("\n presolution->name : %s, resolution->name : %s\n",defaultResn->name,setResn->name);
					if ((strcmp(defaultResn->name,setResn->name) == 0 ))
					{
						IsValidResolution = true;
						__TIMESTAMP();printf("Breaking..Got Default Platform Resolution - %s..\r\n",setResn->name);
						break;
					}
				}
			}

			if (false == IsValidResolution)
			{
				/*Take 480p as resolution if both above cases fail */
                for (i = 0; i < numResolutions; i++)
                {
                    setResn = &(edidData.suppResolutionList[i]);
                    if ((strcmp("480p",setResn->name) == 0 )) 
                	{
						__TIMESTAMP();printf("Breaking..Default to 480p Resolution - %s..\r\n",setResn->name);
						IsValidResolution = true;
						break;
	               	}
				}
			}

			if (false == IsValidResolution)
			{
				/* Boot with  the Resolution Supported by TV and Platform*/
                for (i = 0; i < numResolutions; i++)
                {
                    setResn = &(edidData.suppResolutionList[i]);
                    size_t numResolutions = dsUTL_DIM(kResolutions);
                    for (size_t j = 0; j < numResolutions; j++)
		            {
		                dsVideoPortResolution_t *pfResolution = &kResolutions[j];
		                if (0 == (strcmp(pfResolution->name,setResn->name)))
		                {
		                    __TIMESTAMP();printf("[DsMgr] Boot with TV Supported Resolution %s \r\n",pfResolution->name);
		                    IsValidResolution = true;
		                    break;
		                }    
		            }
				}
			}
		}
	}
	else if (PortType == dsVIDEOPORT_TYPE_COMPONENT || PortType == dsVIDEOPORT_TYPE_BB || PortType == dsVIDEOPORT_TYPE_RF)
	{
		/* Set the Component / Composite  Resolution */	
		numResolutions = dsUTL_DIM(kResolutions);
    	for (i = 0; i < numResolutions; i++)
    	{
    		setResn = &kResolutions[i];
    		if ((strcmp(presolution->name,setResn->name) == 0 ))
    		{
				__TIMESTAMP();printf("Breaking..Got Platform Resolution - %s..\r\n",setResn->name);
        		IsValidResolution = true;
        		break;
    		}
    	}
	}
		/*  If the Persisted Resolution settings does not matches with Platform Resolution - 
			Force Default on Component/Composite 
			This is to keep upward compatible and if we intend to 
			remove any resolution from Dynamic Resolution List
		*/
	if(false == IsValidResolution)
	{
		setResn = &kResolutions[kDefaultResIndex];
	}
	
	/* Set The Video Port Resolution in Requested Handle */
	Setparam.handle = _handle;
	Setparam.toPersist = false;
	
	/* If 4K support is disabled and last known resolution is 4K, default to 720p (aka default resolution) */
	dsForceDisable4KParam_t res_4K_override;
	memset(&res_4K_override, 0, sizeof(res_4K_override));
	_dsGetForceDisable4K((void *) &res_4K_override);
	if(true == res_4K_override.disable)
	{
		if(0 == strncmp(presolution->name, "2160", 4))
		{
			__TIMESTAMP();printf("User persisted 4K resolution. Now limiting to default (720p?) as 4K support is now disabled.\n");
			setResn = &kResolutions[kDefaultResIndex];
		}
	}
	
	Setparam.resolution = *setResn;

	/* Call during Init*/
	#ifdef _INIT_RESN_SETTINGS
		if(0 == iInitResnFlag)
		{
			printf("Init Platform Resolution - %s..\r\n",setResn->name);
			_dsInitResolution(&Setparam);
			return 0 ;
		}
	#endif

	_dsSetResolution(&Setparam);
	return 0 ;
}

/**
 * @brief Thread entry fuction to post Resolution on Hot Plug and Tune ready Events
 *
 * This functions changes the device resolution on Hot Plug and Tune ready Events
 *
 * @param void pointer (NULL)
 *
 * @return void pointer (NULL)
 */
static void* _DSMgrResnThreadFunc(void *arg)
{
   
	/* Loop */
    while (1)
    {
		 __TIMESTAMP(); printf ("_DSMgrResnThreadFunc... wait for for HDMI or Tune Ready Events \r\n");
		
		/*Wait for the Event*/
		pthread_mutex_lock(&tdsMutexLock);
		pthread_cond_wait(&tdsMutexCond, &tdsMutexLock);
		pthread_mutex_unlock(&tdsMutexLock);
	  
	    __TIMESTAMP();printf("%s: Setting Resolution On:: HDMI %s Event  with TuneReady status = %d \r\n",__FUNCTION__, (edisplayEventStatus == dsDISPLAY_EVENT_CONNECTED ? "Connect" : "Disconnect"),iTuneReady);


		//On hot plug event , Remove event source 
		if(hotplug_event_src)
		{
			g_source_remove(hotplug_event_src);
			__TIMESTAMP();printf("Removed Hot Plug Event Time source %d  \r\n",hotplug_event_src);
      hotplug_event_src = 0;
		}

		/*Set the Resolution only on HDMI Hot plug Connect and Tune Ready events */
		if((1 == iTuneReady) && (dsDISPLAY_EVENT_CONNECTED == edisplayEventStatus)) {
			/*Set Video Output Port  Resolution */
                        if(bHDCPAuthenticated)
                        {
                            _SetVideoPortResolution();
			}
                        /* Set audio mode on HDMI hot plug */
			_setAudioMode();	
		}/*Set the Resolution only on HDMI Hot plug - Disconnect and Tune Ready event */
		else if((1 == iTuneReady) && (dsDISPLAY_EVENT_DISCONNECTED == edisplayEventStatus)) {
			 /* * To avoid reoslution settings of HDMI hot plug when TV goes from power OFF to ON condition 
			 	* Delay the setting of resolution by 5 sec. This will help to filter out un-necessary 
			 	* resolution settings on HDMI hot plug.  
			 */
	             bHDCPAuthenticated = false;
                     if(isComponentPortPresent())
                     {
			 hotplug_event_src = g_timeout_add_seconds((guint)5,_SetResolutionHandler,dsMgr_Gloop); 
			__TIMESTAMP();printf("Schedule a handler to set the resolution after 5 sec for %d time src.. \r\n",hotplug_event_src);
                     }
		}
 
     }
    return arg;
}


static gboolean _SetResolutionHandler(gpointer data)
{
        __TIMESTAMP();printf("Set Video Resolution after delayed time .. \r\n");
	_SetVideoPortResolution();
	hotplug_event_src = 0;
	return FALSE;
}


void _setEASAudioMode()
{
    
    if (isEAS != IARM_BUS_SYS_MODE_EAS) {
        __TIMESTAMP();printf("EAS Not In progress..Do not Modify Audio \r\n");  
        return;
    }

	dsAudioGetHandleParam_t getHandle;
	dsAudioSetStereoModeParam_t setMode;
	int numPorts, i = 0;

	numPorts = dsUTL_DIM(kSupportedPortTypes);
	for (i=0; i < numPorts; i++)
	{
		const dsAudioPortType_t *audioPort = &kSupportedPortTypes[i];
		memset(&getHandle, 0, sizeof(getHandle));
		getHandle.type = *audioPort;
		getHandle.index = 0;
		_dsGetAudioPort (&getHandle);

		memset(&setMode, 0, sizeof(setMode));
		setMode.handle = getHandle.handle;
		setMode.toPersist = false;
		_dsGetStereoMode(&setMode);

		if (setMode.mode == dsAUDIO_STEREO_PASSTHRU) {
			/* In EAS, fallsback to Stereo */
			setMode.mode = dsAUDIO_STEREO_STEREO;  
		} 

		__TIMESTAMP();printf("EAS Audio mode for audio port %d is : %d \r\n",getHandle.type, setMode.mode);
		setMode.toPersist = false;
		_dsSetStereoMode (&setMode);
	}
}
/**
 * @brief Local function to get and set audio mode 
 *
 * This functions gets the audio mode from persistent and sets it
 *
 * @param NULL
 *
 * @return NULL
 */
static void _setAudioMode()
{
    if (isEAS == IARM_BUS_SYS_MODE_EAS) {
        __TIMESTAMP();printf("EAS In progress..Do not Modify Audio \r\n");  
        return;
    }

	dsAudioGetHandleParam_t getHandle;
	dsAudioSetStereoModeParam_t setMode;
	int numPorts, i = 0;

	numPorts = dsUTL_DIM(kSupportedPortTypes);
	for (i=0; i < numPorts; i++)
	{
		const dsAudioPortType_t *audioPort = &kSupportedPortTypes[i];
		memset(&getHandle, 0, sizeof(getHandle));
		getHandle.type = *audioPort;
		getHandle.index = 0;
		_dsGetAudioPort (&getHandle);
			
		memset(&setMode, 0, sizeof(setMode));
		setMode.handle = getHandle.handle;
		setMode.toPersist = true;
		_dsGetStereoMode(&setMode);
        if (getHandle.type == dsAUDIOPORT_TYPE_SPDIF) {
        }
        else if (getHandle.type == dsAUDIOPORT_TYPE_HDMI) {
            //check if it is connected
            intptr_t vHandle = 0;
            int autoMode = 0;
            bool connected = 0;
            bool IsSurround = false;
            {
                dsVideoPortGetHandleParam_t param;
                memset(&param, 0, sizeof(param));
                param.type = dsVIDEOPORT_TYPE_HDMI;
                param.index = 0;
                _dsGetVideoPort(&param);
                vHandle = param.handle;
                //printf("Audio port has HDMI handle\r\n");
            }
            {
                dsVideoPortIsDisplayConnectedParam_t param;
                memset(&param, 0, sizeof(param));
                param.handle = vHandle;
                _dsIsDisplayConnected(&param);
                connected = param.connected;
                //printf("Audio port HDMI is connected to sink %d\r\n", connected);
            }

            if (!(connected)) {
           		__TIMESTAMP();printf("HDMI Not Connected ..Do not Set Audio on HDMI !!! \r\n");  
            	continue;
            }

            {
                dsAudioSetStereoAutoParam_t param;
                memset(&param, 0, sizeof(param));
                param.handle = getHandle.handle;
                _dsGetStereoAuto(&param);
                autoMode = param.autoMode;
                //printf("Audio port HDMI is Auto mode %d\r\n", autoMode);
            }
           
           //printf("Audio port HDMI %d is connected %d Auto mode %d\r\n", vHandle, connected, autoMode);
           if (autoMode) {
            /* If auto, then force surround */
                setMode.mode = dsAUDIO_STEREO_SURROUND;
            }
          
           if (0) /* Do not enforce surround protection, let HAL do it*/
            {
				dsVideoPortIsDisplaySurroundParam_t param;
				param.handle = vHandle;
				param.surround = false;
				_dsIsDisplaySurround(&param);
				IsSurround = param.surround;
			}
           else 
           {
               IsSurround = true;
           }
				if (!(IsSurround)) {
            	/* If Surround not supported , then force Stereo */
                setMode.mode = dsAUDIO_STEREO_STEREO;
                __TIMESTAMP();printf("Surround mode not Supported on HDMI ..Set Stereo \r\n");
            }
	    }
    	__TIMESTAMP();printf("Audio mode for audio port %d is : %d \r\n",getHandle.type, setMode.mode);
		setMode.toPersist = false;
		_dsSetStereoMode (&setMode);
	}
}

/* This functions Dump the HDMI EDID Information of the box.
 *
 * @param NULL
 *
 * @return NULL
 */
static void dumpHdmiEdidInfo(dsDisplayEDID_t* pedidData)
{
	__TIMESTAMP();printf("Connected HDMI Display Device Info !!!!!\r\n");

	if (NULL == pedidData) {
		__TIMESTAMP(); printf("Received EDID is NULL \r\n");
		return;
	}    

	if(pedidData->monitorName)
	printf("HDMI  Monitor Name is %s \r\n",pedidData->monitorName);
	printf("HDMI  Manufacturing ID is %d \r\n",pedidData->serialNumber);
	printf("HDMI  Product Code is %d \r\n",pedidData->productCode);
	printf("HDMI  Device Type is  %s \r\n",pedidData->hdmiDeviceType?"HDMI":"DVI");
	printf("HDMI  Sink Device %s a Repeater \r\n",pedidData->isRepeater?"is":"is not");
	printf("HDMI  Physical Address is %d:%d:%d:%d \r\n",pedidData->physicalAddressA,
			pedidData->physicalAddressB,pedidData->physicalAddressC,pedidData->physicalAddressD);
}


static gboolean dumpEdidOnChecksumDiff(gpointer data) {
         __TIMESTAMP();printf("dumpEdidOnChecksumDiff HDMI-EDID Dump>>>>>>>>>>>>>>\r\n");
        intptr_t _displayHandle = 0;
        dsGetDisplay(dsVIDEOPORT_TYPE_HDMI, 0, &_displayHandle);
        if (_displayHandle) {
                int length = 0;
                dsDisplayGetEDIDBytesParam_t EdidBytesParam;
                static int cached_EDID_checksum = 0;
                int current_EDID_checksum = 0;
                memset(&EdidBytesParam,0,sizeof(EdidBytesParam));
                EdidBytesParam.handle = _displayHandle;
                _dsGetEDIDBytes(&EdidBytesParam);
		length = EdidBytesParam.length;

		if((length > 0) && (length <= 512)) {
                    unsigned char* edidBytes = EdidBytesParam.bytes;
                    for (int i = 0; i < (length / 128); i++)
                            current_EDID_checksum += edidBytes[(i+1)*128 - 1];

                    if((cached_EDID_checksum == 0) || (current_EDID_checksum != cached_EDID_checksum)) {
                            cached_EDID_checksum = current_EDID_checksum;
                            __TIMESTAMP();printf("HDMI-EDID Dump BEGIN>>>>>>>>>>>>>>\r\n");
                            for (int i = 0; i < length; i++) {
                                    if (i % 16 == 0) {
                                            printf("\r\n");
                                    }
                                    if (i % 128 == 0) {
                                            printf("\r\n");
                                    }
                                    printf("%02X ", edidBytes[i]);
                            }
                            printf("\nHDMI-EDID Dump END>>>>>>>>>>>>>>\r\n");
                    }
		}
        }
        return false;
}

/** @} */
/** @} */
