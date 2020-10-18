#pragma once
#include "windows.h"

#ifdef __cplusplus
extern "C" {
#endif

	// This file is in the Windows DDK available from Microsoft.
	#include "hidsdi.h"
	#include <setupapi.h>
	#include <dbt.h>
	#include <setupapi.h>
	/* Private define ------------------------------------------------------------*/
	#define     USB_VID			0x0483	//���ֵ�ڵ�Ƭ��������豸�������ж��� 
	#define     USB_PID			0x5750	//���ֵ�ڵ�Ƭ��������豸�������ж���   
	#define		REPORT_COUNT	64		//�˵㳤��


	static BOOL stopWait = FALSE;
	/* Private function prototypes -----------------------------------------------*/

	HANDLE OpenHIDDevice(int overlapped);
	int WaitForDeviceEvent();

#ifdef __cplusplus
}
#endif