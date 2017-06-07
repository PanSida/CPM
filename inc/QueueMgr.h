#ifndef __PTR_QUEUE_MT__1
#define __PTR_QUEUE_MT__1

#include <semaphore.h>
#include <string>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include "TkMacros.h"
using namespace std;

typedef void* QUEUEELEMENT;
#define MAX_QUEUE_LEN	1024
#define TW_INFINITE		-1

typedef  int (* FreeMemEvent)(QUEUEELEMENT ptrObj);

class CPtrQueueMT 
{
// operations
public:
	CPtrQueueMT();
	virtual ~CPtrQueueMT(void);
	/*****************************************************************************
	������: Initialize
	����:  [in] unQueueLen	---- ���г���
		   [in]	hRelMem��ʾ��Ӧ���������Ϣ�����¼���callback����ָ��
	����ֵ:	true		�ɹ�
				false		ʧ��

	˵��:	��������CPtrQueueMT��ʵ������������ã�
			��ɳ�ʼ�����п����źŵƣ�����ͷβָ��Ͷ����ڴ������
			
	*****************************************************************************/
	bool Initialize(TKU32 unQueueLen = MAX_QUEUE_LEN, FreeMemEvent hRelMem = NULL);
	
	/*****************************************************************************
		������: AddTail
		����:   
				[in] ptrObj     ---- ����ӵ�ָ��
				[in] nTimeout  ---- ������ʱʱ��
		����ֵ:
				true   ---- �ɹ�
				false  ---- ʧ��
	
		˵��:   ������ʵ�������β�����һ��ָ�룬���̰߳�ȫ
	
	*****************************************************************************/
	bool AddTail(QUEUEELEMENT ptrObj, int nTimeout = TW_INFINITE);

	/*****************************************************************************
		������: RemoveHead
		����:   
				[out] ptrObj	---- ��ȡ�õ�ָ��
				[in]  nTimeout	---- ������ʱʱ��
		����ֵ:
				true   ---- �ɹ�
				false  ---- ʧ��
	
		˵��:   ������ʵ�ִӶ����ײ�ȡ��һ��ָ�룬���̰߳�ȫ
	
	*****************************************************************************/
	bool RemoveHead(QUEUEELEMENT& ptrObj, int nTimeout = TW_INFINITE);

	/*****************************************************************************
		������: AddHead
		����:   
				[in] ptrObj     ---- ����ӵ�ָ��
				[in] nTimeout  ---- ������ʱʱ��
		����ֵ:
				true   ---- �ɹ�
				false  ---- ʧ��
	
		˵��:   ������ʵ�������ͷ�����һ��ָ�룬���̰߳�ȫ
	
	*****************************************************************************/
	bool AddHead(QUEUEELEMENT ptrObj, int nTimeout = TW_INFINITE);

	/*****************************************************************************
		������: RemoveTail
		����:   
				[out] ptrObj	---- ��ȡ�õ�ָ��
				[in]  nTimeout	---- ������ʱʱ��
		����ֵ:
				true   ---- �ɹ�
				false  ---- ʧ��
	
		˵��:   ������ʵ�ִӶ���β��ȡ��һ��ָ�룬���̰߳�ȫ
	
	*****************************************************************************/
	bool RemoveTail(QUEUEELEMENT& ptrObj, int nTimeout = TW_INFINITE);

	/*****************************************************************************
		������: GotoHead
		����:   
		����ֵ:
				true   ---- �ɹ�
				false  ---- ʧ��
	
		˵��:   ������ʵ�ֽ�m_nCursor�ƶ���m_nHeadλ�ã����̰߳�ȫ
	
	*****************************************************************************/
	void GotoHead(void);
	
	/*****************************************************************************
	������: PeerNext
	����:   
			[out] ptrObj	---- ��ȡ�õ�ָ��
	����ֵ:
			true   ---- �ɹ�
			false  ---- ʧ��

	˵��:   ������ʵ�ִ�m_nCursorλ���Ϸ���һ��ָ�룬����֤�̰߳�ȫ��
			ʵ��ʹ��ʱ��Ҫ�� try...catch...finally

	*****************************************************************************/
	bool PeerNext(QUEUEELEMENT& ptrObj);
	//�ж���Ϣ�����Ƿ��ʼ��
	bool IsInitialized(bool) {return m_bInitialized;}
	
	/*****************************************************************************
	������: Finalize
	����:	��
	����ֵ: ��
	
	˵��:	����������������CPtrQueueMT��ʵ����ɾ��ʱ�������������ã�
			����ͷŶ��п����źŵƾ���Ͷ����ڴ������
	*****************************************************************************/
	void Finalize(void);
	//������Ϣ������
	void BlockProducer(void) {m_bProducerAllowed = false;}
	//ȡ������Ϣ����������
	void UnBlockProducer(void) {m_bProducerAllowed = true;}
	//������Ϣ������
	void BlockConsumer(void) {m_bConsumerAllowed = false;}
	//ȡ������Ϣ�����ߵ�����
	void UnBlockConsumer(void) {m_bConsumerAllowed = true;}
// Implementation
	bool m_bProducerAllowed;
	bool m_bConsumerAllowed;
protected:
	sem_t m_hSemEmptyLck;				// ���пռ������źŵ�
	sem_t m_hSemFullLck;				// �������źŵ�
	sem_t m_hHeadLck;					// ͷָ����
	sem_t m_hTailLck;					// βָ����

	bool m_bInitialized;
	TKU32 m_unQueueLen;					// ��Ϣ���г���

	QUEUEELEMENT* m_ptrQueue;			// ��Ϣ����
	
	int m_nHead;						// ͷ���±�
	int m_nTail;						// β���±�
	int m_nCursor;						// �����α�
	int m_nElemCnt;

	FreeMemEvent OnRelMem;
protected:
	int PrimP(sem_t& hSem, int nTimeout);
	int PrimV(sem_t& hSem);
	int SemWait(sem_t* sem, int nTimeout);
	void Init();
};

#endif
