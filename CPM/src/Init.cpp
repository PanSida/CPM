#include <net/if.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <termio.h>
#include <stdio.h>
#include "Init.h"
#include "HardIF.h"
#include "Crc.h"

/************************************************************************/
/* ϵͳ��Դ����                                                         */
/************************************************************************/
//��Ϣ�����ڴ涨��
static MMUserCtrlBlkStruct g_stHubMUCBArray[MUCB_MAX] = {
	/*
	{�û��ڴ���ƿ���,	�û������ÿ����Ϣ�ĳߴ�,		ʵ���ṩ��ÿԪ�سߴ�,	Ԫ������,			�ڴ����ߴ�}
	*/
	{0,						MAXMSGSIZE*2,					0,						(MEM_MSG_CNT), 		0},
	{1,						MAXMSGSIZE*2,					0,						(MEM_MSG_CNT), 		0},

	{2,						MAXMSGSIZE*2,					0,						(MEM_MSG_CNT),		0},
	{3,						MAXMSGSIZE*2,					0,						(MEM_MSG_CNT), 		0},

	{4,						MAXMSGSIZE*2,					0,						(MEM_MSG_CNT),		0},
	{5,						MAXMSGSIZE*2,					0,						(MEM_MSG_CNT), 		0},

	{6,						MAXMSGSIZE/2,					0,						(MEM_MSG_CNT)/2, 	0},
	{7,						MAXMSGSIZE/2,					0,						(MEM_MSG_CNT)/2, 	0},
	{8,						MAXMSGSIZE/2,					0,						(MEM_MSG_CNT)/2, 	0},
    {9,						MAXMSGSIZE/2,					0,						(MEM_MSG_CNT)/2, 	0},
	{10,					MAXMSGSIZE/2,					0,						(MEM_MSG_CNT)/2, 	0},
	{11,					MAXMSGSIZE/2,					0,						(MEM_MSG_CNT)/2, 	0},
	{12,					MAXMSGSIZE/2,					0,						(MEM_MSG_CNT)/2,	0},
	{13,					MAXMSGSIZE/2,					0,						(MEM_MSG_CNT)/2, 	0},

	{14,					MAXMSGSIZE/2,					0,						(MEM_MSG_CNT)/2, 	0},
	{15,					MAXMSGSIZE/2,					0,						(MEM_MSG_CNT)/2, 	0},
	{16,					MAXMSGSIZE/2,					0,						(MEM_MSG_CNT), 		0},

	{17,					MAXMSGSIZE/2,					0,						(MEM_MSG_CNT)*2, 	0},
	{18,					MAXMSGSIZE,						0,						(MEM_MSG_CNT),		0},
	{19,					MAXMSGSIZE,						0,						(MEM_MSG_CNT),		0},

	{20,					MAXMSGSIZE/2,					0,						(MEM_MSG_CNT), 		0},
	{21,					MAXMSGSIZE,						0,						(MEM_MSG_CNT)/2,	0},

//	{22,					MAXMSGSIZE,						0,						(MEM_MSG_CNT)/2,	0},
	{22,					MAXMSGSIZE,						0,						(MEM_MSG_CNT),		0},

	{23,					MAXMSGSIZE/2,					0,						(MEM_MSG_CNT),		0}
};
//��Ϣ���еĶ���
MsgQCfg stQMC[] = 
{
	//��ҳ���õĲ��֣�������ֱ�ӳ�ʼ��
	{COMBINE_MOD(MOD_CTRL, MOD_ACTION_NET),				QUEUE_ELEMENT_CNT},					//�����������

	{COMBINE_MOD(MOD_CTRL, MOD_DEV_ANALYSE),			QUEUE_ELEMENT_CNT*2},				//��Ѳ�������������

	{COMBINE_MOD(MOD_CTRL, MOD_DEV_MSGCTRL_BOA),		QUEUE_ELEMENT_CNT},					//����ָ���������
	{COMBINE_MOD(MOD_CTRL, MOD_DEV_MSGCTRL_TMN),		QUEUE_ELEMENT_CNT},					//����ָ���������

	{COMBINE_MOD(MOD_CTRL, MOD_DEV_SMS_RECV),			QUEUE_ELEMENT_CNT},					//����Ϣ���ն���
	{COMBINE_MOD(MOD_CTRL, MOD_DEV_SMS_SEND),			QUEUE_ELEMENT_CNT/2},				//����Ϣ���Ͷ���
	//
	{COMBINE_MOD(MOD_CTRL, MOD_DEV_NET_ACTION),			QUEUE_ELEMENT_CNT},					//���綯��ִ�ж���
	{COMBINE_MOD(MOD_CTRL, MOD_DEV_TRANSPARENT),		QUEUE_ELEMENT_CNT}					//͸��������Ϣ����
	
};
//�ڴ����ģ��
CMemMgr			g_MemMgr; 							//�ڴ�����ģ��
CMsgMgrBase		g_MsgMgr;

CDevInfoContainer g_DevInfoContainer;

