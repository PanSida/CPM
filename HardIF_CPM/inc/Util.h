#ifndef __UTIL_H__
#define __UTIL_H__

#include <sys/ioctl.h>
#include <unistd.h>
#include <termio.h>
#include <time.h>
int BAUDRATE(int speed);

// 7bit����
// ����: pSrc - Դ�ַ���ָ��
//       nSrcLength - Դ�ַ�������
// ���: pDst - Ŀ����봮ָ��
// ����: Ŀ����봮����
int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength);

// 7bit����
// ����: pSrc - Դ���봮ָ��
//       nSrcLength - Դ���봮����
// ���: pDst - Ŀ���ַ���ָ��
// ����: Ŀ���ַ�������
int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength);

// 8bit����
// ����: pSrc - Դ�ַ���ָ��
//       nSrcLength - Դ�ַ�������
// ���: pDst - Ŀ����봮ָ��
// ����: Ŀ����봮����
int gsmEncode8bit(const char* pSrc, unsigned char* pDst, int nSrcLength);

// 8bit����
// ����: pSrc - Դ���봮ָ��
//       nSrcLength -  Դ���봮����
// ���: pDst -  Ŀ���ַ���ָ��
// ����: Ŀ���ַ�������
int gsmDecode8bit(const unsigned char* pSrc, char* pDst, int nSrcLength);

// �����ߵ����ַ���ת��Ϊ����˳����ַ���
// �磺"683158812764F8" --> "8613851872468"
// ����: pSrc - Դ�ַ���ָ��
//       nSrcLength - Դ�ַ�������
// ���: pDst - Ŀ���ַ���ָ��
// ����: Ŀ���ַ�������
int gsmSerializeNumbers(const char* pSrc, char* pDst, int nSrcLength);

// ����˳����ַ���ת��Ϊ�����ߵ����ַ�����������Ϊ��������'F'�ճ�ż��
// �磺"8613851872468" --> "683158812764F8"
// ����: pSrc - Դ�ַ���ָ��
//       nSrcLength - Դ�ַ�������
// ���: pDst - Ŀ���ַ���ָ��
// ����: Ŀ���ַ�������
int gsmInvertNumbers(const char* pSrc, char* pDst, int nSrcLength);
void timet2chars(time_t t_time, char* pTime);

void QuickSortRun(float* pData,int left,int right);
void QuickSort(float* pData,int Count);

#endif
