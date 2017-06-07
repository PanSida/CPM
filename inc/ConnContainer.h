#ifndef _CONN_CONTAINER_
#define _CONN_CONTAINER_
#include <utility>
#include <tr1/unordered_map>

//#include <tr1/unordered_set>
#include "ConnMgr.h"
using namespace std;
class ConnContainer{
public:
	ConnContainer();
	~ConnContainer();
	bool Initialize(int nConnCnt); //nConnCnt��2��������ʼ����ϣ��
	//�ж��Ƿ��Ѿ���ʼ��
	bool IsInitialized(void) {return m_bIsInitialized;}
	//����һ�����ӹ�������NetIfBase�Ĺ���
	bool AddConn(ClsConnMgr* mgr, CNetIfBase* ifBase);
	
	//ɾ��һ�����ӣ�nIfIdΪNetIfBase�ڲ�����Ľӿں�
	//�÷��������ͷŽӿں�ΪnIfId��NetIfBaseʵ��
	//bDelNow�����Ƿ������ͷ�
	bool RemoveConn(int nIfId, bool bDelIfNow = true);

	bool RemoveConn(CNetIfBase* ifBase, bool bDelIfNow = true)
	{
		return (ifBase) ? RemoveConn(ifBase->GetIfId(), bDelIfNow) : false;
	}
	
	//���ͷ�NetIfBase��ɨ���̣߳�
	//����RemoveConnʱ����bDelNowΪfalse��NetIfBaseʵ���ڱ��߳��б��ͷ�
	static void* pThrdScanner(void *);
	
	//����NetIfBase��LinkDown�¼���Callback����
	static void OnLinkDown(int nIfId);

private:
	//CFIStack* m_IdStack;
	CFIStackMT* m_IdStack;

	//���ӳع�ϣ��ڵ�ṹ
	typedef struct {
		int nKey;			//nKey = nIfId��ʵ����SocketId
		ClsConnMgr* connMgr;
		CNetIfBase* ifBase;
	}ConnInfo;
	
	
	struct HASHER
	{
		size_t operator()(const int &s) const
		{
			return s;
		}
	};

	struct EQUALFUNC
	{
		bool operator()(const int &a, const int &b) const
		{
			return a==b;
		}
	};

	typedef tr1::unordered_map<int, ConnInfo, HASHER, EQUALFUNC> _CONN_HASH;

	_CONN_HASH* m_pConnHash;

	pthread_mutex_t m_mtxHashLock;

	pthread_t m_thread;
	bool m_bIsInitialized;
};

extern ConnContainer g_ConnContainer;

#endif

