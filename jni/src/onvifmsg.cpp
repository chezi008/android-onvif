#include "onvifmsg.h"

using namespace std;

//extern Config_Info_t    conf;
//extern PUB_FUN			pubfun;
//extern set<string>		g_ovf_devs;

void ONVIF::Soap_perror(struct soap *soap, const char *str)
{
	if (NULL == str) {
		printf("[soap] error: %d, %s, %s\n", soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	}
	else {
		printf("[soap] %s error: %d, %s, %s\n", str, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	}
	return;
}

void* ONVIF::Soap_malloc(struct soap *soap, unsigned int n)
{
	void *p = NULL;

	if (n > 0) {
		p = soap_malloc(soap, n);
		assert(NULL != p);
		memset(p, 0x00, n);
	}
	return p;
}

struct soap* ONVIF::Soap_Init(int timeout)
{
	struct soap *soap = NULL;
	assert(NULL != (soap = soap_new()));
	soap_set_namespaces(soap, namespaces);       // 设置soap的namespaces

	soap->recv_timeout = timeout;                // 设置超时（超过指定时间没有数据就退出）
	soap->send_timeout = timeout;
	soap->connect_timeout = timeout;

//	struct in_addr if_req;
//	if_req.s_addr = inet_addr(conf.websvr_ip);		// 想绑定的IP地址
//	soap->ipv4_multicast_if = (char*)soap_malloc(soap, sizeof(struct in_addr));
//	memset(soap->ipv4_multicast_if, 0, sizeof(struct in_addr));
//	memcpy(soap->ipv4_multicast_if, (char*)&if_req, sizeof(if_req));

//	soap->socket_flags = MSG_NOSIGNAL;         // To prevent connection reset errors
	soap_set_mode(soap, SOAP_C_UTFSTRING);     // 设置为UTF-8编码，否则叠加中文OSD会乱码
	return soap;
}

void ONVIF::Soap_delete(struct soap *soap)
{
	soap_destroy(soap);           // remove deserialized class instances (C++ only)
	soap_end(soap);               // Clean up deserialized data (except class instances) and temporary data
	soap_done(soap);              // Reset, close communications, and remove callbacks
	soap_free(soap);              // Reset and deallocate the context created with soap_new or soap_copy
}

//初始化soap描述消息头
void ONVIF::Init_header(struct soap *soap)
{
	char _HwId[1024];
	unsigned int Flagrand;
	unsigned char macaddr[6];
	struct SOAP_ENV__Header *header = NULL;
	assert(NULL != soap);

	header = (struct SOAP_ENV__Header *)Soap_malloc(soap, sizeof(struct SOAP_ENV__Header));
	soap_default_SOAP_ENV__Header(soap, header);

	// 为了保证每次搜索的时候MessageID都是不相同的！因为简单，直接取了随机值
	srand((int)time(0));
	Flagrand = rand() % 9000 + 1000;   //保证四位整数
	macaddr[0] = 0x1; macaddr[1] = 0x2; macaddr[2] = 0x3; macaddr[3] = 0x4; macaddr[4] = 0x5; macaddr[5] = 0x6;
	sprintf(_HwId, "urn:uuid:%ud68a-1dd2-11b2-a105-%02X%02X%02X%02X%02X%02X", Flagrand, macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
	header->wsa__MessageID = (char *)malloc(100);
	memset(header->wsa__MessageID, 0, 100);
	strncpy(header->wsa__MessageID, _HwId, strlen(_HwId));

	header->wsa__To = (char*)Soap_malloc(soap, strlen(SOAP_TO) + 1);
	header->wsa__Action = (char*)Soap_malloc(soap, strlen(SOAP_ACTION) + 1);
	strcpy(header->wsa__To, SOAP_TO);
	strcpy(header->wsa__Action, SOAP_ACTION);
	soap->header = header;
}

//初始化探测设备的范围和类型
void ONVIF::Init_ProbeType(struct soap *soap, struct wsdd__ProbeType *probe)
{
	struct wsdd__ScopesType *scope = NULL;                                       // 用于描述查找哪类的Web服务
	assert(NULL != soap);
	assert(NULL != probe);

	scope = (struct wsdd__ScopesType *)Soap_malloc(soap, sizeof(struct wsdd__ScopesType));
	soap_default_wsdd__ScopesType(soap, scope);                                  // 设置寻找设备的范围
	scope->__item = (char*)Soap_malloc(soap, strlen(SOAP_ITEM) + 1);
	strcpy(scope->__item, SOAP_ITEM);

	memset(probe, 0x00, sizeof(struct wsdd__ProbeType));
	soap_default_wsdd__ProbeType(soap, probe);
	probe->Scopes = scope;
	probe->Types = (char*)Soap_malloc(soap, strlen(SOAP_TYPES) + 1);      // 设置寻找设备的类型
	strcpy(probe->Types, SOAP_TYPES);
}

//搜索本地设备
void ONVIF::DetectDevice(struct soap *soap)
{
#warning 注释 2020 1.10
//	string addr = "";
//	string devmsg = "";
//	int result = 0;
//	// 用于发送Probe消息
//	struct wsdd__ProbeType      req;
//	// 用于接收Probe应答
//	struct __wsdd__ProbeMatches rep;
//	//struct wsdd__ProbeMatchType *probeMatch;
//
//	Init_header(soap);
//	Init_ProbeType(soap, &req);
//
//	// 向组播地址广播Probe消息
//	result = soap_send___wsdd__Probe(soap, SOAP_MCAST_ADDR, NULL, &req);
//	//printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
//	// 开始循环接收设备发送过来的消息
//	while (SOAP_OK == result)
//	{
//		memset(&rep, 0x00, sizeof(rep));
//		result = soap_recv___wsdd__ProbeMatches(soap, &rep);
//		//printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
//		if (SOAP_OK == result)
//		{
//			//printf("Target ProbeMatchsize   : %d\n", rep.wsdd__ProbeMatches->__sizeProbeMatch);
//			//printf("Target EP Address       : %s\n", rep.wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference.Address);
//			//printf("Target Type             : %s\n", rep.wsdd__ProbeMatches->ProbeMatch->Types);
//			//printf("Target Service Address  : %s\n", rep.wsdd__ProbeMatches->ProbeMatch->XAddrs);
//			if (NULL != rep.wsdd__ProbeMatches) {
//				//for (int i = 0; i < rep.wsdd__ProbeMatches->__sizeProbeMatch; i++) {
//				//probeMatch = rep.wsdd__ProbeMatches->ProbeMatch+i;
//				//}
//				addr = rep.wsdd__ProbeMatches->ProbeMatch->XAddrs;
//				//printf("Find dev addr ----[%s] \n", addr.c_str());
//			}
//			int npos = addr.find_first_of(" ", 0);
//			if (npos > 0)
//				addr = addr.substr(0, npos);
//			devmsg += addr;
//			g_ovf_devs.insert(addr);
//		}
//	}
	//cout << "Get  All  Dev  Msg --- " << devmsg << endl;
}

//获取设备基本信息
string ONVIF::GetDeviceInformation(const string name, const string pass, struct soap *soap, const char *DeviceXAddr)
{
	int result = 0;
	string dev_info = "";
	_tds__GetDeviceInformation           *devinfo_req = new _tds__GetDeviceInformation();
	_tds__GetDeviceInformationResponse   *devinfo_resp = new _tds__GetDeviceInformationResponse();

	assert(NULL != DeviceXAddr);
	SetAuthInfo(soap, name.c_str(), pass.c_str());

	result = soap_call___tds__GetDeviceInformation(soap, DeviceXAddr, NULL, devinfo_req, *devinfo_resp);
	//printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	if (result == SOAP_OK)
	{
		//printf("Manufacturer:       %s\n", devinfo_resp->Manufacturer);
		//printf("Model:              %s\n", devinfo_resp->Model);
		//printf("Serial Number:      %s\n", devinfo_resp->SerialNumber);
		//printf("Hardware Id:        %s\n", devinfo_resp->HardwareId);
		//printf("Firmware Version:   %s\n", devinfo_resp->FirmwareVersion);
		string Manufacturer = devinfo_resp->Manufacturer;
		string Model = devinfo_resp->Model;
		//LOG(LOG_DEBUG, "Get info   name[%s], model[%s]", Manufacturer.c_str(), Model.c_str());
		string::size_type pos;
		pos = Model.find("NVR", 0);
		if (pos != Model.npos)
			Model = "NVR";
		else
			Model = "IPC";
		dev_info = Manufacturer + "$" + Model;
	}

	return dev_info;
}

//功能：设置认证信息
int ONVIF::SetAuthInfo(struct soap *soap, const char *username, const char *password)
{
	int result = 0;
	assert(NULL != username);
	assert(NULL != password);
	result = soap_wsse_add_UsernameTokenDigest(soap, NULL, username, password);
	//printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	return result;
}

//功能：获取设备能力信息
string ONVIF::GetCapabilities(const string name, const string pass, struct soap *soap, const char *DeviceXAddr)
{
	int result = 0;
	string Addr_Info = "empty";
	string media = "empty";
	string ptz = "empty";
	string event = "empty";
	string search = "empty";
	string recording = "empty";
	string receiver = "empty";
	assert(NULL != DeviceXAddr);
	_tds__GetCapabilities *req = new _tds__GetCapabilities();
	_tds__GetCapabilitiesResponse *rep = new _tds__GetCapabilitiesResponse();
	
	SetAuthInfo(soap, name.c_str(), pass.c_str());

	result = soap_call___tds__GetCapabilities(soap, DeviceXAddr, NULL, req, *rep);
	//printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	//printf("ONVIF_GetCapabilities  get  result ---------------  %d \n\n", result);
	if (result == SOAP_OK)
	{
		if (NULL != rep->Capabilities) {
			if ( NULL != rep->Capabilities->Media)
				media = rep->Capabilities->Media->XAddr;
			if (NULL != rep->Capabilities->Events)
				event = rep->Capabilities->Events->XAddr;
			if (NULL != rep->Capabilities->PTZ)
				ptz = rep->Capabilities->PTZ->XAddr;
			if (NULL != rep->Capabilities->Extension->Recording)
				recording = rep->Capabilities->Extension->Recording->XAddr;
			if ( NULL != rep->Capabilities->Extension->Receiver)
				receiver = rep->Capabilities->Extension->Receiver->XAddr;
			if ( NULL != rep->Capabilities->Extension->Search)
				search = rep->Capabilities->Extension->Search->XAddr;
			Addr_Info = media + "$" + ptz + "$" + event + "$" + search + "$" + recording;
		}
	}
	//printf("get Addr_Info --- %s\n", Addr_Info.c_str());
	return Addr_Info;
}


//功能：获取设备的音视频码流配置信息  注意：一个码流（如主码流）可以包含视频和音频数据，也可以仅仅包含视频数据。
int ONVIF::GetProfiles(const string name, const string pass, struct soap *soap, const string DeviceAddr, const char *MediaXAddr, const string dev_type, map<string, PassMsg> &Gmap_passageway_info)
{
	int i = 0;
	int result = 0;
	_trt__GetProfiles            *req = new _trt__GetProfiles();
	_trt__GetProfilesResponse    *rep = new _trt__GetProfilesResponse();

	assert(NULL != MediaXAddr);
	SetAuthInfo(soap, name.c_str(), pass.c_str());

	result = soap_call___trt__GetProfiles(soap, MediaXAddr, NULL, req, *rep);
	//printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	if (result != SOAP_OK)
		return -1;

	//cout << "ONVIF_GetProfiles  rep->__sizeProfiles -------- " << rep->__sizeProfiles << endl;
	for (i = 0; i < rep->__sizeProfiles; i++)
	{
		string token = "";
		string source_token = "";
		char dev_token[64] = { 0 };
		string uri = "";
		int width = 0;
		int height = 0;

		tt__Profile *pro = rep->Profiles[i];
		//获取token信息
		if (NULL != pro->token) {
			token = pro->token;
			//cout << "ONVIF_GetProfiles  token -------- " << token << endl;
		}
		//获取这个token的码流地址
		memcpy(dev_token, token.c_str(), token.length());
		GetStreamUri(name, pass, soap, MediaXAddr, dev_token, uri);
		//cout << "ONVIF_GetProfiles  uri --- " << uri << endl;
		//获取码流分辨率信息
		if (NULL != pro->VideoEncoderConfiguration) {
			if (NULL != pro->VideoEncoderConfiguration->Resolution) {
				width = pro->VideoEncoderConfiguration->Resolution->Width;
				height = pro->VideoEncoderConfiguration->Resolution->Height;
				//cout << "ONVIF_GetProfiles  width--- " << width << ", height --- " << height << endl;
			}
		}
		//获取源token信息，用来分辨是否是同一个码流
		if (NULL != pro->VideoSourceConfiguration) {
			source_token = pro->VideoSourceConfiguration->SourceToken;
			//cout << "ONVIF_GetProfiles  source_token -------- " << source_token << endl;
		}
		else
			source_token = "empty";
		//检查是不是同一个源token的不同分辨率码流
		map<string, PassMsg>::iterator iter;
		iter = Gmap_passageway_info.find(source_token);
		if (iter != Gmap_passageway_info.end()) {
			//检查是否更新
			int last_width = iter->second.width;
			if (last_width < width) {
				Gmap_passageway_info[source_token].width = width;
				Gmap_passageway_info[source_token].height = height;
				strcpy(Gmap_passageway_info[source_token].token, token.c_str());
				strcpy(Gmap_passageway_info[source_token].uri, uri.c_str());
				strcpy(Gmap_passageway_info[source_token].onvif_addr, DeviceAddr.c_str());
				strcpy(Gmap_passageway_info[source_token].type, dev_type.c_str());
			}
		}
		else {
			//添加
			Gmap_passageway_info[source_token].width = width;
			Gmap_passageway_info[source_token].height = height;
			strcpy(Gmap_passageway_info[source_token].token, token.c_str());
			strcpy(Gmap_passageway_info[source_token].uri, uri.c_str());
			strcpy(Gmap_passageway_info[source_token].onvif_addr, DeviceAddr.c_str());
			strcpy(Gmap_passageway_info[source_token].type, dev_type.c_str());
		}
	}
	return result;
}

//功能：获取设备码流地址(RTSP)
int ONVIF::GetStreamUri(const string name, const string pass, struct soap *soap, const char *MediaXAddr, char *ProfileToken, string &media_uri)
{
	int result = 0;
	tt__StreamSetup              *ttStreamSetup = new tt__StreamSetup();
	tt__Transport                *ttTransport = new tt__Transport();
	_trt__GetStreamUri           *req = new _trt__GetStreamUri();
	_trt__GetStreamUriResponse   *rep = new _trt__GetStreamUriResponse();

	assert(NULL != MediaXAddr);
	SetAuthInfo(soap, name.c_str(), pass.c_str());

	ttStreamSetup->Stream = tt__StreamType__RTP_Unicast;
	ttStreamSetup->Transport = ttTransport;
	ttStreamSetup->Transport->Protocol = tt__TransportProtocol__RTSP;
	ttStreamSetup->Transport->Tunnel = NULL;
	req->StreamSetup = ttStreamSetup;
	req->ProfileToken = ProfileToken;

	SetAuthInfo(soap, name.c_str(), pass.c_str());
	result = soap_call___trt__GetStreamUri(soap, MediaXAddr, NULL, req, *rep);
	//printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));

	if (NULL != rep->MediaUri) {
		//printf("Uri:                 %s\n", rep->MediaUri->Uri);
		//printf("InvalidAfterConnect: %d\n", &rep->MediaUri->InvalidAfterConnect);
		//printf("InvalidAfterReboot:  %d\n", &rep->MediaUri->InvalidAfterReboot);
		//printf("Timeout:             %d\n", &rep->MediaUri->Timeout);
		media_uri = rep->MediaUri->Uri;
	}

	return result;
}

//控制云台  持续移动
int ONVIF::PtzContinuousMove(const string name, const string pass, struct soap *soap, char *profiletoken, const char *MediaXAddr, int ptz_command, float ptz_speed)
{
	int result = 0;
	assert(NULL != MediaXAddr);
	_tptz__ContinuousMove			*ptz_req = new _tptz__ContinuousMove();
	_tptz__ContinuousMoveResponse	*ptz_rep = new _tptz__ContinuousMoveResponse();

	ptz_req->ProfileToken = profiletoken;
	tt__PTZSpeed* velocity = new tt__PTZSpeed();
	ptz_req->Velocity = velocity;
	tt__Vector2D* panTilt = new tt__Vector2D();
	ptz_req->Velocity->PanTilt = panTilt;
	char ptz_space[256] = "http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace";
	ptz_req->Velocity->PanTilt->space = ptz_space;
	tt__Vector1D* zoom = new tt__Vector1D();
	ptz_req->Velocity->Zoom = zoom;

	float move_step = ptz_speed / 10;
	if (move_step >= 1)
		move_step = 0.9999;

	switch (ptz_command)
	{
	case PTZ_LEFT:
		ptz_req->Velocity->PanTilt->x = -move_step;
		break;
	case PTZ_RIGHT:
		ptz_req->Velocity->PanTilt->x = move_step;
		break;
	case PTZ_UP:
		ptz_req->Velocity->PanTilt->y = move_step;
		break;
	case PTZ_DOWN:
		ptz_req->Velocity->PanTilt->y = -move_step;
		break;
	case PTZ_LEFTUP:
		ptz_req->Velocity->PanTilt->x = -move_step;
		ptz_req->Velocity->PanTilt->y = move_step;
		break;
	case PTZ_LEFTDOWN:
		ptz_req->Velocity->PanTilt->x = -move_step;
		ptz_req->Velocity->PanTilt->y = -move_step;
		break;
	case PTZ_RIGHTUP:
		ptz_req->Velocity->PanTilt->x = move_step;
		ptz_req->Velocity->PanTilt->y = move_step;
		break;
	case PTZ_RIGHTDOWN:
		ptz_req->Velocity->PanTilt->x = move_step;
		ptz_req->Velocity->PanTilt->y = -move_step;
		break;
	case PTZ_PLUS:
		ptz_req->Velocity->Zoom->x = move_step;
		break;
	case PTZ_MINUS:
		ptz_req->Velocity->Zoom->x = -move_step;
		break;
	default:
		break;
	}
	SetAuthInfo(soap, name.c_str(), pass.c_str());

	result = soap_call___tptz__ContinuousMove(soap, MediaXAddr, NULL, ptz_req, *ptz_rep);
	//printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	if (result != SOAP_OK)
		return -1;
	return result;
}

//控制云台  停止移动
int ONVIF::PtzStop(const string name, const string pass, struct soap *soap, char *profiletoken, const char *MediaXAddr)
{
	int result = 0;
	assert(NULL != MediaXAddr);
	_tptz__Stop				*stop_req = new _tptz__Stop();
	_tptz__StopResponse		*stop_rep = new _tptz__StopResponse();

	stop_req->ProfileToken = profiletoken;
	bool* pantilt = new bool;
	stop_req->PanTilt = pantilt;
	*(stop_req->PanTilt) = true;
	bool* zoom = new bool;
	stop_req->Zoom = zoom;
	*(stop_req->Zoom) = true;

	SetAuthInfo(soap, name.c_str(), pass.c_str());

	result = soap_call___tptz__Stop(soap, MediaXAddr, NULL, stop_req, *stop_rep);
	//printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	if (result != SOAP_OK)
		return -1;
	return result;
}


//功能：获取事件服务支持的功能
int ONVIF::GetServiceCapabilities(const string name, const string pass, struct soap *soap, const char *EventAddr)
{
	int ret = 0;
	assert(NULL != EventAddr);
	_tev__GetServiceCapabilities         *req = new _tev__GetServiceCapabilities();
	_tev__GetServiceCapabilitiesResponse *rep = new _tev__GetServiceCapabilitiesResponse();

	SetAuthInfo(soap, name.c_str(), pass.c_str());

	ret = soap_call___tev__GetServiceCapabilities(soap, EventAddr, NULL, req, *rep);
	if (ret == SOAP_OK) {
		if (NULL != rep->Capabilities) {
			//cout << "SubscriptionPolicy: ----- " << *(rep->Capabilities->WSSubscriptionPolicySupport) << endl;
			//cout << "PullPoint: ----- " << *(rep->Capabilities->WSPullPointSupport) << endl;
			//cout << "Pausable: ----- " << *(rep->Capabilities->WSPausableSubscriptionManagerInterfaceSupport) << endl << endl;
			ret = *(rep->Capabilities->WSPullPointSupport);
		}
	}
	else
	{
		printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	}

	return ret;
}

//功能：使用Pull-Point方式向事件服务订阅感兴趣的事件主题
string ONVIF::CreatePullPointSubscription(const string name, const string pass, struct soap *soap, const char *EventAddr)
{
	int ret = 0;
	string pullpoint_addr = "";
	assert(NULL != EventAddr);
	_tev__CreatePullPointSubscription         *req = new _tev__CreatePullPointSubscription();
	_tev__CreatePullPointSubscriptionResponse *rep = new _tev__CreatePullPointSubscriptionResponse();
	/*
	* 可以通过主题过滤我们所订阅的事件，过滤规则在官方「ONVIF Core Specification」规格说明书「Topic Filter」章节里有详细的介绍。比如：
	* tns1:RuleEngine/TamperDetector/Tamper   只关心遮挡报警
	* tns1:RuleEngine/CellMotionDetector/Motion  移动侦测
	* tns1:RuleEngine/TamperDetector//.       只关心主题TamperDetector树下的事件
	* NULL                                    关心所有事件，即不过滤, 也可以通过 '|' 表示或的关系，即同时关心某几类事件
	*/
	req->InitialTerminationTime = (char*)"PT1M";
	req->Filter = new wsnt__FilterType();
	req->Filter->TopicExpression = new wsnt__TopicExpressionType();
	req->Filter->TopicExpression->Dialect = (char*)"http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet";
	req->Filter->TopicExpression->__mixed = (char*)"tns1:RuleEngine/CellMotionDetector/Motion";

	SetAuthInfo(soap, name.c_str(), pass.c_str());

	ret = soap_call___tev__CreatePullPointSubscription(soap, EventAddr, NULL, req, *rep);
	if (ret != SOAP_OK)
		printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	else
	{
		// 提取pull point地址
		pullpoint_addr = rep->SubscriptionReference.Address;
		//cout << "get pull point addr ============================  " << pullpoint_addr << endl;
	}

	return pullpoint_addr;
}

//功能：使用Pull-Point向PullPoint拉取消息
int ONVIF::PullMessages(const string name, const string pass, struct soap *soap, const char *pullpoint_addr, const string alarm_type, time_t &alarm_time)
{
	int ret = 0;
	string type = "";
	assert(NULL != pullpoint_addr);
	_tev__PullMessages            *req = new _tev__PullMessages();
	_tev__PullMessagesResponse    *rep = new _tev__PullMessagesResponse();
	req->Timeout = (char*)"PT5S";
	req->MessageLimit = 2;

	SetAuthInfo(soap, name.c_str(), pass.c_str());

	ret = soap_call___tev__PullMessages(soap, pullpoint_addr, NULL, req, *rep);
	if (ret != SOAP_OK)
	{
		printf("PullMessages  [%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
		return -1;
	}
	//cout << "PullMessages __sizeNotificationMessage: ------ " << rep->__sizeNotificationMessage << endl;
	//cout << "PullMessages  CurrentTime : -------" << rep->CurrentTime << endl;
	//cout << "PullMessages  EndTime: ------" << rep->TerminationTime << endl;
	if (alarm_type == "Motion")
		ret = Check_event(rep, MOTION_TOPIC, MOTION_NAME, MOTION_VALUE);
	else if (alarm_type == "Temper")
		ret = Check_event(rep, TAMPER_TOPIC, TAMPER_NAME, TAMPER_VALUE);
	if (ret == 1)
		alarm_time = rep->CurrentTime;
	//cout << "PullMessages Check_event alarm_type  ------  " << alarm_type << ",  Check_event   ----  " << ret << ",  alarm_time ---  " << alarm_time << endl;
	return ret;
}

/************************************************************************
**功能：查找指定主题、指定内容的事件
**返回：0表明未找到，非0表明找到
**遮挡报警    "tns1:RuleEngine/TamperDetector/Tamper"         IsTamper
**移动侦测    "tns1:RuleEngine/CellMotionDetector/Motion"     IsMotion
************************************************************************/
string ONVIF::Find_event(_tev__PullMessagesResponse *rep)
{
	int i, j;
	string event_type = "empty";
	if (NULL == rep) {
		return event_type;
	}
	for (i = 0; i < rep->__sizeNotificationMessage; i++) {
		wsnt__NotificationMessageHolderType **p = rep->wsnt__NotificationMessage + i;
		cout << "find_event  get   topic -------- " << (*p)->Topic->__mixed << endl;

		if (0 == strcmp("tns1:RuleEngine/TamperDetector/Tamper", (*p)->Topic->__mixed)) {
			for (j = 0; j < (*p)->Message.tt__Message->Data->__sizeSimpleItem; j++) {
				_tt__ItemList_SimpleItem *a = (*p)->Message.tt__Message->Data->SimpleItem + j;
				cout << "find_event  Tamper  get   name -------- " << a->Name << ",Value ----- " << a->Value << endl;
				if (0 == strcmp("IsTamper", a->Name) && 0 == strcmp("true", a->Value)) {
					event_type = "Tamper";
					return event_type;
				}
				continue;
			}
		}
		else if (0 == strcmp("tns1:RuleEngine/CellMotionDetector/Motion", (*p)->Topic->__mixed)) {
			for (j = 0; j < (*p)->Message.tt__Message->Data->__sizeSimpleItem; j++) {
				_tt__ItemList_SimpleItem *a = (*p)->Message.tt__Message->Data->SimpleItem + j;
				cout << "find_event  Motion  get   name -------- " << a->Name << ",Value ----- " << a->Value << endl;
				if (0 == strcmp("IsMotion", a->Name) && 0 == strcmp("true", a->Value)) {
					event_type = "Motion";
					return event_type;
				}
				continue;
			}
		}
		continue;
	}
	return event_type;
}

int ONVIF::Check_event(struct _tev__PullMessagesResponse *rep, const char *topic, const char *name, const char *value)
{
	int i, j;
	if (NULL == rep) {
		return 0;
	}
	for (i = 0; i < rep->__sizeNotificationMessage; i++) {
		wsnt__NotificationMessageHolderType **p = rep->wsnt__NotificationMessage + i;
		if (NULL == (*p)->Topic) {
			continue;
		}
		if (NULL == (*p)->Topic->__mixed) {
			continue;
		}
		else {
			//cout << "Check_event get  topic ----- " << (*p)->Topic->__mixed << endl;
		}
		if (0 != strcmp(topic, (*p)->Topic->__mixed)) {
			continue;
		}
		if (NULL == (*p)->Message.tt__Message) {
			continue;
		}
		if (NULL == (*p)->Message.tt__Message->Data) {
			continue;
		}
		if (NULL == (*p)->Message.tt__Message->Data->SimpleItem) {
			continue;
		}
		for (j = 0; j < (*p)->Message.tt__Message->Data->__sizeSimpleItem; j++) {
			_tt__ItemList_SimpleItem *a = (*p)->Message.tt__Message->Data->SimpleItem + j;
			if (NULL == a->Name || NULL == a->Value) {
				continue;
			}
			else {
				//cout << "Check_event  get  name ----- " << a->Name << " ,  value ----- " << a->Value << endl;
			}
			if (0 != strcmp(name, a->Name)) {
				continue;
			}
			if (0 != strcmp(value, a->Value)) {
				continue;
			}
			return 1;
		}
	}
	return 0;
}

//功能：退订订阅事件
int ONVIF::Unsubscribe(const string name, const string pass, struct soap *soap, const char *pullpoint_addr)
{
	int ret = 0;
	assert(NULL != pullpoint_addr);
	_wsnt__Unsubscribe                        *req_u = new _wsnt__Unsubscribe();
	_wsnt__UnsubscribeResponse                *rep_u = new _wsnt__UnsubscribeResponse();

	SetAuthInfo(soap, name.c_str(), pass.c_str());

	ret = soap_call___tev__Unsubscribe(soap, pullpoint_addr, NULL, req_u, *rep_u);
	if (ret != SOAP_OK) {
		printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
		return -1;
	}
	return 0;
}

//功能：获取事件属性
int ONVIF::GetEventProperties(const string name, const string pass, struct soap *soap, const char *EventAddr)
{
	int result = 0;
	assert(NULL != EventAddr);
	_tev__GetEventProperties         *req = new _tev__GetEventProperties();
	_tev__GetEventPropertiesResponse *rep = new _tev__GetEventPropertiesResponse();

	SetAuthInfo(soap, name.c_str(), pass.c_str());

	result = soap_call___tev__GetEventProperties(soap, EventAddr, NULL, req, *rep);
	//printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	if (result == SOAP_OK) {
		cout << "GetEventProperties __sizeTopicNamespaceLocation:   " << rep->__sizeTopicNamespaceLocation << endl << endl;
		//cout << "GetEventProperties mixed:   " << rep->wstop__TopicSet->__mixed << endl;
	}
	return result;
}

//功能：获取设备能力信息
int ONVIF::GetReceivers(const string name, const string pass, struct soap *soap, const char *DeviceXAddr, vector<ReceiversMsg> &v_remsg)
{
	int result = 0;
	int Receivers_size = 0;
	assert(NULL != DeviceXAddr);
	_trv__GetReceivers *req = new _trv__GetReceivers();
	_trv__GetReceiversResponse *rep = new _trv__GetReceiversResponse();

	SetAuthInfo(soap, name.c_str(), pass.c_str());

	result = soap_call___trv__GetReceivers(soap, DeviceXAddr, NULL, req, *rep);
	//printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	if (result != SOAP_OK)
		return -1;

	Receivers_size = rep->__sizeReceivers;
	//printf("Receivers_size ------------ %d\n", Receivers_size);
	for (int i = 0; i < Receivers_size; i++) {
		tt__Receiver *re = rep->Receivers[i];
		ReceiversMsg relst;
		if (NULL != re->Token) {
			//printf("Token:    %s\n", re->Token);
			strncpy(relst.token, re->Token, sizeof(relst.token) - 1);
		}
		if (NULL != re->Configuration) {
			//printf("MediaUri:      %s\n", re->Configuration->MediaUri);
			//printf("StreamType:      %d\n", re->Configuration->StreamSetup->Stream);					//Unicast = 0,Multicast = 1
			//printf("StreamType:      %d\n", re->Configuration->StreamSetup->Transport->Protocol);		//UDP = 0,TCP = 1,RTSP = 2,HTTP = 3
			strncpy(relst.mediauri, re->Configuration->MediaUri, sizeof(relst.token) - 1);
			relst.streamtype = re->Configuration->StreamSetup->Stream;
			relst.protocol = re->Configuration->StreamSetup->Transport->Protocol;
			v_remsg.push_back(relst);
		}
	}
	return Receivers_size;
}

//功能：获取录像信息
int ONVIF::GetRecordingSummary(const string name, const string pass, struct soap *soap, const char *DeviceXAddr)
{
	int result = 0;
	assert(NULL != DeviceXAddr);
	_tse__GetRecordingSummary *req = new _tse__GetRecordingSummary();
	_tse__GetRecordingSummaryResponse  *rep = new _tse__GetRecordingSummaryResponse();

	SetAuthInfo(soap, name.c_str(), pass.c_str());

	result = soap_call___tse__GetRecordingSummary(soap, DeviceXAddr, NULL, req, *rep);
	//printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	if (result != SOAP_OK)
		return -1;

	//cout << "ONVIF_GetRecordingSummary  DataFrom:			 " << rep->Summary->DataFrom << endl;
	//cout << "ONVIF_GetRecordingSummary  DataUntil:           " << rep->Summary->DataUntil << endl;
	//cout << "ONVIF_GetRecordingSummary  NumberRecordings:    " << rep->Summary->NumberRecordings << endl;
	return result;
}

//功能：获取录像信息
string ONVIF::FindRecordings(const string name, const string pass, struct soap *soap, const char *DeviceXAddr, char *token)
{
	string search_token = "";
	int result = 0;
	int maxMatches = 10;
	assert(NULL != DeviceXAddr);
	
	_tse__FindRecordings  *req = new _tse__FindRecordings();
	_tse__FindRecordingsResponse   *rep = new _tse__FindRecordingsResponse();
	req->MaxMatches = &maxMatches;
	req->KeepAliveTime = (char*)"PT60S";
	
	tt__SourceReference *source = new tt__SourceReference();
	source->Token = token;
	tt__SearchScope *scope = new tt__SearchScope();
	scope->IncludedSources = &source;
	scope->__sizeIncludedRecordings = 2;
	scope->IncludedRecordings = &token;
	req->Scope = scope;

	SetAuthInfo(soap, name.c_str(), pass.c_str());

	result = soap_call___tse__FindRecordings(soap, DeviceXAddr, NULL, req, *rep);
	//printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	if (result == SOAP_OK)
	{
		search_token = rep->SearchToken;
	}
	//cout << "get  token ------------ " << SearchToken << endl;
	return search_token;
}

//功能：获取录像查询结果信息
int ONVIF::GetRecordingSearchResults(const string name, const string pass, struct soap *soap, const char *DeviceXAddr, char *searchtoken, vector<RecordingSearchResult> &v_results)
{
	string recording_token = "";
	int ret = 0;

#warning yjl 2020 1 10 注释
//	assert(NULL != DeviceXAddr);
//	_tse__GetRecordingSearchResults   *req = new _tse__GetRecordingSearchResults();
//	_tse__GetRecordingSearchResultsResponse    *rep = new _tse__GetRecordingSearchResultsResponse();
//	req->SearchToken = searchtoken;
//	req->WaitTime = (char*)"PT10S";
//
//	SetAuthInfo(soap, name.c_str(), pass.c_str());
//
//	ret = soap_call___tse__GetRecordingSearchResults(soap, DeviceXAddr, NULL, req, *rep);
//	//printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
//	if (ret == SOAP_OK)
//	{
//		//cout << "ONVIF_GetRecordingSearchResults  __sizeRecordingInformation ---------  " << rep->ResultList->__sizeRecordingInformation << endl;
//		int size = rep->ResultList->__sizeRecordingInformation;
//		for (int i = 0; i < size; i++)
//		{
//			struct RecordingSearchResult result;
//			tt__RecordingInformation *info = rep->ResultList->RecordingInformation[i];
//			string token = info->RecordingToken;
//			time_t begin = *info->EarliestRecording;
//			time_t latest = *info->LatestRecording;
//
//			string early = pubfun.StampToDate(begin);
//			string newly = pubfun.StampToDate(latest);
//			printf("get  token[%s], early[%s], newly[%s]\n\n", token.c_str(), early.c_str(), newly.c_str());
//
//			strcpy(result.token, token.c_str());
//			result.earliest = begin;
//			result.latest = latest;
//			v_results.push_back(result);
//		}
//	}
	return ret;
}

int ONVIF::GetRecordings(const string name, const string pass, struct soap *soap, const char *DeviceXAddr,vector<Recording_Msg> &v_record)
{
	int result = 0;
	assert(NULL != DeviceXAddr);

	_trc__GetRecordings* req = new _trc__GetRecordings();
	_trc__GetRecordingsResponse* rep = new _trc__GetRecordingsResponse();

	SetAuthInfo(soap, name.c_str(), pass.c_str());

	result = soap_call___trc__GetRecordings(soap, DeviceXAddr, NULL, req, *rep);
	if (result != SOAP_OK)
	{
		printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	}
	else
	{
		int recording_size = rep->__sizeRecordingItem;
		//cout << "GetRecordings  get  size ----- " << recording_size << endl;
		for (int i = 0; i < recording_size; i++) 
		{
			struct Recording_Msg rdmsg;
			tt__GetRecordingsResponseItem *rd = rep->RecordingItem[i];
			if (NULL != rd->RecordingToken)
			{
				string recording_token = rd->RecordingToken;
				strcpy(rdmsg.token, recording_token.c_str());
				//cout << "get  recording_token === " << recording_token << endl;
			}
			if (NULL != rd->Configuration) {
				string content = rd->Configuration->Content;
				//cout << "get content --- " << content << endl;
				string  RetentionTime = rd->Configuration->MaximumRetentionTime;
				//cout << "get RetentionTime --- "<< RetentionTime << endl;
			}
			if (NULL != rd->Configuration->Source) {
				string name = rd->Configuration->Source->Name;
				//cout << "get name --- " << name << endl;
				string sourceid = rd->Configuration->Source->SourceId;
				//cout << "get sourceid --- " << name << endl;
				string addr = rd->Configuration->Source->Address;
				strcpy(rdmsg.addr, addr.c_str());
				//cout << "get addr --- " << name << endl;
				string description = rd->Configuration->Source->Description;
				//cout << "get description --- " << name << endl;
				string location = rd->Configuration->Source->Location;
				//cout << "get location --- " << name << endl;
			}
			v_record.push_back(rdmsg);
		}
	}
	return result;
}

//功能：获取录像回放地址信息
string ONVIF::GetReplayUri(const string name, const string pass, struct soap *soap, const char *DeviceXAddr, char *recordingtoken)
{
	int ret = 0;
	string recording_uri = "";
	assert(NULL != DeviceXAddr);
	_trp__GetReplayUri  *req = new _trp__GetReplayUri();
	_trp__GetReplayUriResponse   *rep = new _trp__GetReplayUriResponse();
	req->RecordingToken = recordingtoken;

	SetAuthInfo(soap, name.c_str(), pass.c_str());

	ret = soap_call___trp__GetReplayUri(soap, DeviceXAddr, NULL, req, *rep);
	//printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	if (ret == SOAP_OK)
	{
		recording_uri = rep->Uri;
	}
	return recording_uri;
}

string ONVIF::GetRecordingEvents(const string name, const string pass, struct soap *soap, const char *DeviceXAddr, time_t begin, time_t end, char *recording_token)
{
	int ret = 0;
	string search_token = "";
	assert(NULL != DeviceXAddr);

	_tse__FindEvents *req = new _tse__FindEvents();
	_tse__FindEventsResponse *rep = new _tse__FindEventsResponse();

	tt__SourceReference *source = new tt__SourceReference();
	source->Token = recording_token;
	tt__SearchScope *scope = new tt__SearchScope();
	scope->IncludedSources = &source;
	scope->IncludedRecordings = &recording_token;
	req->Scope = scope;
	req->StartPoint = begin;
	req->EndPoint = &end;

	SetAuthInfo(soap, name.c_str(), pass.c_str());

	ret = soap_call___tse__FindEvents(soap, DeviceXAddr, NULL, req, *rep);
	//printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	if (ret == SOAP_OK)
	{
		search_token = rep->SearchToken;
	}
	return search_token;
}

int ONVIF::GetRecordingEventsResults(const string name, const string pass, struct soap *soap, const char *DeviceXAddr, char *search_token,set<string> &s_eventrets)
{
	int ret = 0;
	assert(NULL != DeviceXAddr);

	_tse__GetEventSearchResults *req = new _tse__GetEventSearchResults();
	_tse__GetEventSearchResultsResponse *rep = new _tse__GetEventSearchResultsResponse();
	req->SearchToken = search_token;

	SetAuthInfo(soap, name.c_str(), pass.c_str());

	ret = soap_call___tse__GetEventSearchResults(soap, DeviceXAddr, NULL, req, *rep);
	if (ret == SOAP_OK)
	{
		int size = rep->ResultList->__sizeResult;
		cout << "get size --- " << size << endl;
		for (int i = 0; i < size; i++) {
			tt__FindEventResult *result = rep->ResultList->Result[i];
			string recording_token = result->RecordingToken;
			cout << "get  recording_token ============ " << recording_token << endl;
			s_eventrets.insert(recording_token);
		}
	}
	else {
		printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
		ret = -1;
	}
	return ret;
}

int ONVIF::EndSearch(const string name, const string pass, struct soap *soap, const char *DeviceXAddr, char *search_token)
{
	int ret = 0;
	assert(NULL != DeviceXAddr);

	_tse__EndSearch *req = new _tse__EndSearch();
	_tse__EndSearchResponse *rep = new _tse__EndSearchResponse();
	req->SearchToken = search_token;

	SetAuthInfo(soap, name.c_str(), pass.c_str());

	ret = soap_call___tse__EndSearch(soap, DeviceXAddr, NULL, req, *rep);
	if (ret != SOAP_OK)
	{
		printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
		ret = -1;
	}
	return ret;
}

int ONVIF::Subscription(const string name, const string pass, struct soap *soap, const char *EventAddr, string onvif_addr, string &manger_addr)
{
	int ret = 0;
	assert(NULL != EventAddr);
	_wsnt__Subscribe         *req = new _wsnt__Subscribe();
	_wsnt__SubscribeResponse *rep = new _wsnt__SubscribeResponse();
	/*
	* 可以通过主题过滤我们所订阅的事件，过滤规则在官方「ONVIF Core Specification」规格说明书「Topic Filter」章节里有详细的介绍。比如：
	* tns1:RuleEngine/TamperDetector/Tamper   只关心遮挡报警
	* tns1:RuleEngine/CellMotionDetector/Motion  移动侦测
	* tns1:RuleEngine/TamperDetector//.       只关心主题TamperDetector树下的事件
	* NULL                                    关心所有事件，即不过滤, 也可以通过 '|' 表示或的关系，即同时关心某几类事件
	*/
	//设置订阅管理器初始化时间
	//sprintf(req->InitialTerminationTime, "PT%dH%dM%dS", EventInfomation->Hour, EventInfomation->Min, EventInfomation->Sec);
	req->InitialTerminationTime = (char*)"PT8760H";
	req->Filter = new wsnt__FilterType();
	req->Filter->TopicExpression = new wsnt__TopicExpressionType();
	req->Filter->TopicExpression->Dialect = (char*)"http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet";
	req->Filter->TopicExpression->__mixed = (char*)"tns1:RuleEngine/CellMotionDetector/Motion";
	//将本地URL地址发送给订阅管理器
	char svr_addr[256] = { 0 };
#warning yjl 2020 1 10 注释
//	sprintf(svr_addr, "http://%s:%s/onvif_notify/addr=%s", conf.websvr_ip, conf.websvr_port, onvif_addr.c_str());
	req->ConsumerReference.Address = svr_addr;
	//cout << "Subscription  svr_addr --- " << svr_addr << endl;

	SetAuthInfo(soap, name.c_str(), pass.c_str());

	ret = soap_call___tev__Subscribe(soap, EventAddr, NULL, req, *rep);
	if (ret != SOAP_OK)
	{
		printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
		return -1;
	}
	else
		manger_addr = rep->SubscriptionReference.Address;
	return 0;
}

string ONVIF::Snapshot(const string name, const string pass, struct soap *soap, const char *MediaXAddr, char *ProfileToken)
{
	int ret = 0;
	string snapshort_uri = "";
	assert(NULL != MediaXAddr);
	assert(NULL != ProfileToken);

	_trt__GetSnapshotUri *req = new _trt__GetSnapshotUri();
	_trt__GetSnapshotUriResponse *rep = new _trt__GetSnapshotUriResponse();
	req->ProfileToken = ProfileToken;

	SetAuthInfo(soap, name.c_str(), pass.c_str());

	ret = soap_call___trt__GetSnapshotUri(soap, MediaXAddr, NULL, req, *rep);
	if (ret != SOAP_OK)
	{
		printf("[%d] error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
	}
	else
	{
		if (NULL != rep->MediaUri) {
			if (NULL != rep->MediaUri->Uri) {
				//cout << "get  snapshort_uri -----  " << rep->MediaUri->Uri << " ----" << endl;
				snapshort_uri = rep->MediaUri->Uri;
				//无认证信息的uri：http://192.168.110.23/onvif-http/snapshot?Profile_1
				//带认证信息的uri：http://name:pass@192.168.110.23/onvif-http/snapshot?Profile_1
				//增加认证信息
				string auth_msg = name + ":" + pass + "@";
				snapshort_uri.insert(7, auth_msg.c_str());
			}
		}
	}
	return snapshort_uri;
}







//通过广播，查找本地ONVIF设备
int ONVIF::FindDevice()
{
	// 初始化soap环境变量
	struct soap* soap;
	soap = Soap_Init(20);
	assert(NULL != (soap));

	DetectDevice(soap);
	return 0;
}

//获取ONVIF设备的媒体服务地址
void ONVIF::GetOnvifMediaUri(const string name, const string pass, string onvif_addr,const string media_addr, const string dev_type, map<string, PassMsg> &map_passmsg)
{
	//初始化soap环境变量
	struct soap* soap;
	soap = Soap_Init(20);
	assert(NULL != (soap));

	//得到主次码流的「媒体配置信息」
	GetProfiles(name, pass, soap, onvif_addr.c_str(), media_addr.c_str(), dev_type, map_passmsg);
}

//获取设备基础信息
string ONVIF::GetDevInfo(const string name, const string pass, string onvif_addr)
{
	// 初始化soap环境变量
	struct soap* soap;
	soap = Soap_Init(20);
	assert(NULL != (soap));
	string devinfo = GetDeviceInformation(name, pass, soap, onvif_addr.c_str());
	return devinfo;
}

//获取ONVIF设备基础服务地址
string ONVIF::GetOnvifServeAddr(const string name, const string pass, string onvif_addr)
{
	//初始化soap环境变量
	struct soap* soap;
	soap = Soap_Init(20);
	assert(NULL != (soap));
	//获取设备能力信息
	return GetCapabilities(name, pass, soap, onvif_addr.c_str());
}

//云台控制
int ONVIF::DoPtz(const string name, const string pass, string onvif_addr, const string ptz_addr, const string ptz_token, const string dev_type, int ptz_type, int ptz_command, float ptz_speed)
{
	char		dev_token[64] = { 0 };
	// 初始化soap环境变量
	struct soap* soap;
	soap = Soap_Init(20);
	assert(NULL != (soap));

	if (ptz_token.empty() || ptz_addr.empty()) {
		//cout << "token is empty \n" << endl;
		return -1;
	}

	//cout << "ptz_token is -------- " << ptz_token << endl;
	memcpy(dev_token, ptz_token.c_str(), ptz_token.length());

	if ((ptz_type == 5 || ptz_type == 9) && ptz_command != 12)
		PtzContinuousMove(name, pass, soap, dev_token, ptz_addr.c_str(), ptz_command, ptz_speed);
	else if ((ptz_type == 5 || ptz_type == 9) && ptz_command == 12)
		PtzStop(name, pass, soap, dev_token, ptz_addr.c_str());

	return 0;
}

//检查是否支持PullPoint拉取事件
int ONVIF::CheckPullPoint(const string name, const string pass, const string event_addr)
{
	int ret = 0;
	// 初始化soap环境变量
	struct soap* soap;
	soap = Soap_Init(10);
	assert(NULL != (soap));

	//检查设备是否支持PULL_POINT拉点
	ret = GetServiceCapabilities(name, pass, soap, event_addr.c_str());
	if(ret == 1)
	{
		string pullpoint_addr = CreatePullPointSubscription(name, pass, soap, event_addr.c_str());
		//printf("GetEvents get  pullpoint_addr ---------- %s\n", pullpoint_addr.c_str());
		if (pullpoint_addr.empty())
		{
			//cout << "GetEvents  Create PullPoint  Subscription  ERROR " << endl;
			return -1;
		}
	}
	return ret;
}

//获取ONVIF设备的事件
string ONVIF::GetEvents(const string name, const string pass, const string event_addr, const string alarm_type)
{
	int ret = 0;
	string pullpoint_addr = "";
	string alarm_time = "";
	// 初始化soap环境变量
	struct soap* soap;
	soap = Soap_Init(10);
	assert(NULL != (soap));

	// 使用Pull-Point方式订阅事件并获取拉取事件地址
	pullpoint_addr = CreatePullPointSubscription(name, pass, soap, event_addr.c_str());
	//printf("GetEvents get  pullpoint_addr ---------- %s\n", pullpoint_addr.c_str());
	if (pullpoint_addr.empty())
	{
		cout << "GetEvents  Create PullPoint  Subscription  ERROR " << endl;
		return alarm_time;
	}
	//拉取事件
	time_t event_time;
	ret = PullMessages(name, pass, soap, pullpoint_addr.c_str(), alarm_type, event_time);
	if (ret< 0)
	{
		printf("GetEvents PullMessages Error, ret ---------- %d\n", ret);
		//取消订阅
		Unsubscribe(name, pass, soap, pullpoint_addr.c_str());
		return alarm_time;
	}
	else if(ret == 1){
#warning yjl 2020 1 10
//		alarm_time = pubfun.StampToDate(event_time);
		//cout << "GetEvents  alarm_time ===  " << alarm_time << endl;
	}

	//取消订阅
	Unsubscribe(name, pass, soap, pullpoint_addr.c_str());
	//printf("GetEvents  Unsubscribe OVER \n");

	return alarm_time;
}

//获取录像信息
int ONVIF::GetRecordingMsg(const string name, const string pass, string search_addr,time_t begin, time_t end, string &replay_uri)
{
	int ret = 0;
	// 初始化soap环境变量
	struct soap* soap;
	soap = Soap_Init(10);
	assert(NULL != (soap));

	//GetRecordingSummary( name, pass, soap, search_addr.c_str());

	vector<Recording_Msg> v_rdmsg;
	ret = GetRecordings(name, pass, soap, search_addr.c_str(), v_rdmsg);
	cout << "GetRecordings  ret  ----- " << ret <<", recording size --- "<<v_rdmsg.size() << endl;
	string token = v_rdmsg[0].token;
	cout << "GetRecordings get  channel_1  token --- " << token << endl;

	string event_token = GetRecordingEvents(name, pass, soap, search_addr.c_str(), begin, end, (char*)token.c_str());
	cout << "GetRecordingEvents  get  event_token --- " << event_token << endl;

	set<string> s_eventrets;
	ret = GetRecordingEventsResults(name, pass, soap, search_addr.c_str(), (char*)event_token.c_str(), s_eventrets);
	cout << "GetRecordingEventsResults  get  ret --- " << ret << " , events_token  size --- " << s_eventrets.size() << endl;

	ret = EndSearch(name, pass, soap, search_addr.c_str(), (char*)event_token.c_str());
	cout << "EndSearch  get  ret --- " << ret << endl;

	for(set<string>::iterator it=s_eventrets.begin();it!=s_eventrets.end();++it)
	{
		string recording_token = *it;
		string uri = GetReplayUri(name, pass, soap, search_addr.c_str(), (char*)recording_token.c_str());
		cout << "GetReplayUri  get   uri --- " << uri << endl;
		if (it == s_eventrets.begin())
			replay_uri = uri;
	}
	
	return ret;
}

//订阅事件自动通知
int ONVIF::SubscriptionEvents(const string name, const string pass, const string event_addr, string onvif_addr, string &manger_addr)
{
	int ret = 0;
	// 初始化soap环境变量
	struct soap* soap;
	soap = Soap_Init(10);
	assert(NULL != (soap));

	//订阅事件  SubscriptionRequest
	ret = Subscription(name, pass, soap, event_addr.c_str(), onvif_addr, manger_addr);
	cout << "SubscriptionEvents  ret --- " << ret << endl;
	return ret;
}
//取消自动通知订阅
int ONVIF::UnsubscribeEvents(const string name, const string pass, const string manger_addr)
{
	int ret = 0;
	// 初始化soap环境变量
	struct soap* soap;
	soap = Soap_Init(10);
	assert(NULL != (soap));

	//取消订阅
	ret = Unsubscribe(name, pass, soap, manger_addr.c_str());
	cout << "Unsubscribe ret ---  " << ret << endl;
	return ret;
}

//抓取图片
string ONVIF::GetSnapshotUri(const string name, const string pass, const string media_addr, string token)
{
	//获取抓图地址
	struct soap* soap;
	soap = Soap_Init(10);
	assert(NULL != (soap));

	string uri = "";
	uri = Snapshot(name, pass, soap, media_addr.c_str(), (char*)token.c_str());

	return uri;
}
