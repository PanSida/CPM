#ifndef _ADC_SPI_API_
#define _ADC_SPI_API_ 
 
/**********************************************************************************
�����ơ��� adc_spi.c
�����ܡ��� ���ϲ���õ�Api
���޸ġ��� 1.2011-05-18 1.0.0 Created By Microsys Access Gateway 
����ء��� 
**********************************************************************************/

#define DEBUG_SPI_API   0x1 //���Եȼ�
 
//��ʼ��
	int ApiSpiOpen(const char *c_pchPathName, int iFlags);

	int ApiSpiClose(const int c_iHandle);


	//��ADC 
	int ApiReadADC(const int c_iHandle, float *pfRecvBuf, const int c_iBufLen, const int c_iADCIndex);


	//������
	int ApiSerialRead(const int c_iHandle, unsigned char *pchRecvBuf, const int c_iBufLen, const int c_iSerialNum);

	//д����
	int ApiSerialWrite(const int c_iHandle, unsigned char *pchSendBuf, const int c_iBufLen, const int c_iSerialNum);

	//��Gpio
	int ApiReadGpio(const int c_iHandle, unsigned char *pucRecvBuf, const int iBufLen, const int c_iGpioIndex);


	/**********************************************************************************
	���������� ApiWriteGpioAI
	�����ܡ��� ����GPIO  AI
	���������� 1.[in] const int c_iHandle,      
				2.[in] const int c_iFlag,       [0,1]      1-��       0-��
				3.[in] const int c_iAIIndex     [1,4]    
				//PC4  	AI1-Light  //PC5  	AI2-Light  //PB0  	AI3-Light  //PB1  	AI4-Light
				//��λ��ʾ���ֵBIT0~BIT3.��ӦλΪ1����1. 0��Ч
	�����ء��� int , ʵ��д���ֽ���
	**********************************************************************************/
	int ApiWriteGpioAI(const int c_iHandle, const int c_iFlag, const int c_iAIIndex);



	/**********************************************************************************
	���������� ApiWriteGpioGdo
	�����ܡ��� ����GPIO  GDO
	���������� 1.[in] const int c_iHandle,      
								   2.[in] const int c_iData,            [0,1]      1-��       0-��
								   3.[in] const int c_iGdoIndex     [1,8]    
									//PA8  G-DO1  //PC9  G-DO2 //PC8 G-DO3	 //PC7  G-DO4 //PB12 	G-12VCC1 
									//RS485L-1       //RS485L-2      //RS485L-3
					//��λ��ʾ���ֵBIT0~BIT4 ��ӦλΪ1����1. 0��Ч
	                               
	�����ء��� int , ʵ��д���ֽ���
	**********************************************************************************/
	int ApiWriteGpioGdo(const int c_iHandle, const int c_iFlag, const int c_iGdoIndex);



	/**********************************************************************************
	���������� ApiWriteGpioDI
	�����ܡ��� ����GPIO  DI
	���������� 1.[in] const int c_iHandle,      
				2.[in] const int c_iData,       [0,1]      1-��       0-��
				3.[in] const int c_iAIIndex     [1,4]    
				//PC10  DI4-Light  //PC11  DI3-Light  //PC12  DI2-Light  //PD2  DI1-Light
				//��λ��ʾ���ֵBIT0~BIT3��ӦλΪ1����1. 0��Ч
	                               
	�����ء��� int , ʵ��д���ֽ���
	**********************************************************************************/
	int ApiWriteGpioDI(const int c_iHandle, const int c_iFlag, const int c_iDIIndex);




	/**********************************************************************************
	���������� ApiWriteBaudRate
	�����ܡ��� ���ò�����
	���������� 1.[in] const int c_iHandle,      
								   2.[in] const unsigned char c_ucIndex,         [1,8] 
								   ��8Ϊֵ��ʾҪ���õ�BaudRate   �����������Ĭ��9600
								   1  '1200
								   2  '2400
								   3  '4800
								   4  '9600
								   5  '19200
								   6  '38400
								   7  '57600
								   8  '115200
								   ����'9600
								   3.[in] const int c_iSerialIndex      {0,1,2} ------ {����1������2������3}   
	�����ء��� int , ʵ��д���ֽ���
	���޸ġ��� 1.2011-05-19  1.0.0 Created By Microsys Access Gateway 
								   2.2011-05-19  1.0.0 Edit By ZhouX 
	����ء��� 
	**********************************************************************************/
	int ApiWriteBaudRate(const int c_iHandle, const int c_iBaudRateIndex, const int c_iSerialIndex);




	/**********************************************************************************
	���������� ApiWriteOthers
	�����ܡ��� ����ֹͣλ[1,2]
	���������� 1.[in] const int c_iHandle,      
								   2.[in] const int c_iStopBit,         [1,2] 
								   3.[in] const int c_iOddEvent     {0,1,2}----- {�ޣ��棬ż}
								   ��8Ϊֵ��ʾҪ���õĲ���   �����������Ĭ��ֹͣλ1������żУ��
								   ֹͣλ   Bit1  Bit0
									 1       0    0
									 2       0    1
									 1       ����
	                               
								   ��żλ   Bit3  Bit2
									 ��      0    0
									 ��      0    1
									 ż      1    0
									 ��       ����
								   4.[in] const int c_iSerialIndex      {0,1,2} ------ {����1������2������3}
	�����ء��� int , ʵ��д���ֽ���
	**********************************************************************************/
	int ApiWriteOthers(const int c_iHandle, const int c_iStopBit, const int c_iOddEvent, const int c_iSerialIndex);




	/**********************************************************************************
	���������� ApiReadRts
	�����ܡ��� ��RTS
					����RTS״̬���������ֱ�����������ڣ��õ�8λ����ʾ��
					BIT0, BIT1, BIT2�ֱ��������״̬λ��1Ϊ�ߣ�0Ϊ��
	���������� 1.[in] const int c_iHandle,      
								   2.[in] unsigned char *pucRecvBuf,     ��ȡ�����ݴ�ŵĵ�ַ   
								   3.[in] const int iBufLen    ��Ҫ��ȡ���ֽ�����1��
	�����ء��� int , ʵ�ʶ�ȡ�����ֽ���                               
	**********************************************************************************/
	int ApiReadRts(const int c_iHandle, unsigned char *pucRecvBuf, const int iBufLen);




	/**********************************************************************************
	���������� ApiReadMcuVersion
	�����ܡ��� ��Mcu�汾
	���������� 1.[in] const int c_iHandle,      
								   2.[in] unsigned char *pucRecvBuf,     ��ȡ�����ݴ�ŵĵ�ַ   
								   3.[in] const int iBufLen    ��Ҫ��ȡ���ֽ�����1��
	�����ء��� int , ʵ�ʶ�ȡ�����ֽ���                               
	**********************************************************************************/
	int ApiReadMcuVersion(const int c_iHandle, unsigned char *pucRecvBuf, const int iBufLen);







	/**********************************************************************************
	���������� ApiClearFIFO
	�����ܡ��� ���arm7����
	���������� 1.[in] const int c_iHandle,      
	�����ء��� int , ʵ��д���ֽ���
	���޸ġ��� 1.2011-05-31  1.0.0 Created By ZhouX
	����ء��� 
	**********************************************************************************/
	int ApiClearFIFO(const int c_iHandle);


	/**********************************************************************************
	���������� ApiReadRTC
	�����ܡ��� 
	����������  1.[in]      const int c_iHandle,      
									2.[out]   unsigned char *ucRecvBuf,  
									3.[in]      const int c_iRtcIndex,  
	�����ء��� int , ʵ�ʶ������ֽ���
	���޸ġ��� 1.2011-06-14  1.0.0 Created By ZhouX
	����ء��� c_iRtcIndex [0, 6]   �ֱ����
	enum{
		E_Second = 0, 
		E_Minute,
		E_Hour,
		E_Date,
		E_Month,
		E_Day,   
		E_Year,  
	}
	**********************************************************************************/
	int ApiReadRTC(const int c_iHandle, const int c_iRtcIndex, unsigned char *ucRecvBuf);



	/**********************************************************************************
	���������� ApiWriteRTC
	�����ܡ��� 
	���������� 1.[in] const int c_iHandle,      
	�����ء��� int , ʵ��д���ֽ���
	���޸ġ��� 1.2011-06-14  1.0.0 Created By ZhouX
	����ء��� c_iRtcIndex [0, 6]   �ֱ����
	enum{
		E_Second = 0, 
		E_Minute,
		E_Hour,
		E_Date,
		E_Month,
		E_Day,   
		E_Year,  
	}
	**********************************************************************************/
	int ApiWriteRTC(const int c_iHandle, const int c_iRtcIndex, const unsigned char c_ucValue);


	int ApiSpiUpdateRead(const int c_iHandle, unsigned char *pchRecvBuf, const int c_iBufLen, const int c_iSerialNum);
	int ApiSpiUpdateWrite(const int c_iHandle, unsigned char *pchSendBuf, const int c_iBufLen, const int c_iSerialNum);

	int ApiClearFIFONew(const int c_iHandle, const int c_index);
#endif

