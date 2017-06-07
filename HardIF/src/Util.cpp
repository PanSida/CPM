#include <string.h>
#include <stdio.h>
#include "Util.h"

int		speed_arr[] = {B115200,	B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300,};
int name_arr[] = {115200, 57600, 38400,  19200,  9600,  4800,  2400,  1200,  300,};
int BAUDRATE(int speed)
{
	int i;
	int ret = 0;
	for( i=0; i < (int)(sizeof(speed_arr)/sizeof(int)); i++)
	{
		if(speed == name_arr[i])
		{
			ret = speed_arr[i];
			break;
		}
	}
	return ret;
}

// �ɴ�ӡ�ַ���ת��Ϊ�ֽ�����
// �磺"C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
// ����: pSrc - Դ�ַ���ָ��
//       nSrcLength - Դ�ַ�������
// ���: pDst - Ŀ������ָ��
// ����: Ŀ�����ݳ���
int gsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
	for (int i = 0; i < nSrcLength; i += 2)
	{
		// �����4λ
		if ((*pSrc >= '0') && (*pSrc <= '9'))
		{
			*pDst = (*pSrc - '0') << 4;
		}
		else
		{
			*pDst = (*pSrc - 'A' + 10) << 4;
		}

		pSrc++;

		// �����4λ
		if ((*pSrc>='0') && (*pSrc<='9'))
		{
			*pDst |= *pSrc - '0';
		}
		else
		{
			*pDst |= *pSrc - 'A' + 10;
		}

		pSrc++;
		pDst++;
	}

	// ����Ŀ�����ݳ���
	return (nSrcLength / 2);
}

// �ֽ�����ת��Ϊ�ɴ�ӡ�ַ���
// �磺{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01" 
// ����: pSrc - Դ����ָ��
//       nSrcLength - Դ���ݳ���
// ���: pDst - Ŀ���ַ���ָ��
// ����: Ŀ���ַ�������
int gsmBytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
	const char tab[]="0123456789ABCDEF";    // 0x0-0xf���ַ����ұ�

	for (int i = 0; i < nSrcLength; i++)
	{
		*pDst++ = tab[*pSrc >> 4];        // �����4λ
		*pDst++ = tab[*pSrc & 0x0f];    // �����4λ
		pSrc++;
	}

	// ����ַ����Ӹ�������
	*pDst = '\0';

	// ����Ŀ���ַ�������
	return (nSrcLength * 2);
}

// 7bit����
// ����: pSrc - Դ�ַ���ָ��
//       nSrcLength - Դ�ַ�������
// ���: pDst - Ŀ����봮ָ��
// ����: Ŀ����봮����
int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
	int nSrc;        // Դ�ַ����ļ���ֵ
	int nDst;        // Ŀ����봮�ļ���ֵ
	int nChar;        // ��ǰ���ڴ���������ַ��ֽڵ���ţ���Χ��0-7
	unsigned char nLeft;    // ��һ�ֽڲ��������

	// ����ֵ��ʼ��
	nSrc = 0;
	nDst = 0;

	// ��Դ��ÿ8���ֽڷ�Ϊһ�飬ѹ����7���ֽ�
	// ѭ���ô�����̣�ֱ��Դ����������
	// ������鲻��8�ֽڣ�Ҳ����ȷ����
	while (nSrc < nSrcLength)
	{
		// ȡԴ�ַ����ļ���ֵ�����3λ
		nChar = nSrc & 7;

		// ����Դ����ÿ���ֽ�
		if(nChar == 0)
		{
			// ���ڵ�һ���ֽڣ�ֻ�Ǳ�����������������һ���ֽ�ʱʹ��
			nLeft = *pSrc;
		}
		else
		{
			// ���������ֽڣ������ұ߲��������������ӣ��õ�һ��Ŀ������ֽ�
			*pDst = (*pSrc << (8-nChar)) | nLeft;

			// �����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������
			nLeft = *pSrc >> nChar;

			// �޸�Ŀ�괮��ָ��ͼ���ֵ
			pDst++;
			nDst++;
		}

		// �޸�Դ����ָ��ͼ���ֵ
		pSrc++;
		nSrc++;
	}

	// ����Ŀ�괮����
	return nDst;
}

// 7bit����
// ����: pSrc - Դ���봮ָ��
//       nSrcLength - Դ���봮����
// ���: pDst - Ŀ���ַ���ָ��
// ����: Ŀ���ַ�������
int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
	int nSrc;        // Դ�ַ����ļ���ֵ
	int nDst;        // Ŀ����봮�ļ���ֵ
	int nByte;        // ��ǰ���ڴ���������ֽڵ���ţ���Χ��0-6
	unsigned char nLeft;    // ��һ�ֽڲ��������

	// ����ֵ��ʼ��
	nSrc = 0;
	nDst = 0;

	// �����ֽ���źͲ������ݳ�ʼ��
	nByte = 0;
	nLeft = 0;

	// ��Դ����ÿ7���ֽڷ�Ϊһ�飬��ѹ����8���ֽ�
	// ѭ���ô�����̣�ֱ��Դ���ݱ�������
	// ������鲻��7�ֽڣ�Ҳ����ȷ����
	while(nSrc<nSrcLength)
	{
		// ��Դ�ֽ��ұ߲��������������ӣ�ȥ�����λ���õ�һ��Ŀ������ֽ�
		*pDst = ((*pSrc << nByte) | nLeft) & 0x7f;

		// �����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������
		nLeft = *pSrc >> (7-nByte);

		// �޸�Ŀ�괮��ָ��ͼ���ֵ
		pDst++;
		nDst++;

		// �޸��ֽڼ���ֵ
		nByte++;

		// ����һ������һ���ֽ�
		if(nByte == 7)
		{
			// ����õ�һ��Ŀ������ֽ�
			*pDst = nLeft;

			// �޸�Ŀ�괮��ָ��ͼ���ֵ
			pDst++;
			nDst++;

			// �����ֽ���źͲ������ݳ�ʼ��
			nByte = 0;
			nLeft = 0;
		}

		// �޸�Դ����ָ��ͼ���ֵ
		pSrc++;
		nSrc++;
	}

	// ����ַ����Ӹ�������
	*pDst = '\0';

	// ����Ŀ�괮����
	return nDst;
}

