
#ifndef YkPosdll
#define YkPosdll


typedef int (__stdcall *CallBack)(unsigned char  pParam);

#ifdef __cplusplus
extern "C"
{
#endif


#define POS_SUCCESS                     1001 // ����ִ�гɹ�

#define POS_FAIL                        1002 // ����ִ��ʧ��

#define POS_ERROR_INVALID_HANDLE        1101 // �˿ڻ��ļ��ľ����Ч

#define POS_ERROR_INVALID_PARAMETER     1102 // ������Ч

#define POS_ERROR_NOT_BITMAP            1103 // ����λͼ��ʽ���ļ�

#define POS_ERROR_NOT_MONO_BITMAP       1104 // λͼ���ǵ�ɫ��

#define POS_ERROR_BEYONG_AREA           1105 // λͼ������ӡ������
											 // ����Ĵ�С

#define POS_ERROR_INVALID_PATH          1106 // û���ҵ�ָ�����ļ�
											 // ·��������

#define POS_COM_DTR_DSR 0x00 // ������ΪDTR/DST
  
#define POS_COM_RTS_CTS 0x01 // ������ΪRTS/CTS 

#define POS_COM_XON_XOFF 0x02 // ������ΪXON/OFF 

#define POS_COM_NO_HANDSHAKE 0x03 // ������ 

#define POS_OPEN_PARALLEL_PORT 0x12 // �򿪲���ͨѶ�˿� 

#define POS_OPEN_USB_PORT 0x13 // ��USBͨѶ�˿� 

#define POS_OPEN_PRINTNAME 0X14 //�򿪴�ӡ����������

#define POS_OPEN_NETPORT 0X15 // ����̫����ӡ��

#define POS_FONT_TYPE_STANDARD 0x00 // ��׼ ASCII
 
#define POS_FONT_TYPE_COMPRESSED 0x01 // ѹ�� ASCII 
 
#define POS_FONT_TYPE_UDC 0x02 // �û��Զ����ַ�
 
#define POS_FONT_TYPE_CHINESE 0x03 // ��׼ �����塱
 
#define POS_FONT_STYLE_NORMAL 0x00 // ����
 
#define POS_FONT_STYLE_BOLD 0x08 // �Ӵ�
 
#define POS_FONT_STYLE_THIN_UNDERLINE 0x80 // 1��ֵ��»���
 
#define POS_FONT_STYLE_THICK_UNDERLINE 0x100 // 2��ֵ��»���
 
#define POS_FONT_STYLE_UPSIDEDOWN 0x200 // ���ã�ֻ��������Ч��
 
#define POS_FONT_STYLE_REVERSE 0x400 // ���ԣ��ڵװ��֣�
 
#define POS_FONT_STYLE_SMOOTH 0x800 // ƽ���������ڷŴ�ʱ��
 
#define POS_FONT_STYLE_CLOCKWISE_90 0x1000 // ÿ���ַ�˳ʱ����ת 90 ��

#define POS_PRINT_MODE_STANDARD 0x00 // ��׼ģʽ����ģʽ��
 
#define POS_PRINT_MODE_PAGE 0x01 // ҳģʽ
 
#define POS_PRINT_MODE_BLACK_MARK_LABEL 0x02 // �ڱ�Ǳ�ǩģʽ(���ִ�ӡ������Ӳ��֧��)


#define POS_PRINT_MODE_WHITE_MARK_LABEL 0x03 //�ױ�Ǳ�ǩģʽ (���ִ�ӡ������Ӳ��֧��)
#define POS_PRINT_MODE_VIRTUAL_PAGE 0x04 //����ҳģʽ����̬��������棩


#define POS_BARCODE_TYPE_UPC_A 0x41 // UPC-A
 
#define POS_BARCODE_TYPE_UPC_E 0x42 // UPC-C
 
#define POS_BARCODE_TYPE_JAN13 0x43 // JAN13(EAN13)
 
#define POS_BARCODE_TYPE_JAN8 0x44 // JAN8(EAN8)
 
#define POS_BARCODE_TYPE_CODE39 0x45 // CODE39
 
#define POS_BARCODE_TYPE_ITF 0x46 // INTERLEAVED 2 OF 5
 
#define POS_BARCODE_TYPE_CODEBAR 0x47 // CODEBAR
 
#define POS_BARCODE_TYPE_CODE93 0x48 // 25
 
#define POS_BARCODE_TYPE_CODE128 0x49 // CODE 128
 
#define POS_FONT_TYPE_STANDARD 0x00 // ��׼ASCII 
 
#define POS_FONT_TYPE_COMPRESSED 0x01 // ѹ��ASCII
 
#define POS_HRI_POSITION_NONE 0x00 // ����ӡ
 
#define POS_HRI_POSITION_ABOVE 0x01 // ֻ�������Ϸ���ӡ
 
#define POS_HRI_POSITION_BELOW 0x02 // ֻ�������·���ӡ
 
#define POS_HRI_POSITION_BOTH 0x03 // �����ϡ��·�����ӡ
 
#define POS_BITMAP_PRINT_NORMAL 0x00 // ����
 
#define POS_BITMAP_PRINT_DOUBLE_WIDTH 0x01 // ����
 
#define POS_BITMAP_PRINT_DOUBLE_HEIGHT 0x02 // ����
 
#define POS_BITMAP_PRINT_QUADRUPLE 0x03 // �����ұ���
 
#define POS_CUT_MODE_FULL 0x00 // ȫ��
 
#define POS_CUT_MODE_PARTIAL 0x01 // ����


#define POS_CUT_MODE_ALL 0x02 //�������/ȫ�е���Ū��ֱ����ֽ

 
#define POS_AREA_LEFT_TO_RIGHT 0x0 // ���Ͻ�
 
#define POS_AREA_BOTTOM_TO_TOP 0x1 // ���½�
 
#define POS_AREA_RIGHT_TO_LEFT 0x2 // ���½�
 
#define POS_AREA_TOP_TO_BOTTOM 0x3 // ���Ͻ�



#define POS_BITMAP_MODE_8SINGLE_DENSITY 0x00 // 8�㵥�ܶ� 
#define POS_BITMAP_MODE_8DOUBLE_DENSITY 0x01 // 8��˫�ܶ� 
#define POS_BITMAP_MODE_24SINGLE_DENSITY 0x20 // 24�㵥�ܶ� 
#define POS_BITMAP_MODE_24DOUBLE_DENSITY 0x21 // 24��˫�ܶ� 



#define	PRINTER_TYPE 1   ///  1  �嵥��ӡ���ظɲ� 0   ��Ʊ��ӡ��


///����
#define COM1	1
#define COM2	2
#define COM3	3
#define COM4	4
#define COM5	5
#define COM6	6
#define COM7	7
#define COM8	8
#define COM9	9
#define COM10	10
///����
#define LPT1	11
#define LPT2	12

///USB ��
#define USB		13

///(���߻�����WIFI)���� ��ӡ��
#define NET		14



#define S_COMMUNICATION_OK	0   ///����
#define S_COMMUNICATION_FAILED	-1010    ///ͨѶ����
#define S_DEVICE_FAULT	1011    ///�豸����
#define S_PAPER_OUT	1012    ////ȱֽ
#define S_PAPER_JAM		1013     ///��ֽ
#define S_PAPER_NEAR_END	1014   ///ֽ����
#define S_COVER_OPEN  1015   ///���ǣ�̧�ˣ���  
#define S_DEVICE_BUSY	1016    ////�豸æ





//�������ܣ�ͨ��ϵ�к����Ӵ�ӡ���豸,
//����: szSerial -- �豸ϵ�к� 
//�������ͣ��ַ���ָ�룬�����12��,�����ִ�Сд
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
//�������ͣ�����

int OpenUSBDeviceBySerial(char *szSerial);


//�������ܣ�ö���������������Ĵ�ӡ���豸ϵ�к�
//����: szSerial -- �豸ϵ�к����飬����������Ԥ�ȷ����㹻���ڴ�ռ� ����200 �ֽ�
//�������ͣ��ַ���ָ��
//����ֵ��>0 ����ö���豸��   -1  --  ʧ��
//�������ͣ�����

//int EnumUSBDeviceSerials(char *szSerials[10]);
int EnumUSBDeviceSerials(char szSerials[10][20]);



//HANDLE OpenUsbPrinterByName(LPCTSTR PrinterName);
HANDLE OpenUsbPrtByName(LPCTSTR PrinterName);



//�������ܣ����Ӵ�ӡ���豸
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkOpenDevice(int iport, int baud);

//�������ܣ��Ͽ���ӡ�豸
//��������
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkCloseDevice();

//�������ܣ���ȡ�豸�Ĳ������
//��������
//����ֵ��>0 -- ���ֵ   -1  --  ʧ��
int __stdcall YkGetDeviceHandle();


//�������ܣ��豸�Ƿ��Ѿ������Ӽ����(��֧��USB�ڴ�ӡ��)
//��������
//����ֵ��>0 -- ����   -1  --  û������
int __stdcall YkIsConnected();



//�������ܣ���ʼ����ӡ�� ��������ESC @��
//��������
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkInitPrinter();

//�������ܣ��ѽ�Ҫ��ӡ���ַ��������ӡ��������(ע: �ﵽ����ʱ���Զ���ӡ����)
//������pstr -- ��Ҫ�ʹ�ӡ�����ַ������ݻ��壬len -- �ַ������ݳ���
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkPrintStr(char *pstr);

//�������ܣ���ӡ���س�,������ֽ��������CR��
//��������
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkEnter();

//�������ܣ���ӡ�����У���ֽ����һ���ס�������LF��
//��������
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkFeedPaper();

//�������ܣ�ҳģʽ�£�ȡ����ӡ���ݡ�������CAN��
//��������
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int  __stdcall YkCancelPrintInPM();

//�������ܣ�ʵʱ��Ӧ��������,  ��������DLE ENQ n��
//������n=1���Ӵ���״̬�ָ����Ӵ���״̬���ֵ������¿�ʼ��ӡ��
//������n=2��������ջ���������ӡ���������ݡ�
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkResponse(int n);

//�������ܣ�Tabˮƽ��λ,����һ��ˮƽ��λ��λ�ÿ�ʼ��ӡ����������HT��
//�������� 
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkTabMove(); 

//�������ܣ�ҳģʽ�£���ӡ���ݡ�������ESC FF��
//�������� 
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkPrintInPM();          

//�������ܣ����������ַ��Ҽ�࣬�԰��Ϊ�趨��λ��������ESC SP n��
//������n = 0~255 ,default = 0
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetCharRightSpace(int n=0);

//�������ܣ������ַ���ӡ��ʽ��������ESC ! n��
//������n = 0~255 ,default = 0
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetFontStyle(int n=0);

//�������ܣ����þ��Դ�ӡλ�ã����������:ESC $ nL nH��
//������nL -- λ����ֵ�ĵ��ֽڣ�nH -- λ����ֵ�ĸ��ֽ� ,0 �� (nL + nH �� 256) �� 65535 (0 �� nL �� 255 , 0 �� nH �� 255)
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int YkSetAbsPrnPos(int nL,int nH);

//�������ܣ�ʹ��/�����û��Զ����ַ������������:ESC % n��
//������n = 0~255 ,default = 0
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkEnableUserDefineChar(int n=0);

//�������ܣ������û��Զ����ַ��� ��������ESC & y c1 c2 [ x1 d1...d(y��x1)]...[ xk d1...d(y��xk)]��
//������c1, c2�ֱ�Ϊ��ʼ�����ֹ�룬���Ϊ95����
//�������ַ���ȣ��Ե�Ϊ��λ������ʲô�����С����
//������code --�Զ����ַ�������ֵ������ʹ��Font B����,�ѿո���Ϊ�ڿ� ����Ϊc1=0x20��c2=0x20��code[0]=0xff��code[1]=0xff...code[26]=0xff
// YkSetUserDefineChar(0x20,0x20,9,code);
///�����������˵����
//y==3
//0 �� x �� 12 [Font A (12 �� 24)]
//0 �� x �� 9 [Font B (9 �� 17)]
//k = c2 �C c1 +1
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetUserDefineChar(int c1,int c2,int x, unsigned char code[]);

//�������ܣ�ʹ�ܻ�����ַ����»��߹���   ��������ESC - n��
//������n=1 ʹ�ܣ�n=0 ���ã�Ĭ��ֵn��0��  
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkEnableUnderLine(int n=0);           

//�������ܣ������ַ��о�ΪĬ��ֵ3.75���� ��������ESC 2��
//��������
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetDefaultLineSpace();

//�����ַ��о� ��������ESC 3 n��
//������n=0~255,Ĭ�� n=30
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetLineSpace(float fLineSpace=3.75);

//�������ܣ�ȡ���û��Զ����ַ�, ��������ESC ? n��
//������n=32~126
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkCancelUserDefChar(int n);  

//�������ܣ�����ˮƽ�Ʊ�λ�� ��������ESC D��
//������tabstr --tab λ��ֵ��ɵ��ַ������У���
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetTabPos(char * tabstr);   

//�������ܣ�ʹ�ܻ���ü��ش�ӡģʽ �������� ESC E n��
//������n =0 ���� n=1 ʹ��
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetEmphasized(int n);

//�������ܣ�ʹ�ܻ�����ص���ӡģʽ �������� ESC G��
//������n =0 ���� n=1 ʹ��
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkOverlap(int n);

//�������ܣ���ӡ����ֽ n ����(ע�����Ծ�ȷ��ֽ 1����=0.125mm) �������� ESC J n��
//������n =0~255
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkPrnAndFeedPaper(int n);

//�������ܣ�����ҳģʽ���� ��������ESC L��
//��������
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkEnablePM();

//�������ܣ�ѡ���ַ�����  ��������ESC M��
//������n=0  ѡ������ A (12��24); n=1 ѡ������ B (9��17)
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSelectFont(int n);

//�������ܣ�����11�����ҵĲ�ͬASCII�ַ��� ��������ESC R n��
//������ n ���Ҵ���
/*
	n	 International character set

	0	 U.S.A.
	1	 France
	2	 Germany
	3	 U.K.
	4	 Denmark I
	5	 Sweden
	6	 Italy
	7	 Spain I
	8	 Japan
	9	 Norway
	10	 Denmark II
	11	 Spain II
	12	 Latin America
	13	 Korea
	14	 Slovenia / Croatia
	15	 China
*/
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetInterCharSet(int n=0);

//�������ܣ���ҳģʽ�л�����׼ģʽ ��������ESC S��
//��������
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkEnterSM();

//�������ܣ�ҳģʽ�£����ô�ӡ����,  ��������ESC T n��
//������n=0~3 ; 
/*
	n	Print direction		Starting position
	0	Left to right		Upper left
	1	Bottom to top		Lower left
	2	Right to left		Lower right
	3	Top to bottom		Upper right
*/
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetDirectionInPM(int n);

//�������ܣ�ʹ�ܻ����˳ʱ��90����ת�ַ���ӡ,  ��������ESC V n��
//������n=0����˳ʱ��90����ת��ӡ n=1 ʹ��˳ʱ��90����ת��ӡ
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkClockwiseD90(int n);

//�������ܣ�ҳģʽ�£��������ô�ӡ���򣬴�ӡҳ���ȷ�Χ(76~185mm),��ӡ���(���72mm) ��������ESC W xL xH yL yH dxL dxH dyL dyH��
//������left ��ӡ�������Ͻ�x���� top ��ӡ�������Ͻ�y���� width  ��ӡ������ height ��ӡ����߶�
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetPrnAreaInPM(int left,int top,int width,int height);

//�������ܣ�������Դ�ӡλ��     ��������ESC \ nL nH��
//������nL ,nH ;  ʵ��λ��= (nL + nH x 256)x0.125 ����
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetRelPrnPos(int nL,int nH);

//�������ܣ����ô�ӡʱ�Ķ��뷽ʽ   ��������ESC  a n��
//������n=0 �����    n=1 ����  n=2	�Ҷ���
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetAlign(int n);

//�������ܣ����ò�ֽ���������ȱֽ�ź�   ��������ESC c 3 n��
//������n=0~255 , ��λ���壬�������˵������˵��
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetPaperSensor(int n);

//�������ܣ�����ֽ��ʱֹͣ��ӡ    ��������ESC c 4 n��
//������n=0~255 , ��λ���壬�������˵������˵��
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetSensorToStopPrint(int n);

//�������ܣ�ʹ�ܻ���ô�ӡ������ϵĿ���  ��������ESC c 5 n��
//������n=0 ���� n=1 ʹ��
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkEnablePanelButton(int n=0);

//�������ܣ���ӡ����ֽn�ַ���   ��������ESC d n��
//������n �ַ�����
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkPrnAndFeedLine(int n); 

//�������ܣ������ַ������      ��������ESC t n��
//������n �������� ȡֵ��Χ��0 �� n �� 5 , 16 �� n �� 19 , n = 255
/*
n	Character code table
0	Page 0 [PC437 (USA: Standard Europe)]
1	Page 1 [Katakana]
2	Page 2 [PC850 (Multilingual)]
3	Page 3 [PC860 (Portuguese)]
4	Page 4 [PC863 (Canadian-French)
5	Page 5 [PC865 (Nordic)]
16	Page 16 [WPC1252]
17	Page 17 [PC866 (Cyrillic #2)]
18	Page 18 [PC852 (Latin 2)]
19	Page 19 [PC858 (Euro)]
255 Page 255 [User-defined page]
*/
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetCharCodeTable(int n);

//�������ܣ��ߺڱ�ֽ����ӡ��ʼλ��    ��������GS FF��
//��������
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkFeedToStartPos();

//�������ܣ������ַ�����             ��������GS ! n��
//������hsize ˮƽ�Ŵ��� vsize ��ֱ���ڱ��� ,ȡֵ��Χ0~7 
/*
hsize ,vsize ֵ�뱶����Ӧ��ϵ
0	1����ԭ��С��
1	2��
2	3��
3	4��
4	5��
5	6��
6	7��
7	8��
*/
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetCharSize(int hsize,int vsize);

//�������ܣ�ҳģʽ�£����ô�ӡ�����ھ��Դ�ֱ��ӡ��ʼλ��    ��������GS $ nL nH��
//������nL nH λ��ֵ  λ�� = (nL + nH x256) x 0.125 ����
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int YkSetAbsVertPosInPM(int nL,int nH);

//�������ܣ����в��Դ�ӡ         ��������GS ( A pL pH n m��
//������n,m
/*
	n ָ������ʱ��ֽ����Դ

	0	Basic sheet (roll paper)
	1	Roll paper
	2	Roll paper

	m ָ��������ʽ

	1	Hexadecimal dump print
	2	Printer status print
	3	Rolling pattern print

*/
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkDoTestPrint(int n,int m);



//�������ܣ�������˳��û�����ģʽ    ���������:GS ( E pL pH ��
//������m=1 ����  m=2 �˳�
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkUserDefCmd(int m);



//�������ܣ�����MemorySwitch����                    ���������:GS ( E pL pH �� 
//������n=0~7��ӦMemorySwitch����1��8��
//����������memory[]��Ӧmemory1~8ÿ������ֵ��             
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetMemorySwitch(int n, int memory[]);  ///???

//�������ܣ���ȡMemorySwitch����ֵ      ���������:GS ( E pL pH��
//������n=0~7��ӦMemorySwitch����1��8��
//����������memory[]���ڴ洢��������Ӧmemory1~8ÿ������ֵ��             
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkGetMemorySwitch(int n,unsigned char memory[11]); ///???

//�������ܣ����úڱ����˺ֽλ�ú���ʼ��ӡλ��        ���������:GS ( F pL pH a m��
//����a =1 ������ʼ��ӡλ�õ��趨ֵ a=2 ���ÿ�ʼ��ֽλ�õ��趨ֵ
//����m = 0 ָ��Ϊǰ���ֽ�ķ��� m = 1 ָ��Ϊ�����ֽ�ķ���
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetBlackMarkParam(int a,int m,int nL,int nH);

//�������ܣ�(ע��������״̬����������ͬ)��������FF��
//	ҳģʽ����ӡ�󷵻ص���׼ģʽ
//	�ڱ�ģʽ����ӡ���ߺڱ굽��ӡ��ʼλ�á� 
//��������
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkPrnAndBackToStd();

//�������ܣ����ƴ�ӡ������ֵ��         ���������:GS ( M pL pH a n m��
//������
/*
	�������������������������ݡ�

	n	����

	1	������GS ( F �����õ����ݱ��浽�û�NV �洢����
	2	���û�NV �洢����������GS ( F �����õ����ݡ�
	3	ָ���ڳ�ʼ�趨ʱ��ֹ�������Զ������������

	m ָ���������£�

	m = 0	��ù��ο��ֲ���������GS ( F ����ĳ�ʼ�趨ֵ��ͬ��
	m = 1	��������Ĵ洢����
*/
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkDefineControl(int n,int m);

//�������ܣ���������HRI�ַ��Ĵ�ӡλ��             ���������:GS H n��
//������n=0 ����ӡ,n=1 �������Ϸ�,n=2�������·�,n=3��������Ϸ����·�
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetHRIPos(int n=0);

//�������ܣ���ȡ��ӡ��ID         ���������:GS I n��
//������n=1~3   ��ͬ�ͺŻ��ͣ��������ݲ�ͬ�����˵����
/*
n	��ӡ��ID
1	��ӡ���ͺ�ID 
2	����ID ��˵�������ͱ�
3	�̼��汾ID 
*/
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkGetPrinterID(int n);

//�������ܣ�������߾�             ���������:GS L nL nH��
//������nL nH  ��߾�=(nL + nH x 256) x 0.125 ����
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetLeftMargin(int nL,int nH);


//�������ܣ���ӡλ������Ϊ��ӡ�����         ���������:GS T n��
//������n=0 ɾ����ӡ�������е��������ݺ����ô�ӡλ��Ϊ��ӡ����ʼ�� n=1 ����ӡ�������е��������ݴ�ӡ�����ô�ӡλ��Ϊ��ӡ����ʼ��
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkToLineHome(int n);

//�������ܣ�ִ����ֽ������������ֽ    ��������GS V m��
//������m=66 n:��ӡ����ֽ��(��ֽλ��+ [n �� 0.125 ����{0.0049Ӣ��}])����ֽ��һ��n=0
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkCutPaper(int m,int n);

//�������ܣ����ô�ӡ�����ȡ�           ��������GS W nL nH��
//������nL --��ӡ�����ȵ��ֽ� nH -- ��ӡ�����ȸ��ֽ�
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetPrnAreaWidth(int nL,int nH);

//�������ܣ�ҳģʽ�£���������ڵ�ǰλ�õĴ�ֱ��ӡ���λ��  ��������GS \ nL nH��
//������nL --λ�õ��ֽ� nH -- λ�ø��ֽ�
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetRelVertPosInPM(int nL,int nH);

//�������ܣ�ʹ�ܻ�����Զ�״̬�ظ�����(ASB)        ��������GS a n��
//������0 -- ����  1 -- ʹ��
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkEnableASB(int n);

//�������ܣ�ʹ�ܻ����ƽ��ģʽ       ��������GS b n��
//������0 -- ����  1 -- ʹ��
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkEnableSmoothPrn(int n);

// �������ܣ����������HRI�ַ�����          ��������GS f n��
//������0 -- ����A (12 �� 24)  1 -- ����B (9 �� 17)
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetHRICharStyle(int n);

//�������ܣ���������߶�             ��������GS h n��
//������n=1~255  ��ֱ����ĵ���
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetBarCodeHeight(float fHeight);

//�������ܣ���ӡ����                  ��������GS k m n d1 ... dn��
//������m=65~75
/*
m	������ϵͳ		�ַ�����			��ע
65	UPC-A			11 �� n �� 12		48 �� d �� 57
66	UPC-E			11 �� n �� 12		48 �� d �� 57
67	JAN13 (EAN13)	12 �� n �� 13		48 �� d �� 57
68	JAN8 (EAN8)		7 �� n �� 8			48 �� d �� 57
69	CODE39			1 �� n �� 255		48 �� d �� 57, 65 �� d �� 90, 32, 36,37, 43, 45, 46, 47
70	ITF				1 �� n �� 255 (n Ϊż��) 48 �� d �� 57
71	CODABAR			1 �� n �� 255		48 �� d �� 57, 65 �� d �� 68, 36, 43,45, 46, 47, 58
72	CODE93			1 �� n �� 255		0 �� d �� 127
73	CODE128			1 �� n �� 255		0 �� d �� 127
74	��׼EAN13		12 �� n �� 13		48 �� d �� 57
75	��׼EAN8		7 �� n �� 8			48 �� d �� 57
*/
//������n �ϱ����ַ�����
//������barcode Ҫת��Ϊ���������
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkPrintBarCode(int m,int n,char * barcode);

//�������ܣ���ȡ��ӡ��״̬,   ��������DLE EOT  n��
//������n = 1~5 
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkGetStatus(unsigned char n);

//�������ܣ���ȡ��ӡ��״̬                   ��������GS r n��
//������n=1 ���ʹ�ӡֽ������״̬
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkGetPrinterStatus(unsigned char n);

//�������ܣ�����������              ��������GS w n��
//����������n=2~6
/*
											��Ԫ������
  n		��Ԫ�����뵥λ���(����)	խ�����(����)		�������(����)
  2		0.250						0.250				0.625
  3		0.375						0.375				1.000
  4		0.560						0.500				1.250
  5		0.625						0.625				1.625
  6		0.750						0.750				2.000
*/
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetBarCodeWidth(float fWdith);

//�������ܣ����ú����ַ���ӡģʽ��ϡ���������FS ! n��
//������n=0~255 �������˵�����б��
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetChineseFontStyle(int n);

//�������ܣ����뺺�ִ�ӡ��ʽ����������FS &��
//��������
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkEnableChineseMode();


//�������ܣ��˳����ִ�ӡ��ʽ����������FS .��
//��������
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkDisableChineseMode();

//�������ܣ�ʹ�ܻ���ú����»���ģʽ����������FS - n��
//������n=0 ���� n=1 ʹ��
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkEnableChineseUnderLine(int n);


//�������ܣ������û��Զ��庺�� ��������FS 2 c1 c2 d1...dk��
//������c1��c2�����û��Զ��庺�����ڵ���λ�룬���˵�������纺����GB18030,c1 = FE A1 �� c2 �� FE  ,k=72,����dԪ��ȡֵ��Χ0~255
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetUserDefChinese(int c1,int c2,int code[]);

//�������ܣ������û��Զ����ַ�����ϵͳ    ��������FS C n��
//�������Զ����ַ�����ϵͳ���� n= 0~1
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetUserDefChineseArea(int n);

//�������ܣ����ú����ַ����Ҽ��   ��������FS S n1 n2��
//����������n1,�Ҽ��n2,0 �� n1 �� 255,0 �� n2 �� 255
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetChineseLeftRightSpace(int n1,int n2);

//�������ܣ�ʹ�ܻ���ú����ı���С��ӡ����������FS W n��
//������n=0 ���� n=1 ʹ��
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetChinese4TimesSize(int n);

//�������ܣ�ʹ�ܻ���õߵ���ӡģʽ���������: ESC { n��
//������n=0 ���� n=1 ʹ��
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkEnableUpsidedown(int n);

//�������ܣ���ӡԤ����λͼ      ���������: FS p n m��
//������m ��ӡ�ܶ�
/*  m	Ч��	��ֱ�ܶ�(dpi)	ˮƽ�ܶ�(dpi)
	0	��ͨ	203				203
	1	����	203				101
	2	����	101				203
	3	4��		101				101
*/
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkPrintDefineBitmap(int m,int n); 

//�������ܣ�����Ԥ����λͼ      ���������: FS q n��
//������szBmpFile��Ҫ���ص�λͼ��ȫ·����
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkDefineBitmap(char *szBmpFile);

//�������ܣ���ʱ��ӡBMPͼ��    ���������:ESC * m nL nH d1...dk ��
//������szBmpFile��Ҫ���ص�λͼ��ȫ·����
/*
	m		��ֱ�ܶ�	ˮƽ�ܶ�
	0		60			90
	1		60			180
	32		180			90
	33		180			180
	һ�� m=33
*/
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkPrintBitmap(char *szBmpFile,int m);

//�������ܣ���װͼ�κ�ʱ��ӡ��       ����������ִ��GS * ����ִ�� GS / m��
//������szBmpFile��Ҫ���ص�λͼ��ȫ·����
/*
	m	Ч��	��ֱ�ܶ�(dpi)	ˮƽ�ܶ�(dpi)
	0	��ͨ	180			180
	1	����	180			90
	2	����	90			180
	3	�ı�	90			90
	һ�� m=0
*/
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkDownloadBitmapAndPrint(char *szBmpFile,int m);


///�������ܣ�����Ǯ��������ʽ    �������� ESC p m t1 t2��
//������ʹ���Ǹ��ܽ��������  m=0  2��  m=1  5��    ������Ϊ t1*2ms t2*2ms ������Ҫ�� t1 < t2 ��һ�� t1=150 t2=250
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetCashBoxDriveMode(int m,int t1,int t2);


//�������ܣ����õ�״̬�ص�����(ע���������ܷ���״̬�Ĵ�ӡ���ͺţ��紮�ڴ�ӡ��)
//������pCallBack -- �ص�����ָ��
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkSetCallBack(CallBack pCallBack);

//�������ܣ�ʹ�ܴ�ӡ״̬�ص������������ڻص��������ݴ�ӡ��״̬����ҵ������
//������enable = 1 ��ʹ��  =0 ����
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkEnableCallBack(int enable=0);


//�������ܣ�ֱ�����豸���ݽ���ͨ��(ע: ����Ϥ�豸���ܣ�������ֱ��ʹ��)
//������pdata -- ��Ҫ�ʹ�ӡ�������ݻ��壬len -- ���ݳ���
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkDirectIO(unsigned char *pdata,int len);


//�������ܣ�д���ӡ�����кţ���ʹ���ں��ݿ�﹫˾��
//������pdata -- ��Ҫд��Ĵ�ӡ�����кŵĵ����ݻ���,�ַ�����0��β
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkWriteSerialNo(char *pdata);

//�������ܣ���ȡ��ӡ�����кţ���ʹ���ں��ݿ�﹫˾��
//������pdata -- ��Ҫ���մ�ӡ�����кŵĵ����ݻ���,�ַ�����0��β
//����ֵ��0 -- �ɹ�   -1  --  ʧ��
int __stdcall YkReadSerialNo(char *pdata);

int __stdcall YkPrintRasterBmp(char *szBmpFile);

int __stdcall YkGetState();



int __stdcall YkPrintBitmapMatrix(char *szBmpFile);


////ͨ�ú���

/*
������������������
		 ��������			����		��������		��ע
��ڲ�����
		iInDoubleHieght		int			����
		iInDoubleWide		int			����
			iInUnderLine		int			�»���
													
													
����ֵ��					int						0:�ɹ�
													1:ʧ��					
*/

int __stdcall SetPrintFont(int iInDoubleHeight,int iInDoubleWide,int iInUnderLine);


/*
����������������߾�
		 ��������			����		��������		��ע
��ڲ�����
		iInDistance			int			��߾൥λ		 ��λ��0.1mm
����ֵ��					int							0:�ɹ�
														1:ʧ��					
*/

int __stdcall SetLeftDistance(int iInDistance);

/*
���������������и�
		 ��������			����		��������		��ע
��ڲ�����
		iInDistance			int			�м��		 ��λ��0.1mm ����0ʱĬ�ϸ�3.75mm
����ֵ��					int							0:�ɹ�
														1:ʧ��					
*/

int __stdcall SetRowDistance(int iInDistance);


/*
������������ӡλͼLOGO
		 ��������			����		��������		��ע
��ڲ�����
		pcInBmpFile			char*		Bmp�ļ���		֧�����·���;���·����ͼƬ���úڰ׵�ɫbmp��ʽ(һ��ռһλ),�ֱ���300*100����,��С������5k
����ֵ��					int							0:�ɹ�
														1:ʧ��					
*/

int __stdcall PrintBmp(char* pcInBmpFile);


/*
������������ӡ�ַ�
		 ��������			����		��������		��ע
��ڲ�����
		pcInstring			char*		���ӡ�ַ�		���Զ�����
����ֵ��					int							0:�ɹ�
														1:ʧ��					
*/

int __stdcall PrintString(char* pcInstring);


/*
������������ӡ�ַ�
		 ��������			����		��������				��ע
��ڲ�����
		pcInstring			char*		���ӡ��һ���ַ�		�Զ�����
����ֵ��					int							0:�ɹ�
														1:ʧ��					
*/

int __stdcall PrintLine(char* pcInstring);


/*
������������ֽ
		 ��������			����		��������				��ע
��ڲ�����
		iInCutMode			int		��ֽ��ʽ		0:ȫ��  1:����
����ֵ��					int							0:�ɹ�
														1:ʧ��					
*/

int __stdcall CutPaper(int iInCutMode);


/*
�������������úڱ���ֽλ��
		 ��������			����		��������				��ע
��ڲ�����
		iInn				int			�ڱ���ֽλ��		��λ:0.1mm n=0:��ֽ���ڱ�λ�� n>0��ֽλ���ںڱ��±� n<0 ��ֽλ���ںڱ��ϱ�
����ֵ��					int								0:�ɹ�
															1:ʧ��					
*/

int __stdcall SetBlackMark(int iInn);


/*
������������ֽ
		 ��������			����		��������				��ע
��ڲ�����
		fInDistance			float		mm		��λ:mm 
����ֵ��					int								0:�ɹ�
															1:ʧ��					
*/

int __stdcall FeedPaper(float fInDistance);


//int WINAPI PrintRasterBmp(unsigned char *szBmpFileData,int len);


/*
�������������ö�ά����PDF417ģ���С���
		 ��������			����		��������				��ע
��ڲ�����
		ModWidth			int       ��Χ 2~8  ��Ĭ��= 3
����ֵ��					int								0:�ɹ�
															1:ʧ��					

*/

int __stdcall SetPDF417ModWidth(int ModWidth=3);


/*
�������������ö�ά����PDF417ģ���С�߶�
		 ��������			����		��������				��ע
��ڲ�����
		ModHeight			int       ��Χ 2~8  ��Ĭ��= 3
����ֵ��					int								0:�ɹ�
															1:ʧ��					

*/

int __stdcall SetPDF417ModHeight(int ModHeight=3);



/*
�������������ö�ά����PDF417������
		 ��������			����		��������				��ע
��ڲ�����
		Level			int       ��Χ 1~40  ��Ĭ��= 1
����ֵ��					int								0:�ɹ�
															1:ʧ��					

*/

int __stdcall SetPDF417Level(int Level=1);



/*
�������������û�ȡ����׼PDF417����
		 ��������			����		��������				��ע
��ڲ�����
		iStd			int       ��=0�����ñ�׼PDF417���� ��=1�����ýض�PDF417����  Ĭ��Ϊ0
����ֵ��					int								0:�ɹ�
															1:ʧ��					

*/

int __stdcall SetStdPDF417(int iStd=0);



/*
��������������PDF417����������
		 ��������			����		��������				��ע
��ڲ�����
		iRow,iCol			int       Ĭ��iCol=0;iRow=0;  0�� iCol �� 30 ; iRow=0 �� 3��iRow �� 90; 
		��iCol=0����������������Ч��ӡ��Χ�Զ�������
		��iCol��0��������������ΪiCol�У�
		��iRow=0����������������Ч��ӡ��Χ�Զ�������
		��iRow��0��������������ΪiRown�У�

����ֵ��					int								0:�ɹ�
															1:ʧ��					

*/

int __stdcall SetPDF417RowCol(int iRow=0,int iCol=0);



/*
������������ӡ��PDF417����
		 ��������			����		��������				��ע
��ڲ�����
		pCode  ,�������ݴ��ָ��	unsigned char* 
		iLen, �������ݳ���    int   0<iLen<256

����ֵ��					int								0:�ɹ�
															1:ʧ��					

*/

int __stdcall PrintPDF417Code(unsigned char *pCode,int iLen);



/*
��������������QR�����ģ���С
		 ��������			����		��������				��ע
��ڲ�����
		ModWidth			int       ��Χ 2~16  ��Ĭ��= 3
����ֵ��					int								0:�ɹ�
															1:ʧ��					

*/

int __stdcall SetQRModSize(int ModWidth=3);



/*
��������������QR����ľ���ȼ�
		 ��������			����		��������				��ע
��ڲ�����
		Level			int       ��Χ 48~51  ��Ĭ��= 48
����ֵ��					int								0:�ɹ�
															1:ʧ��					

*/

int __stdcall SetQRLevel(int Level=48);



/*
������������ӡQR����
		 ��������			����		��������				��ע
��ڲ�����
		pCode  ,�������ݴ��ָ��	unsigned char* 
		iLen, �������ݳ���    int   0<iLen<256

����ֵ��					int								0:�ɹ�
															1:ʧ��					

*/

int __stdcall PrintQRCode(unsigned char *pCode,int iLen);

// ��������Դ�ӡλ���趨�� ���ס����ESC $  @16
int  __stdcall SetAbsPrnPos(float fPos);



/*
������������ӡPDF417��QR����
		 ��������			����		��������				��ע
��ڲ�����
		iType  ,��ά������   int   iType=10 PDF417 	1 �� ���� �� 255	0 �� ����ֵ��Χ �� 255 
								   iType=11 QRCODE 	1 �� ���� �� 928	0 < ����ֵ��Χ �� 255
		pCode  ,�������ݴ��ָ��	unsigned char* 
		iLen, �������ݳ���    int   0<iLen<256

����ֵ��					int								0:�ɹ�
															1:ʧ��					

*/

int __stdcall Print2DCode(int iType,unsigned char *pCode,int iLen);




/*
������������ӡDataMatrix��ά����
		 ��������			����		��������				��ע
��ڲ�����
		pCode  ,�������ݴ��ָ��	unsigned char* 
		iLen, �������ݳ���    int   0<iLen<256

����ֵ��					int								0:�ɹ�
															1:ʧ��					

*/

int __stdcall PrintDataMatrixCode(unsigned char *pCode,int iLen,int iDotWidth);





/*
������������ӡPDF417��ά����
		 ��������			����		��������				��ע
��ڲ�����
		pCode  ,�������ݴ��ָ��	unsigned char* 
		iLen, �������ݳ���    int   0<iLen<256

����ֵ��					int								0:�ɹ�
															1:ʧ��					

*/

int __stdcall PrintPDF417(unsigned char *pCode,int iLen,int iModSize,int iDotWidth);




/*
������������ʼ����ӡ����
		 ��������			����		��������				��ע
��ڲ�����  ��


����ֵ��					int								0:�ɹ�
															1:ʧ��					

*/

int __stdcall InitPrinter();




int __stdcall PrintRaster24Bmp(char *szBmpFile);




//////2016-12-13 �¶���һ��ͨ�ã����׼���������Ժ���ƹ���ͻ������ͻ����������������Ӻ��� 


/////     ���º����ݲ�֧��  �������У�


/*
�����������򿪶˿ڡ�


����

lpName

[in] ָ���� null ��β�Ĵ�ӡ�����ƻ�˿����ơ�

������nParam��ֵΪPOS_COM_DTR_DSR��POS_COM_RTS_CTS��POS_COM_XON_XOFF��POS_COM_NO_HANDSHAKE ʱ�� ��COM1������COM2������COM3������COM4���ȱ�ʾ���ڣ�

������nParam��ֵΪPOS_OPEN_PARALLEL_PORTʱ����LPT1������LPT2���ȱ�ʾ���ڣ�

������nParam��ֵΪPOS_OPEN_USB_PORTʱ����VUSB001������VUSB002������VUSB003������VUSB004���ȱ�ʾUSB�˿ڡ�

������nParam��ֵΪPOS_OPEN_PRINTNAMEʱ����ʾ��ָ���Ĵ�ӡ��(ע��Ҫ��װwindows��ӡ�����������������ʹ��)��

������nParam��ֵΪPOS_OPEN_NETPORTʱ����ʾ��ָ��������ӿ�(��ӡ��Ĭ�Ϲ����˿ں�Ϊ9100)���硰192.168.10.251����ʾ����ӿ�IP��ַ��


nComBaudrate

[in] ָ�����ڵĲ����ʣ�bps���� 

����Ϊ����ֵ֮һ��

9600��19200��38400��115200�ȡ�

�����ֵ���ӡ�����ͺ��й�


nParam

[in] ָ�����ڵ������ƣ����֣���ʽ�����ʾͨѶ��ʽ����ο�����lpName��˵����

����Ϊ����ֵ֮һ��

Flag Value Meaning 
POS_COM_DTR_DSR 0x00 ������ΪDTR/DST  
POS_COM_RTS_CTS 0x01 ������ΪRTS/CTS 
POS_COM_XON_XOFF 0x02 ������ΪXON/OFF 
POS_COM_NO_HANDSHAKE 0x03 ������ 
POS_OPEN_PARALLEL_PORT 0x12 �򿪲���ͨѶ�˿� 
POS_OPEN_USB_PORT 0x13 ��USBͨѶ�˿� 
POS_OPEN_PRINTNAME 0X14 �򿪴�ӡ���������� 
POS_OPEN_NETPORT 0x15 ������ӿ� 

����ǰ����Ҳͳ��ΪӲ�������ƣ�һ��ѡ�� RTS/CTS ��ʽ��



����ֵ

����������óɹ�������һ���Ѵ򿪵Ķ˿ھ����

�����������ʧ�ܣ�����ֵΪ INVALID_HANDLE_VALUE ��-1����



��ע

1������򿪵��ǲ��ڣ�LPT1��LPT2����USB������ӿڣ���ô����Ĳ��� nComBaudrate�������ԣ���������Ϊ0�����Ҳ���nParam����ָ��ΪPOS_OPEN_PARALLEL_PORT��ͬ������USB�˿ڻ��ӡ����������nParam����ָ��Ϊ��Ӧ�Ľӿڲ�����

2������� lpName ָ����ͨѶ�˿ڱ���������ռ�ã���ô����ֵΪ INVALID_HANDLE_VALUE��

3�������������Ҳ����INVALID_HANDLE_VALUE��

4�����ͨѶ�˿��Ѿ��򿪣���᳢�Թر��Ѿ��򿪵Ķ˿ڣ�Ȼ����ȥ�򿪡�

5���������nParamָ����USB�˿�,���ж�̨USB��ӡ��ͬʱʹ�ã�����Ҫʹ�����ù��������ÿ̨��ӡ��ָ���������𿪡� 

6������ο� POS_Close��POS_Reset��


*/

HANDLE __stdcall POS_Open(

LPCTSTR lpName, 

int nComBaudrate, 

int nParam

);



/*
����

�ر��Ѿ��򿪵Ĳ��ڻ򴮿ڣ�USB�˿ڣ�����ӿڻ��ӡ����


����

�ޡ�


����ֵ��

����������óɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

*/

int __stdcall POS_Close(void);


/*
����

��λ��ӡ�����Ѵ�ӡ�������е�����������ַ����иߵ����ñ��������ӡģʽ���ָ����ϵ�ʱ��ȱʡģʽ��


����

��



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE



��ע

�����ɹ�ִ�к���

1�����ջ������е�ָ�����

2���궨�屣����

3��Flash �е�λͼ������

4��Flash �е����ݱ�����

5��DIP���ص����ò������ٴμ�⡣

*/


int __stdcall POS_Reset(void);



/*
����

���ô�ӡ���Ĵ�ӡģʽ��



����

nPrintMode

[in] ָ����ӡģʽ��

����Ϊ����ֵ֮һ��

Flag Value Meaning 
POS_PRINT_MODE_STANDARD 0x00 ��׼ģʽ����ģʽ�� 
POS_PRINT_MODE_PAGE 0x01 ҳģʽ (���ִ�ӡ������Ӳ��֧��)
POS_PRINT_MODE_BLACK_MARK_LABEL 0x02 �ڱ�Ǳ�ǩģʽ (���ִ�ӡ������Ӳ��֧��)
POS_PRINT_MODE_WHITE_MARK_LABEL 0x03 �ױ�Ǳ�ǩģʽ (���ִ�ӡ������Ӳ��֧��)
POS_PRINT_MODE_VIRTUAL_PAGE 0x04 ����ҳģʽ����̬��������棩




����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PARAMETER



��ע

1�����ִ�ӡ���ͺŲ�֧�ֺڱ�Ǳ�ǩģʽ�Ͱױ�Ǳ�ǩģʽ����ο�����û��ֲᡣ

2������ο� POS_CutPaper��


*/


int __stdcall POS_SetMode(

int nPrintMode

);


/*


����

���ô�ӡ�����ƶ���λ��



����

nHorizontalMU

[in] ��ˮƽ�����ϵ��ƶ���λ����Ϊ 25.4 / nHorizontalMU ���ס�

����Ϊ0��255��


nVerticalMU

[in] �Ѵ�ֱ�����ϵ��ƶ���λ����Ϊ 25.4 / nVerticalMU ���ס�

����Ϊ0��255��




����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PARAMETER


*/

int __stdcall POS_SetMotionUnit(

int nHorizontalMU,

int nVerticalMU 

);



/*


����

ѡ������ַ����ʹ���ҳ��



����

nCharSet

[in] ָ�������ַ�������ͬ�Ĺ����ַ�����0x23��0x7E��ASCII��ֵ��Ӧ�ķ��Ŷ����ǲ�ͬ�ġ�

����Ϊ�����б�������ֵ֮һ��

Value Meaning 
0x00 U.S.A 
0x01 France 
0x02 Germany 
0x03 U.K. 
0x04 Denmark I 
0x05 Sweden 
0x06 Italy 
0x07 Spain I 
0x08 Japan 
0x09 Nonway 
0x0A Denmark II 
0x0B Spain II 
0x0C Latin America 
0x0D Korea 

nCodePage

[in] ָ���ַ��Ĵ���ҳ����ͬ�Ĵ���ҳ��0x80��0xFF��ASCII��ֵ��Ӧ�ķ��Ŷ����ǲ�ͬ�ġ�

����Ϊ�����б�������ֵ֮һ��

Value Meaning 
0x00 PC437 [U.S.A. Standard Europe 
0x01 Reserved 
0x02 PC850 [Multilingual] 
0x03 PC860 [Portuguese] 
0x04 PC863 [Canadian-French] 
0x05 PC865 [Nordic] 
0x12 PC852 
0x13 PC858 



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_PARAMETER

POS_ERROR_INVALID_HANDLE



��ע

1����Щ��ӡ�����ܲ�֧�������ַ��������ҳ����ϸ��Ϣ��ο���ӡ���������Ż򸽴����û��ֲᡣ

2������nCodePageֵ�Ŀ�ȡ��ΧΪ0��255������������е�����ֵ�����Ժ�ʹ�á�



*/
int __stdcall POS_SetCharSetAndCodePage(

int nCharSet,

int nCodePage

);


/*


����

��ǰ��ֽ��



����

�ޡ�



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE



��ע

1������ڱ�׼��ӡģʽ����ģʽ���´�ӡ�ı������ӡ�������е����ݣ��Ҵ�ӡλ���Զ��ƶ�����һ�е����ס�

2������ڱ�׼��ӡģʽ����ģʽ���´�ӡλͼ������ָ����λ�ô�ӡλͼ���Ҵ�ӡλ���Զ��ƶ�����һ�е����ס�

3�������ҳģʽ���ǩģʽ�£������Ҫ��ӡ������������ָ����λ�ã�ͬʱ�Ѵ�ӡλ���ƶ�����һ�����ף����ǲ���������ֽ����ӡ������һֱ������ POS_PL_Print ����ʱ�Ŵ�ӡ��

*/

int __stdcall POS_FeedLine(void);


/*
����

�����ַ����иߡ�



����

nDistance

[in] ָ���иߵ�����

����Ϊ 0 �� 255��ÿ��ľ������ӡͷ�ֱ�����ء�



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PARAMETER



��ע

1��������и�����Ϊ0�����ӡ��ʹ���ڲ���Ĭ���и�ֵ����1/6Ӣ�硣�����ӡͷ����ֱ���Ϊ180dpi ���൱�� 31 ��ߡ�

2������и߱�����ΪС�ڵ�ǰ���ַ��߶ȣ����ӡ����ʹ�õ�ǰ�ַ��߶�Ϊ�иߡ�

3������ο� POS_SetRightSpacing��


*/

int __stdcall POS_SetLineSpacing(

int nDistance

);



/*

����

�����ַ����Ҽ�ࣨ���������ַ��ļ�϶���룩��



����

nDistance

[in] ָ���Ҽ��ĵ�����

����Ϊ 0 �� 255��ÿ��ľ������ӡͷ�ֱ�����ء�



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PARAMETER



��ע

1���ַ��Ҽ��������ڱ�׼ģʽ��ҳģʽ���ǩģʽ�Ƕ����ġ�

2������ַ��Ŵ����ַ��Ҽ��ͬ���Ŵ�

3������ο� POS_SetLineSpacing��


*/

int __stdcall POS_SetRightSpacing(

int nDistance

);



/*
����

Ԥ����һ��λͼ����ӡ���� RAM �У�ͬʱָ����λͼ�� ID �š�



����

pszPath

[in] ָ���� null ��β�ı�ʾλͼ·�������ļ������ַ�����


nID

[in] ָ����Ҫ���ص�λͼ�� ID �š�

����Ϊ 0 �� 7��



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PATH

POS_ERROR_INVALID_PARAMETER

POS_ERROR_NOT_BITMAP

POS_ERROR_NOT_MONO_BITMAP

POS_ERROR_BEYOND_AREA



��ע

1����Ҫ���ص�λͼ��С���ܳ��� 900 ƽ������ (��ԼΪ 240�� �� 240 ��)��

2��λͼ�����ǵ�ɫ�ġ�

3��һ���ӡ���ڲ��Ŀ��� RAM �ռ�Ϊ 8K �ֽڡ�

4��λͼ�� ID �Ų�Ҫ���������ġ�

5������ο�POS_PreDownloadBmpsToFlash��POS_S_PrintBmpInRAM��POS_PL_PrintBmpInRAM��


*/

int __stdcall POS_PreDownloadBmpToRAM(

char *pszPath

);


/*
��

����

Ԥ����һ�������ɷ�λͼ����ӡ���� Flash �С�



����

pszPaths

[in] ָ���������λͼ���ļ�·���������Ƶ��ַ������顣


nCount

[in] ָ����Ҫ���ص�λͼ������

����Ϊ1 �� 255��



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS.

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_PATH

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PARAMETER

POS_ERROR_NOT_BITMAP

POS_ERROR_NOT_MONO_BITMAP

POS_ERROR_BEYONG_AREA



��ע

1��ÿ��λͼ�����ݴ�С���ܳ��� 8K �ֽڣ���ԼΪ 256 �� �� 256 �㣩��

2��λͼ����Ϊ��ɫλͼ��

3�����ص� Flash �е�λͼ��ͼ����λͼ�ĸ���������˳����ء�λͼ������˳����ļ����������е�˳��һ�£��������ġ��磺��һ��λͼ��ͼ��Ϊ 1���ڶ���Ϊ 2���Դ����ơ�

4��ÿ�����ض�����ϴ����ص� Flash �е�λͼ�������

5���ص�󲻻ᱻ�����

6����ӡ���ڲ�Flash �Ŀ�ʹ�ÿռ�������ӡ���ͺ��йء�

7������ο�POS_PreDownloadBmpToRAM��POS_S_PrintBmpInFlash��


*/

int __stdcall POS_PreDownloadBmpsToFlash(

char* pszPaths[],

int nCount

);



/*

����

ͨ�����ڲ�ѯ��ӡ����ǰ��״̬���˺����Ƿ�ʵʱ�ġ�



����

pszStatus

[out] ָ�򷵻ص�״̬���ݵĻ���������������СΪ 1 ���ֽڡ�

���صĸ�״̬λ�������±���ʾ��

Bit Status Meaning 
0��1 0/1 ��ֽ������ֽ / ֽ���þ� 
2��3 0/1 ��ӡͷ����ֽ / ��ֽ 
4��5 0/1 Ǯ������������ 3 �ĵ�ƽΪ�� / �ߣ���ʾ�򿪻�رգ� 
6��7 0 �������̶�Ϊ0�� 

nTimeouts

[in] ���ò�ѯ״̬ʱ��Լ�ĳ�ʱʱ�䣨���룩��



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE



��ע

1���˺����ڲ���ͨѶ����Ч��

2�������ͺŵĴ�ӡ�����ϸǴ򿪻��ӡͷ̧��ȱֽ��Feed�����µ�����£����ܷ��ش�ӡ����״̬��

3������ο� POS_RTQueryStatus��


*/

int __stdcall POS_QueryStatus(

char *pszStatus,

int nTimeouts

);


/*

����

ͨ�����ڷ��ص�ǰ��ӡ����״̬���˺�����ʵʱ�ġ�



����

pszStatus

[out] ָ����շ���״̬�Ļ���������������СΪ 1 ���ֽڡ� 

���صĸ�״̬λ�������±���ʾ��

Bit Status Meaning 
0 0/1 Ǯ������������ 3 �ĵ�ƽΪ��/�ߣ���ʾ�򿪻�رգ� 
1 0/1 ��ӡ������/�ѻ� 
2 0/1 �ϸǹر�/�� 
3 0/1 û��/������Feed�����¶���ֽ 
4 0/1 ��ӡ��û��/�г��� 
5 0/1 �е�û��/�г��� 
6 0/1 ��ֽ/ֽ������ֽ����������̽�⣩ 
7 0/1 ��ֽ/ֽ�þ���ֽ������̽�⣩ 



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE



��ע

1���˺����ڲ���ͨѶ����Ч��

2�������ͺŵĴ�ӡ�����ϸǴ򿪻��ӡͷ̧��ȱֽ��Feed�����µ�����£����ܷ��ش�ӡ����״̬��

3������ο� POS_QueryStatus��

*/

int __stdcall POS_RTQueryStatus(

char *pszStatus

);



/*

����

ͨ������ӿڲ�ѯ���ص�ǰ��ӡ����״̬��



����

ipAddress

[in] �豸IP��ַ���硰192.168.10.251����

pszStatus

[out] ָ����շ���״̬�Ļ���������������СΪ 1 ���ֽڡ� 

���صĸ�״̬λ�������±���ʾ��

Bit Status Meaning 
0 0/1 Ǯ������������ 3 �ĵ�ƽΪ��/�ߣ���ʾ�򿪻�رգ� 
1 0/1 ��ӡ������/�ѻ� 
2 0/1 �ϸǹر�/�� 
3 0/1 û��/������Feed�����¶���ֽ 
4 0/1 ��ӡ��û��/�г��� 
5 0/1 �е�û��/�г��� 
6 0/1 ��ֽ/ֽ������ֽ����������̽�⣩ 
7 0/1 ��ֽ/ֽ�þ���ֽ������̽�⣩ 



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE


*/
int __stdcall POS_NETQueryStatus(

char *ipAddress,

char *pszStatus

);



/*

����

��Ǯ�����ŷ��������Դ�Ǯ�䡣



����

nID

[in] ָ��Ǯ������š�

����Ϊ����ֵ֮һ��

Value Meaning 
0x00 Ǯ������������2 
0x01 Ǯ������������5 

nOnTimes

[in] ָ����Ǯ�䷢�͵ĸߵ�ƽ���屣��ʱ�䣬�� nOnTimes �� 2 ���롣

����Ϊ1 �� 255��


nOffTimes

[in] ָ����Ǯ�䷢�͵ĵ͵�ƽ���屣��ʱ�䣬�� nOffTimes �� 2 ���롣

����Ϊ1 �� 255��



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS.

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PARAMETER



��ע

1��������� nOffTimes ��ֵС�� nOnTimes, ����Ǯ�䷢�͵ĵ͵�ƽ����ı���ʱ��ΪnOnTimes �� 2 ���롣

2����ο�Ǯ�乩Ӧ���ṩ��������ϡ�



*/


int __stdcall POS_KickOutDrawer(

int nID,

int nOnTimes,

int nOffTimes

);



/*
����

��ֽ��



����

nMode

[in] ָ����ֽģʽ��

����Ϊ����ֵ֮һ��

Flag Value Meaning 
POS_CUT_MODE_FULL 0x00 ȫ�� 
POS_CUT_MODE_PARTIAL 0x01 ���� 
POS_CUT_MODE_ALL 0x02 �������/ȫ�е���Ū��ֱ����ֽ

nDistance

[in] ָ����ֽ���ȵĵ�����

����Ϊ 0 �� 255��ÿ��ľ������ӡͷ�ֱ�����ء�



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PARAMETER



��ע

1�����ָ��Ϊȫ�У������ nDistance ���ԡ�

2�����ָ��Ϊ���У����ӡ����ֽ nDistance �㣬Ȼ����ֽ��

3������ο� POS_SetMode��


*/

int __stdcall POS_CutPaper(

int nMode,

int nDistance

);


/*


����

�½�һ����ӡ��ҵ��



����

��



����ֵ

��������ɹ���ΪTRUE������ΪFALSE��



��ע

�û������ڶ����ҵ��ʼʱ����һ�θú�����Ҳ����ÿ����ҵ���ô˹��ܡ�




*/

BOOL __stdcall POS_StartDoc();



/*


����

����һ����ӡ��ҵ��


����

��


����ֵ

��������ɹ���ΪTRUE������ΪFALSE��


��ע

�û������ڶ����ҵ����ʱ����һ�θú�����Ҳ����ÿ����ҵ���ô˹��ܡ�


*/

BOOL __stdcall POS_EndDoc();




/*
��

����

��ʼ�ѷ�����ӡ�����˿ڣ������ݱ��浽ָ�����ļ���



����

lpFileName

[in] �������ݵ��ļ����ƣ���null��β���ַ����������Ǿ���·����Ҳ���������·����


bToPrinter

[in] 

TRUE ��ָ���Ƿ��ڱ������ݵ��ļ���ͬʱ��������Ҳ���͵���ӡ�����˿ڣ��� 

FALSE ��ָ���Ƿ��ڱ������ݵ��ļ���ͬʱ����������Ҳ���͵���ӡ�����˿ڣ���



����ֵ

�ޡ�



��ע

1�� ���ָ�����ļ����ڣ�����׷�ӷ�ʽ���ϰ����ݱ��浽���ļ������ָ�����ļ������ڣ���ᴴ����Ȼ������׷�ӷ�ʽ�������ݡ�

2�� ����ο�POS_EndSaveFile��


*/
void __stdcall POS_BeginSaveFile(

LPCTSTR lpFileName,

BOOL bToPrinter

);



/*

����

�����������ݵ��ļ��Ĳ�����


����

�ޡ� 


����ֵ

�ޡ�


*/
void __stdcall POS_EndSaveFile();



/*

����

���ñ�׼ģʽ�µĴ�ӡ�����ȡ�



����

nWidth

[in] ָ����ӡ����Ŀ�ȡ�

����Ϊ 0 �� 65535�㡣



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PARAMETER



��ע

1���˺���ֻ��������Ч��

2�����ڲ�ͬ�Ĵ�ӡ���ͺ��в�ͬ�Ĵ�ӡͷ��ȣ����ڲ��������Ŀ�ȣ������Կɴ�ӡ����Ŀ���ǲ�ͬ�ġ�

3�������ӡ����Ŀ������С��һ���ַ���ȣ��򵱴�ӡ�����ܴ�ӡһ���ַ�ʱ�����Զ����һ�����չ��һ���ַ��Ŀ�ȣ�����ӡλͼʱ����ӡ������ͬ���ķ�ʽ����չ����ӡ��

4�������ӡ����Ŀ�ȼ�����߾��ȴ��ڿɴ�ӡ�����ȣ����ӡ��ʹ�õĴ�ӡ�����ȴ�С�ǿɴ�ӡ�����Ⱥ���߾���֮�

5������ο�POS_PL_SetArea��POS_SetMotionUnit��


*/
int __stdcall POS_S_SetAreaWidth(

int nWidth

);


/*


����

�ѽ�Ҫ��ӡ���ַ������ݷ��͵���ӡ�������У���ָ��X ����ˮƽ���ϵľ�����ʼ��λ�ã�ָ��ÿ���ַ���Ⱥ͸߶ȷ����ϵķŴ��������ͺͷ��



����

pszString

[in] ָ���� null ��β���ַ�����������


nOrgx

[in] ָ�� X ����ˮƽ������ʼ��λ������߽�ĵ�����

����Ϊ 0 �� 65535��


nWidthTimes

[in] ָ���ַ��Ŀ�ȷ����ϵķŴ�����

����Ϊ 1�� 6��


nHeightTimes

[in] ָ���ַ��߶ȷ����ϵķŴ�����

����Ϊ 1 �� 6��


nFontType

[in] ָ���ַ����������͡�

����Ϊ�����б�������ֵ֮һ��

Flag Value Meaning 
POS_FONT_TYPE_STANDARD 0x00 ��׼ ASCII 
POS_FONT_TYPE_COMPRESSED 0x01 ѹ�� ASCII  
POS_FONT_TYPE_UDC 0x02 �û��Զ����ַ� 
POS_FONT_TYPE_CHINESE 0x03 ��׼ �����塱 


nFontStyle

[in] ָ���ַ���������

����Ϊ�����б��е�һ�������ɸ���

Flag Value Meaning 
POS_FONT_STYLE_NORMAL 0x00 ���� 
POS_FONT_STYLE_BOLD 0x08 �Ӵ� 
POS_FONT_STYLE_THIN_UNDERLINE 0x80 1��ֵ��»��� 
POS_FONT_STYLE_THICK_UNDERLINE 0x100 2��ֵ��»��� 
POS_FONT_STYLE_UPSIDEDOWN 0x200 ���ã�ֻ��������Ч�� 
POS_FONT_STYLE_REVERSE 0x400 ���ԣ��ڵװ��֣� 
POS_FONT_STYLE_SMOOTH 0x800 ƽ���������ڷŴ�ʱ�� 
POS_FONT_STYLE_CLOCKWISE_90 0x1000 ÿ���ַ�˳ʱ����ת 90 �� 



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PARAMETER



��ע

1���ڴ�ӡ���У�һ���ڲ����������������±���ʾ�����ǲ�ͬ���ͻ�������ͬ�����Բο���ӡ���Ĳ������š�

Font Type Size (W * H) 
Standard ASCII 12 * 24 / 13 * 24 
Compressed ASCII 9 * 17 
��׼���� 24 * 24 

2������ַ����nFontStyle������Ϊ�����ԣ�POS_FONT_STYLE_REVERSE��������˳ʱ����ת90�ȣ�POS_FONT_STYLE_CLOCKWISE_90��������ô��ϸ�»��ߣ�POS_FONT_STYLE_THIN_UNDERLINE���� �͡����»��ߣ�POS_FONT_STYLE_THICK_UNDERLINE�����Ĺ��ܽ���Ч��

3������ں����ַ�ģʽ�£���һ�ε���ʱ����׼ASCII�ַ��������ַ��ź�ȫ���µķ��ţ��ͺ��ֿ��Ի�ϴ�ӡ�������ASCII�ַ�ģʽ�£���ֻ���Դ�ӡ��׼ ASCII �ַ���ѹ����ASCII�ַ���

4���˺�������������ӡ������ַ���������һֱ�����ú��� POS_FeedLine ʱ�Ž���ʵ�ʵĽ�ֽ����ӡ�Ķ��������ǣ����������ַ����Ѿ����ڿɴ�ӡ���ʱ����ô���Զ���ֽ����ӡ��

5������ο� POS_PL_TextOut��



*/
int __stdcall POS_S_TextOut(

char *pszString,

int nOrgx,

int nWidthTimes,

int nHeightTimes,

int nFontType,

int nFontStyle

);



/*

����

���ز���ӡλͼ



����

pszPath

[in] ָ����null ��β�İ���λͼ�ļ�·���������Ƶ��ַ�����


nOrgx

[in] ָ����Ҫ��ӡ��λͼ����߽�ľ��������

����Ϊ 0�� 65535 �㡣


nMode

[in] ָ��λͼ�Ĵ�ӡģʽ��

����Ϊ�����б�������ֵ֮һ��

Flag Value Meaning 
POS_BITMAP_MODE_8SINGLE_DENSITY 0x00 8�㵥�ܶ� 
POS_BITMAP_MODE_8DOUBLE_DENSITY 0x01 8��˫�ܶ� 
POS_BITMAP_MODE_24SINGLE_DENSITY 0x20 24�㵥�ܶ� 
POS_BITMAP_MODE_24DOUBLE_DENSITY 0x21 24��˫�ܶ� 


����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PATH

POS_ERROR_INVALID_PARAMETER

POS_ERROR_NOT_BITMAP

POS_ERROR_NOT_MONO_BITMAP

POS_ERROR_BEYOND_AREA



��ע

1��λͼ�����Դ��� 8K �ֽڡ�

2��λͼ�����ǵ�ɫ�ġ�

3����ӡ�������и߱�����Ϊ 31 ��ߡ�

4������ο� POS_PreDownloadBmpToRAM��POS_PreDownloadBmpsToFlash��POS_PL_DownloadBmpAndPrint�� POS_SetLineSpacing��



*/

int _stdcall POS_S_DownloadAndPrintBmp(

char *pszPath,

int nOrgx,

int nMode

);




/*

����

��ӡ�Ѿ����ص� RAM �е�λͼ��



����

nID

[in] ָ��λͼ�� ID �š�

����Ϊ 0 �� 7��


nOrgx

[in] ָ����Ҫ��ӡ��λͼ����߽�ľ��������

����Ϊ 0�� 65535 �㡣


nMode

[in] ָ��λͼ�Ĵ�ӡģʽ��

����Ϊ�����б�������ֵ֮һ��

Flag Value Meaning 
POS_BITMAP_PRINT_NORMAL 0x00 ���� 
POS_BITMAP_PRINT_DOUBLE_WIDTH 0x01 ���� 
POS_BITMAP_PRINT_DOUBLE_HEIGHT 0x02 ���� 
POS_BITMAP_PRINT_QUADRUPLE 0x03 �����ұ��� 



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PARAMETER



��ע

�ο����� POS_PreDownloadBmpToRAM��POS_PL_PrintBmpInRAM��


*/

int __stdcall POS_S_PrintBmpInRAM(

int nOrgx,

int nMode

);



/*

����

��ӡ�Ѿ����ص� Flash �е�λͼ��



����

nID

[in] ָ��λͼ�� ID �š�

����Ϊ 1 �� 255��


nOrgx

[in] ָ����Ҫ��ӡ��λͼ����߽�ľ��������

����Ϊ 0�� 65535 �㡣


nMode

[in] ָ��λͼ�Ĵ�ӡģʽ��

����Ϊ�����б�������ֵ֮һ��

Flag Value Meaning 
POS_BITMAP_PRINT_NORMAL 0x00 ���� 
POS_BITMAP_PRINT_DOUBLE_WIDTH 0x01 ���� 
POS_BITMAP_PRINT_DOUBLE_HEIGHT 0x02 ���� 
POS_BITMAP_PRINT_QUADRUPLE 0x03 �����ұ��� 


����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PARAMETER



��ע

�ο����� POS_PreDownloadBmpsToFlash��

*/

int __stdcall POS_S_PrintBmpInFlash(

int nID,

int nOrgx,

int nMode

);


/*

����

���ò���ӡ���롣



����

pszInfoBuffer

[in] ָ���� null ��β���ַ�����ÿ���ַ�����ķ�Χ�͸�ʽ��������������йء�


nOrgx

[in] ָ����Ҫ��ӡ�������ˮƽ��ʼ������߽�ľ��������

����Ϊ 0 ��65535��


nType

[in] ָ����������͡�

����Ϊ�����б�������ֵ֮һ������ο�����¼ B ����˵������

Flag Value Meaning 
POS_BARCODE_TYPE_UPC_A 0x41 UPC-A 
POS_BARCODE_TYPE_UPC_E 0x42 UPC-C 
POS_BARCODE_TYPE_JAN13 0x43 JAN13(EAN13) 
POS_BARCODE_TYPE_JAN8 0x44 JAN8(EAN8) 
POS_BARCODE_TYPE_CODE39 0x45 CODE39 
POS_BARCODE_TYPE_ITF 0x46 INTERLEAVED 2 OF 5 
POS_BARCODE_TYPE_CODEBAR 0x47 CODEBAR 
POS_BARCODE_TYPE_CODE93 0x48 25 
POS_BARCODE_TYPE_CODE128 0x49 CODE 128 


nWidthX

[in] ָ������Ļ���Ԫ�ؿ�ȡ�

����Ϊ�����б�������ֵ��n��֮һ��

n ������ģ����
�������ͣ� ˫����ģ���ȣ���ɢ�ͣ� 
խԪ�ؿ�� ��Ԫ�ؿ�� 
2 0��25mm 0��25mm 0��625mm 
3 0��375mm 0��375mm 1��0mm 
4 0��5mm 0��5mm 1��25mm 
5 0��625mm 0��625mm 1��625mm 
6 0��75mm 0��75mm 1.875mm 


nHeight

[in] ָ������ĸ߶ȵ�����

����Ϊ 1 �� 255 ��Ĭ��ֵΪ162 �㡣


nHriFontType

[in] ָ�� HRI��Human Readable Interpretation���ַ����������͡�

����Ϊ�����б�������ֵ֮һ��

Flag Value Meaning 
POS_FONT_TYPE_STANDARD 0x00 ��׼ASCII 
POS_FONT_TYPE_COMPRESSED 0x01 ѹ��ASCII 


nHriFontPosition

[in] ָ��HRI��Human Readable Interpretation���ַ���λ�á�

����Ϊ�����б�������ֵ֮һ��

Flag Value Meaning 
POS_HRI_POSITION_NONE  0x00 ����ӡ 
POS_HRI_POSITION_ABOVE 0x01 ֻ�������Ϸ���ӡ 
POS_HRI_POSITION_BELOW 0x02 ֻ�������·���ӡ 
POS_HRI_POSITION_BOTH  0x03 �����ϡ��·�����ӡ 


nBytesToPrint

[in] ָ���ɲ��� pszInfoBufferָ����ַ�������������Ҫ���͸���ӡ�����ַ�����������ֵ�����������йء�



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PARAMETER



��ע

����ο�POS_PL_SetBarcode �� ����¼ B ����˵������


*/

int __stdcall POS_S_SetBarcode(

char *pszInfoBuffer,

int nOrgx,

int nType,

int nWidthX,

int nHeight,

int nHriFontType,

int nHriFontPosition,

int nBytesToPrint

);




/*

����

����ҳ��Ĵ�ӡ����



����

nOrgx

[in] ָ������� X ��ˮƽ���������ʼ�����߽�ľ��롣

����Ϊ 0 �� 65535��


nOrgy

[in] ָ������� Y ����ֱ���������ʼ����ϱ߽磨��ǰ��ӡͷλ�ã��ľ��������

����Ϊ 0 �� 65535��


nWidth

[in] ָ����ӡ����Ŀ�ȣ�ˮƽ���򣩡�

����Ϊ 0 �� 65535��


nHeight

[in] ָ����ӡ����ĸ߶ȣ���ֱ���򣩡�

����Ϊ 0 �� 65535��


nDirection

[in] ָ����ӡ����ķ���ԭ��λ�ã���

����Ϊ�����б�������ֵ֮һ��

Flag Value Starting Position 
POS_AREA_LEFT_TO_RIGHT  0 ���Ͻ� 
POS_AREA_BOTTOM_TO_TOP 1 ���½� 
POS_AREA_RIGHT_TO_LEFT 2 ���½� 
POS_AREA_TOP_TO_BOTTOM 3 ���Ͻ� 



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PARAMETER



��ע

1�����ڲ�ͬ�Ĵ�ӡ���ͺ��в�ͬ�Ĵ�ӡͷ��ȣ����ڲ��������Ŀ�ȣ������Կɴ�ӡ����Ŀ���ǲ�ͬ�ġ�ͬ�����ɴ�ӡ����ĸ߶�Ҳ�п��ܲ�ͬ��һ���Լ�� 128 �� 8 �㣬������� nWidth �� nHeight ��ָ����ֵ���������ơ���ô�ɴ�ӡ�����Զ�����Ϊʵ�ʵĿɴ�ӡ����

2�������ӡ����Ŀ�Ȼ�߶�����Ϊ0�����ӡ��ָֹͣ��Ĵ������������ܵ������ݶ�������ͨ���ַ����ݡ�

3�������ӡ�������ʼ�㳬����ӡ���Ŀɴ�ӡ�������ӡ��ָֹͣ��Ĵ������������ܵ������ݶ�������ͨ���ַ����ݡ�



*/
int __stdcall POS_PL_SetArea(

int nOrgx,

int nOrgy,

int nWidth,

int nHeight,

int nDirection

);



/*

����

�ѽ�Ҫ��ӡ���ַ������ݷ��͵���ӡ�������У���ָ��X ����ˮƽ���ϵľ�����ʼ��λ�ã�ָ��ÿ���ַ���Ⱥ͸߶ȷ����ϵķŴ��������ͺͷ��



����

pszString

[in] ָ���� null ��β���ַ�����������


nOrgx

[in] ָ�� X ����ˮƽ������ʼ��λ������߽�ĵ�����

����Ϊ 0 �� 65535��


nOrgy

[in] ָ�� Y ���򣨴�ֱ������ʼ��λ�����ϱ߽�ĵ�����

����Ϊ 0 �� 65535��


nWidthTimes

[in] ָ���ַ��Ŀ�ȷ����ϵķŴ�����

����Ϊ 1�� 6��


nHeightTimes

[in] ָ���ַ��߶ȷ����ϵķŴ�����

����Ϊ 1 �� 6��


nFontType

[in] ָ���ַ����������͡�

����Ϊ�����б�������ֵ֮һ��

Flag Value Meaning 
POS_FONT_TYPE_STANDARD 0x00 ��׼ ASCII 
POS_FONT_TYPE_COMPRESSED 0x01 ѹ�� ASCII  
POS_FONT_TYPE_UDC 0x02 �û��Զ����ַ� 
POS_FONT_TYPE_CHINESE 0x03 ��׼ �����塱 

nFontStyle

[in] ָ���ַ���������

����Ϊ�����б��е�һ�������ɸ���

Flag Value Meaning 
POS_FONT_STYLE_NORMAL 0x00 ���� 
POS_FONT_STYLE_BOLD 0x08 �Ӵ� 
POS_FONT_STYLE_THIN_UNDERLINE 0x80 1��ֵ��»��� 
POS_FONT_STYLE_THICK_UNDERLINE 0x100 2��ֵ��»��� 
POS_FONT_STYLE_REVERSE 0x400 ���ԣ��ڵװ��֣� 
POS_FONT_STYLE_SMOOTH 0x800 ƽ���������ڷŴ�ʱ�� 



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PARAMETER



��ע

1���ڴ�ӡ���У�һ���ڲ����������������±���ʾ�����ǲ�ͬ���ͻ�������ͬ�����Բο���ӡ���Ĳ���������

Font Type Size (W * H) 
Standard ASCII 12 * 24 / 13 * 24 
Compressed ASCII  9 * 17 
��׼���� 24 * 24 

2������ַ����nFontStyle������Ϊ�����ԣ�POS_FONT_STYLE_REVERSE������ô��ϸ�»��ߣ�POS_FONT_STYLE_THIN_UNDERLINE���� �͡����»���POS_FONT_STYLE_THICK_UNDERLINE�����Ĺ��ܽ���Ч��

3������ں����ַ�ģʽ�£���һ�ε���ʱ��׼ASCII�ַ��������ַ��ţ��ͺ��ֿ��Ի�ϴ�ӡ�������ASCII�ַ�ģʽ�£���ֻ���Դ�ӡ��׼ ASCII �ַ���ѹ����ASCII�ַ���

4���˺�������������ӡ������ַ���������һֱ�����ú��� POS_PL_Print ʱ�Ž���ʵ�ʵĽ�ֽ����ӡ�Ķ�����

5������ο� POS_S_TextOut��


*/

int __stdcall POS_PL_TextOut(

char *pszString,

int nOrgx,

int nOrgy,

int nWidthTimes,

int nHeightTimes,

int nFontType,

int nFontStyle

);



/*
����

����λͼ����ӡ�������С�



����

pszPath

[in] ָ����null ��β�İ���λͼ�ļ�·���������Ƶ��ַ�����


nOrgx

[in] ָ����Ҫ��ӡ��λͼ����߽�ľ��������

����Ϊ 0�� 65535 �㡣


nOrgy

[in] ָ����Ҫ��ӡ��λͼ���ϱ߽�ľ��������

����Ϊ 0�� 65535 �㡣


nMode

[in] ָ��λͼ�Ĵ�ӡģʽ��

����Ϊ�����б�������ֵ֮һ��

Flag Value Meaning 
POS_BITMAP_MODE_8SINGLE_DENSITY 0x00 8�㵥�ܶ� 
POS_BITMAP_MODE_8DOUBLE_DENSITY 0x01 8��˫�ܶ� 
POS_BITMAP_MODE_24SINGLE_DENSITY 0x20 24�㵥�ܶ� 
POS_BITMAP_MODE_24DOUBLE_DENSITY 0x21 24��˫�ܶ� 


����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PATH

POS_ERROR_INVALID_PARAMETER

POS_ERROR_NOT_BITMAP

POS_ERROR_NOT_MONO_BITMAP

POS_ERROR_BEYOND_AREA



��ע

1��λͼ�����Դ��� 8K �ֽڡ�

2��λͼ�����ǵ�ɫ�ġ�

3����ӡ�������и߱�����Ϊ 31 ��ߡ�

4���˺�������������ֽ����ӡλͼ������һֱ�ȵ����ú��� POS_PL_Print ʱ�Ŵ�ӡ��

5������ο� POS_PreDownloadBmpToRAM��POS_PreDownloadBmpsToFlash��POS_S_DownloadAndPrintBmp�� POS_SetLineSpacing��


*/
int __stdcall POS_PL_DownloadAndPrintBmp(

char *pszPath,

int nOrgx,

int nOrgy,

int nMode

);


/*

����

��ӡ�Ѿ����ص� RAM �е�λͼ��



����

nID

[in] ָ��λͼ�� ID �š�

����Ϊ 0 �� 7��


nOrgx

[in] ָ����Ҫ��ӡ��λͼ����߽�ľ��������

����Ϊ 0�� 65535 �㡣


nOrgy

[in] ָ����Ҫ��ӡ��λͼ���ϱ߽�ľ��������

����Ϊ 0�� 65535 �㡣


nMode

[in] ָ��λͼ�Ĵ�ӡģʽ��

����Ϊ�����б�������ֵ֮һ��

Flag Value Meaning 
POS_BITMAP_PRINT_NORMAL 0x00 ���� 
POS_BITMAP_PRINT_DOUBLE_WIDTH 0x01 ���� 
POS_BITMAP_PRINT_DOUBLE_HEIGHT 0x02 ���� 
POS_BITMAP_PRINT_QUADRUPLE 0x03 �����ұ��� 


����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PARAMETER



��ע

1���˺�������������ֽ����ӡλͼ������һֱ�ȵ����ú��� POS_PL_Print ʱ�Ŵ�ӡ��

2���ο����� POS_PreDownloadBmpToRAM��POS_S_PrintBmpInRAM��

*/
int __stdcall POS_PL_PrintBmpInRAM(

int nOrgx,

int nOrgy,

int nMode

);



/*

����

�������롣



����

pszInfoBuffer

[in] ָ���� null ��β���ַ�����ÿ���ַ�����ķ�Χ�͸�ʽ��������������йء�


nOrgx

[in] ָ����Ҫ��ӡ���������ʼ������߽�ľ��������

����Ϊ 0 ��65535��


nOrgy

[in] ָ����Ҫ��ӡ���������ʼ�����ϱ߽�ľ��������

����Ϊ 0 ��65535��


nType

[in] ָ����������͡�

����Ϊ�����б�������ֵ֮һ������ο�����¼ B ����˵������

Flag Value Meaning 
POS_BARCODE_TYPE_UPC_A 0x41 UPC-A 
POS_BARCODE_TYPE_UPC_E 0x42 UPC-C 
POS_BARCODE_TYPE_JAN13 0x43 JAN13(EAN13) 
POS_BARCODE_TYPE_JAN8 0x44 JAN8(EAN8) 
POS_BARCODE_TYPE_CODE39 0x45 CODE39 
POS_BARCODE_TYPE_ITF 0x46 INTERLEAVED 2 OF 5 
POS_BARCODE_TYPE_CODEBAR 0x47 CODEBAR 
POS_BARCODE_TYPE_CODE93 0x48 25 
POS_BARCODE_TYPE_CODE128 0x49 CODE 128 

nWidthX

[in] ָ������Ļ���Ԫ�ؿ�ȡ�

����Ϊ�����б�������ֵ��n��֮һ��

n ������ģ����
�������ͣ� ˫����ģ���ȣ���ɢ�ͣ� 
խԪ�ؿ�� ��Ԫ�ؿ�� 
2 0��25mm 0��25mm 0��625mm 
3 0��375mm 0��375mm 1��0mm 
4 0��5mm 0��5mm 1��25mm 
5 0��625mm 0��625mm 1��625mm 
6 0��75mm 0��75mm 1.875mm 


nHeight

[in] ָ������ĸ߶ȵ�����

����Ϊ 1 �� 255 ��


nHriFontType

[in] ָ�� HRI��Human Readable Interpretation���ַ����������͡�

����Ϊ�����б�������ֵ֮һ��

Flag Value Meaning 
POS_FONT_TYPE_STANDARD 0x00 ��׼ASCII 
POS_FONT_TYPE_COMPRESSED 0x01 ѹ��ASCII 

nHriFontPosition

[in] ָ��HRI��Human Readable Interpretation���ַ���λ�á�

����Ϊ�����б�������ֵ֮һ��

Flag Value Meaning 
POS_HRI_POSITION_NONE  0x00 ����ӡ 
POS_HRI_POSITION_ABOVE 0x01 ֻ�������Ϸ���ӡ 
POS_HRI_POSITION_BELOW 0x02 ֻ�������·���ӡ 
POS_HRI_POSITION_BOTH  0x03 �����ϡ��·�����ӡ 


nBytesToPrint

[in] ָ���ɲ��� pszInfoBufferָ����ַ�������������Ҫ���͸���ӡ�����ַ�����������ֵ�����������йء�



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PARAMETER



��ע

1���˺�������������ӡ���룬����һֱ�����ú��� POS_PL_Printʱ�Ŵ�ӡ��

2������ο� POS_S_SetBarcode �� ����¼ B ����˵������

*/
int __stdcall POS_PL_SetBarcode(

char *pszInfoBuffer,

int nOrgx,

int nOrgy,

int nType,

int nWidthX,

int nHeight,

int nHriFontType,

int nHriFontPosition,

int nBytesToPrint 

);




/*


����

��ӡҳ���ǩ�������е����ݡ�



����

�ޡ�



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE



��ע

1����������ɹ����򽫽�ֽ����ӡƱ�棬����ҳ���������ǩ���������ݻ��Ǳ������ţ������ٴε��ô˺���������ӡҳ���������ǩ�������е�Ʊ�档

2�����Ե��� POS_PL_Clear �����ҳ���������ǩ�������е����ݡ�

*/
int __stdcall POS_PL_Print(void);


/*
 
����
 
���Ʊ��ͱ�ǩ�Ĵ�ӡ�������е����ݡ�


 
����
 
�ޡ�


 
����ֵ
 
��������ɹ����򷵻�ֵΪ POS_SUCCESS��
 
�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��
POS_FAIL
POS_ERROR_INVALID_HANDLE


 
��ע
 
1����������ɹ������ӡ���ڲ��ĵ�ǰҳ�������������
 
2������ο� POS_PL_Print��POS_SetMode��


*/
int __stdcall POS_PL_Clear(void);



/*


����

�������ݵ��˿ڻ��ļ���



����

hPort

[in] �˿ڻ��ļ������


pszData

[in] ָ��Ҫ���͵����ݻ�������


nBytesToWrite

[in] ָ����Ҫ���͵����ݵ��ֽ�����



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PARAMETER



��ע

1���˺������������ԡ�

2������ο� POS_ReadFile��

*/
int __stdcall POS_WriteFile(

HANDLE hPort,

char *pszData,

int nBytesToWrite

);




/*

����

�Ӵ��ڣ���USB�˿ڻ��ļ������ݵ�ָ���Ļ�������



����

hPort

[in] �˿ڻ��ļ������


pszData

[in] ָ��Ҫ��ȡ�����ݻ�������


nBytesToWrite

[in] ָ����Ҫ��ȡ�����ݵ��ֽ�����



����ֵ

��������ɹ����򷵻�ֵΪ POS_SUCCESS��

�������ʧ�ܣ��򷵻�ֵΪ����ֵ֮һ��

POS_FAIL

POS_ERROR_INVALID_HANDLE

POS_ERROR_INVALID_PARAMETER



��ע

1���˺������������ԡ�

2���˺�����֧������ӿڡ�

3������ο� POS_WriteFile��


*/
int __stdcall POS_ReadFile(

HANDLE hPort,

char *pszData,

int nBytesToRead,

int nTimeouts

);



/*


����

�ı�dll�ڲ��Ķ˿ڻ��ļ������



����

hNewHandle

[in] �����滻dll�ڲ��ľ����



����ֵ

��������ɹ����򷵻�dll�ڲ��ľ������ʹ��hNewHandle�滻�ڲ������

�������ʧ�ܣ��򷵻� INVALID_HANDLE_VALUE��-1����



��ע

1���˺������������ԡ�

2������ο� POS_WriteFile��POS_ReadFile��


*/
HANDLE __stdcall POS_SetHandle(

HANDLE hNewHandle

);



/*

����

��ȡ��ǰ dll �ķ����汾�š�




����

pnMajor

[out] ���汾�š�


pnMinor

[out] �ΰ汾�š�




����ֵ

��������ɹ����򷵻ش� dll �Ľ������ڡ�




��ע

�˺���һ���������Ի����ʹ�á�



*/
int __stdcall POS_GetSWVersionInfo(

int *pnMajor,

int *pnMinor

);


/*

����

��ȡ��ǰ dll �ķ����汾�š�




����

pnMajor

[out] ���汾�š�


pnMinor

[out] �ΰ汾�š�




����ֵ

��������ɹ����򷵻ش� dll �Ľ������ڡ�




��ע

�˺���һ���������Ի����ʹ�á�



*/
int __stdcall POS_GetHWVersionInfo(

int *pnMajor,

int *pnMinor

);



#ifdef __cplusplus
}
#endif

#endif