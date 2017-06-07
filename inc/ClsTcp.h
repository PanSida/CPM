#ifndef _TCP_HDR_
#define _TCP_HDR_

#include "ThreadPool.h"
#include "TkMacros.h"   // define macro: TRACE
#include "ClsSocket.h"

#define DEF_LINK_NMB	128
#define MAX_LINK_NMB	256

//TCP socket������
class ClsTCPSocket :public ClsSocket {
public:
	ClsTCPSocket(int nSocket = -1);
    ~ClsTCPSocket()
    {
		TRACE(ClsTCPSocket::~ClsTCPSocket);
	}

	//��ñ��ؼ����˿���Ϣ�����ڴ�ӡ��ʾ
	char * GetLocal(char *buf, int sz);

	//���ر��ؼ����˿ڵ�ַ�Ͷ˿�
	void GetLocal(sockaddr_in *sock) { memcpy(sock, &m_local, sizeof(sockaddr_in)); }
	//���ر��ؼ����˿ڵ�ַ�Ͷ˿�
	const sockaddr_in *GetLocal() { return (const sockaddr_in *)&m_local; }
	//����socket������ʱ
	inline void SetTimeout(int nMSec = DEF_TIMEOUT)
	{
		m_nTimeout = nMSec;
	}

protected:
    struct sockaddr_in 	m_local;
	TKU32 m_nTimeout;
	enum{
		DEF_TIMEOUT = 500
	};
};

//TCP�ͻ���socket
class ClsTCPClientSocket: public ClsTCPSocket
{
public:
	ClsTCPClientSocket();
	//ʹ��һ���Ѿ����ӵ�socket��ʼ��
	ClsTCPClientSocket(int nClientId);
	//���ӵ�������ia��port�ϵ��������
	//bReconnect���Ͽ��������ӣ�����������
	int Connect(const ClsInetAddress &ia, tpport_t port, bool bReconnect = false);
	//�ж��Ƿ��Ѿ�������
	inline bool IsConnected(void) {return (SOCKET_CONNECTED == m_state);}
	// send 
	int Send(void *buf, size_t len);

	int Recv(char *buf, int len);

public:	
	//���öԷ�������ַ�Ͷ˿�
    void SetPeer(const ClsInetAddress &ia, tpport_t port);

	// NAME: SetPeer()
	// SUMMARY:
	//     It is used to set the peer address with ip address and port
	//
	//���öԷ�������ַ�Ͷ˿�
    void SetPeer(TKU32 ipAddr, tpport_t port);

	//��ÿͻ��˶˿���Ϣ�����ڴ�ӡ��ʾ
	char * GetPeer(char *buf, int sz);
	//���ؿͻ��˶˿ڶ�Ӧ�ĶԷ���ַ�Ͷ˿�
	inline void GetPeer(TKU32 *ip, u_short *port)
	{
		*ip = ntohl(m_peer.sin_addr.s_addr);
		*port = ntohs(m_peer.sin_port);
	}
	//���ؿͻ��˶˿ڶ�Ӧ�ĶԷ���ַ�Ͷ˿�
	const sockaddr_in *GetPeer() { return (const sockaddr_in *)&m_peer; }
	//���ؿͻ��˶˿ڶ�Ӧ�ĶԷ���ַ�Ͷ˿�
	void GetPeer(sockaddr_in *sock) { memcpy(sock, &m_peer, sizeof(sockaddr_in)); }
private:
    struct sockaddr_in 	m_peer;
    bool m_bIsConnected;
	fd_set m_stReadFDs;
	fd_set m_stWriteFDs;
};



//Tcp Server Socket
typedef enum {
	AM_CALLBACK,	//Acceptȱʡ����Callbackģʽ������OnAccept����ָ��
	AM_INHERIT,		//ClsTcpSvrSocket�ļ̳��ཨ����ü̳�ģʽ
	AM_MAX
}ConnAcceptMode;