// 8bit����
// ����: pSrc - Դ�ַ���ָ��
//       nSrcLength - Դ�ַ�������
// ���: pDst - Ŀ����봮ָ��
// ����: Ŀ����봮����
int gsmEncode8bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
	// �򵥸���
	memcpy(pDst, pSrc, nSrcLength);

	return nSrcLength;
}

// 8bit����
// ����: pSrc - Դ���봮ָ��
//       nSrcLength -  Դ���봮����
// ���: pDst -  Ŀ���ַ���ָ��
// ����: Ŀ���ַ�������
int gsmDecode8bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
	// �򵥸���
	memcpy(pDst, pSrc, nSrcLength);

	// ����ַ����Ӹ�������
	*pDst = '\0';

	return nSrcLength;
}

// ����˳����ַ���ת��Ϊ�����ߵ����ַ�����������Ϊ��������'F'�ճ�ż��
// �磺"8613851872468" --> "683158812764F8"
// ����: pSrc - Դ�ַ���ָ��
//       nSrcLength - Դ�ַ�������
// ���: pDst - Ŀ���ַ���ָ��
// ����: Ŀ���ַ�������
int gsmInvertNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
	int nDstLength;        // Ŀ���ַ�������
	char ch;            // ���ڱ���һ���ַ�

	// ���ƴ�����
	nDstLength = nSrcLength;

	// �����ߵ�
	for(int i=0; i<nSrcLength;i+=2)
	{
		ch = *pSrc++;        // �����ȳ��ֵ��ַ�
		*pDst++ = *pSrc++;    // ���ƺ���ֵ��ַ�
		*pDst++ = ch;        // �����ȳ��ֵ��ַ�
	}

	// Դ��������������
	if(nSrcLength & 1)
	{
		*(pDst-2) = 'F';    // ��'F'
		nDstLength++;        // Ŀ�괮���ȼ�1
	}

	// ����ַ����Ӹ�������
	*pDst = '\0';

	// ����Ŀ���ַ�������
	return nDstLength;
}

// �����ߵ����ַ���ת��Ϊ����˳����ַ���
// �磺"683158812764F8" --> "8613851872468"
// ����: pSrc - Դ�ַ���ָ��
//       nSrcLength - Դ�ַ�������
// ���: pDst - Ŀ���ַ���ָ��
// ����: Ŀ���ַ�������
int gsmSerializeNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
	int nDstLength;        // Ŀ���ַ�������
	char ch;            // ���ڱ���һ���ַ�

	// ���ƴ�����
	nDstLength = nSrcLength;

	// �����ߵ�
	for(int i=0; i<nSrcLength;i+=2)
	{
		ch = *pSrc++;        // �����ȳ��ֵ��ַ�
		*pDst++ = *pSrc++;    // ���ƺ���ֵ��ַ�
		*pDst++ = ch;        // �����ȳ��ֵ��ַ�
	}

	// �����ַ���'F'��
	if(*(pDst-1) == 'F')
	{
		pDst--;
		nDstLength--;        // Ŀ���ַ������ȼ�1
	}

	// ����ַ����Ӹ�������
	*pDst = '\0';

	// ����Ŀ���ַ�������
	return nDstLength;
}



void timet2chars(time_t t_time, char* pTime)
{
	int year=0,mon=0,day=0,hour=0,min=0,sec=0;
	struct tm *m_tm;
	m_tm = localtime(&t_time);
	year = m_tm->tm_year+1900;
	mon = m_tm->tm_mon+1;
	day = m_tm->tm_mday;
	hour = m_tm->tm_hour;
	min = m_tm->tm_min;
	sec = m_tm->tm_sec;
	sprintf(pTime, "%04d-%02d-%02d %02d:%02d:%02d", year, mon, day, hour, min, sec);
}

void QuickSortRun(float* pData,int left,int right) 
{ 
    int i,j; 
    float middle,iTemp; 
    i = left; 
    j = right; 
    middle = pData[(left+right)/2]; //���м�ֵ 
    do{ 
      while((pData[i]<middle) && (i<right))//����ɨ�������ֵ���� 
        i++;      
      while((pData[j]>middle) && (j>left))//����ɨ�������ֵ���� 
        j--; 
      if(i<=j)//�ҵ���һ��ֵ 
      { 
        //���� 
        iTemp = pData[i]; 
        pData[i] = pData[j]; 
        pData[j] = iTemp; 
        i++; 
        j--; 
      } 
    }while(i<=j);//�������ɨ����±꽻����ֹͣ�����һ�Σ� 

    //����߲�����ֵ(left<j)���ݹ����� 
    if(left<j) 
      QuickSortRun(pData,left,j); 
    //���ұ߲�����ֵ(right>i)���ݹ��Ұ�� 
    if(right>i) 
      QuickSortRun(pData,i,right); 
} 

void QuickSort(float* pData,int Count) 
{ 
    QuickSortRun(pData,0,Count-1); 
} 

