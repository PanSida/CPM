#ifndef _ANALYZER_H
#define _ANALYZER_H


#include "Shared.h"
#include "Define.h"
#include <sys/time.h>

#include <tr1/unordered_map>
using namespace std;

#define MAX_CNT_ACTFORMAT  (2048)//V1.0.1.2
#define MAX_CNT_ATTRACT	   (2048)//V1.0.1.2
#define MAX_CNT_AGENT	   (2048)//V1.0.1.2

typedef struct _ACT_FORMAT    //�����������ô洢
{
	int iSeq;				//��ˮ��
    char szCondition[129];       //����������Ч����
    char szValibtime[31];        //����������Чʱ������
    int  iInterval;       //�������
	int  iTimes		;       //��������
    char szActDevId[DEVICE_ID_LEN_TEMP];              //Ŀ���豸ID
    char szActActionId[DEVICE_ACTION_SN_LEN_TEMP];               //Ŀ���豸sn
    char szActValue[256];         //��������
    time_t time_lastTime;     //�ϴ�����ʱ��
	int	times_lastTime;			//�ϴ���������
}ACT_FORMAT, *PACT_FORMAT;

typedef union _LastValue{
	short			shortValue;
	int				iValue;
	float			fValue;
	double		    dfValue;
	unsigned char	szbValue[128];
	char			cValue;
	char			szcValue[128];
	unsigned char	szBcdValue[128];
}UnionLastValue;

typedef struct _MSG_ANALYSE_HASH
{
    char key[DEVICE_ATTR_ID_LEN_TEMP];              //�豸ID��
	char dataStatus;           //�豸�Զ���״̬
	char deviceStatus;         //�豸�Զ���״̬
    char standard[60];         //��׼��Χ
    char statusDef[128];       //�豸״̬ת����
    int  actCnt;
	UnionLastValue unionLastValue;
	struct timeval LastTime;            //�ϴβ���ʱ��
	int agentSeq[MAX_CNT_ACTFORMAT];
}MSG_ANALYSE_HASH,*PMSG_ANALYSE_HASH;
class CAnalyser
{
public:
    CAnalyser(void);
	virtual ~CAnalyser(void);

    virtual bool Initialize(TKU32 unHandleAnalyser);
    bool addNode(MSG_ANALYSE_HASH hash_node);              //��ϣ�����ӽڵ�
    bool delNode(MSG_ANALYSE_HASH hash_node);              //��ϣ��ɾ���ڵ�
	bool updateNodePara(MSG_ANALYSE_HASH hash_node, int paraId);
    bool searchNode(MSG_ANALYSE_HASH hash_node);           //�ڹ�ϣ������ҽڵ�

    bool insertActData(char *inMsg);
    bool GetHashData(MSG_ANALYSE_HASH& hash_node);          //ȡ�ù�ϣ��ڵ�����
    static int InitActHashCallback(void *data, int n_column, char **column_value, char **column_name);
    int InitActHashCallbackProc(int n_column, char **column_value, char **column_name);
    void f_DelData();

	bool InitAgentInfo(const char* sqlData);
	static int InitAgentInfoCallback(void *data, int n_column, char **column_value, char **column_name);    //��ѯ������ʱ�Ļص�����
	int InitAgentInfoCallbackProc(int n_column, char **column_value, char **column_name);

private:
    static void *pAnalyserThrd(void* arg);
    void MsgAnalyzer();              //�����̺߳���

	void UpdateDeviceStatus(char* deviceId, char* status, char* strTime);//�豸״̬���
	void InsertDataTable(MSG_ANALYSE_HASH hash_node, const char* tableName, char* time, char* szValue , int status, bool bRedo);
	void InsertATSTable(MSG_ANALYSE_HASH hash_node, char* szTableName, char* time, char* szValue , char* pUserId);

    void DecodeAct(char* deviceId, const char* sn, int vType, char* value, int* pAgentSeq, int actCnt, char* strTime);    //���ݽ���

	void DoOffLineAct(char* deviceId, const char* sn, int* pAgentSeq,  int actCnt, bool bIsReNotice);
    
    bool InitAnalyseHash();          //��ϣ�����ݳ�ʼ��
    void getAgentMsg(int* pAgentSeq,   AgentInfo stAgent);   //�����������ô洢��
    bool conditionCheck(int vType, const char* value, char* split_result_pos);    //����ƥ��
	bool f_valueChangedCheck(int vType, char* value, UnionLastValue unionLastValue);
    bool f_timeCheck(time_t tm_NowTime, char *timeFormat);
    bool deviceStatusUpdate(char* devId, int status);
    int analogCheck( int channel, int baseValue, float analogValue );    //����ģ����׼ȷ����֤
    int switchCheck( int channel, float baseValue, int switchValue );    //������׼ȷ����֤

    class Lock
	{
	private:
		pthread_mutex_t	 m_lock;
	public:
		Lock() { pthread_mutex_init(&m_lock, NULL); }
		bool lock() { return pthread_mutex_lock(&m_lock)==0; }
		bool unlock() { return pthread_mutex_unlock(&m_lock)==0; }
	}CTblLock;

private:                         //��ϣ��ڵ���
	typedef  tr1::unordered_map<string, MSG_ANALYSE_HASH> HashedInfo;
    HashedInfo* m_pActHashTable;           //�豸������ϣ��
    TKU32 m_unHandleAnalyser;              //������о��

	typedef  tr1::unordered_map<int, ACT_FORMAT> HashedAgentInfo;
	HashedAgentInfo* m_pAgentHashTable;

public:
	bool InitActHashData(char* sqlData);
	bool AnalyseHashDeleteByNet( char* device_id);
	bool AnalyseHashDeleteByDeviceAttr( char* device_attr_id);

	int AnalyseHashAgentAdd( AgentInfo stAgentInfo);
	int AnalyseHashAgentUpdate(AgentInfo stAgentInfo);
	bool AnalyseHashAgentDelete(AgentInfo stAgentInfo);
	int AnalyseHashAgentCheck( AgentInfo stAgentInfo);

public:
	bool ConditionCheck(char* attrId, int vType, const char* curValue, char* strConditions);
	bool IsTrue(char* pAttrId, int vType, const char* curValue, char* expression);

	void transPolish1(char* szRdCalFunc, char* tempCalFunc);
	bool IsAlp(char c);
	bool IsDigit(char* ch);
	bool IsAttrId(char* ch);
	int compvalue1(char* pAttrId, int vType, const char* curValue, char* tempCalFunc, float& fRsult);

};


#endif

