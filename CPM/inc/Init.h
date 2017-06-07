#ifndef __INIT_H__
#define __INIT_H__

#include <iostream>

#include "Shared.h"
#include "Define.h"
#include "ThreadPool.h"
#include "NetIfBase.h"
#include "MsgMgrBase.h"
#include "MemMgmt.h"
#include "SeqRscMgr.h"

#include "RouteMgr.h"
#include "ConnContainer.h"
#include "ConnMgr.h"
#include "ClsSocket.h"

#include "des.h"

#include "Analyser.h"
#include "NetDataContainer.h"

#include "Apm.h"

#include "MsgCtl.h"

//#include "HardIF.h"
//#include "GSMBaseSiemens.h"
#include "GSMCtrl.h"

//#include "ADCBase.h"
//#include "CPMUtil.h"

//#include "AudioPlay.h"

#include "SqlCtl.h"

#include "DeviceIfBase.h"
#include "DevInfoContainer.h"
#include "DeviceIf485.h"
//#include "DeviceIfSpi485.h"
#include "DeviceIfAD.h"

#include "OnTimeCtrl.h"
#include "Transparent.h"

//extern CADCBase g_CADCBaseA;
//extern CADCBase g_CADCBaseB;

extern CMemMgr g_MemMgr; //�ڴ����ģ��

extern CDevInfoContainer g_DevInfoContainer;

extern CNetIfBase g_NetIfTcpClient;               //���ϼ�ƽ̨������
extern CMsgCtlClient g_MsgCtlCliNet;

extern CBOAMsgCtlSvr g_MsgCtlSvrBoa;					//TCP Server����ҳ�����������ݴ���ģ��
extern CTMNMsgCtlSvr g_MsgCtlSvrTmn;					//TCP Server���ն˽����������ݴ���ģ��

extern ConnContainer g_ConnContainer;
extern CRouteMgr g_RouteMgr;
extern CMsgMgrBase g_MsgMgr;

extern CAnalyser g_Analyser;

class CNetDataContainer;
extern CNetDataContainer g_NetContainer;

extern CGSMCtrl	g_GSMCtrl;

extern int gDataLogFlag;
extern bool gDataLogSockFlag;
extern char gDataLogFile[100];

extern char g_szEdId[24];
extern char  g_szPwd[20];
extern int g_upFlag;	

extern char	g_szLocalIp[24];
extern char g_szVersion[20];							//CPM�汾��

extern int	g_iTTLSampleCount;						//��������������
extern int	g_iADSampleCount;						//ģ������������


extern TKU32 g_ulHandleNetBoaSvrSend;							//���緢�Ͷ��о��
extern TKU32 g_ulHandleNetBoaSvrRecv;							//������ն��о��

extern TKU32 g_ulHandleNetTmnSvrSend;							//���緢�Ͷ��о��
extern TKU32 g_ulHandleNetTmnSvrRecv;							//������ն��о��

extern TKU32 g_ulHandleNetCliSend;							//���緢�Ͷ��о��
extern TKU32 g_ulHandleNetCliRecv;							//������ն��о��

extern CDeviceIfBase* g_ptrDeviceIf485_1;
extern CDeviceIfBase* g_ptrDeviceIf485_2;
extern CDeviceIfBase* g_ptrDeviceIf485_3;
extern CDeviceIfBase* g_ptrDeviceIf485_4;
extern CDeviceIfBase* g_ptrDeviceIf485_5;
extern CDeviceIfBase* g_ptrDeviceIf485_6;
extern CDeviceIfBase* g_ptrDeviceIf485_7;
extern CDeviceIfBase* g_ptrDeviceIf485_8;
extern CDeviceIfBase* g_ptrDeviceIfAD;

extern TKU32 g_ulHandleActionList481_1;					//
extern TKU32 g_ulHandleActionList481_2;					//
extern TKU32 g_ulHandleActionList481_3;					//
extern TKU32 g_ulHandleActionList481_4;					//
extern TKU32 g_ulHandleActionList481_5;					//
extern TKU32 g_ulHandleActionList481_6;					//
extern TKU32 g_ulHandleActionList481_7;					//
extern TKU32 g_ulHandleActionList481_8;					//
extern TKU32 g_ulHandleActionListAD;

extern TKU32 g_ulHandleAnalyser;							//���ݷ���ģ����������о��
extern TKU32 g_ulHandleBoaMsgCtrl;                             //�·�ָ��������о��
extern TKU32 g_ulHandleTmnMsgCtrl;                             //�·�ָ��������о��

//extern CGSMBaseSiemens g_GSMBaseSiemens;
extern TKU32 g_ulHandleGSM;

//extern int g_hdlGPIO;

//extern CAudioPlay g_AudioPlay;

//extern CSqlCtl g_SqlCtl;
extern COnTimeCtrl g_COnTimeCtrl;
extern MODE_CFG g_ModeCfg;
extern RECORD_COUNT_CFG g_RecordCntCfg;

extern CTransparent g_CTransparent;
extern TKU32 g_ulHandleTransparent;

class CInit {
	
public :
	CInit(){};
    ~CInit(){};
	bool Initialize(void);
	bool WaitForCmd(void);

private:
	bool MemInit(void);


	bool TcpServerInit();
	bool TCPSvrInit(void);
	bool TcpSvrBoa();
	bool TcpSvrTmn();
	bool TcpClientInit();

	void ReadConfigFile();

	void Terminate(void);

};
#endif

