#ifndef __ADC_BASE_H__
#define __ADC_BASE_H__

typedef enum _ADC_BAUDRATE
{
	ADC_BAUDRATE_1200 = 1,
	ADC_BAUDRATE_2400,
	ADC_BAUDRATE_4800,
	ADC_BAUDRATE_9600,
	ADC_BAUDRATE_19200,
	ADC_BAUDRATE_38400,
	ADC_BAUDRATE_57600,
	ADC_BAUDRATE_115200
}ADC_BAUDRATE;
class CADCBase {
	
public :
	CADCBase(int mode);
    ~CADCBase();
    bool Initialize(char* devName);
	void Terminate();
	bool SetBaudRate(int serial_index, ADC_BAUDRATE baudrate);
    bool SetSerialOpt(int serial_index, const int c_iStopBit, const int c_iOddEvent);

	int SerialRecv(int serial_index, unsigned char* outBuf, int needlen, int timeout);//�������ݶ�ȡ
	int SerialSend(int serial_index, unsigned char* inBuf, int bufLen);//��������д��

	bool ADGet(int AD_Index, float &outValue);//ģ������ȡ
    void DOAISet( int DO_Index, int status);  //ģ�����ڵƿ���
	bool DIGet(int DI_Index, unsigned char &outValue);//�����������ȡ
    void DODISet( int DO_Index, int status);//��������״̬����

	void DOSet(int DO_Index, int outValue);//���������
	void DOSetE(int DO_Index, int outValue);//�̵�����Դ�������CADCBase��mode��������modeΪ1����ú�����Ч
	static int f_getBaudRateEnum(unsigned int baudrate);
private:
	int m_iMode;
	int m_hDevHandle;
	float m_ADPara[4];
};
#endif

