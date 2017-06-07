#ifndef _MSG_CTL_
#define _MSG_CTL_

#include <tr1/unordered_map>
#include "Shared.h"
#include "Define.h"
#include "Init.h"
#include "SeqRscMgr.h"
#include "MsgCtlBase.h"
#include "Md5.h"
#include "Apm.h"
#include "NetApp.h"
using namespace std;
#pragma  pack(1)
typedef struct _MSG_ACTION_HASH
{
	TKU32   Id;				 //���
	TKU32   unMsgLen;        //����
	TKU32   unMsgCode;       //��Ϣ��
	TKU32   unStatus;        //��Ϣִ��״̬
	TKU32   unMsgSeq;        //��Ϣ���к�	
	TKU32   unRspTnlId;      //������Ϣʹ�õ�ͨ����

	char	szReserve[21];   //������
	char	szStatus[5];	 //�����״̬
	char	szDeal[5];		 //����ָ��

	time_t	act_time;
	int		source;		     //��Դ,�μ�ö��ACTION_SOURCE
}MSG_ACTION_HASH, *PMSG_ACTION_HASH;

typedef  tr1::unordered_map<TKU32, MSG_ACTION_HASH> HashedActInfo;

typedef struct _APM_INFO 
{
	int iApmId;
	char szApmPwd[17];
}APM_INFO, *PAPM_INFO;

#pragma pack()
//��BOA��ҳͨѶ��TCPSVR��
class CBOAMsgCtlSvr : CMsgCtlBase
{
protected:
	static bool SVR_Submit_Func (CBOAMsgCtlSvr *_this, char *msg);
	static bool SVR_Terminate_Func (CBOAMsgCtlSvr *_this, char *msg);

public:
	virtual bool Initialize(TKU32 unInputMask, const char *szCtlName);
    bool doSVR_Submit_Func(char *msg);
    bool doSVR_Terminate_Func (char *msg);
	bool doSVR_Err_Func (char *msg);
	bool Dispatch(CBOAMsgCtlSvr *_this, char *pMsg, char *pBuf, AppInfo *stAppInfo, BYTE Mod);
	void MD5(unsigned char *szBuf, unsigned char *src, unsigned int len);

    bool addNode(MSG_ACTION_HASH hash_node);
//    bool GetHashData(MSG_ACTION_HASH& hash_node);
	bool GetAndDeleteHashData(MSG_ACTION_HASH& hash_node);

//	bool GetSocketId (char* channelId, TKU32& unClientId);
	TKU32 GetSeq()
	{
		if (0xffffffff == m_Seq++)
		{
			m_Seq = 0;
		}
		return m_Seq;
	}

public:
	//�����ʹ�����lsd
	//bool InitApmHashData(char* sqlData);
	//static int InitApmHashCallback(void *data, int n_column, char **column_value, char **column_name);
	//int InitApmHashCallbackProc(int n_column, char **column_value, char **column_name);

private://hashtable ���

	HashedActInfo*  m_ptrActInfoTable;
    class Lock
	{
	private:
		pthread_mutex_t	 m_lock;
	public:
		Lock() { pthread_mutex_init(&m_lock, NULL); }
		bool lock() { return pthread_mutex_lock(&m_lock)==0; }
		bool unlock() { return pthread_mutex_unlock(&m_lock)==0; }
	}CTblLock;
private:
    static void *pActCtrlThrd(void* arg);
    void MsgActCtrl();
    //HashedInfo* m_pSocketHashTable;           //socket���ӿͻ��˹�ϣ��
	TKU32 ulHandleTcpSvr;
    TKU32 ulHandleActCtrl;
    int m_id;
	TKU32 m_Seq;
	int m_MaxApmCount;
};
//�������ʹ�����TERMINAL ͨѶ��TCPSVR��
class CTMNMsgCtlSvr : CMsgCtlBase
{
protected:
	static bool SVR_Submit_Func (CTMNMsgCtlSvr *_this, char *msg);
	static bool SVR_Terminal_Sub_Func (CTMNMsgCtlSvr *_this, char *msg);
	static bool SVR_Connect_Func (CTMNMsgCtlSvr *_this, char *msg);
	static bool SVR_Terminate_Func (CTMNMsgCtlSvr *_this, char *msg);
	static bool SVR_Err_Func (CTMNMsgCtlSvr *_this, char *msg);

public:
	virtual bool Initialize(TKU32 unInputMask, const char *szCtlName);
	bool doSVR_Submit_Func(char *msg);
	bool doSVR_Terminal_Sub_Func(char *msg);
	bool doSVR_Connect_Func (char *msg);
	bool doSVR_Terminate_Func (char *msg);
	bool doSVR_Err_Func (char *msg);
	bool ApmValid(char* Reserve, char* Deal, char* D_Status, char *szApmId, char *szPwd, BYTE *btAuth);
	bool Dispatch(CTMNMsgCtlSvr *_this, char *pMsg, char *pBuf, AppInfo *stAppInfo, BYTE Mod);
	void MD5(unsigned char *szBuf, unsigned char *src, unsigned int len);

//	bool addNode(MSG_ACTION_HASH hash_node);
//	bool GetHashData(MSG_ACTION_HASH& hash_node);
//	bool GetAndDeleteHashData(MSG_ACTION_HASH& hash_node);

	bool GetSocketId (char* channelId, TKU32& unClientId);
	TKU32 GetSeq()
	{
		if (0xffffffff == m_Seq++)
		{
			m_Seq = 0;
		}
		return m_Seq;
	}

//public:
	//�����ʹ�����lsd
	//bool InitApmHashData(char* sqlData);
	//static int InitApmHashCallback(void *data, int n_column, char **column_value, char **column_name);
	//int InitApmHashCallbackProc(int n_column, char **column_value, char **column_name);

/*private://hashtable ���
	typedef  tr1::unordered_map<TKU32, MSG_ACTION_HASH> HashedActInfo;
	HashedActInfo*  m_ptrActInfoTable;
	class Lock
	{
	private:
		pthread_mutex_t	 m_lock;
	public:
		Lock() { pthread_mutex_init(&m_lock, NULL); }
		bool lock() { return pthread_mutex_lock(&m_lock)==0; }
		bool unlock() { return pthread_mutex_unlock(&m_lock)==0; }
	}CTblLock;*/
private:
	static void *pActCtrlThrd(void* arg);
	void MsgActCtrl();
	//HashedInfo* m_pSocketHashTable;           //socket���ӿͻ��˹�ϣ��
	TKU32 ulHandleTcpSvr;
	TKU32 ulHandleActCtrl;
	int m_id;
	TKU32 m_Seq;
	int m_MaxApmCount;
};
//���ϼ�ƽ̨ͨѶ��TCP CLIENT��
class CMsgCtlClient : public CMsgCtlBase
{
protected:
	static bool NetFuncSubmit(CMsgCtlClient *_this, char *msg);
	static bool NetFuncDeliver(CMsgCtlClient *_this, char *msg);
	static bool NetErr(CMsgCtlClient *_this, char *msg);
    

public:
	~CMsgCtlClient(void);
	virtual bool Initialize(TKU32 unInputMask,const char *szCtlName);


private:
	TKU32 ulHandleHubNet;
};

#endif
