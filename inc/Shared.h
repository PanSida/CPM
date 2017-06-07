#ifndef __SHARED_H__
#define __SHARED_H__
//Linux ר��ͷ�ļ�������ƽ̨ʹ�ø�����Ҫ�޸�
#include <string>
#include <stdio.h>
#include <pthread.h>
#include <assert.h> 
#include <fstream>
#include <time.h>
#include <sys/time.h>
//#include <sys/timeb.h>
#include "TkMacros.h"
#include "Md5.h"
using namespace std;

typedef unsigned long DWORD;
typedef unsigned char BYTE;


//��������
#define DATALOG_FILE ""
#define DST_ADDR_LEN 30
#define SMS_LEN 160
#define SYS_QUEUE_CNT	        (1024*2) 	//ϵͳ�ж�����
#define COMBINE_MOD(a, b)	((a << 16)|b) 	//ģ���Ψһ��

#define ICLEN					16

#define PRT_INTERVAL 1	//��ӡ������ϢƵ��

#pragma pack(1)

//ͨ��Э������
typedef enum{
	NP_UDP = 0,
	NP_TCP,
	NP_TCPSVR,
	NP_MAX
}NPType;

//�ӿ�ģ��������Ϣ�������
typedef enum{
	CODEC_CMD = 0,  //�յ�������
	CODEC_RESP,
	CODEC_ERR,
	CODEC_NEED_DATA,
	CODEC_MAX,
	CODEC_TRANS,
	CODEC_CONN
}CodecType;

typedef enum{
	FUNC_USSD_BEGIN = 0x20,
	FUNC_USSD_END = 0x21,
	FUNC_USSD_CONTINUE = 0x22,
	FUNC_USSD_ERROR = 0x23,
	FUNC_USSD_ABORT =0x24,
	
	FUNC_MT_READ = 0x50,  //MT��־����
	FUNC_APMINFO = 0x51,  //ȡӦ�ô���ģ����Ϣ
	FUNC_MO_INSERT = 0x53,  //MO��־����
	FUNC_MT_INSERT = 0x54,  //MT��־����
	FUNC_MT_UPDATE_ID = 0x55,  //�޸�MT��־Ψһ��
	FUNC_MT_UPDATE_RESP = 0x56,  //�޸���־�Ļ�Ӧ
	
	FUNC_HEXINFO = 0xE0,//��ʾ
	FUNC_STRINFO = 0xE1,
	FUNC_OMSENABLED = 0xE2,
	FUNC_OMSDISABLED = 0xE3,
	FUNC_CLIENTINFO = 0xE4,
	FUNC_SMS = 0xE5,
	FUNC_IF_TEST = 0xE6,
	FUNC_DB_BACKUP = 0xE7,
	FUNC_RESTART = 0xE8,
	
	// 0xF0~0xFF Ϊϵͳ����
	FUNC_ERR = 0xF1,		// ����
	
}FuncCode;

//ͨ�ų�������
//ͨ�ų�������
typedef enum{
	COMM_CONNECT 	= 1,	//����
	COMM_TERMINATE 	= 2,	//��ֹ
	COMM_ACTIVE_TEST = 3,	//����
	COMM_SUBMIT 	= 4,	//�ύ
	COMM_DELIVER 	= 5,	//ת��
	COMM_ERR		= 9,		//���󱨸�
	COMM_RSP = 0x80000000, 	//��Ӧ���
	COMM_MASK = 0x7FFFFFFF,	//��������
	COMM_MAXSEQ = 0x0FFFFFFFF, //���ͨ�����к�
	COMM_ACTIVE_TEST_START = 3, //���԰����Կ�ʼ����
	COMM_ACTIVE_TEST_END = 6, //���԰����Կ�ʼ����
	COMM_RECV_MAXBUF = 1024*6,	//���ջ���������
	COMM_SEND_MAXNUM = 3,		//��Ϣ���͵�����ظ�����
	COMM_RECV_MAXNUM = 6,
	COMM_PENDING_TIME = 200,
	COMM_TCPSRV_MAXClIENT = 256//TCP�����������ͻ�������
}CommCode; 