/************************************************************************/
/* ����ͨ��ģ��-TCP Server                                              */
/************************************************************************/
//TCP Serverģ��
ModIfInfo stIfTcpSvrBoa = {
	"Config.ini",
	"MOD_IF_SVR_BOA",
	1,
	MOD_IF_SVR_NET_BOA,
	MOD_IF_SVR_CTRL_BOA,
	MOD_IF_SVR_NET_BOA,
	MUCB_IF_NET_SVR_RCV_BOA,
	MUCB_IF_NET_SVR_SEND_BOA,
	NP_TCPSVR,
	true,
	-1,
	true,
	1024,
	NULL,    		// OnLogin CallbackLogin;
	NULL,     		// OnEncode CallbackOnEncode;
	DeCodeSVR,      // OnDecode CallbackOnDecode;
	true,
	true
};
ModIfInfo stIfTcpSvrTmn = {
	"Config.ini",
	"MOD_IF_SVR_TMN",
	2,
	MOD_IF_SVR_NET_TMN,
	MOD_IF_SVR_CTRL_TMN,
	MOD_IF_SVR_NET_TMN,
	MUCB_IF_NET_SVR_RCV_TMN,
	MUCB_IF_NET_SVR_SEND_TMN,
	NP_TCPSVR,
	true,
	-1,
	true,
	1024,
	NULL,    		// OnLogin CallbackLogin;
	NULL,     		// OnEncode CallbackOnEncode;
	DeCodeSVR,      // OnDecode CallbackOnDecode;
	true,
	true
};
//TCP Server����ģ��
CRouteMgr			g_RouteMgr;
ConnContainer		g_ConnContainer;
//BOA
ClsSvrConnMgr		ConnMgrHubBoa(COMM_TCPSRV_MAXClIENT);		//Boa,Count=1
CNetIfBase			g_NetIfSvrBoa(stIfTcpSvrBoa);				//NetIf������
CBOAMsgCtlSvr		g_MsgCtlSvrBoa;								//TCP Server�������ݴ���ģ��
TKU32				g_ulHandleNetBoaSvrSend;					//���緢�Ͷ��о��
TKU32				g_ulHandleNetBoaSvrRecv;					//������ն��о��
TKU32				g_ulHandleNetSvrBoaAction;					//���綯��ִ�ж��о��
//TERMINAL
ClsSvrConnMgr		ConnMgrHubTmn(COMM_TCPSRV_MAXClIENT);
CNetIfBase			g_NetIfSvrTmn(stIfTcpSvrTmn);				//NetIf������
CTMNMsgCtlSvr		g_MsgCtlSvrTmn;								//TCP Server�������ݴ���ģ��
TKU32				g_ulHandleNetTmnSvrSend;					//���緢�Ͷ��о��
TKU32				g_ulHandleNetTmnSvrRecv;					//������ն��о��
TKU32				g_ulHandleNetSvrTmnAction;					//���綯��ִ�ж��о��

/************************************************************************/
/* ����ͨ��ģ��-TCP Client  ����ƽ̨                                    */
/************************************************************************/
ModIfInfo stIfNet = {
	"Config.ini",
	"PARENT",
    0,
	MOD_IF_CLI_NET,
	MOD_IF_CLI_CTRL,
	MOD_IF_CLI_NET,
	MUCB_IF_NET_CLI_RCV,
	MUCB_IF_NET_CLI_SEND,
	NP_TCP,
	false,
	-1,
	false,
	-1,
	LoginSvr,
	NULL,
	NULL,
	true,
	true	
};
//�ͻ���
char			g_szEdId[24];
char			g_szPwd[20];
int				g_upFlag = 0;						//����ƽ̨��ǣ�0�����ӣ�1����

CNetIfBase		g_NetIfTcpClient(stIfNet);          //Client������������
CMsgCtlClient	g_MsgCtlCliNet;						//Client�������ݴ���ģ��
TKU32			g_ulHandleNetCliSend;				//���緢�Ͷ��о��
TKU32			g_ulHandleNetCliRecv;				//������ն��о��

CNetDataContainer	g_NetContainer;							//����ӿ��������������ڽ���ͨ�����紫��Ĵ�������

/************************************************************************/
/* CPM�ӿ�ģ��                                                          */
/************************************************************************/
/*ϵͳ��*/
//CADCBase	g_CADCBaseA(1);/*/dev/spidev1.1��*/
//CADCBase	g_CADCBaseB(0);/*/dev/spidev1.2��*/

//int			g_hdlGPIO = -1;/*GPIO�ӿھ��*/

/*GSMģ��*/
//CGSMBaseSiemens		g_GSMBaseSiemens;
CGSMCtrl			g_GSMCtrl;						//GSM������Ϣ����ģ��
TKU32				g_ulHandleGSM;					//GSM���ƶ��о��
TKU32				g_ulHandleSMSSend;				//����Ϣ���Ͷ��о��
TKU32				g_ulHandleSMSRecv;				//����Ϣ���ն��о��

