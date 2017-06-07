#ifndef __MEM_MGMT__
#define __MEM_MGMT__
#include <pthread.h>

#define MAX_MUCB_NUM	64

//	�ӿں����ĺ궨�壬֧���ڴ�DEBUGģʽ
#ifdef MM_DEBUG
	#define MM_ALLOC(ulMH)			g_MemMgr.DbgAlloc(ulMH)
	#define MM_FREE(ptrMB)			g_MemMgr.DbgFree(ptrMB)
	#define MM_CHECK(ptrMB)			g_MemMgr.DbgCheckT(ptrMB)

	#define MM_ALLOC_H(ulMH)		g_MemMgr.DbgAlloc_H(ulMH)
	#define MM_FREE_H(ptrMB, ulMH)	g_MemMgr.DbgFree_H(ptrMB)
	#define MM_FREE_G(ptrMB, ulMH)	g_MemMgr.DbgFree_G(ptrMB)
	#define MM_CHECK_H(ptrMB, ulMH)	g_MemMgr.DbgCheck_H(ptrMB)
#else
	#define MM_ALLOC(ulMH)			g_MemMgr.Alloc(ulMH)
	#define MM_FREE(ptrMB)			g_MemMgr.Free(ptrMB)
	#define MM_CHECK(ptrMB)			g_MemMgr.Check(ptrMB)

	#define MM_ALLOC_H(ulMH)		g_MemMgr.Alloc_H(ulMH)
	#define MM_FREE_H(ptrMB, ulMH)	g_MemMgr.Free_H(ptrMB, ulMH)
	#define MM_FREE_G(PtrMB)		g_MemMgr.Free_G(ptrMB)
	#define MM_CHECK_H(ptrMB, ulMH) g_MemMgr.Check_H(ptrMB, ulMH)
#endif


// �û��ڴ������ƿ�ṹ
typedef struct{
	unsigned long			ulUserMCBHdl;		// �û��ڴ���ƿ���
	unsigned long			ulOrgElemSize;		// �û������ÿ����Ϣ�ĳߴ�
	unsigned long			ulActElemSize;		// ʵ���ṩ��ÿԪ�سߴ磨ϵͳ����Ϊ���Ա�sizeof(unsigned long)������
	unsigned long			ulElemCount;		// Ԫ������
	unsigned long			ulMemSize;			// �ڴ����ߴ� = ulActElemSize * ulElemCount
}MMUserCtrlBlkStruct;

// �����ڴ�ָ������
typedef void* MMMemPtr;

// �ڴ������ƿ�ṹ
typedef struct{
	MMUserCtrlBlkStruct stUCM;				// �û��ڴ������ƿ�
	char*				ptrMem;				// �ڴ���
	MMMemPtr*			ptrMemPtr;			// �ڴ��ַ����
	unsigned long		ulMemCnt;			// ���������ʣ���ڴ������
	pthread_mutex_t     csLock;				// �����ٽ���
}MMCtrlBlkStruct;


// �ڴ�����������
typedef enum{
	MMO_OK = 1,					// �ڴ��������ɹ�
	MMO_FAILED = 0				// �ڴ�������ʧ��
}MMOREnum;

class CMemMgr{
public:
	CMemMgr(void);
	~CMemMgr(void);
	//	��ʼ���ڴ����ģ��
	MMOREnum	Initialize(unsigned long ulMaxRsvBlk = MAX_MUCB_NUM);

//�ڴ��������ģʽ���ڴ���ͷ�ʱ���������Ӧ��������
public:		
	// �û�ע�ᱣ��ʹ�õ��ڴ�
	MMOREnum	RegRsvMem_H(MMUserCtrlBlkStruct* pstMUCB);
	//	����ĳ���ڴ�,�ڴ��ͷ���������
	void*		Alloc_H(unsigned long ulMCBHandle);
	//	ָ������ͷ��ڴ棬�ٶȿ�(�ؼ��ǽ���ظ��ͷ�����)
	MMOREnum	Free_H(void*& ptrMemBlock, unsigned long ulMCBHandle);
	//	��ָ������ͷ��ڴ�
	MMOREnum	Free_G(void*& ptrMemBlock);

//�ڴ�����߼�ģʽ���ڴ���ͷ�ʱ���������Ӧ��������
public:		
	// �û�ע�ᱣ��ʹ�õ��ڴ�
		MMOREnum	RegRsvMem(MMUserCtrlBlkStruct* pstMUCB);
	//	����ĳ���ڴ�,�ڴ��ͷ���������
		void*		Alloc(unsigned long ulMCBHandle);
	//	�ͷ��ڴ�,�ڴ��ͷ���������
		MMOREnum	Free(void*& ptrMemBlock);

#if defined(MM_DEBUG)

// =====================DEBUG �汾����==============================//

	//	DEBUG�棺����ĳ���ڴ�
	void*		DbgAlloc_H(unsigned long ulMCBHandle, 
		const char* lpszFileName = __FILE__, int nLine = __LINE__);
	//	DEBUG�棺ָ�������ͷ��ڴ棬�ٶȿ�(�ؼ��ǽ���ظ��ͷ�����)
	MMOREnum	DbgFree_H(void*& ptrMemBlock, unsigned long ulMCBHandle, 
		const char* lpszFileName = __FILE__, int nLine = __LINE__);
	//	DEBUG�棺�ͷ��ڴ�(�ؼ��ǽ���ظ��ͷ�����)
	MMOREnum	DbgFree_G(void*& ptrMemBlock, 
		const char* lpszFileName = __FILE__, int nLine = __LINE__);
	//	DEBUG�棺��鵱ǰʹ�õ��ڴ�ָ���Ƿ�Ϸ�
	void		DbgCheck_H(void* ptrMemBlock, unsigned long ulMCBHandle, 
		const char* lpszFileName = __FILE__, int nLine = __LINE__);


	//	DEBUG�棺����ĳ���ڴ�
	void*		DbgAlloc(unsigned long ulMCBHandle, 
		const char* lpszFileName = __FILE__, int nLine = __LINE__);
	//	DEBUG�棺�ͷ��ڴ�
	MMOREnum	DbgFree(void*& ptrMemBlock,  
		const char* lpszFileName = __FILE__, int nLine = __LINE__);
	//	��鵱ǰʹ�õ��ڴ�ָ���Ƿ�Ϸ�
	void		DbgCheck(void* ptrMemBlock, 
		const char* lpszFileName = __FILE__, int nLine = __LINE__);

#endif
	
private:
	// ����ͳ�ʼ���ڴ���ƿ�����
	MMCtrlBlkStruct *m_pstMMCBArray;
	
    // ����ڴ���ƿ�����
    	unsigned long m_ulMaxMUCBNum;
    
	// �ڴ���ƿ�ʹ�ü�����
	unsigned long m_ulMMCBUseCnt;
	
	// �ڴ�ע���ٽ���
	pthread_mutex_t m_mtxRegLock;
};

extern CMemMgr g_MemMgr;

#endif
