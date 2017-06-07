#ifndef __NET_APP_H__
#define __NET_APP_H__

#include "Define.h"
#pragma pack(1)

//2020 �豸״̬��ѯ
typedef struct _StrNetDevStatusQuery{      
	char szReserve[20];             //������
	char szStatus[4];				//�����״̬
	char szDeal[4];					//����ָ��
	char szIds[1024];
}StrNetDevStatusQuery, *PStrNetDevStatusQuery;

//2021 �豸�������ݲ�ѯ
typedef struct _StrNetDevParasQuery{      
	char szReserve[20];             //������
	char szStatus[4];				//�����״̬
	char szDeal[4];					//����ָ��
	char szIds[1024];
}StrNetDevParasQuery, *PStrNetDevParasQuery;

//2022 �豸ģʽ���ݲ�ѯ
typedef struct _StrNetDevModesQuery{      
	char szReserve[20];             //������
	char szStatus[4];				//�����״̬
	char szDeal[4];					//����ָ��
	char szIds[1024];
}StrNetDevModesQuery, *PStrNetDevModesQuery;

//2000 �豸״̬��ѯ
typedef struct _StrNetDevQuery{      
	char szReserve[20];             //������
	char szStatus[4];				//�����״̬
	char szDeal[4];					//����ָ��
	char szDevType[DEVICE_TYPE_LEN];				//�豸�ͺ�
	char szId[DEVICE_INDEX_LEN];					//�豸���
}StrNetDevQuery, *PStrNetDevQuery;

//2001/2002 �豸���/�޸�
typedef struct _StrNetDevUpdate{      
	char szReserve[20];             //������
	char szStatus[4];				//�����״̬
	char szDeal[4];					//����ָ��
	char szDevType[DEVICE_TYPE_LEN];				//�豸�ͺ�
	char szId[DEVICE_INDEX_LEN];					//�豸���
	char szReserve1[DEVICE_ID_LEN];				//�ϼ��豸
	char szDevAppType[2];			//�豸Ӧ������
	char szChannelId[2];			//ͨ�����
	char szSelfId[20];				//��ͨ����
	char szCName[30];				//�豸����
	char szBrief[20];				//�豸���
	char szPrivateAttr[255];		//�豸˽�в���
	char szReserve2[128];		//�豸˽�в���
	char szMemo[128];			//�豸��ע
}StrNetDevUpdate, *PStrNetDevUpdate;

//2003 �豸ɾ��
typedef struct _StrNetDevDel{      
	char szReserve[20];             //������
	char szStatus[4];				//�����״̬
	char szDeal[4];					//����ָ��
	char szDevType[DEVICE_TYPE_LEN];				//�豸�ͺ�
	char szId[DEVICE_INDEX_LEN];					//�豸���
}StrNetDevDel, *PStrNetDevDel;

//2005 �豸�ɼ��������/�޸�
typedef struct _StrNetDevAttrUpdate{      
	char szReserve[20];             //������
	char szStatus[4];				//�����״̬
	char szDeal[4];					//����ָ��
	char szDevType[DEVICE_TYPE_LEN];				//�豸�ͺ�
	char szId[DEVICE_INDEX_LEN];					//�豸���
	char szAttrId[DEVICE_SN_LEN];				//�豸�ɼ����Ա��
	char szSecureTime[20];			//����ʱ��
	char szStandardValue[60];		//����ֵ��Χ
	char szPrivateAttr[255];		//����˽�в���
	char szV_DevType[DEVICE_TYPE_LEN];			//�����豸����
	char szV_Id[DEVICE_INDEX_LEN];					//�����豸���
	char szV_AttrId[DEVICE_SN_LEN];				//�����豸���Ա��
	char cIsShow;					//�Ƿ���������
	char szS_define[128];
	char szAttr_Name[20];
	char cStatus;
	char szValueLimit[64];
}StrNetDevAttrUpdate, *PStrNetDevAttrUpdate;

