#ifndef _AD_Proc_H_
#define _AD_THRD_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "Define.h"
#include "ADCBase.h"


typedef int (*CADCValueNotice_CallBack)(void* userData, int iChannel, unsigned char* value, int len);//�����������ݱ仯֪ͨ�ص�

class CADProc
{
public:
	/******************************************************************************/
	/*                       �����ӿڶ���                                         */
	/******************************************************************************/
	CADProc(CADCBase*, CADCBase*);
	virtual ~CADProc(void);

	bool Initialize(CADCValueNotice_CallBack ADCNotic_CallBack, void* userData);
	bool Terminate();
	//������ģ����
private:
	class LockAD
	{
	private:
		pthread_mutex_t	 m_lock;
	public:
		LockAD() { pthread_mutex_init(&m_lock, NULL); }
		bool lock() { return pthread_mutex_lock(&m_lock)==0; }
		bool unlock() { return pthread_mutex_unlock(&m_lock)==0; }
	}CLockAD;

public:
	static void * ADThrd(void *arg);
	void ADThrdProc(void);
	int PollProc(int index);

	int GetADValue(int Id, float& fInValue);
	int SetADValue(int Id, int iOutValue);
private:
	/******************************************************************************/
	/*                           ��������                                         */
	/******************************************************************************/
	CADCBase* m_ADCBase_A;
	CADCBase* m_ADCBase_B;

	int	m_iTTLSampleCount;						//��������������
	int	m_iADSampleCount;						//ģ������������

	pthread_t	m_MainThrdHandle;

	float		m_DeviceValue[25];
	STR_DEVICE_AD m_strDeviceAD[25]; /* ����������*8 ģ��������*8 ���������*8 �̵������*1 */

	/******************************************************************************/
	/*                       ˽�к�������                                         */
	/******************************************************************************/
	CADCValueNotice_CallBack NoticeCallBack;
	void* callbackData;
};



#endif
