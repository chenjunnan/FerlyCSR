#include "USB.h"
#include "shlwapi.h"

#ifdef __cplusplus
extern "C" {
#endif

	/*int WaitForDeviceEvent()
	{
		int dev_status = 0;
		GUID hidGuid;
		HidD_GetHidGuid(&hidGuid);
		HDEVINFO hDevInfo;
		SP_DEVICE_INTERFACE_DATA devInfoData;
		devInfoData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);
		int deviceNo = 0;
		SetLastError(NO_ERROR);
		while (!stopWait)
		{
			if (GetLastError() == ERROR_NO_MORE_ITEMS)
			{
				if (!dev_status)
				{
					return 0;
				}
				dev_status = 0;
				deviceNo = 0;
			}
			hDevInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));
			if (hDevInfo == INVALID_HANDLE_VALUE)
			{
				DWORD w = GetLastError();
				return 1;
			}
			

			if (SetupDiEnumInterfaceDevice(hDevInfo, 0, &hidGuid, deviceNo, &devInfoData))
			{
				ULONG  requiredLength = 0;
				SetupDiGetInterfaceDeviceDetail(hDevInfo,
					&devInfoData,
					NULL,
					0,
					&requiredLength,
					NULL);
				PSP_INTERFACE_DEVICE_DETAIL_DATA devDetail = (SP_INTERFACE_DEVICE_DETAIL_DATA*)malloc(requiredLength);
				devDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
				if (!SetupDiGetInterfaceDeviceDetail(hDevInfo,
					&devInfoData,
					devDetail,
					requiredLength,
					NULL,
					NULL))
				{
					free(devDetail);
					SetupDiDestroyDeviceInfoList(hDevInfo);
					return 1;
				}
				
				if (StrStr(devDetail->DevicePath,L"#vid_0483&pid_5750"))
				{
					dev_status = 1;
				}
				free(devDetail);
				SetupDiDestroyDeviceInfoList(hDevInfo);
				++deviceNo;
			}
		}
		return 1;
	}*/

	HANDLE OpenHIDDevice(int overlapped)
	{
		HANDLE hidHandle = NULL;
		GUID hidGuid;
		HidD_GetHidGuid(&hidGuid);
		HDEVINFO hDevInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));
		if (hDevInfo == INVALID_HANDLE_VALUE)
		{
			return INVALID_HANDLE_VALUE;
		}
		SP_DEVICE_INTERFACE_DATA devInfoData;
		devInfoData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);
		int deviceNo = 0;
		SetLastError(NO_ERROR);
		while (GetLastError() != ERROR_NO_MORE_ITEMS && deviceNo < 20)
		{
			if (SetupDiEnumInterfaceDevice(hDevInfo, 0, &hidGuid, deviceNo, &devInfoData))
			{
				ULONG  requiredLength = 0;
				SetupDiGetInterfaceDeviceDetail(hDevInfo,
					&devInfoData,
					NULL,
					0,
					&requiredLength,
					NULL);
				PSP_INTERFACE_DEVICE_DETAIL_DATA devDetail = (SP_INTERFACE_DEVICE_DETAIL_DATA*)malloc(requiredLength);
				devDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
				if (!SetupDiGetInterfaceDeviceDetail(hDevInfo,
					&devInfoData,
					devDetail,
					requiredLength,
					NULL,
					NULL))
				{
					free(devDetail);
					SetupDiDestroyDeviceInfoList(hDevInfo);
					return INVALID_HANDLE_VALUE;
				}
				if (overlapped)
				{
					hidHandle = CreateFile(devDetail->DevicePath,
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						FILE_FLAG_OVERLAPPED,
						NULL);
				}
				else
				{
					hidHandle = CreateFile(devDetail->DevicePath,
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						0,
						NULL);
				}
				free(devDetail);
				_HIDD_ATTRIBUTES hidAttributes;
				HidD_GetAttributes(hidHandle, &hidAttributes);
				if (USB_VID == hidAttributes.VendorID
					&& USB_PID == hidAttributes.ProductID)
				{
					//printf("找到了我想要的设备....\r\n");
					return hidHandle;
				}
				else
				{
					CloseHandle(hidHandle);
					hidHandle = INVALID_HANDLE_VALUE;
					++deviceNo;
				}
			}
		}

		SetupDiDestroyDeviceInfoList(hDevInfo);
		return INVALID_HANDLE_VALUE;
	}

#ifdef __cplusplus
}
#endif