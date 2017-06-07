#include <stdio.h>
#include <semaphore.h>
#include <assert.h>
#include <stdlib.h>
#include <string>

#include "TkMacros.h"
#include "MemMgmt.h"
using namespace std;

#define	MM_MAX_MEM_SIZE	8192

#ifdef MM_DEBUG
	#define	MM_WARN_MSG(WarnMsg, t, l, s)	\
			DBG((WarnMsg, t, l, s)); 


static const char gc_szSampMem[MM_MAX_MEM_SIZE] = {0};
#endif

// ����ͳ�ʼ���ڴ���ƿ�����
// ===========================ģ��ʵ��=============================<===//
CMemMgr::CMemMgr(void)
{
	TRACE(CMemMgr::CMemMgr);
	m_pstMMCBArray = NULL;
	m_ulMaxMUCBNum = 0;
	m_ulMMCBUseCnt = 0;
	memset(&m_mtxRegLock, 0, sizeof(pthread_mutex_t));
}

CMemMgr::~CMemMgr(void)
{
	TRACE(CMemMgr::~CMemMgr);
	if (!m_pstMMCBArray)		// �ڴ���ƿ�����δ��ʼ����
	{
		return;
	}

	pthread_mutex_lock(&m_mtxRegLock);
	for (int i = 0; i < (int)m_ulMMCBUseCnt; i++)
	{
		MMCtrlBlkStruct* pstCurMMCB = &m_pstMMCBArray[i];
		pthread_mutex_lock(&pstCurMMCB->csLock);
		try
		{
			if (NULL != pstCurMMCB->ptrMem)
			{
				free(pstCurMMCB->ptrMem);
				pstCurMMCB->ptrMem = NULL;
				if (NULL != pstCurMMCB->ptrMemPtr)
				{
					free(pstCurMMCB->ptrMemPtr);
					pstCurMMCB->ptrMemPtr = NULL;
				}
			}
			pstCurMMCB->ptrMem = NULL;
			pstCurMMCB->ptrMemPtr = NULL;
		}
		catch (...)
		{
			pstCurMMCB->ptrMem = NULL;
			pstCurMMCB->ptrMemPtr = NULL;
		}
		pthread_mutex_unlock(&pstCurMMCB->csLock);
		pthread_mutex_destroy(&pstCurMMCB->csLock);
	}

	free(m_pstMMCBArray);
	m_pstMMCBArray = NULL;
	pthread_mutex_unlock(&m_mtxRegLock);
	pthread_mutex_destroy(&m_mtxRegLock);
}

//	��ʼ���ڴ����ģ��
MMOREnum    CMemMgr::Initialize(unsigned long ulMaxRsvBlk)
{
	TRACE(CMemMgr::Initialize);
	if (m_pstMMCBArray)			// �ڴ���ƿ������Ѿ���ʼ����
	{
		return MMO_OK;
	}

	if (ulMaxRsvBlk > 0)
	{
		m_ulMaxMUCBNum = ulMaxRsvBlk;
	} else
	{
		return MMO_FAILED;
	}

	m_pstMMCBArray = (MMCtrlBlkStruct*)malloc(sizeof(MMCtrlBlkStruct) * m_ulMaxMUCBNum);
	if (NULL != m_pstMMCBArray)
	{
		memset(m_pstMMCBArray, 0, sizeof(MMCtrlBlkStruct) * m_ulMaxMUCBNum);
		pthread_mutex_init(&m_mtxRegLock, NULL);
		return MMO_OK;
	} 
	else
	{
		return MMO_FAILED;
	}
}

