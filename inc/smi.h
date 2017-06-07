#ifndef _SMI_DEAL_H
#define _SMI_DEAL_H

#ifdef __cplusplus
    extern "C" {
#endif

int ApiSmiMacRead (const HANDLE hDevice, const int PhyId, const int RegNum);
int ApiSmiMacWrite (const HANDLE hDevice, const int PhyId, const int RegNum, const int WriteData);


//�ѱ������ڵĹر�      0x1d    0x8
int PowerDownNetPortOne(const HANDLE hDevice);

//�ѱ��ϵ����ڿ���      0x1d    0x0
int PowerUpNetPortOne(const HANDLE hDevice);

//��������ڵĹر�      0x2d    0x8
int PowerDownNetPortTwo(const HANDLE hDevice);

//����ߵ����ڿ���      0x2d    0x0
int PowerUpNetPortTwo(const HANDLE hDevice);




//�Ź�
int ReleaseWatchDog(void);

//���ÿ��Ź���ʱʱ��
int SetWatchDogTimer(int iSeconds);

//ι��
int FeedWatchDog(void);



#ifdef __cplusplus
    }
#endif


#endif

