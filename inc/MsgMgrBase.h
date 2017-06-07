#ifndef _SYS_MSG_MGR_
#define _SYS_MSG_MGR_

#include <iostream>
#include <assert.h>
#include <stdio.h>
#include "TkMacros.h"
#include "Shared.h"
#include "Stack.h"
#include "QueueMgr.h"

extern FreeMemEvent g_QEMHandler;

//��Ϣ�����״̬����
typedef enum{
	MMR_OK = 0,
	MMR_VOLUME_LIMIT,
	MMR_TUNNEL_EXIST,
	MMR_TUNNEL_NOT_EXIST,
	MMR_GEN_ERR,
	MMR_PEER_EOQ,
	MMR_TUNNEL_MAX
}MMRType;
//��Ϣ�������
class CMsgMgrBase{
public:
	//constructor��ȷ����Ϣ���и���
	CMsgMgrBase();
	//destructor�����ٱ�����ʵ��
	virtual ~CMsgMgrBase(void);
	//��ʼ���������ڴ档ʹ��ʵ��ǰ����ɹ����ñ�����
	virtual bool Initialize(int nQueueCnt);
	//����/����һ����Ϣͨ����
	//unMask= Դģ���<<16 | Ŀ��ģ��š�ģ��Ŷ�����Share.h�е�SysModType
	//unQueueLenָ������Ϣͨ���ĳ���(�������ɵ���Ϣ����)
//	MMRType SetTunnel(TKU32 unMask, TKU32 unQueueLen);
	//������Ϣͨ����nTIDΪ���ص���Ϣͨ����
	MMRType AllocTunnel(TKU32 unMask, TKU32 unQueueLen, 
							size_t& nTID, FreeMemEvent hdl = g_QEMHandler);
	//�ͷ���Ϣͨ����nTIDΪ��Ϣͨ����
	MMRType FreeTunnel(int nTID);
	//��һ��ָ������Ϣͨ�������ͨ����unHandle��
	MMRType Attach(TKU32 unMask, TKU32& unHandle);
	//����һ����Ϣ��ָ������Ϣͨ��(������Ϣ���е�β��)��
	//qeMsgΪ��Ϣָ�롣
	//nTimeoutΪ�ȴ�ʱ�䣬ȱʡΪ��Զ�ȴ������ȴ���0��ʱ�䵥λ1/1000�롣
	MMRType SendMsg(TKU32 unHandle, QUEUEELEMENT qeMsg, int nTimeout = TW_INFINITE);
	//����һ����Ϣ��ָ������Ϣͨ��(������Ϣ���е��ײ�)��
	MMRType InsertMsg(TKU32 unHandle, QUEUEELEMENT qeMsg, int nTimeout = TW_INFINITE);
	//��ָ����Ϣͨ���л��һ���µ���Ϣ(ȡ��Ϣ�����ײ�����Ϣ)
	MMRType GetFirstMsg(TKU32 unHandle, QUEUEELEMENT &qeMsg, int nTimeout = TW_INFINITE);
	//��ָ����Ϣͨ���л��һ���µ���Ϣ(ȡ��Ϣ����β������Ϣ)
	MMRType GetLastMsg(TKU32 unHandle, QUEUEELEMENT& qeMsg, int nTimeout = TW_INFINITE);
	//ʹ��Ϣͨ���ı����α�ص��ײ���׼����ʼ����
	MMRType GotoHead(TKU32 unHandle);
	//�鿴��Ϣͨ������һ����Ϣ��GotoHead���״ε���
	//���õ���һ����Ϣ���̲߳���ȫ����������á�
	//����ʱ��ʹ��try...catch������
	MMRType PeerNext(TKU32 unHandle, QUEUEELEMENT& qeMsg);
	
	MMRType ConsumerAllowed(TKU32 unHandle, bool isallow);

private:
	int m_nMaxQueueCnt;					//�����Ϣ���и���
	TKU32* m_pIdxArray;					//�����б�
	CPtrQueueMT* m_pQueueArray;			//��Ϣ�����б�
	bool *m_StateArray;					//״̬����
	bool m_Initialized;					//ģ���Ƿ��ʼ��
	CFIStack* m_pQIDStack;				//ͨ���Ŷ�ջ
	pthread_mutex_t m_mtxTnlLock;		//ͨ�������ٽ���
};

extern CMsgMgrBase  g_MsgMgr;

#endif