//485����ģ�飨��Ѳ��ִ�ж�����
CDeviceIfBase* g_ptrDeviceIf485_1;
CDeviceIfBase* g_ptrDeviceIf485_2;
CDeviceIfBase* g_ptrDeviceIf485_3;
CDeviceIfBase* g_ptrDeviceIf485_4;
CDeviceIfBase* g_ptrDeviceIf485_5;
CDeviceIfBase* g_ptrDeviceIf485_6;
CDeviceIfBase* g_ptrDeviceIf485_7;
CDeviceIfBase* g_ptrDeviceIf485_8;

TKU32 g_ulHandleActionList481_1;					//
TKU32 g_ulHandleActionList481_2;					//
TKU32 g_ulHandleActionList481_3;					//
TKU32 g_ulHandleActionList481_4;					//
TKU32 g_ulHandleActionList481_5;					//
TKU32 g_ulHandleActionList481_6;					//
TKU32 g_ulHandleActionList481_7;					//
TKU32 g_ulHandleActionList481_8;					//

//��������ģ����
CDeviceIfBase*	g_ptrDeviceIfAD;
TKU32 g_ulHandleActionListAD;					//

//���ݷ���ģ��
CAnalyser g_Analyser;								//���ݷ���ģ��
TKU32 g_ulHandleAnalyser;							//���ݷ���ģ����������о��


/************************************************************************/
/* sqlite3���ݿ⴦��ģ��                                                */
/************************************************************************/
CSqlCtl g_SqlCtl;

/************************************************************************/
/* ����                                                                 */
/************************************************************************/
TKU32			g_ulHandleBoaMsgCtrl;					//�첽�����Ӧ���о��
TKU32			g_ulHandleTmnMsgCtrl;					//�첽�����Ӧ���о��

/************************************************************************/
/* ��������Ƶ                                                           */
/************************************************************************/
//CAudioPlay g_AudioPlay;

/************************************************************************/
/* ����ģ��	                                                            */
/************************************************************************/

char				g_szLocalIp[24] = {0};							//����ip
char				g_szVersion[20] = {0};							//CPM�汾��
int					g_iTTLSampleCount = 1;						//��������������
int					g_iADSampleCount = 30;						//ģ������������

/************************************************************************/
/* ģ������	                                                            */
/************************************************************************/
MODE_CFG g_ModeCfg;

/************************************************************************/
/* ��¼��������	                                                        */
/************************************************************************/
RECORD_COUNT_CFG g_RecordCntCfg;

/************************************************************************/
/* ��ʱ��������	                                                        */
/************************************************************************/
COnTimeCtrl g_COnTimeCtrl;


/************************************************************************/
/* ͸������ҵ��ģ��	                                                        */
/************************************************************************/
CTransparent g_CTransparent;
TKU32 g_ulHandleTransparent;

