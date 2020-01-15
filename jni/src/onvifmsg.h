#ifndef _ONVIFMSG_H_
#define _ONVIFMSG_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "soapH.h"
#include "soapStub.h"
#include "wsseapi.h"
#include "threads.h"

#include <string>
#include <map>
#include <set>
#include <vector>
//#include "../../common/publicfun.h"
//#include "dnovf.h"

using namespace std;

//msgtype
#define PTZ_CRUISE				0//巡航（开、关）
#define PTZ_APERTURE			1//光圈（加、减）
#define PTZ_FOCUS				2//聚焦（加、减）
#define PTZ_LIGHT				3//照明（开、关）
#define PTZ_WIPER				4//雨刷（开、关）
#define PTZ_ZOOM				5//变倍（加、减）
#define PTZ_SPEED				6//转速
#define PTZ_TYPESTOP			7//转速
#define PTZ_PRESETTING			8//预制位
#define PTZ_DIRECTION			9//方向(对应8个msgcode)
//msgcode
#define PTZ_PLUS				0		//加	
#define PTZ_MINUS				1		//减
#define PTZ_ON					2		//开
#define PTZ_OFF					3		//关
#define PTZ_UP					4		//上
#define PTZ_DOWN				5		//下
#define PTZ_LEFT				6		//左
#define PTZ_RIGHT				7		//右
#define PTZ_LEFTUP				8		//左上
#define PTZ_RIGHTUP				9		//右上
#define PTZ_LEFTDOWN			10		//左下
#define PTZ_RIGHTDOWN			11		//右下
#define PTZ_CODESTOP			12		//停止

#define SOAP_TO         "urn:schemas-xmlsoap-org:ws:2005:04:discovery"
#define SOAP_ACTION     "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"
#define SOAP_MCAST_ADDR "soap.udp://239.255.255.250:3702"                       // onvif规定的组播地址239.255.255.250:3702
#define SOAP_ITEM       ""                                                      // 寻找的设备范围
#define SOAP_TYPES      "dn:NetworkVideoTransmitter"                            // 寻找的设备类型 dn:NetworkVideoTransmitter | tds:Device

/* 遮挡报警 */
#define TAMPER_TOPIC            "tns1:RuleEngine/TamperDetector/Tamper"
#define TAMPER_NAME             "IsTamper"
#define TAMPER_VALUE            "true"
/* 移动侦测 */
#define MOTION_TOPIC            "tns1:RuleEngine/CellMotionDetector/Motion"
#define MOTION_NAME             "IsMotion"
#define MOTION_VALUE            "true"

#define SOAP_CHECK_ERROR(result, soap, str) \
    do { \
        if (SOAP_OK != (result) || SOAP_OK != (soap)->error) { \
            soap_perror((soap), (str)); \
            if (SOAP_OK == (result)) { \
                (result) = (soap)->error; \
            } \
        } \
} while (0)


/* 视频编码器配置信息 */
struct tagVideoEncoderConfiguration
{
	char token[64];                                               // 唯一标识该视频编码器的令牌字符串
	int Width;                                                                  // 分辨率
	int Height;
};

/* 设备配置信息 */
struct tagProfile {
	char token[64];                                               // 唯一标识设备配置文件的令牌字符串
	struct tagVideoEncoderConfiguration venc;                                   // 视频编码器配置信息
};

struct ReceiversMsg
{
	char token[64];
	char mediauri[256];
	int streamtype;
	int protocol;
};

struct PassMsg
{
	char    token[64];
	int     width;
	int     height;
	char    uri[256];
	char    onvif_addr[128];
	char	type[32];
};

struct Recording_Msg
{
	char	token[64];
	char	addr[64];
};

struct RecordingSearchResult
{
	time_t	earliest;
	time_t	latest;
	char	token[64];
};

class ONVIF
{
public:
	ONVIF() {}

	~ONVIF() {}

private:
	//内部使用函数
	struct soap* Soap_Init(int timeout);
	void Soap_delete(struct soap* _soap);
	void Soap_perror(struct soap* _soap, const char *str);
	void* Soap_malloc(struct soap *soap, unsigned int n);
	void Init_header(struct soap *soap);
	void Init_ProbeType(struct soap *soap, struct wsdd__ProbeType *probe);
	int SetAuthInfo(struct soap *soap, const char *username, const char *password);


