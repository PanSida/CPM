#ifndef _UDP_HDR_
#define _UDP_HDR_

#include "ClsSocket.h"
#include "TkMacros.h"   // define macro: TRACE

class ClsUDPSocket :public ClsSocket 
{
    struct sockaddr_in 	m_peer;
    struct sockaddr_in 	m_local;

public:
	ClsUDPSocket(void);

    ~ClsUDPSocket()
    {
		TRACE(~ClsUDPSocket);
	}

	// create a UDP socket which binds to a specific interface
	ClsUDPSocket(const ClsInetAddress &ia, tpport_t port);

	// SetServerSocket()
	// set the socket server address,
	// return 	false			server address already set or socket not created.
	//			true			it is set successfully
	//���ñ��ؼ�����ַ�Ͷ˿�
	bool SetServerSocket(const ClsInetAddress &ia, tpport_t port);

	//
	// check whether the UDP socket server is bounded
	//
	inline bool IsSocketBound(void) const { return SOCKET_BOUND == m_state;}

	// send 
	inline int Send(char *buf, size_t len, struct sockaddr *peer, TKU32 addrlen)
	{ 
		return sendto(m_socket, (const char *)buf, len, 0, peer, addrlen);
	}

	// send 
	inline int Send(char *buf, size_t len)
	{ 
		return sendto(m_socket, (const char *)buf, len, 0, (struct sockaddr*)&m_peer, 
						(TKU32) sizeof(m_peer));
	}
	//���öԷ�������ַ�Ͷ˿�
    void SetPeer(const ClsInetAddress &ia, tpport_t port);

	// NAME: SetPeer()
	// SUMMARY:
	//     It is used to set the peer address with ip address and port
	//
	//���öԷ�������ַ�Ͷ˿�
    void SetPeer(TKU32 ipAddr, tpport_t port);

	//���ĳ���˿��ϵĶԷ�������ַ
    ClsInetAddress GetPeer(tpport_t *port) const;

	int RecvFrom(char *buf, int len)
	{ 
    	int peerLen = sizeof(m_peer);

		return recvfrom(m_socket, buf, len, 0, 
				(struct sockaddr *)&m_peer, (socklen_t *)&peerLen);
	}

	int RecvFrom(char *buf, int len, struct sockaddr *srcaddr, int *addrlen)
	{ 
		return recvfrom(m_socket, buf, len, 0, srcaddr, (socklen_t *) addrlen);
	}

	//��ÿͻ��˶˿���Ϣ�����ڴ�ӡ��ʾ
	char * GetPeer(char *buf, int sz);
	//��ñ��ؼ����˿���Ϣ�����ڴ�ӡ��ʾ
	char * GetLocal(char *buf, int sz);

	//���ؿͻ��˶˿ڶ�Ӧ�ĶԷ���ַ�Ͷ˿�
	inline void GetPeer(TKU32 *ip, u_short *port)
	{
		*ip = ntohl(m_peer.sin_addr.s_addr);
		*port = ntohs(m_peer.sin_port);
	}

	//���ؿͻ��˶˿ڶ�Ӧ�ĶԷ���ַ�Ͷ˿�
	void GetPeer(sockaddr_in *sock) { memcpy(sock, &m_peer, sizeof(sockaddr_in)); }
	//���ؿͻ��˶˿ڶ�Ӧ�ĶԷ���ַ�Ͷ˿�
	const sockaddr_in *GetPeer() { return (const sockaddr_in *)&m_peer; }
	//���ر��ؼ����˿ڵ�ַ�Ͷ˿�
	void GetLocal(sockaddr_in *sock) { memcpy(sock, &m_local, sizeof(sockaddr_in)); }
	//���ر��ؼ����˿ڵ�ַ�Ͷ˿�
	const sockaddr_in *GetLocal() { return (const sockaddr_in *)&m_local; }
};

#endif