// �û�ע�ᱣ��ʹ�õ��ڴ�
MMOREnum    CMemMgr::RegRsvMem_H(MMUserCtrlBlkStruct* pstMUCB)
{
	TRACE(CMemMgr::RegRsvMem_H);
	MMOREnum enmMMOResult = MMO_OK;

	if (!m_pstMMCBArray)
	{
		return MMO_FAILED;
	}

	pthread_mutex_lock(&m_mtxRegLock);
	if ((NULL == pstMUCB) || (m_ulMMCBUseCnt >= m_ulMaxMUCBNum) || 
		(0 == pstMUCB->ulOrgElemSize) || (0 == pstMUCB->ulElemCount))
	{
		enmMMOResult = MMO_FAILED;
	} 
	else
	{
		// �û���������
		MMCtrlBlkStruct* pstCurMMCB = &m_pstMMCBArray[m_ulMMCBUseCnt];
		MMUserCtrlBlkStruct& stCurMUCB = pstCurMMCB->stUCM;
		stCurMUCB.ulUserMCBHdl  = m_ulMMCBUseCnt;
		stCurMUCB.ulOrgElemSize = pstMUCB->ulOrgElemSize;
		stCurMUCB.ulElemCount   = pstMUCB->ulElemCount;
		stCurMUCB.ulActElemSize = pstMUCB->ulOrgElemSize;
		pstCurMMCB->ulMemCnt    = pstMUCB->ulElemCount;

		int nLea = (stCurMUCB.ulOrgElemSize % sizeof(unsigned long));
		if(0 != nLea) // ��������ַsizeof(unsigned long)�ֽڶ���
		stCurMUCB.ulActElemSize += (sizeof(unsigned long) - nLea);

		stCurMUCB.ulMemSize = stCurMUCB.ulActElemSize * stCurMUCB.ulElemCount;

		if (stCurMUCB.ulActElemSize < stCurMUCB.ulOrgElemSize)
		{
			enmMMOResult = MMO_FAILED;
//			DBG(("\nMMInitialize Failed for ActElemSize overflow with MCBHandle %d\n", i));
		} 
		else
		{
			pstCurMMCB->ptrMem = (char*)malloc(stCurMUCB.ulMemSize);
			if (NULL == pstCurMMCB->ptrMem)
			{
				enmMMOResult = MMO_FAILED;
			} 
			else
			{
				memset(pstCurMMCB->ptrMem, 0, stCurMUCB.ulMemSize);
				pstCurMMCB->ptrMemPtr = (MMMemPtr*)malloc(sizeof(MMMemPtr) * stCurMUCB.ulElemCount);
				if (NULL == pstCurMMCB->ptrMemPtr)
				{
					enmMMOResult = MMO_FAILED;
				} 
				else
				{
					for (int j = 0; j < (int)stCurMUCB.ulElemCount; j++)
					{
						pstCurMMCB->ptrMemPtr[j] = 
						(void*)&pstCurMMCB->ptrMem[j * stCurMUCB.ulActElemSize];
					}

					pthread_mutex_init(&pstCurMMCB->csLock, NULL);

					pstMUCB->ulActElemSize = stCurMUCB.ulActElemSize;
					pstMUCB->ulMemSize = stCurMUCB.ulMemSize;
					pstMUCB->ulUserMCBHdl = m_ulMMCBUseCnt;
					m_ulMMCBUseCnt++;
				}
			}
		}
	}
	pthread_mutex_unlock(&m_mtxRegLock);
	return enmMMOResult;
}


//	����һ��ĳ���ڴ�
void*   CMemMgr::Alloc_H(unsigned long ulMCBHandle)
{
	TRACE(CMemMgr::Alloc_H);
	assert(NULL != m_pstMMCBArray);
	void* ptrResult = NULL;
	if (ulMCBHandle < m_ulMaxMUCBNum)
	{
		MMCtrlBlkStruct* pstCurMMCB = &m_pstMMCBArray[ulMCBHandle];
		pthread_mutex_lock(&pstCurMMCB->csLock);
		if ((NULL != pstCurMMCB->ptrMem) && 
			(NULL != pstCurMMCB->ptrMemPtr) &&
			(pstCurMMCB->ulMemCnt > 0))
		{
			pstCurMMCB->ulMemCnt--;
			ptrResult = (void*)pstCurMMCB->ptrMemPtr[pstCurMMCB->ulMemCnt];
			pstCurMMCB->ptrMemPtr[pstCurMMCB->ulMemCnt] = NULL;
		}
		pthread_mutex_unlock(&pstCurMMCB->csLock);
	}
	return ptrResult;           
}

