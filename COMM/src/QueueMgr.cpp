#include "QueueMgr.h"



/*****************************************************************************
	������: CPtrQueueMT
	����:   [in] unQueueLen	---- ���г���
	����ֵ: ��

	˵��:	���캯������������CPtrQueueMT��ʵ��������ʱ���ã�
			��ɳ�ʼ�����п����źŵƣ�����ͷβָ��Ͷ����ڴ������
*****************************************************************************/
CPtrQueueMT::CPtrQueueMT()
{
	Init();
}

void CPtrQueueMT::Init()
{
	// ��ʼ��ͷβָ��
	m_nHead = 0;
	m_nTail = 0;
	m_nElemCnt = 0;
	m_nCursor = -1;
	OnRelMem = NULL;
	m_bInitialized = false;
	m_bProducerAllowed = false;
	m_bConsumerAllowed = false;
	memset(&m_hSemEmptyLck, 0, sizeof(sem_t));
	memset(&m_hSemFullLck, 0, sizeof(sem_t));
	memset(&m_hHeadLck, 0, sizeof(sem_t));
	memset(&m_hTailLck, 0, sizeof(sem_t));
}
/*****************************************************************************
	������: Initialize
	����:  [in] unQueueLen	---- ���г���
	����ֵ:	true		�ɹ�
				false		ʧ��

	˵��:	��������CPtrQueueMT��ʵ������������ã�
			��ɳ�ʼ�����п����źŵƣ�����ͷβָ��Ͷ����ڴ������
*****************************************************************************/
bool CPtrQueueMT::Initialize(TKU32 unQueueLen, FreeMemEvent hRelMem)
{
	Finalize();
	Init();
	OnRelMem = hRelMem;
	memset(&m_hSemEmptyLck, 0, sizeof(sem_t));
	m_hSemEmptyLck.__align = unQueueLen;
	if (-1 == sem_init(&m_hSemEmptyLck, 0, unQueueLen))
	{
		return false;
	}

	memset(&m_hSemFullLck, 0, sizeof(sem_t));
	m_hSemFullLck.__align = 0;
	if (-1 == sem_init(&m_hSemFullLck, 0, (TKU32)0))
	{
		return false;
	}

	int nVal = 0;
	sem_getvalue(&m_hSemFullLck, &nVal);


	memset(&m_hHeadLck, 0, sizeof(sem_t));
	m_hHeadLck.__align = 1;
	if (-1 == sem_init(&m_hHeadLck, 0, (TKU32)1))
	{
		return false;
	}

	memset(&m_hTailLck, 0, sizeof(sem_t));
	m_hTailLck.__align = 1;
	if (-1 == sem_init(&m_hTailLck, 0, (TKU32)1))
	{
		return false;
	}

	// ����ָ������ڴ�

	m_unQueueLen = unQueueLen;
	m_ptrQueue = NULL;
	m_ptrQueue = (QUEUEELEMENT *)new char[sizeof(QUEUEELEMENT) * unQueueLen];
	if (!m_ptrQueue)
		return false;

	m_bProducerAllowed = true;
	m_bConsumerAllowed = true;
	m_bInitialized = true;
	return true;
}

/*****************************************************************************
������: Finalize
����:	��
����ֵ: ��

˵��:	����������������CPtrQueueMT��ʵ����ɾ��ʱ�������������ã�
		����ͷŶ��п����źŵƾ���Ͷ����ڴ������
*****************************************************************************/
void CPtrQueueMT::Finalize(void)
{
	if (true == m_bInitialized)
	{
		m_bProducerAllowed = false;
		m_bConsumerAllowed = false;
		sem_wait(&m_hHeadLck);
		sem_wait(&m_hTailLck);
		sem_destroy(&m_hHeadLck);
		sem_destroy(&m_hTailLck);
		sem_destroy(&m_hSemEmptyLck);
		sem_destroy(&m_hSemFullLck);

		if (NULL != OnRelMem)
			try
			{
				while (m_nElemCnt-- > 0)
				{
					OnRelMem(m_ptrQueue[m_nHead]);
					m_nHead = (m_nHead + 1) % m_unQueueLen;
				}
			} catch (...)
			{
			}

		delete[] m_ptrQueue;

		m_bInitialized = false;
	}
}

/*****************************************************************************
	������: ~CPtrQueueMT
	����:   ��
	����ֵ: ��
	
	˵��:	������������������CPtrQueueMT��ʵ����ɾ��ʱ���ã�
			����ͷŶ��п����źŵƾ���Ͷ����ڴ������
*****************************************************************************/
CPtrQueueMT::~CPtrQueueMT(void)
{
	Finalize();
}

/*****************************************************************************
	������: PrimP
	����:   
			[in] hSem      ---- �ź������
			[in] nTimeout ---- ��ʱ
	����ֵ:
			SEM_PICKED    ---- ȡ��һ���ź���
			SEM_TIOMEOUT  ---- ��ʱ
			SEM_ERROR     ---- ����

	˵��:   ������ʵ�־���ϵͳ�����е�P����,
			����ɿ��̿���.
*****************************************************************************/
int CPtrQueueMT::PrimP(sem_t& hSem, int nTimeout)
{
	assert(nTimeout >= TW_INFINITE);
	if (TW_INFINITE == nTimeout)
	{
		return sem_wait(&hSem);
	} else if (0 == nTimeout)
	{
		return sem_trywait(&hSem);
	} else
	{
		return SemWait(&hSem, nTimeout);
	}
}

