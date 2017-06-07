#ifndef _SEQ_RSC_MGR_
#define _SEQ_RSC_MGR_

#include <pthread.h>
typedef int SEQNO;

typedef  int (* GetValEvent)(const int nKey);
typedef  int (* GetKeyEvent)(const SEQNO nVal);

class CSeqRscMgr{
public:
	CSeqRscMgr(void);
	virtual ~CSeqRscMgr(void);
	//ʹ��ǰ����ɹ����õķ�����
	//nNmbΪ���ظ����õ����кŸ�����
	//hGetValΪKeyӳ��Ϊʵ�����кŵĺ���ָ��,
	//hGetKeyΪʵ�����к�ӳ��ΪKey�ĺ���ָ��
	bool Initialize(int nNmb, GetValEvent hGetVal, GetKeyEvent hGetKey);
	//�ж��Ƿ�ɹ���ʼ��
	bool IsInitialized(void) {return m_bInitialized;}
	
	//���ʵ�����кţ����кŴ�nVal����
	bool Alloc(int& nVal);
	//�������кţ�
	bool Free(const SEQNO val);
	
	//������кŵ�������
	int GetTotalVol(void) {return m_nRscCnt;}
	//�����ʹ�õ����кŸ���
	int GetUsedVol(void) {return m_nUsedCnt;}
	//�ж�һ��key�����к��Ƿ�����ʹ����
	bool IsInUse(int& nKey);
	//�����������ʹ�õ����кţ�buffΪ��Ϣ��ŵĻ��棬sizeΪ������ֽ���
	char* sPrintRscInUse(char* buff, int size);
private:
	int* m_KeyArray;
	SEQNO* m_SeqArray;
	bool* m_RscState;
	int m_nUsedCnt;
	int m_nRscCnt;
	bool m_bInitialized;
	pthread_mutex_t     m_tmLock;				// �����ٽ���
	GetValEvent OnGetVal;
	GetKeyEvent OnGetKey;
};

extern CSeqRscMgr g_SeqRscMgr;

#endif  