//Accept�¼��ص�����ָ������
typedef void (* AcceptEvent)(int nSocket);

//TCP���񻧶˼�����socket
class ClsTCPSvrSocket :public ClsTCPSocket
{
public:
	//���캯����nMaxLinks��ʾ���ɽ���������, 
	//MAX_LINK_NMB�ֶ���Ϊ256��DEF_LINK_NMBΪ128
	ClsTCPSvrSocket(int nMaxLinks = DEF_LINK_NMB);
	// create a TCP Server socket which binds to a specific interface
	ClsTCPSvrSocket(const ClsInetAddress &ia, tpport_t port, int nMaxLinks = DEF_LINK_NMB);

   virtual ~ClsTCPSvrSocket()
    {
		TRACE(~ClsTCPSvrSocket);
		Final();
	}
	// SetServerSocket()
	// set the socket server address,
	// return 	false			server address already set or socket not created.
	//			true			it is set successfully
	//���ñ��ؼ�����ַ�Ͷ˿�
	bool SetServerSocket(const ClsInetAddress &ia, tpport_t port);

	//����Accept�¼�����ʱ���õ��¼���Ӧģʽ
	ConnAcceptMode GetAcceptMode(void) {return m_enmMode;}
	/********************************************************************/
	/*����Accept�¼�����ʱ���õ��¼���Ӧģʽ							*/
	/*AM_CALLBACKģʽ������CallbackOnAccept�ص�����ָ��					*/
	/*AM_INHERITģʽ������OnAccept�麯����								*/
	/*�̳���Ӧ��д�÷����Ա����¼��Ĵ���Ȩ							*/
	/********************************************************************/
	void SetAcceptMode(ConnAcceptMode &mode) { m_enmMode = mode;}

	//���AM_CALLBACKģʽ��CallbackOnAccept�ص�����ָ��
	AcceptEvent GetAcceptHandler(void) {return 	CallbackOnAccept;}
	//����AM_CALLBACKģʽ��CallbackOnAccept�ص�����ָ��
	//�������ʹ�õ�CallbackOnAccept�ص�����ָ��
	AcceptEvent SetAcceptHandler(AcceptEvent newHandler) 
	{
		AcceptEvent oldHandler = CallbackOnAccept;
		CallbackOnAccept = newHandler;
		return 	oldHandler;
	}

	//�ж�Server socket��ǰ�Ƿ���ܿͻ�����������
	bool GetAcceptAllowed(void) {return m_AcceptAllowed;}
	//����Sever socket��ǰ�Ƿ���ܿͻ�����������
	void SetAcceptAllowed(bool acceptAllowed) {m_AcceptAllowed = acceptAllowed;}

	//��ָ����ַ�Ͷ˿��ϼ���������0��ʾ�ɹ�
	int Listen(void);
	//����Server socket���Ӵ����̣߳�������modeָ����ģʽ����Accept�¼�
	bool Run(ConnAcceptMode mode = AM_CALLBACK);
	//��������������һ����Server socket���������ӱ��ر�ʱӦ���ø÷���
	void ReduceConn(void) {m_nUsedLinkNmb--;}
	//��ʽ�ͷ���Դ�������߳�
	void Final(void);
	//Server socket���Ӵ����߳�
	static void* AcceptThrd(void*);
	
protected:
	virtual void init(int nMaxLinks);
	virtual int Accept(void);
	virtual bool OnAccept(int nSocket) {TRACE(ClsTCPSvrSocket::OnAccept); return false;}
private:
	int m_nMaxLinkNmb;
	int m_nUsedLinkNmb;
	int m_nIOCtrlFlag;
	fd_set m_stReadFDs;
	int m_lastLinkPos;
	bool m_AcceptAllowed;
	ConnAcceptMode m_enmMode;
	AcceptEvent CallbackOnAccept;
	bool m_bIsInitialized;
	pthread_t m_thread;
};


#endif
