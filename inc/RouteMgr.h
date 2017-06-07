#ifndef _ROUTE_MGR_
#define _ROUTE_MGR_

#include <string>

#include <tr1/unordered_map>
#include "ClsSocket.h"
using namespace std;
#define DEF_ROUTE_NMB	512

class CRouteMgr
{
public:
	CRouteMgr();
	~CRouteMgr();
	//ʹ��ǰ����ɹ����õķ�����
	//nmb��ʾ·�ɵ�����
	bool Initialize(const int nmb = DEF_ROUTE_NMB);
	//�ж��Ƿ�ɹ���ʼ��
	bool IsInitialized(void) {return m_bInitialized;}
	//�����Ϣͨ���ŵ�Ip��ַ��·��ӳ��
	//TID��ʾͨ����
	//ia��port��ʾ��ַ�Ͷ˿�
	bool AddRouteI2A(const int TID, const ClsInetAddress &ia, short int port);
	//����ͬ�ϣ�ia�ĸ�λΪ��ַ����ʼ��
	bool AddRouteI2A(const int TID, const TKU32 ia, short int port);
	//ɾ����Ϣͨ���ŵ�Ip��ַ��·��ӳ��
	bool DelRouteI2A(const int TID);
	//���Ip��ַ����Ϣͨ���ŵ�·��ӳ��
	//TID��ʾͨ����
	//ia��port��ʾ��ַ�Ͷ˿�
	bool AddRouteA2I(const ClsInetAddress &ia, short int port, const int TID);
	//����ͬ�ϣ�ia�ĸ�λΪ��ַ����ʼ��
	bool AddRouteA2I(const TKU32 ia, short int port, const int TID);
	//ɾ��Ip��ַ����Ϣͨ���ŵ�·��ӳ��
	bool DelRouteA2I(const ClsInetAddress &ia, short int port);
	//����ͬ�ϣ�ia�ĸ�λΪ��ַ����ʼ��
	bool DelRouteA2I(const TKU32 ia, short int port);
	//����ͨ���Ŷ�Ӧ��ip��ַ
	// if address returned with struct memory zeroed, means lookup failed.
	sockaddr_in GetAddrByTID(const int TID);
	//����Ip��ַ��Ӧ��ͨ����
	// return -1, means lookup failed.
	int GetTIDByAddr(const ClsInetAddress &ia, short int port);
	//����ͬ�ϣ�ia�ĸ�λΪ��ַ����ʼ��
	int GetTIDByAddr(const TKU32 ia, short int port);

private:	
	typedef struct {
		int nTID;			//nKey = nIfId��ʵ����SocketId
		unsigned long addr;
		tpport_t port;
	}RouteNode;
	
// Hash TID to Address	
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

	typedef tr1::unordered_map<int, RouteNode, HASHER, EQUALFUNC> _I2A_HASH;
	typedef tr1::unordered_map<int, RouteNode, HASHER, EQUALFUNC> _A2I_HASH;

	_I2A_HASH* m_phtI2A;	//TID to Address
	_A2I_HASH* m_phtA2I;	//Address to TID

	bool m_bInitialized;

	pthread_mutex_t m_mtxI2ALock;
	pthread_mutex_t m_mtxA2ILock;

};

extern CRouteMgr g_RouteMgr;


#endif