//	ָ�������ͷ��ڴ棬�ٶȿ�(�ؼ��ǽ���ظ��ͷ�����)
MMOREnum    CMemMgr::Free_H(void*& ptrMemBlock, unsigned long ulMCBHandle)
{
	TRACE(CMemMgr::Free_H);
	assert(NULL != m_pstMMCBArray);
	MMOREnum enmMMOResult = MMO_FAILED;
	if ((NULL != ptrMemBlock) && (ulMCBHandle < m_ulMaxMUCBNum))
	{
		MMCtrlBlkStruct* pstCurMMCB = &m_pstMMCBArray[ulMCBHandle];
		MMUserCtrlBlkStruct& stCurMUCB = pstCurMMCB->stUCM;
		pthread_mutex_lock(&pstCurMMCB->csLock);
		if ((NULL != pstCurMMCB->ptrMem) &&
			(NULL != pstCurMMCB->ptrMemPtr) &&
			(pstCurMMCB->ulMemCnt < stCurMUCB.ulElemCount) &&
			(pstCurMMCB->ptrMem <= (char*)ptrMemBlock) && 
			((char*)ptrMemBlock < (pstCurMMCB->ptrMem + stCurMUCB.ulMemSize)) &&
			(0 == (((char*)ptrMemBlock - pstCurMMCB->ptrMem) % stCurMUCB.ulActElemSize)))
		{
			if (NULL == pstCurMMCB->ptrMemPtr[pstCurMMCB->ulMemCnt])
			{
				pstCurMMCB->ptrMemPtr[pstCurMMCB->ulMemCnt] = ptrMemBlock;

				memset(ptrMemBlock, 0, stCurMUCB.ulActElemSize);
				pstCurMMCB->ulMemCnt++;
				ptrMemBlock = NULL;
				enmMMOResult = MMO_OK;
			}
		}
		pthread_mutex_unlock(&pstCurMMCB->csLock);
	}
	return enmMMOResult;
}

//	�ͷ��ڴ�(�ؼ��ǽ���ظ��ͷ�����)
MMOREnum    CMemMgr::Free_G(void*& ptrMemBlock)
{
	TRACE(CMemMgr::Free_G);
	assert(NULL != m_pstMMCBArray);
	int i = 0;  
	MMOREnum enmMMOResult = MMO_FAILED;
	if (NULL != ptrMemBlock)
	{
		for (i = 0; i < (int)m_ulMaxMUCBNum; i++)
		{
			MMCtrlBlkStruct* pstCurMMCB = &m_pstMMCBArray[i];
			MMUserCtrlBlkStruct& stCurMUCB = pstCurMMCB->stUCM;
			pthread_mutex_lock(&pstCurMMCB->csLock);
			if ((NULL != pstCurMMCB->ptrMem) &&
				(NULL != pstCurMMCB->ptrMemPtr) &&
				(pstCurMMCB->ulMemCnt < stCurMUCB.ulElemCount) &&
				(pstCurMMCB->ptrMem <= (char*)ptrMemBlock) && 
				((char*)ptrMemBlock < (pstCurMMCB->ptrMem + stCurMUCB.ulMemSize)) &&
				(0 == (((char*)ptrMemBlock - pstCurMMCB->ptrMem) % stCurMUCB.ulActElemSize)))
			{
				if (NULL == pstCurMMCB->ptrMemPtr[pstCurMMCB->ulMemCnt])
				{
					pstCurMMCB->ptrMemPtr[pstCurMMCB->ulMemCnt] = ptrMemBlock;

					memset(ptrMemBlock, 0, stCurMUCB.ulActElemSize);
					pstCurMMCB->ulMemCnt++;
					ptrMemBlock = NULL;
					enmMMOResult = MMO_OK;
				}
				break;
			}
			pthread_mutex_unlock(&pstCurMMCB->csLock);
		}       
	}
	return enmMMOResult;
}

// �û�ע�ᱣ��ʹ�õ��ڴ�,�ڴ��ͷ���������
MMOREnum   CMemMgr::RegRsvMem(MMUserCtrlBlkStruct* pstMUCB)
{
	TRACE(CMemMgr::RegRsvMem);
	pstMUCB->ulOrgElemSize += sizeof(unsigned long);
	return RegRsvMem_H(pstMUCB);
}

//	����ĳ���ڴ�,�ڴ��ͷ���������
void*  CMemMgr::Alloc(unsigned long ulMCBHandle)
{
	TRACE(CMemMgr::Alloc);
	void* p = Alloc_H(ulMCBHandle);
	if (p != NULL)
	{
		*(unsigned long*)p = ulMCBHandle;
		return((char*)p + sizeof(unsigned long));
	} else
		return NULL;
}

