#ifndef _SOCKET_HDR_
#define _SOCKET_HDR_

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <poll.h> 
#include <errno.h>

#include "TkMacros.h"   // define macro: TRACE
using namespace std;
typedef int tpport_t;

enum SocketState
{
    SOCKET_INITIAL,
    SOCKET_AVAILABLE,
    SOCKET_BOUND,
    SOCKET_CONNECTED,
    SOCKET_RECONNECTED,
    SOCKET_CONNECTING
};

//IP��ַ���ͣ�����ͳһIP V4��ַ�ı��
class ClsInetAddress 
{
    struct in_addr    m_ipaddr;
	bool			  m_valid;
public:
	ClsInetAddress(void)
	{
		m_valid = false;	
        m_ipaddr.s_addr = 0;
	}
	//����UINT��ʼ����ַ�࣬��λ�ǵ�ַ�Ŀ�ʼ����
	ClsInetAddress(TKU32 ipAddr) { Set(ipAddr); }
	//����in_addr���ͳ�ʼ����ַ��
	ClsInetAddress(struct in_addr addr) { Set(addr); }
	//���õ�ַ�ַ�����ʼ����ַ��
	ClsInetAddress(const char *address) { Set(address); }
	//����UINT���õ�ַ����λ�ǵ�ַ�Ŀ�ʼ����
	inline void Set(TKU32 ipAddr)
	{
		m_valid = true;	
        m_ipaddr.s_addr = htonl(ipAddr);
	}
	//����in_addr�������õ�ַ
	inline void Set(struct in_addr addr)
	{
		m_valid = true;
		m_ipaddr = addr;
	}
	//���õ�ַ�ַ������õ�ַ
	bool Set(const char *address);
	//�жϵ�ַ�Ƿ���Ч
	inline bool operator!() const{return !m_valid;}
	
	//���in_addr���͵�IP��ַ
	struct in_addr GetAddress(void) const {return m_ipaddr;}
	//���UINT���͵�IP��ַ
	inline TKU32 GetIpAddress(void){ return ntohl(m_ipaddr.s_addr);}
	//��ӡ��ַ��Ϣ��buf����Ϣ��ŵ��ڴ棬lenָbuf���ֽ���
	void SPrint(char *buf, int len);
};


//Socket������
class ClsSocket 
{
protected:
	SocketState     m_state;
	int 			m_socket;
public:
	/********************************************************************/
	/*���캯����domainһ��ΪAF_INET										*/
	/*typeһ��ΪSOCK_DGRAM��SOCK_STREAM���ֱ��ӦUDP��TCP				*/
	/*protocolһ����0������ο�ϵͳAPI: socket							*/
	/*nSocketһ��Ϊ-1���������ʾʹ��һ�����е����ӳ�ʼ��Socket			*/
	/********************************************************************/
	ClsSocket(int domain, int type, int protocol = 0, int nSocket = -1);
    ~ClsSocket()
    {
		TRACE(~ClsSocket);
		EndSocket();
	}
	
	/********************************************************************/
	/*�ж�Socket�������Ƿ��������ݻ�����								*/
	/*timeoutָ�ȴ�ʱ�䣬��λ���룬-1��ʾ�ȴ�ֱ�����µ����ݻ�����		*/
	/*retOnInt��ʾ�Ƿ���ϵͳ�жϵ���ʱ���ص���							*/
	/********************************************************************/
	bool IsPending(int timeout = -1, bool retOnInt = false);
	//�ж�socket�Ƿ��Ѿ���
	inline bool IsSocketBound(void) const { return SOCKET_BOUND == m_state; }
	//������һ��socket����
	inline int GetLastSockErr(void) const { return errno; }
	//�ر�socket����
	void EndSocket(void);
	//����socket ID
	int GetSockId(void) {return m_socket;}
	//�ж�socket ID�Ƿ���Ч��δ��ʼ��
    inline bool operator!() const {return m_socket < 0;};
    //�趨socket����/������ģʽ��ȱʡΪ����ģʽ
	inline int SetCompletion(bool block = true)
	{
		int fflags = fcntl(m_socket, F_GETFL);
		switch( block )
		{
			case false:
				fflags |= O_NONBLOCK;
				break;
			case true:
				fflags &=~ O_NONBLOCK;
				break;
		}
		return fcntl(m_socket, F_SETFL, fflags);
	}
	//����Э��ջ���ջ����С
	inline int SetRcvBuf(int size) 
		{ return setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (const char*)&size, sizeof(size));}
	//����Э��ջ���ͻ����С
	inline int SetSndBuf(int size)
		{ return setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (const char*)&size, sizeof(size));}
	//���Э��ջ���ջ����С
	inline int GetRcvBuf(int *size) 
		{ int len = sizeof(*size); return getsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char *)size, (socklen_t *) len);}
	//���Э��ջ���ջ����С
	inline int GetSndBuf(int *size)
		{ int len = sizeof(*size); return getsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char *)size, (socklen_t *) len);}
	//����socket�Ƿ����ã�Server���������һ��Ӧ����Ϊtrue
	inline int SetSockOptReuse(bool reuse)
	{ 
		int optval = reuse?1:0;
    	return setsockopt( m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
	}
};
#endif