/*****************************************************************************
	������: PrimV
	����:   [in] hSem      ---- �ź������
	����ֵ:
			true   ---- �ɹ�
			false  ---- ʧ��

	˵��:   ������ʵ�־���ϵͳ�����е�V����,
			����ɿ��̿���.

*****************************************************************************/
int CPtrQueueMT::PrimV(sem_t& hSem)
{
	return sem_post(&hSem);
}

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
bool CPtrQueueMT::AddHead(QUEUEELEMENT ptrObj, int nTimeout)
{
	if (false == m_bProducerAllowed)
		return false;
	if (PrimP(m_hSemEmptyLck, nTimeout) != 0)
		return false;
	if (PrimP(m_hHeadLck, TW_INFINITE) != 0)
	{
		PrimV(m_hSemEmptyLck);
		return false;       
	}

	//�ع�ͷָ��
	m_nHead = (m_nHead - 1 + m_unQueueLen) % m_unQueueLen;

	//��һ��ָ��������
	m_ptrQueue[m_nHead] = ptrObj;
	m_nElemCnt++;

	if (PrimV(m_hHeadLck) != 0)
		return false;

	if (PrimV(m_hSemFullLck) != 0)
		return false;

	return true;    
}


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
bool CPtrQueueMT::AddTail(QUEUEELEMENT ptrObj, int nTimeout)
{
	if (false == m_bProducerAllowed)
		return false;
	if (PrimP(m_hSemEmptyLck, nTimeout) != 0)
		return false;
	if (PrimP(m_hTailLck, TW_INFINITE) != 0)
	{
		PrimV(m_hSemEmptyLck);
		return false;       
	}
	//��һ��ָ��������
	m_ptrQueue[m_nTail] = ptrObj;
	//�޸�βָ��
	m_nTail = (m_nTail + 1) % m_unQueueLen;
	m_nElemCnt++;

	if (PrimV(m_hTailLck) != 0)
		return false;

	if (PrimV(m_hSemFullLck) != 0)
		return false;
	return true;    
}


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
bool CPtrQueueMT::RemoveHead(QUEUEELEMENT& ptrObj, int nTimeout)
{
	if (false == m_bConsumerAllowed)
		return false;
	ptrObj = NULL;

	if (PrimP(m_hSemFullLck, nTimeout) != 0)
		return false;

	if (PrimP(m_hHeadLck, TW_INFINITE) != 0)
	{
		PrimV(m_hSemFullLck);
		return false;
	}

	ptrObj = m_ptrQueue[m_nHead];
	m_nHead = (m_nHead + 1) % m_unQueueLen;
	m_nElemCnt--;

	if (PrimV(m_hHeadLck) != 0)
		return false;
	if (PrimV(m_hSemEmptyLck) != 0)
		return false;

	return true;
}


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
bool CPtrQueueMT::RemoveTail(QUEUEELEMENT& ptrObj, int nTimeout)
{
	if (false == m_bConsumerAllowed)
		return false;
	ptrObj = NULL;

	if (PrimP(m_hSemFullLck, nTimeout) != 0)
		return false;

	if (PrimP(m_hTailLck, TW_INFINITE) != 0)
	{
		PrimV(m_hSemFullLck);
		return false;
	}

	//�ع�βָ��
	m_nTail = (m_nTail - 1 + m_unQueueLen) % m_unQueueLen;

	if (m_nCursor == m_nTail)
		m_nCursor = -1;

	//����һ��ָ��
	ptrObj = m_ptrQueue[m_nTail];
	m_nElemCnt--;

	if (PrimV(m_hTailLck) != 0)
		return false;
	if (PrimV(m_hSemEmptyLck) != 0)
		return false;

	return true;
}

/*****************************************************************************
	������: GotoHead
	����:   
	����ֵ:
			true   ---- �ɹ�
			false  ---- ʧ��

	˵��:   ������ʵ�ֽ�m_nCursor�ƶ���m_nHeadλ�ã����̰߳�ȫ

*****************************************************************************/
void CPtrQueueMT::GotoHead(void)
{
	m_nCursor = m_nHead;
}
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
bool CPtrQueueMT::PeerNext(QUEUEELEMENT& ptrObj)
{
	bool bResult = true;
	ptrObj = NULL;

	try
	{
		if (-1 != m_nCursor)
		{
			if (m_nCursor == m_nTail)
			{
				m_nCursor = -1;
				bResult = false;
			}
			ptrObj = m_ptrQueue[m_nCursor];
			m_nCursor = (m_nCursor + 1) % m_unQueueLen;
		} else
		{
			bResult = false;
		}
	} catch (...)
	{
		bResult = false;
	}
	return bResult;
}

int CPtrQueueMT::SemWait(sem_t* sem, int nTimeout)
{
	assert(nTimeout > 0);

	if (0 == sem_trywait(sem))
		return 0;

	timespec tTime = {0, 10000000};
	timespec tRem = {0};
	int nTimeSlips = (nTimeout / 10) - 1;
	int mTimeLeft = (nTimeout % 10) *  1000000;
	for (int i = 0; i < nTimeSlips; i++)
	{
		if (0 == sem_trywait(sem))
			return 0;
		else
			nanosleep(&tTime, &tRem);
	}
	if (0 == sem_trywait(sem))
		return 0;
	else
	{
		if (mTimeLeft)
		{
			tTime.tv_nsec = mTimeLeft;
			nanosleep(&tTime, &tRem);
			if (0 == sem_trywait(sem))
				return 0;
		}
	}
	return -1;
}