bool CInit::Initialize(void)
{
//��ȡConfig.ini
	ReadConfigFile();
	
//sqlite3���ݿ�
	if (false == g_SqlCtl.Initialize(DB_TODAY_PATH))return false;

	//��ȡ�ӿ�������Ϣ
	memset((unsigned char*)&g_ModeCfg, 0, sizeof(MODE_CFG));
	g_SqlCtl.getModeConfig(g_ModeCfg);

	//��ȡϵͳ����������Ϣ
	memset((unsigned char*)&g_RecordCntCfg, 0, sizeof(RECORD_COUNT_CFG));
	g_SqlCtl.getRecordCountConfig(g_RecordCntCfg);
	
//�ڴ��ʼ��
	if(false == MemInit()) return false;			//�洢��ʼ��

//�豸��ʼ��
	if(false == g_DevInfoContainer.Initialize())return false;

//Ӳ���豸��ʼ��
	//GSM�ײ㷢�͡����ն���
	g_ulHandleSMSSend = 0;			
	if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_CTRL, MOD_DEV_SMS_SEND), g_ulHandleSMSSend) )//����Ϣ���Ͷ��о��
		return false;
	DBG(("Init g_ulHandleSMSSend[%d]\n", g_ulHandleSMSSend));
	g_ulHandleSMSRecv = 0;			
	if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_CTRL, MOD_DEV_SMS_RECV), g_ulHandleSMSRecv) )//����Ϣ���ն��о��
		return false;
	DBG(("Init g_ulHandleSMSRecv[%d]\n", g_ulHandleSMSRecv));

	STR_SMS_CFG strSmsCfg;
	strSmsCfg.ulHandleGSMSend = g_ulHandleSMSSend;
	strSmsCfg.ulHandleGSMRecv = g_ulHandleSMSRecv;
	HardIF_Initialize(CDevInfoContainer::ADCValueNoticeCallBack, (void*)&g_DevInfoContainer, strSmsCfg);

	//485_1
	if(1 == g_ModeCfg.RS485_1)
	{
		g_ulHandleActionList481_1 = 0;			
		if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_CTRL, MOD_ACTION_485_1), g_ulHandleActionList481_1) )//��ȡ485-1�������о��
			return false;
		g_ptrDeviceIf485_1 = new CDeviceIf485();
		if (SYS_STATUS_SUCCESS != g_ptrDeviceIf485_1->Initialize(INTERFACE_RS485_1, g_ulHandleActionList481_1))return false;
	}

	//485_2
	if(1 == g_ModeCfg.RS485_2)
	{
		g_ulHandleActionList481_2 = 0;			
		if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_CTRL, MOD_ACTION_485_2), g_ulHandleActionList481_2) )//��ȡ485-2�������о��
			return false;
		g_ptrDeviceIf485_2 = new CDeviceIf485();
		if (SYS_STATUS_SUCCESS != g_ptrDeviceIf485_2->Initialize(INTERFACE_RS485_2, g_ulHandleActionList481_2))return false;
	}

	//485_3
	if(1 == g_ModeCfg.RS485_3)
	{
		g_ulHandleActionList481_3 = 0;			
		if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_CTRL, MOD_ACTION_485_3), g_ulHandleActionList481_3) )//��ȡ485-3�������о��
			return false;
		g_ptrDeviceIf485_3 = new CDeviceIf485();
		if (SYS_STATUS_SUCCESS != g_ptrDeviceIf485_3->Initialize(INTERFACE_RS485_3, g_ulHandleActionList481_3))return false;
	}

	//485_4
	if(1 == g_ModeCfg.RS485_4)
	{
		g_ulHandleActionList481_4 = 0;			
		if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_CTRL, MOD_ACTION_485_4), g_ulHandleActionList481_4) )//��ȡ485-4�������о��
			return false;
		g_ptrDeviceIf485_4 = new CDeviceIf485();
		if (SYS_STATUS_SUCCESS != g_ptrDeviceIf485_4->Initialize(INTERFACE_RS485_4, g_ulHandleActionList481_4))return false;
	}

	//485_5
	if(1 == g_ModeCfg.RS485_5)
	{
		g_ulHandleActionList481_5 = 0;			
		if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_CTRL, MOD_ACTION_485_5), g_ulHandleActionList481_5) )//��ȡ485-5�������о��
			return false;
		g_ptrDeviceIf485_5 = new CDeviceIf485();	
		if (SYS_STATUS_SUCCESS != g_ptrDeviceIf485_5->Initialize(INTERFACE_RS485_5, g_ulHandleActionList481_5))return false;
	}

	//485_6
	if(1 == g_ModeCfg.RS485_6)
	{
		g_ulHandleActionList481_6 = 0;			
		if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_CTRL, MOD_ACTION_485_6), g_ulHandleActionList481_6) )//��ȡ485-6�������о��
			return false;
		g_ptrDeviceIf485_6 = new CDeviceIf485();
		if (SYS_STATUS_SUCCESS != g_ptrDeviceIf485_6->Initialize(INTERFACE_RS485_6, g_ulHandleActionList481_6))return false;
	}

	//485_7
	if(1 == g_ModeCfg.RS485_7)
	{
		g_ulHandleActionList481_7 = 0;			
		if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_CTRL, MOD_ACTION_485_7), g_ulHandleActionList481_7) )//��ȡ485-7�������о��
			return false;
		g_ptrDeviceIf485_7 = new CDeviceIf485();
		if (SYS_STATUS_SUCCESS != g_ptrDeviceIf485_7->Initialize(INTERFACE_RS485_7, g_ulHandleActionList481_7))return false;
	}

	//485_8
	if(1 == g_ModeCfg.RS485_8)
	{
		g_ulHandleActionList481_8 = 0;			
		if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_CTRL, MOD_ACTION_485_8), g_ulHandleActionList481_8) )//��ȡ485-8�������о��
			return false;
		g_ptrDeviceIf485_8 = new CDeviceIf485();
		if (SYS_STATUS_SUCCESS != g_ptrDeviceIf485_8->Initialize(INTERFACE_RS485_8, g_ulHandleActionList481_8))return false;
	}

	//AD
	if(1 == g_ModeCfg.Ad)
	{
		g_ulHandleActionListAD = 0;			
		if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_CTRL, MOD_ACTION_ADS), g_ulHandleActionListAD) )//��ȡģ�����������������о��
			return false;
		g_ptrDeviceIfAD = new CDeviceIfAD();
		if (SYS_STATUS_SUCCESS != g_ptrDeviceIfAD->Initialize(INTERFACE_AD_A0, g_ulHandleActionListAD))return false;
	}

	if(false == TcpClientInit()) return false;

	if(false == TcpServerInit()) return false;

	//��ʼ�����紫�нӿ�
	if(1 == g_ModeCfg.NetTerminal)
	{
		if( false == g_NetContainer.Initialize()) return false;
	}

	//GSM��ѯ����������
	g_ulHandleGSM = 0;			
	if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_CTRL, MOD_ACTION_GSM), g_ulHandleGSM) )//����Ϣ���Ͷ��о��
		return false;
	if(1 == g_ModeCfg.GSM)
	{
		if(!g_GSMCtrl.Initialize(g_ulHandleGSM))
			return false;
	}

	//͸������ҵ����ģ��
	if(1 == g_ModeCfg.Transparent)
	{
		g_ulHandleTransparent = 0;			
		if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_CTRL, MOD_DEV_TRANSPARENT), g_ulHandleTransparent) )//��ȡ�к������ݴ�����о��
			return false;
		g_CTransparent.Initialize(g_ulHandleTransparent);
	}

	//������ҳ��Ϣ����ģ��
	g_ulHandleBoaMsgCtrl = 0;
	if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_CTRL, MOD_DEV_MSGCTRL_BOA), g_ulHandleBoaMsgCtrl) )		//��ȡ����ָ��������о��
		return false;
	if(!g_MsgCtlSvrBoa.Initialize(COMBINE_MOD(stIfTcpSvrBoa.self,stIfTcpSvrBoa.ctrl), "TCPSVR_BOA�����߳�����"))
	{
		DBG(("BoaMsgCtlSvr Initialize failed!\n"));
		return false;
	}

	//�����ն���Ϣ����ģ��
	g_ulHandleTmnMsgCtrl = 0;
	if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_CTRL, MOD_DEV_MSGCTRL_TMN), g_ulHandleTmnMsgCtrl) )		//��ȡ����ָ��������о��
		return false;
	if(!g_MsgCtlSvrTmn.Initialize(COMBINE_MOD(stIfTcpSvrTmn.self,stIfTcpSvrTmn.ctrl),"TCPSVR_TMN�����߳�����"))
	{
		DBG(("MsgCtlSvr Initialize failed!\n"));
		return false;
	}

	//����ģ��
	g_ulHandleAnalyser = 0;			
	if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_CTRL, MOD_DEV_ANALYSE), g_ulHandleAnalyser) )//��ȡ���ݷ���ģ����������о��
		return false;

	if(false == g_Analyser.Initialize(g_ulHandleAnalyser))return false;         //�������ݷ���ģ��

	//��ʱ��������
	if(false == g_COnTimeCtrl.Initialize())return false;

	for (int i=0; i<3; i++)
	{
		HardIF_AllLightON();
		sleep(1);
		HardIF_AllLightOFF();
		sleep(1);
	}

	DBG(("CInit::Initialize success\n"));
	return true;
}
void CInit::Terminate(void)
{

}