//ϵͳ(����)״̬
typedef enum{
	STA_SUCCESS = 0,		//�ɹ�
	STA_TIMEOUT=1,			//��ʱ
	STA_BUSY=2,				//ϵͳæ
	STA_IF_DOWN=3,			//�ӿڲ�ͨ
	STA_MEM_ALLOC_FAILED=4,	//�ڴ����ʧ��
	STA_SEND_FAILED=5,		//���ݽ���ʧ��
	STA_SYSTEM_ERR=6,	    //ϵͳ����
	STA_QOSMAX = 7,         // ��ֵ����				
	STA_USRPWDERR=8,        //�û����벻��ȷ
	STA_APM_NONEXIST=9,		//APM������
	STA_MSGINVALID=11,      // ��Ϣ��������Ч(��ʽ����ȷ)
	STA_UNKNOWN=0xFF		//δ֪����
}StaCode;

typedef enum{
	ERR_CRITICAL = 0,	//����
	ERR_MAJOR = 1,		// ��Ҫ
	ERR_MINOR =2 ,  
	ERR_GENERAL =3,		//һ��
	ERR_PROMPT = 4		//��ʾ
}ErrClass;

typedef enum{
	ERR_ENV = 0,	
	ERR_DEV = 1,	
	ERR_COM =2 ,  
	ERR_SOFT =3,	
	ERR_QOS = 4,	
	ERR_INFO = 5	
}ErrType;

typedef struct{
        char szAppId[21];
}AppInfo;//134

//�澯��Ϣ�ṹ
typedef struct{
	BYTE btClass;
	BYTE btErrNo;
	char szModName[30];
	char szErrInfo[255];
}Alarm;

typedef struct{
	BYTE btDCS;		//2004-05-08 15:16 Yan Add
	BYTE btResp_Id[64]; 
	BYTE btStatus;	
}RespMsg;


//��Ϣͷ����
typedef struct MsgHdr{
	TKU32 unMsgLen;		//����
	TKU32 unMsgCode;	//��Ϣ��
	TKU32 unStatus;		//��Ϣִ��״̬
	TKU32 unMsgSeq;		//��Ϣ���к�	
	TKU32 unRspTnlId; 	//������Ϣʹ�õ�ͨ����
}MsgHdr;

//��Ϣ��
typedef struct Msg{	
	char szApmId[20];
	union {
		Alarm stAlarm;
	};
}Msg;

const unsigned int MSGSTART = 22;
//ϵͳ��ģ���Ŷ���
const unsigned int MSGHDRLEN = sizeof(MsgHdr);
//const unsigned int APPINFOLEN = sizeof(AppInfo);
const unsigned int MSGPRELEN = 20;
const unsigned int MSGLEN = sizeof(Msg);

#define MAXMSGSIZE 2*1024   //(MSGHDRLEN + MSGLEN) //�ڴ���Ϣ���С
#define MAXMSGSIZE_L 512		//С�ڴ���Ϣ���С

#define MEM_MSG_CNT 512 + 50 //�ڴ���Ϣ����

#define QUEUE_ELEMENT_CNT 512  //������Ԫ�ظ���

//��Ϣ���ж���
typedef struct  {
	TKU32 unDirect;	//��Ϣ���б�ʶ
	TKU32 unQLen;	//��Ϣ���г���
}MsgQCfg;

typedef bool (*OnLogin)(void *pSock);
typedef int (*OnEncode)(void* pMsg, void* pBuf);
typedef CodecType (*OnDecode)(void* pMsg, void* pBuf, void* pRespBuf, int &RespLen, int& nUsed);

