#ifndef _NETDATACONTAINER_H
#define _NETDATACONTAINER_H


#include <tr1/unordered_map>
#include "Shared.h"
#include "Init.h"
using namespace std;
typedef struct
{
    char DeviceId[DEVICE_ID_LEN_TEMP];			//�豸���
	int seq;
	char ctime[21];		//���ݲɼ�ʱ��
}MSG_NETDATA_HASH, *PMSG_NETDATA_HASH;

typedef struct _ACTION_MSG_WAIT_TERMINAL_RESP_HASH
{
	char Id[21];			//
	time_t	act_time;
	ACTION_MSG   stActionMsg;
}ACTION_MSG_WAIT_TERMINAL_RESP_HASH, *PACTION_MSG_WAIT_TERMINAL_RESP_HASH;


typedef  tr1::unordered_map<string, MSG_NETDATA_HASH> HashedNetDataInfo;

/****************************����ִ�л�Ӧ����*********************************/
typedef  tr1::unordered_map<string, ACTION_MSG_WAIT_TERMINAL_RESP_HASH> HashedDeviceInfoACTTERMINALRESP;

class CNetDataContainer
{
public:
    CNetDataContainer(void);
    virtual ~CNetDataContainer(void);

    bool Initialize(void);
	bool updateNetData(char *inMsg, int msgLen, char* outBuf, int& outLen);
	bool SendRespMsg(char* dstId, int seq, int status, unsigned char* sendBuf, int sendlen, TKU32 soketId);

    bool getNetData(char *DevId, PQUERY_INFO_CPM pResp);        //ȡ������ֵ

public:
	bool InitDeviceInfo();
	static int InitDeviceIndexCallBack(void* data, PDEVICE_INFO_POLL pNode);
	int InitDeviceIndexCallBackProc(PDEVICE_INFO_POLL pNode);   //��ѯ���ʼ��


private:
    class Lock
	{
	private:
		pthread_mutex_t	 m_lock;
	public:
		Lock() { pthread_mutex_init(&m_lock, NULL); }
		bool lock() { return pthread_mutex_lock(&m_lock)==0; }
		bool unlock() { return pthread_mutex_unlock(&m_lock)==0; }
	}CTblLock;
    HashedNetDataInfo* m_pNetDataHashTable;           //�豸������ϣ��

private:
	class LockActionResp
	{
	private:
		pthread_mutex_t	 m_lock;
	public:
		LockActionResp() { pthread_mutex_init(&m_lock, NULL); }
		bool lock() { return pthread_mutex_lock(&m_lock)==0; }
		bool unlock() { return pthread_mutex_unlock(&m_lock)==0; }
	}CLockActionResp;
	HashedDeviceInfoACTTERMINALRESP* m_HashTableActResp;      //������Ӧ��ϣ��
public:
	int DoAction(ACTION_MSG actionMsg);
	bool actionRespProc(char *inMsg, int msgLen);
	bool ActionRespTableInsert(ACTION_MSG& stMsgAction, char* actSeq);
private:
	int PollProc(PDEVICE_INFO_POLL device_node, ACTION_MSG& actionMsg);
	int ActionProc(PDEVICE_INFO_ACT device_node, ACTION_MSG& actionMsg);
	static void *ActCtrlThrd(void* arg);
	void ActCtrlThrdProc();
	bool GetAndDeleteHashData(ACTION_MSG_WAIT_TERMINAL_RESP_HASH& hash_node);
	bool ActionRespTableGetAndDelete(ACTION_MSG_WAIT_TERMINAL_RESP_HASH& hash_node);
	
	TKU32 m_ActRespKey;
	TKU32 GetActionRespTableKey(char* outBuf)
	{
		if (m_ActRespKey < 0xffffffff)
			m_ActRespKey++;
		else
			m_ActRespKey = 0;
		sprintf(outBuf, "%020d", m_ActRespKey);						//Reserve
		return m_ActRespKey;
	}

	bool CheckTime(struct tm *st_tmNowTime, DEVICE_INFO_POLL device_node);

public:
	bool addNode(char* deviceId);
	bool deleteNode(char* deviceId);
	int  deleteNodeByDevice(char* deviceId);
	bool m_IsInitialized;

protected:
	char m_szCurDeviceAttrId[DEVICE_ATTR_ID_LEN_TEMP];
};

#endif