//������Ϣͨ���������ռ䣬ϵͳ��Ϣͨ��MUCB_MAX��
bool CInit::MemInit()
{
	if (MMO_OK != g_MemMgr.Initialize(MUCB_MAX))
	{
		DBG(("MMInitialize failed!\n"));
		return false;
	}	
	for (int i = 0; i < MUCB_MAX; i++)// ����Ϣ������ֵ������������Ӧ�Ŀռ�
	{
		if (!g_MemMgr.RegRsvMem(&g_stHubMUCBArray[i]))
		{
			return false;
		}
	}
	if (! g_MsgMgr.Initialize(SYS_MOD_MAX))//��Ϣ��������ͷ����
	{
		DBG(("MsgMgr.Initialize failed\n"));
		return false;
	}

	//������Ϣͨ������������Ϣ���г�ʼ�� lsd��ҳ���ò���
	size_t nTID = 0;
	//�ͻ���������ն���
	if (MMR_OK != g_MsgMgr.AllocTunnel(COMBINE_MOD(MOD_IF_CLI_NET, MOD_IF_CLI_CTRL), g_RecordCntCfg.NetClientRecv_Max, nTID))
	{
		DBG(("SetTunnel failed\n"));
		return false;
	}
	//�ͻ������緢�Ͷ���	
	if (MMR_OK != g_MsgMgr.AllocTunnel(COMBINE_MOD(MOD_IF_CLI_CTRL, MOD_IF_CLI_NET), g_RecordCntCfg.NetClientSend_Max, nTID))
	{
		DBG(("SetTunnel failed\n"));
		return false;
	} 

	//��ҳ������ն���
	if (MMR_OK != g_MsgMgr.AllocTunnel(COMBINE_MOD(MOD_IF_SVR_NET_BOA, MOD_IF_SVR_CTRL_BOA), g_RecordCntCfg.NetRecv_Max, nTID))
	{
		DBG(("SetTunnel failed\n"));
		return false;
	}
	//��ҳ�����Ͷ���	
	if (MMR_OK != g_MsgMgr.AllocTunnel(COMBINE_MOD(MOD_IF_SVR_CTRL_BOA, MOD_IF_SVR_NET_BOA), g_RecordCntCfg.NetSend_Max, nTID))
	{
		DBG(("SetTunnel failed\n"));
		return false;
	} 

	//�ն˷�����ն���
	if (MMR_OK != g_MsgMgr.AllocTunnel(COMBINE_MOD(MOD_IF_SVR_NET_TMN, MOD_IF_SVR_CTRL_TMN), g_RecordCntCfg.NetRecv_Max, nTID))
	{
		DBG(("SetTunnel failed\n"));
		return false;
	}
	//�ն˷����Ͷ���	
	if (MMR_OK != g_MsgMgr.AllocTunnel(COMBINE_MOD(MOD_IF_SVR_CTRL_TMN, MOD_IF_SVR_NET_TMN), g_RecordCntCfg.NetSend_Max, nTID))
	{
		DBG(("SetTunnel failed\n"));
		return false;
	} 


	//485-1�������
	if (MMR_OK != g_MsgMgr.AllocTunnel(COMBINE_MOD(MOD_CTRL, MOD_ACTION_485_1), g_RecordCntCfg.RS485_1_Max, nTID))
	{
		DBG(("SetTunnel failed\n"));
		return false;
	}
	//485-2�������
	if (MMR_OK != g_MsgMgr.AllocTunnel(COMBINE_MOD(MOD_CTRL, MOD_ACTION_485_2), g_RecordCntCfg.RS485_2_Max, nTID))
	{
		DBG(("SetTunnel failed\n"));
		return false;
	}
	//485-3�������
	if (MMR_OK != g_MsgMgr.AllocTunnel(COMBINE_MOD(MOD_CTRL, MOD_ACTION_485_3), g_RecordCntCfg.RS485_3_Max, nTID))
	{
		DBG(("SetTunnel failed\n"));
		return false;
	}
	//485-4�������
	if (MMR_OK != g_MsgMgr.AllocTunnel(COMBINE_MOD(MOD_CTRL, MOD_ACTION_485_4), g_RecordCntCfg.RS485_4_Max, nTID))
	{
		DBG(("SetTunnel failed\n"));
		return false;
	}
	//485-5�������
	if (MMR_OK != g_MsgMgr.AllocTunnel(COMBINE_MOD(MOD_CTRL, MOD_ACTION_485_5), g_RecordCntCfg.RS485_5_Max, nTID))
	{
		DBG(("SetTunnel failed\n"));
		return false;
	}
	//485-6�������
	if (MMR_OK != g_MsgMgr.AllocTunnel(COMBINE_MOD(MOD_CTRL, MOD_ACTION_485_6), g_RecordCntCfg.RS485_6_Max, nTID))
	{
		DBG(("SetTunnel failed\n"));
		return false;
	}
	//485-7�������
	if (MMR_OK != g_MsgMgr.AllocTunnel(COMBINE_MOD(MOD_CTRL, MOD_ACTION_485_7), g_RecordCntCfg.RS485_7_Max, nTID))
	{
		DBG(("SetTunnel failed\n"));
		return false;
	}
	//485-8�������
	if (MMR_OK != g_MsgMgr.AllocTunnel(COMBINE_MOD(MOD_CTRL, MOD_ACTION_485_8), g_RecordCntCfg.RS485_8_Max, nTID))
	{
		DBG(("SetTunnel failed\n"));
		return false;
	}
	//ADS�������
	if (MMR_OK != g_MsgMgr.AllocTunnel(COMBINE_MOD(MOD_CTRL, MOD_ACTION_ADS), g_RecordCntCfg.ADS_Max, nTID))
	{
		DBG(("SetTunnel failed\n"));
		return false;
	}
	//GSM�������
	if (MMR_OK != g_MsgMgr.AllocTunnel(COMBINE_MOD(MOD_CTRL, MOD_ACTION_GSM), g_RecordCntCfg.GSM_Max, nTID))
	{
		DBG(("SetTunnel failed\n"));
		return false;
	}
	//Ĭ�ϲ���lsd
	for (int i = 0; i < (int)(sizeof(stQMC) / sizeof(MsgQCfg)); i++)//������Ϣͨ������������Ϣ���г�ʼ��
	{
		if (MMR_OK != g_MsgMgr.AllocTunnel(stQMC[i].unDirect, stQMC[i].unQLen, nTID))
		{
			DBG(("SetTunnel failed\n"));
			return false;
		} 
	}
	return true;
}