//	�ͷ��ڴ�(�ؼ��ǽ���ظ��ͷ�����),�ڴ��ͷ���������
MMOREnum   CMemMgr::Free(void*& ptrMemBlock)
{
	TRACE(CMemMgr::Free);
	MMOREnum enmResult = MMO_OK;
	void* p = (char*)ptrMemBlock - sizeof(unsigned long);
	if (MMO_OK == Free_H(p, *(unsigned long*)p))
		ptrMemBlock = NULL;
	else
		enmResult = MMO_FAILED;

	return enmResult;
}


// =====================DEBUG �汾����==============================//
#if defined(MM_DEBUG)

//	DEBUG�棺����ĳ���ڴ�
void*   CMemMgr::DbgAlloc_H(unsigned long ulMCBHandle, const char* lpszFileName, int nLine)
{
	TRACE(CMemMgr::DbgAlloc_H);
	unsigned long ulLeft2Cmp = 0;
	unsigned long ulCmpLen = 0;
	unsigned long ulCmpLea = 0;
	void* pResult = Alloc_H(ulMCBHandle);
	if (!pResult)
	{
		MM_WARN_MSG("\nCMemMgr::Alloc Failed with MCBHandle %d at line = %d in %s\n", 
					(int)ulMCBHandle, nLine, lpszFileName);
	} 
	else if (ulMCBHandle < m_ulMaxMUCBNum)
	{
		MMUserCtrlBlkStruct& stCurMUCB = m_pstMMCBArray[ulMCBHandle].stUCM;
		if (stCurMUCB.ulActElemSize > MM_MAX_MEM_SIZE)
		{
			ulCmpLen = MM_MAX_MEM_SIZE;
			ulLeft2Cmp = stCurMUCB.ulActElemSize;
			while (ulLeft2Cmp)
			{
				if (memcmp((void*)((char*)pResult + ulCmpLea), gc_szSampMem, ulCmpLen))
				{
					MM_WARN_MSG("\nNew Mem Elem from MMAlloc is not Zero with MCBHandle %d at line = %d in %s\n",
								(int)ulMCBHandle, nLine, lpszFileName);
					break;
				}
				else
				{
					ulLeft2Cmp = ulLeft2Cmp - MM_MAX_MEM_SIZE;
					ulCmpLea += MM_MAX_MEM_SIZE;
					if (ulLeft2Cmp < MM_MAX_MEM_SIZE)
					{
						ulCmpLen = ulLeft2Cmp;
					}
				}
			}
		} 
		else if (memcmp(pResult, gc_szSampMem, stCurMUCB.ulActElemSize))
		{
			MM_WARN_MSG("\nNew Mem Elem from CMemMgr::Alloc is not Zero with MCBHandle %d at line = %d in %s\n",
						(int)ulMCBHandle, nLine, lpszFileName);
		}
	} else
	{
		MM_WARN_MSG("\nCMemMgr::Alloc with wrong MCBHandle %d at line = %d in %s\n", 
					(int)ulMCBHandle, nLine, lpszFileName);
	}
	return pResult;
}

//	DEBUG�棺ָ�������ͷ��ڴ棬�ٶȿ�(�ؼ��ǽ���ظ��ͷ�����)
MMOREnum    CMemMgr::DbgFree_H(void*& ptrMemBlock, unsigned long ulMCBHandle, const char* lpszFileName, int nLine)
{
	TRACE(CMemMgr::DbgFree_H);
	MMOREnum    enmResult = Free_H(ptrMemBlock, ulMCBHandle);
	if (MMO_OK != enmResult)
	{
		MM_WARN_MSG("\nCMemMgr::Free_H Failed with MCBHandle %d at line = %d in %s\n",
					(int)ulMCBHandle, nLine, lpszFileName);
	}
	return enmResult;
}

//	DEBUG�棺�ͷ��ڴ�(�ؼ��ǽ���ظ��ͷ�����)
MMOREnum    CMemMgr::DbgFree_G(void*& ptrMemBlock, const char* lpszFileName, int nLine)
{
	TRACE(CMemMgr::DbgFree_G);
	MMOREnum    enmResult = Free_G(ptrMemBlock);
	if (MMO_OK != enmResult)
	{
		MM_WARN_MSG("\nCMemMgr::Free_G Failed %s at line = %d in %s\n",
					"", nLine, lpszFileName);
	}
	return enmResult;
}