typedef struct{
	char szIniFileName[128];	//�����ļ�
	char szModName[30];			//ģ����
    BYTE pid;               //ͨ����
  	BYTE self;				//ģ���ʶ
 	BYTE ctrl;				//����ģ��
	BYTE remote;			//Զ��ģ��
	TKU32 unMemHdl;				//�����߳������ڴ�����ľ��
	TKU32 unMemHdlSend;			//�����߳������ڴ�����ľ��
 	NPType npType;				//����ͨ��Э��  	
	bool bTcpIsSvrConn; 		//����ǿͻ���sock��Զ�˵�ַ�Ͷ˿ڶ������ļ����
	int nTcpSocket;				//socketֵ
	bool bTcpIsShareMode; 		//����ǹ���ģʽ���µ���Ϣͨ����NetIfBase����
	int nTcpDynTnlSize;			//����ģʽ�½��Ķ�̬ͨ������
	OnLogin CallbackLogin;		//Login�ص�����ָ�룬Ϊ�ձ�ʾ����ҪLogin
	OnEncode CallbackOnEncode;	//����ص�����ָ�룬Ϊ�ձ�ʾ����Ҫ����
	OnDecode CallbackOnDecode;	//����ص�����ָ�룬Ϊ�ձ�ʾ����Ҫ����
	bool bIsZeroSeq;				//���԰����ͷ�ʽ��"��"Ϊ�յ�����ʱ,���Է��ͼ�����0
	bool bIsTest;				//�Ƿ����������ԣ�"��"����
}ModIfInfo;

//����������
int HexToInt(string str);//16�����ִ�ת����
string IntToHex(int datalen);//����ת16�����ִ�
DWORD TurnDWord(const DWORD value);//4�ֽڸߵ�λ����
void AddMsg(unsigned char *str,int num);//��ʾ��Ϣ����
void AddMsg_NoSep(unsigned char *str,int num);
bool ErrMsg(TKU32 ulHandle, unsigned long ulMCBHandle, char *msg);//����Ϣ����Ϣ����
int writelog(char *path,string log);//д��־�ļ�
int UnicodeToGB(BYTE* in, char* out);
bool GetApmId(char *dest,const char *src);
//void MD5(unsigned char *szBuf, unsigned char *src, unsigned int len);
bool GetRespId(BYTE *dst, BYTE *src,int pLen);
//��־����
bool DataLog(BYTE pSrcType, char * pTitle, char *pSrc, int pLen);
int WriteDataLog(string pLog);
string MemToStr(char *pBuf, int pLen);
bool DataLogSock(BYTE pSrcType, char *pTitle, char *pSrc, int pLen);
//��Ϣת��
bool SendMsg(TKU32 ulHandle, unsigned long ulMCBHandle, char *msg);//����Ϣ����Ϣ����
bool SendMsg_Ex(TKU32 ulHandle, unsigned long ulMCBHandle, char *msg, int sendLen);
bool SendToOms(TKU32 pHandle, int pTBLMUCBEnum, bool pEnabled, BYTE pMod,char *pApmId, BYTE pMsgType, char *pSrc, int pLen);

//����ת��
void Time2Chars(time_t* t_time, char* pTime);
void Str2time(char* strTime, time_t* ntime);

typedef CodecType (*OnDecodeReader)(unsigned char* pMsg, int& nUsed, unsigned char* out, BYTE msgType);
typedef int (*OnEncodeReader)(unsigned char id, unsigned char* pMsg, int sendlen, unsigned char* out, unsigned char msgType);
string GetNowtimeStr();
string BytesToStr(BYTE *pBuf, int pLen);
short ByteToShort(BYTE *pBuf);
void strRightFillSpace(const char *src, int inLen,  char* out, int outlen);

bool filecopy(char* srcfile, char* dstfile);

char* ltrim(char* s, int len);
char* rtrim(char* s, int len);
char* trim(char* s, int len);
TKU16 ConvertUShort(TKU16 value);
void DEBUG_LOG(char* pStr);

int  String2Hex(unsigned char* pDest, unsigned char* pSrc, int iLenSrc);
int  Hex2String(char* pSrc, char* pDest, int iLenDest);

void transPolish(char* szRdCalFunc, char* tempCalFunc);         //����ʽת�沨��ʽ
float compvalue(char* tempCalFunc, float* midResult);           //�沨��ʽ����
#pragma pack()

#endif