//����TcpServer����
bool CInit::TcpServerInit()
{
	//BOA
	g_ulHandleNetBoaSvrRecv = 0;
	if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_IF_SVR_NET_BOA, MOD_IF_SVR_CTRL_BOA), g_ulHandleNetBoaSvrRecv) )//��ȡTcpServer���ؽ��ն��о��
		return false;

	g_ulHandleNetBoaSvrSend = 0;	
	if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_IF_SVR_CTRL_BOA, MOD_IF_SVR_NET_BOA), g_ulHandleNetBoaSvrSend) )//��ȡTcpServer���ط��Ͷ��о��
		return false;

	if(false == g_NetIfSvrBoa.Initialize())           // TCPServer
	{
		DBG(("g_NetIfSvrBoa.Initialize failed\n"));
		return false;
	}

	//TERMINAL
	g_ulHandleNetTmnSvrRecv = 0;
	if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_IF_SVR_NET_TMN, MOD_IF_SVR_CTRL_TMN), g_ulHandleNetTmnSvrRecv) )//��ȡTcpServer���ؽ��ն��о��
		return false;

	g_ulHandleNetTmnSvrSend = 0;	
	if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_IF_SVR_CTRL_TMN, MOD_IF_SVR_NET_TMN), g_ulHandleNetTmnSvrSend) )//��ȡTcpServer���ط��Ͷ��о��
		return false;

	if(false == g_NetIfSvrTmn.Initialize())           // TCPServer
	{
		DBG(("g_NetIfSvrTmn.Initialize failed\n"));
		return false;
	}

	//��Զ�̽����TCP�ͻ��������ջ�͹�ϣ��
	if (false == g_ConnContainer.Initialize(COMM_TCPSRV_MAXClIENT))
	{
		return false;
	}	

	//��Զ�̽����TCP�ͻ����������ͨ��ID�ͷ���ͨ����ID��ϣ��A2I��I2A
	if (false == g_RouteMgr.Initialize(COMM_TCPSRV_MAXClIENT))
	{
		return false;
	}

	if (false == TcpSvrBoa())
	{
		return false;
	}

	if (false == TcpSvrTmn())
	{
		return false;
	}

	DBG(("TcpServer init success!\n"));
	return true;
}