//2006 �豸�����������/�޸�
typedef struct _StrNetDevActionUpdate{      
	char szReserve[20];             //������
	char szStatus[4];				//�����״̬
	char szDeal[4];					//����ָ��
	char szDevType[DEVICE_TYPE_LEN];				//�豸�ͺ�
	char szId[DEVICE_INDEX_LEN];					//�豸���
	char szActionId[DEVICE_ACTION_SN_LEN];				//�豸�ɼ����Ա��
	char szActionName[20];			//����ʱ��
	char cStatus;		
	char szPrivateAttr[255];
	char szVDevType[DEVICE_TYPE_LEN];
	char szVDevId[DEVICE_INDEX_LEN];
	char szVActionId[DEVICE_ACTION_SN_LEN];
}StrNetDevActionUpdate, *PStrNetDevActionUpdate;

//2007 �豸�ɼ�����ɾ��
typedef struct _StrNetDevAttrDel{      
	char szReserve[20];             //������
	char szStatus[4];				//�����״̬
	char szDeal[4];					//����ָ��
	char szDevType[DEVICE_TYPE_LEN];				//�豸�ͺ�
	char szId[DEVICE_INDEX_LEN];					//�豸���
	char szAttrId[DEVICE_SN_LEN];				//�豸�ɼ����Ա��
}StrNetDevAttrDel, *PStrNetDevAttrDel;

//2004 �豸��������
typedef struct _StrNetDevAttrAction{      
	char szReserve[20];             //������
	char szStatus[4];				//�����״̬
	char szDeal[4];					//����ָ��
	char szDevType[DEVICE_TYPE_LEN];				//�豸�ͺ�
	char szId[DEVICE_INDEX_LEN];					//�豸���
	char szAttrId[DEVICE_ACTION_SN_LEN];				//�豸�ɼ����Ա��
	char ACTIONS[1024];				//��������
}StrNetDevAttrAction, *PStrNetDevAttrAction;

//2008/2009  ��ʱ�������/�޸�
typedef struct _StrNetTimeTaskUpdate{
    char szReserve[20];             //������
	char szStatus[4];				//�����״̬
	char szDeal[4];					//����ָ��
    char szSeq[10];                 //Ψһ��
	char szId[DEVICE_ID_LEN];					//�豸���
	char szSn[DEVICE_ACTION_SN_LEN];				    //�豸�������
    char szTime[12];                 //��ʱʱ��---lsd��Ϊ12λ
    char szValue[256];              //��������
    char szMemo[128];               //����˵��
}StrNetTimeTaskUpdate, *PStrNetTimeTaskUpdate;

//2010  ��ʱ����ɾ��
typedef struct _StrNetTimeTaskDel{
    char szReserve[20];             //������
	char szStatus[4];				//�����״̬
	char szDeal[4];					//����ָ��
    char szSeq[10];                 //Ψһ��
}StrNetTimeTaskDel, *PStrNetTimeTaskDel;

//2011/2012 �豸�����������/�޸�
typedef struct _StrNetDevAgentUpdate{      
	char szReserve[20];             //������
	char szStatus[4];				//�����״̬
	char szDeal[4];					//����ָ��
	char szSeq[10];					//�������
	char szDevType[DEVICE_TYPE_LEN];				//�豸�ͺ�
	char szId[DEVICE_INDEX_LEN];					//�豸���
	char szAttrId[DEVICE_SN_LEN];				//�豸�ɼ����Ա��
	char szCondition[128];			//������������
	char szValibtime[30];			//������Чʱ��
	char szInterval[4];					//�������
	char szTimes[4];					//��������
	char szActDevType[DEVICE_TYPE_LEN];			//�����豸����
	char szActDevId[DEVICE_INDEX_LEN];				//�����豸���
	char szActActionId[DEVICE_ACTION_SN_LEN];			//�����豸�������
	char szActValue[256];			//����ʱ��������
	char cType;						//�������ͣ�0:�ɼ�������1:��������
}StrNetDevAgentUpdate, *PStrNetDevAgentUpdate;

//3001�豸��������
typedef struct _StrNetDevDefenceSet{      
	char szReserve[20];             //������
	char szStatus[4];				//�����״̬
	char szDeal[4];					//����ָ��
	char szDevType[DEVICE_TYPE_LEN];				//�豸�ͺ�
	char szId[DEVICE_INDEX_LEN];					//�豸���
	char cType;						//��������������
}StrNetDevDefenceSet, *PStrNetDevDefenceSet;