	//获取基础信息函数
	void DetectDevice(struct soap *soap);
	string GetDeviceInformation(const string name, const string pass, struct soap *soap, const char *DeviceXAddr);
	string GetCapabilities(const string name, const string pass, struct soap *soap, const char *DeviceXAddr);
	int GetProfiles(const string name, const string pass, struct soap *soap, const string DeviceAddr, const char *MediaXAddr, const string type, map<string, PassMsg> &Gmap_passageway_info);
	int GetStreamUri(const string name, const string pass, struct soap *soap, const char *MediaXAddr, char *ProfileToken, string &media_uri);


	//控制台函数
	int PtzContinuousMove(const string name, const string pass, struct soap *soap, char *profiletoken, const char *MediaXAddr, int ptz_command, float ptz_speed);
	int PtzStop(const string name, const string pass, struct soap *soap, char *profiletoken, const char *MediaXAddr);



	//PULL-POINT报警事件函数
	string CreatePullPointSubscription(const string name, const string pass, struct soap *soap, const char *EventAddr);
	int PullMessages(const string name, const string pass, struct soap *soap, const char *pullpoint_addr, const string alarm_type, time_t &alarm_time);
	string Find_event(_tev__PullMessagesResponse *rep);
	int Unsubscribe(const string name, const string pass, struct soap *soap, const char *pullpoint_addr);
	int Check_event(struct _tev__PullMessagesResponse *rep, const char *topic, const char *name, const char *value);
	//订阅事件Basic Notification
	int Subscription(const string name, const string pass, struct soap *soap, const char *EventAddr, string onvif_addr, string &manger_addr);

	//录像相关
	int GetRecordingSummary(const string name, const string pass, struct soap *soap, const char *DeviceXAddr);
	int GetRecordings(const string name, const string pass, struct soap *soap, const char *DeviceXAddr, vector<Recording_Msg> &v_record);
	string FindRecordings(const string name, const string pass, struct soap *soap, const char *DeviceXAddr, char* token);
	int GetRecordingSearchResults(const string name, const string pass, struct soap *soap, const char *DeviceXAddr, char *searchtoken, vector<RecordingSearchResult> &v_results);
	string GetReplayUri(const string name, const string pass, struct soap *soap, const char *DeviceXAddr, char *recordingtoken);
	string GetRecordingEvents(const string name, const string pass, struct soap *soap, const char *DeviceXAddr, time_t begin, time_t end, char *recording_token);
	int GetRecordingEventsResults(const string name, const string pass, struct soap *soap, const char *DeviceXAddr, char *search_token, set<string> &s_eventrets);
	int EndSearch(const string name, const string pass, struct soap *soap, const char *DeviceXAddr, char *search_token);

	
	//抓图
	string Snapshot(const string name, const string pass, struct soap *soap, const char *MediaXAddr, char *ProfileToken);

	//其他
	int GetServiceCapabilities(const string name, const string pass, struct soap *soap, const char *EventAddr);
	int GetEventProperties(const string name, const string pass, struct soap *soap, const char *EventAddr);
	int GetReceivers(const string name, const string pass, struct soap *soap, const char *DeviceXAddr, vector<ReceiversMsg> &v_remsg);
public:
	int FindDevice();

	int DoPtz(const string name, const string pass, string onvif_addr, const string ptz_addr,const string ptz_token, const string dev_type, int ptz_type, int ptz_command, float ptz_speed);

	void GetOnvifMediaUri(const string name, const string pass, string onvif_addr, const string media_addr, const string dev_type, map<string, PassMsg> &map_passmsg);

	int CheckPullPoint(const string name, const string pass, const string event_addr);

	string GetEvents(const string name, const string pass, const string event_addr, const string alarm_type);

	string GetDevInfo(const string name, const string pass, string onvif_addr);

	int GetRecordingMsg(const string name, const string pass, string search_addr, time_t begin, time_t end, string &replay_uri);

	string GetOnvifServeAddr(const string name, const string pass, string onvif_addr);

	int SubscriptionEvents(const string name, const string pass, const string event_addr, string onvif_addr, string &manger_addr);

	int UnsubscribeEvents(const string name, const string pass, const string manger_addr);

	string GetSnapshotUri(const string name, const string pass, const string media_addr, string token);
};

#endif