//����Tcp Client
bool CInit::TcpClientInit()
{
	g_ulHandleNetCliRecv = 0;
	if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_IF_CLI_NET, MOD_IF_CLI_CTRL), g_ulHandleNetCliRecv) )//��ȡ�ϼ����ؽ��ն��о��
		return false;

	g_ulHandleNetCliSend = 0;	
	if(MMR_OK != g_MsgMgr.Attach( COMBINE_MOD(MOD_IF_CLI_CTRL, MOD_IF_CLI_NET), g_ulHandleNetCliSend) )//��ȡ�ϼ����ط��Ͷ��о��
		return false;

	if( g_upFlag == 1 )
	{
		if(false == g_NetIfTcpClient.Initialize())		// TCPClient
		{
			DBG(("g_NetIfTcpClient.Initialize failed\n"));
			return true;
		}
		if(!g_MsgCtlCliNet.Initialize(COMBINE_MOD(stIfNet.self,stIfNet.ctrl),"TcpClient�����߳�����"))
		{
			DBG(("szMsgCtlBcmpp Initialize failed!\n"));
			return true;
		}
	}
	
	DBG(("TcpClientInit success\n"));
	return true;
}

bool CInit::TcpSvrBoa()
{
	//����TCP server	
	ConnMgrHubBoa.SetModIfInfo(stIfTcpSvrBoa, &g_NetIfSvrBoa);

	int port;
	char ip[20] = {0};

	//if (-1 == INI_READ_STRING(stIfTcpSvr, "LocalAddr", ip)  ||           //ȡ��IP�Ͷ˿ں�
	int sock;
	struct sockaddr_in sin;
	struct ifreq ifr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
	{
		return false;
	}

	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	if (ioctl(sock, SIOCGIFADDR, &ifr) < 0)
	{
		return false;
	}

	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	sprintf(ip, "%s", inet_ntoa(sin.sin_addr));
	DBG(("ip:[%s]\n", ip));

	if( -1 == INI_READ_INT(stIfTcpSvrBoa, "LocalPort", port) )
		return false;

	DBG(("Net If Init....ip[%s] port[%d]\n", ip, port));
	ClsInetAddress addr(ip);
	if (false == ConnMgrHubBoa.ListenAt(addr, port))                        //����IP�Ͷ˿�
	{
		DBG(("Fail at :  connMgr.ListenAt('%s', %d)\n", ip, port));
		return false;
	}
	strcpy(g_szLocalIp, ip);
	DBG(("G_LocalIp[%s]\n", g_szLocalIp));
	//char buf[256] = {0};
	//ConnMgrHub.GetSvrLocal(buf, sizeof(buf));
	return true; 
}

bool CInit::TcpSvrTmn()
{
	//����TCP server	
	ConnMgrHubTmn.SetModIfInfo(stIfTcpSvrTmn, &g_NetIfSvrTmn);

	int port;
	char ip[20] = {0};

	//if (-1 == INI_READ_STRING(stIfTcpSvr, "LocalAddr", ip)  ||           //ȡ��IP�Ͷ˿ں�
	int sock;
	struct sockaddr_in sin;
	struct ifreq ifr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
	{
		return false;
	}

	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	if (ioctl(sock, SIOCGIFADDR, &ifr) < 0)
	{
		return false;
	}

	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	sprintf(ip, "%s", inet_ntoa(sin.sin_addr));
	DBG(("ip:[%s]\n", ip));

	if( -1 == INI_READ_INT(stIfTcpSvrTmn, "LocalPort", port) )
		return false;

	DBG(("Net If Init....ip[%s] port[%d]\n", ip, port));
	ClsInetAddress addr(ip);
	if (false == ConnMgrHubTmn.ListenAt(addr, port))                        //����IP�Ͷ˿�
	{
		DBG(("Fail at :  connMgr.ListenAt('%s', %d)\n", ip, port));
		return false;
	}
	strcpy(g_szLocalIp, ip);
	DBG(("G_LocalIp[%s]\n", g_szLocalIp));
	//char buf[256] = {0};
	//ConnMgrHub.GetSvrLocal(buf, sizeof(buf));
	return true; 
}