//3003�豸ͬ��ָ��
typedef struct _StrNetDevSyn{      
	char szReserve[20];             //������
	char szStatus[4];				//�����״̬
	char szDeal[4];					//����ָ��
}StrNetDevSyn, *PStrNetDevSyn;

//�����·�
typedef struct _StrNetCmd{      
	char szReserve[20];             //������
	char szStatus[4];				//�����״̬
	char szDeal[4];					//����ָ��
	char szDevType[DEVICE_TYPE_LEN];				//�豸�ͺ�
	char szId[DEVICE_INDEX_LEN];					//�豸���
	char szActionId[DEVICE_ACTION_SN_LEN];				//�豸�������Ա��
	char szOprator[10];				//�豸����Ա
	char szActionValue[256];				//����ֵ
}StrNetCmd, *PStrNetCmd;
/*�����·�
typedef struct _StrNetSCHSysCfg{      
	char szReserve[20];             //������
	char szStatus[4];				//�����״̬
	char szDeal[4];					//����ָ��
	char szTimeStatusChange[10];	//
	char szTimeOutDoor[10];			//
	char szCountIndoor[10];			//
	char szTimeIndoor[10];			//
}StrNetSCHSysCfg, *PStrNetSCHSysCfg;*/

typedef struct _MSG_NET_UPLOAD
{
	char szReserve[20];             //������
	char szStatus[4];				//�����״̬
	char szDeal[4];					//����ָ��
	char szData[2020];				//����
}MSG_NET_UPLOAD, *PMSG_NET_UPLOAD;
#pragma pack()

void Net_App_Resp_Proc(void* data, char* outBuf, int outLen,  unsigned int seq);
int Net_Dev_Status_Query(void * userData);
int Net_Dev_Paras_Query(void * msg);
int Net_Dev_Modes_Query(void * msg);

int Net_Dev_Query(const char* inBuf, int inLen, char* outBuf, int& outLen);
int Net_Dev_Real_Status_Query(const char* inBuf, int inLen, char* outBuf, int& outLen);

//�豸���
int Net_Dev_Add(const char* inBuf, int inLen, char* outBuf, int& outLen);

//�豸����
int Net_Dev_Update(const char* inBuf, int inLen, char* outBuf, int& outLen);

//�豸ɾ��
int Net_Dev_Del(const char* inBuf, int inLen, char* outBuf, int& outLen);

//�豸�ɼ������޸�
int Net_Dev_Attr_Update(const char* inBuf, int inLen, char* outBuf, int& outLen);
//�豸���������޸�
int Net_Dev_Action_Update(const char* inBuf, int inLen, char* outBuf, int& outLen);

//��ʱ�������
int Net_Time_Task_Add(const char* inBuf, int inLen, char* outBuf, int& outLen);

//��ʱ�����޸�
int Net_Time_Task_Update(const char* inBuf, int inLen, char* outBuf, int& outLen);

//��ʱ����ɾ��
int Net_Time_Task_Del(const char* inBuf, int inLen, char* outBuf, int& outLen);

int Net_Dev_Agent_Add(const char* inBuf, int inLen, char* outBuf, int& outLen);
int Net_Dev_Agent_Update(const char* inBuf, int inLen, char* outBuf, int& outLen);
int Net_Dev_Agent_Del(const char* inBuf, int inLen, char* outBuf, int& outLen);


//������������
//int Net_Dev_Defence_Update(const char* inBuf, int inLen, char* outBuf, int& outLen);

//�����·�
int Net_Dev_Action(const char* inBuf, int inLen, unsigned int seq, int actionSource);

bool Net_Up_Msg(int upCmd, char* buf, int bufLen);
int Net_Dev_Syn(const char* inBuf, int inLen, char* outBuf, int& outLen);
int Net_Dev_Attr_Syn(const char* inBuf, int inLen, char* outBuf, int& outLen);
int Net_Dev_Act_Syn(const char* inBuf, int inLen, char* outBuf, int& outLen);


int Net_GSM_READ_IMSI(const char* inBuf, int inLen, char* outBuf, int& outLen);
#endif