//	DEBUG�棺��鵱ǰʹ�õ��ڴ�ָ���Ƿ�Ϸ�
void    CMemMgr::DbgCheck_H(void* ptrMemBlock, unsigned long ulMCBHandle, const char* lpszFileName, int nLine)
{
	TRACE(CMemMgr::DbgCheck_H);
	if (NULL != ptrMemBlock)
	{
		if (ulMCBHandle >= m_ulMaxMUCBNum)
		{
			MM_WARN_MSG("\nCall CMemMgr::Check_H with wrong MCBHandle %d at line = %d in %s\n",
						(int)ulMCBHandle, nLine, lpszFileName);
		} else
		{
			MMUserCtrlBlkStruct& stCurMUCB = m_pstMMCBArray[ulMCBHandle].stUCM;
			MMCtrlBlkStruct* pstCurMMCB = &m_pstMMCBArray[ulMCBHandle];
			pthread_mutex_lock(&pstCurMMCB->csLock);
			if (NULL == pstCurMMCB->ptrMem)
			{
				MM_WARN_MSG("\nMM_CHECK Found m_pstMMCBArray[x].PtrMem not initialized with MCBHandle %d at line = %d in %s\n",
							(int)ulMCBHandle, nLine, lpszFileName);
			} else if (NULL == pstCurMMCB->ptrMemPtr)
			{
				MM_WARN_MSG("\nMM_CHECK Found m_pstMMCBArray[x].PtrMemPtr not initialized with MCBHandle %d at line = %d in %s\n",
							(int)ulMCBHandle, nLine, lpszFileName);
			} else if ((pstCurMMCB->ptrMem > (char*)ptrMemBlock) || 
					   ((char*)ptrMemBlock >= (pstCurMMCB->ptrMem + stCurMUCB.ulMemSize)))
			{
				MM_WARN_MSG("\nMM_CHECK Found Mem Address Used is not in address range with MCBHandle %d at line = %d in %s\n",
							(int)ulMCBHandle, nLine, lpszFileName);
			} else if (0 != (((char*)ptrMemBlock - pstCurMMCB->ptrMem) % pstCurMMCB->stUCM.ulActElemSize))
			{
				MM_WARN_MSG("\nMM_CHECK Found Mem Address Used is not aligned with MCBHandle %d at line = %d in %s\n",
							(int)ulMCBHandle, nLine, lpszFileName);
			}
			pthread_mutex_unlock(&pstCurMMCB->csLock);
		}
	}
}

//	DEBUG�棺����ĳ���ڴ�
void*   CMemMgr::DbgAlloc(unsigned long ulMCBHandle, const char* lpszFileName, int nLine)
{
	TRACE(CMemMgr::DbgAlloc);
	void* p = DbgAlloc_H(ulMCBHandle, lpszFileName, nLine);
	if (p != NULL)
	{
		*(unsigned long*)p = ulMCBHandle;
		return((char*)p + sizeof(unsigned long));
	}
	else
		return NULL;
}

//	DEBUG�棺ָ�������ͷ��ڴ棬�ٶȿ�(�ؼ��ǽ���ظ��ͷ�����)
MMOREnum    CMemMgr::DbgFree(void*& ptrMemBlock, const char* lpszFileName, int nLine)
{
	TRACE(CMemMgr::DbgFree);
	void* p = (char*)ptrMemBlock - sizeof(unsigned long);
	MMOREnum    enmResult = Free(ptrMemBlock);
	if (MMO_OK != enmResult)
	{
		MM_WARN_MSG("\nCMemMgr::Free Failed with MCBHandle %u at line = %d in %s\n",
					*(int*)p, nLine, lpszFileName);
	}
	return enmResult;
}

//	DEBUG�棺��鵱ǰʹ�õ��ڴ�ָ���Ƿ�Ϸ�
void    CMemMgr::DbgCheck(void* ptrMemBlock, const char* lpszFileName, int nLine)
{
	TRACE(CMemMgr::DbgCheck);
	void* p = (char*)ptrMemBlock - sizeof(unsigned long);
	return DbgCheck_H(p, *(unsigned long*)p);
}

#endif