bool CInit::WaitForCmd()
{
	//Ϊ������
	char Cmd[256] = {0};
	bool test = true;
	int iTime = -1;
	int iWatchDogHandle = -1;
	int iDogFlag = 0;
	int iResetFlag = 0;
	get_ini_int("Config.ini","SYSTEM","WatchDog", 0, &iDogFlag);
	get_ini_int("Config.ini","SYSTEM","ResetEnable", 0, &iResetFlag);
	bool bDogOpen = iDogFlag == 1?true:false;
	DBG(("DogFlag[%d] iResetFlag[%d]\n",iDogFlag, iResetFlag));
	if( bDogOpen )
	{
		iWatchDogHandle = open ("/dev/watchdog", O_RDWR);//O_RDWR
		if (0 > iWatchDogHandle)
		{
			DBG(("���Ź��豸��ʧ�ܣ���ȷ���Ƿ��������ں�\n"));
			return false;
		}
		//�������Ź� 
		ioctl(iWatchDogHandle, WDIOC_SETOPTIONS, WDIOS_ENABLECARD);

		iTime = 20;
		printf("Set watchdog timer %d seconds\n", iTime);
		ioctl(iWatchDogHandle, WDIOC_SETTIMEOUT, &iTime);

		ioctl(iWatchDogHandle, WDIOC_GETTIMEOUT, &iTime);
		printf("Time = %d\n", iTime);
	}

	DBG(("CInit::WaitForCmd .... Pid[%d]\n", getpid()));
	bool g_bThrdReadStop = false;
	int resetFlag = 0;
	while (!g_bThrdReadStop)
	{	
		if (bDogOpen)
		{
			write(iWatchDogHandle, "1", 1);//ι��
		}

		g_COnTimeCtrl.OnTimeCtrlProc();
		//DBG(("GDO0 [%d]\n", EmApiGetGDO0(g_hdlGPIO)));

		//��λ�ж�
		if(1 == iResetFlag)
		{
			if(0 == HardIF_GetGD0())
				resetFlag++;
			else
				resetFlag = 0;
			if(5 == resetFlag)
			{				
				if(-1 == system("cp -r /root/network_bak /root/network"))
				{
					printf("cp -r /root/network_bak /root/network failed \n");
					resetFlag = 0;
					continue;
				}
				if (-1 == system("chmod +x /root/network"))
				{
					printf("chmod +x /root/network failed \n");
					resetFlag = 0;
					continue;
				}
				if (-1 == system("/root/network"))
				{
					printf("/root/network failed \n");
					resetFlag = 0;
					continue;
				}
				printf("end reset network resetFlag[%d]\n",resetFlag);
			}
		}
		if( test )
		{
			sleep(1);
			continue;
		}
		memset(Cmd, 0, sizeof(Cmd));
		scanf("%s", Cmd);
		switch (atoi(Cmd))
		{
		case 0:
			g_bThrdReadStop = true;
			break;
		case 1://��������1
			{

			}
			break;
		case 3://RF433���Ͳ���
			{

			}
			break;				
		}
		sleep(1);
	}
	return true;
}


void CInit::ReadConfigFile()
{
	memset(g_szEdId, 0, sizeof(g_szEdId));
	memset(g_szPwd, 0, sizeof(g_szPwd));
	g_upFlag = 0;
	memset(g_szVersion, 0, sizeof(g_szVersion));

	get_ini_string("Config.ini","PARENT","RemoteLoginId" ,"=", g_szEdId, sizeof(g_szEdId));
	get_ini_string("Config.ini","PARENT","RemoteLoginPwd" ,"=", g_szPwd, sizeof(g_szPwd));
    get_ini_int("Config.ini","PARENT","RemoteStatus", 0, &g_upFlag);

	get_ini_string("Config.ini","SYSTEM","Version" ,"=", g_szVersion, sizeof(g_szVersion));

    get_ini_int("Config.ini","INTER_CONFIG","TTLSampleCount", 0, &g_iTTLSampleCount);
    get_ini_int("Config.ini","INTER_CONFIG","ADSampleCount", 0, &g_iADSampleCount);

	DBG(("RemoteLoginId[%s] RemoteLoginPwd[%s] RemoteStatus[%d] Version[%s] TTLSampleCount[%d] ADSampleCount[%d]\n", g_szEdId, g_szPwd, g_upFlag, g_szVersion, g_iTTLSampleCount, g_iADSampleCount));

}

//--------------------------------END OF FILE---------------------------------------
