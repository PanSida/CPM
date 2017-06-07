#ifndef _CONN_MGR_
#define _CONN_MGR_

#include "Shared.h"
#include "ClsTcp.h"
#include "NetIfBase.h"


typedef enum{
	CLT_CONN = 0,
	SVR_CONN
}ConnType;

class ClsConnMgr
{
public:
	ClsConnMgr(void);
	
	virtual ~ClsConnMgr(void);
//	void SetID(int nId) {m_nId = nId;}
//	int GetID(void) {return m_nId;}
	//���ýӿ�ģ�������Ϣ
	virtual void SetModIfInfo(ModIfInfo& info) {m_info = info;}
	//��ýӿ�ģ�������Ϣ
	const ModIfInfo& GetModIfInfo(void) {return m_info;}
	//LinkDown�¼�������
	virtual void OnLinkDown(CNetIfBase* ifBase){}
	//������������
	virtual ConnType GetConnType(void) {return CLT_CONN;}
protected:
	ModIfInfo m_info;
//	int m_nId;
};

//�ͻ������ӹ�����
class ClsCltConnMgr: public ClsConnMgr
{
public:
	ClsCltConnMgr(void): ClsConnMgr() {};
	virtual ~ClsCltConnMgr(void);
	//���ýӿ�ģ�������Ϣ
	virtual void SetModIfInfo(ModIfInfo& info);
	//����m_info����һ�����ض����������ӣ�������ַ�Ͷ˿���m_infoָ����ini�ļ��л�ȡ
	bool CreateConn(void);
	//�ر�ID��ΪnSocket�����ӣ��ͷŶ�Ӧ��NetIfBase��Դ
	bool DropConn(int nSocket);
	//NetIfBase���ӶϿ��¼�������
	void OnLinkDown(CNetIfBase* ifBase) {}
	//������������
	virtual ConnType GetConnType(void) {return CLT_CONN;}
};

class ClsSvrConnMgr: public ClsConnMgr, ClsTCPSvrSocket
{
public:
	ClsSvrConnMgr(int nMaxLinks = MAX_LINK_NMB): ClsConnMgr(), ClsTCPSvrSocket(nMaxLinks) {};
	virtual ~ClsSvrConnMgr(void);
	char * GetSvrLocal(char * buf,int bufsize){ GetLocal(buf, bufsize);return buf; }
	bool ListenAt(ClsInetAddress& ia,  tpport_t port);
	//���ýӿ�ģ�������Ϣ
	virtual void SetModIfInfo(ModIfInfo& info, CNetIfBase* pNetIf=NULL);
	//NetIfBase���ӶϿ��¼�������
	void OnLinkDown(CNetIfBase* ifBase);
	//������������
	virtual ConnType GetConnType(void) {return SVR_CONN;}
	
protected:
	//Server socket���������¼�������
	CNetIfBase* m_pNetIf;
	virtual bool OnAccept(int nSocket);
};

#endif



