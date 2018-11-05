#include "stdafx.h"
#include "HIDAccess.h"
#include <dbt.h>
#include <iomanip>

#define HARDWARE_REPORT_ID (0)

CHIDAccess::CHIDAccess()
{
	::InitializeCriticalSection(&m_csWrite);
	::InitializeCriticalSection(&m_csRead);
}


CHIDAccess::~CHIDAccess()
{
	::DeleteCriticalSection(&m_csWrite);
	::DeleteCriticalSection(&m_csRead);
}


// List all HID class device in system
int CHIDAccess::ListAllHidDevice(vector<pair<wstring, wstring>>& vidPidProduct)
{
	cout << "ListAllHidDevice Start" << endl;
	vidPidProduct.clear();
	GUID guid;
	HidD_GetHidGuid(&guid);
	HDEVINFO hDevInfo = ::SetupDiGetClassDevsW(&guid, NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));

	SP_DEVICE_INTERFACE_DATA devInterfaceData = { 0 };
	devInterfaceData.cbSize = sizeof(devInterfaceData);

	// Enum all hid devices
	for (int idx = 0; ::SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &guid, idx, &devInterfaceData); idx++){
		DWORD length = 0;
		SetupDiGetDeviceInterfaceDetailW(hDevInfo, &devInterfaceData, NULL, 0, &length, NULL);
		PSP_DEVICE_INTERFACE_DETAIL_DATA pDevtail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(length);
		pDevtail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		// Get device detail info
		if (SetupDiGetDeviceInterfaceDetailW(hDevInfo, &devInterfaceData, pDevtail, length, NULL, NULL)){
			// Device handle
			HANDLE devHandle = CreateFileW(pDevtail->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
				(LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, 0, NULL);
			if (devHandle == INVALID_HANDLE_VALUE){
				//cout << "devHandle Invalid" << endl;
				free(pDevtail);
				continue;
			}
			//std::wcout << "path:" << pDevtail->DevicePath << endl;

			HIDD_ATTRIBUTES hidAttributes;
			hidAttributes.Size = sizeof(hidAttributes);
			if (!HidD_GetAttributes(devHandle, &hidAttributes)){
				cout << "HidD_GetAttributes failed:" << GetLastError() << endl;
				free(pDevtail);
				::CloseHandle(devHandle);
				continue;
			}
			WCHAR bufVidPid[64] = { 0 };
			swprintf_s(bufVidPid, L"vid:%04x_pid:%04x", hidAttributes.VendorID, hidAttributes.ProductID);
			//std::wcout << bufVidPid << endl;

			WCHAR bufProduct[MAX_PATH] = { 0 };
			ULONG len = MAX_PATH * 2;
			HidD_GetProductString(devHandle, bufProduct, len);
			//std::wcout << "Product:" << bufProduct << endl;

			vidPidProduct.push_back(pair<wstring, wstring>(bufVidPid, bufProduct));
			::CloseHandle(devHandle);
		}
		else{
			cout << "SetupDiGetDeviceInterfaceDetailW failed:" << ::GetLastError() << endl;
		}


		free(pDevtail);
		
	}
	::SetupDiDestroyDeviceInfoList(hDevInfo);

	cout << "ListAllHidDevice Fin" << endl;
	return 0;
}

// Get Hid device handle by vid and pid 
int CHIDAccess::OpenHidDevice(USHORT vid, USHORT pid, HANDLE* writeHandle, HANDLE* readHandle)
{
    int ret = -1;
	GUID guid;
	HidD_GetHidGuid(&guid);
	HDEVINFO hDevInfo = ::SetupDiGetClassDevsW(&guid, NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));

	SP_DEVICE_INTERFACE_DATA devInterfaceData = { 0 };
	devInterfaceData.cbSize = sizeof(devInterfaceData);
	 
	// Enum all hid devices
	for (int idx = 0; ::SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &guid, idx, &devInterfaceData); idx++){
		DWORD length = 0;
		SetupDiGetDeviceInterfaceDetailW(hDevInfo, &devInterfaceData, NULL, 0, &length, NULL);
		PSP_DEVICE_INTERFACE_DETAIL_DATA pDevtail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(length);
		pDevtail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		// Get device detail info
		if (SetupDiGetDeviceInterfaceDetailW(hDevInfo, &devInterfaceData, pDevtail, length, NULL, NULL)){
			// Device handle
            HANDLE devHandle = CreateFileW(pDevtail->DevicePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL| FILE_FLAG_OVERLAPPED, NULL);
			if (devHandle == INVALID_HANDLE_VALUE){
				free(pDevtail);
				continue;
			}
			 
			HIDD_ATTRIBUTES hidAttributes;
			hidAttributes.Size = sizeof(hidAttributes);
			if (!HidD_GetAttributes(devHandle, &hidAttributes)){
				cout << "HidD_GetAttributes failed:" << GetLastError() << endl;
				free(pDevtail);
				::CloseHandle(devHandle);
				continue;
			}
			if (vid == hidAttributes.VendorID && pid == hidAttributes.ProductID){
				// Get HID device info
				//HidD_SetNumInputBuffers(devHandle, 64);		// Input report buffer 64 reports
				PHIDP_PREPARSED_DATA pp_data = NULL;
				BOOLEAN res = HidD_GetPreparsedData(devHandle, &pp_data);
				if (!res){
					cout << "Get HID preparse data error" << endl;
					free(pDevtail);
					::CloseHandle(devHandle);
					continue;
				}
				HIDP_CAPS caps;
				NTSTATUS nt_res = HidP_GetCaps(pp_data, &caps);
				if (nt_res != HIDP_STATUS_SUCCESS){
					cout << "Get HID Caps error" << endl;
					free(pDevtail);
					::CloseHandle(devHandle);
					continue;
				}
				m_nInputReportSize = caps.InputReportByteLength;
				m_nOutputReportSize = caps.OutputReportByteLength;
				cout << "HID report size:" << m_nInputReportSize << " " << m_nOutputReportSize << endl;

				// Regist device TODO: device disconnect report
#if 0
				DEV_BROADCAST_DEVICEINTERFACE DevInt;
				memset(&DevInt, 0, sizeof(DEV_BROADCAST_DEVICEINTERFACE));
				DevInt.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
				DevInt.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
				DevInt.dbcc_classguid = guid;
				::RegisterDeviceNotification(NULL, &DevInt, DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);
#endif

				// break when found the first matched device
                if (NULL != readHandle && NULL != writeHandle){
                    *readHandle = devHandle;
                    *writeHandle = CreateFileW(pDevtail->DevicePath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                        (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, NULL, NULL);
                }
                else{
                    ::CloseHandle(devHandle);
                }
				free(pDevtail);

                ret = 0;
				break;
			}
			::CloseHandle(devHandle);
		}
		free(pDevtail);
	}

	::SetupDiDestroyDeviceInfoList(hDevInfo);
	
    return ret;
}

// Close Device Handle
int CHIDAccess::CloseHidDevice(HANDLE* handle)
{
	if (*handle){
		::CloseHandle(*handle);
		*handle = NULL;
	}
	return 0;
}

// Send Synchronously
int CHIDAccess::SendHidMsg(HANDLE handle, BYTE* msg, UINT msgLen)
{
	if (!handle){
        cout << "SendHidMsg invalid handle" << endl;
		return -1;
	}

    DWORD writeLen = 0;
    BOOL writeRet = FALSE;
	if (msgLen >= m_nOutputReportSize){
        int cnt = 0;
		// Send multi packages
        for (BYTE* pSend = msg; pSend < msg + msgLen; pSend += (m_nOutputReportSize - 1)){
            BYTE* sendBuf = new BYTE[m_nOutputReportSize];
            memset(sendBuf, 0, m_nOutputReportSize);
            sendBuf[0] = HARDWARE_REPORT_ID;
            memcpy(sendBuf + 1, pSend, msg + msgLen - pSend > (m_nOutputReportSize - 1) ? (m_nOutputReportSize - 1) : msg + msgLen - pSend);
            ::EnterCriticalSection(&m_csWrite);
            writeRet = ::WriteFile(handle, sendBuf, m_nOutputReportSize, &writeLen, NULL);
            ::LeaveCriticalSection(&m_csWrite);
            delete[] sendBuf;
            if (!writeRet){
                break;
            }
            cout << "Send " << ++cnt << " OK" << endl;
        }
	}
	else{
        //  Send only 1 package
        BYTE* sendBuf = new BYTE[m_nOutputReportSize + 1];
        memset(sendBuf, 0, m_nOutputReportSize);
        sendBuf[0] = HARDWARE_REPORT_ID;
        memcpy(sendBuf + 1, msg, msgLen);
        ::EnterCriticalSection(&m_csWrite);
        writeRet = ::WriteFile(handle, sendBuf, m_nOutputReportSize, &writeLen, NULL);
        ::LeaveCriticalSection(&m_csWrite);
        delete[] sendBuf;
	}
    
    if (!writeRet){
        cout << "WriteFile Fialed:" << ::GetLastError() << endl;
        return -2;
	}
	return 0;
}

// Recv
int CHIDAccess::RecvHidMsg(HANDLE handle, BYTE* buf, UINT& msgLen, UINT timeout)
{
	int ret = 0;
	if (!handle){
		return -1;
	}
	DWORD recvLen = 0;
	::EnterCriticalSection(&m_csRead);
    // Asynchronously read
    BOOL readRet = ::ReadFile(handle, buf, m_nInputReportSize, NULL, &m_overlapped);
	::LeaveCriticalSection(&m_csRead);
    int errCode = ::GetLastError();
    if (!readRet && ERROR_IO_PENDING != errCode){
        printf("ReadFile Fialed:0x%02X\n", errCode);
        if (ERROR_DEVICE_NOT_CONNECTED == errCode){
            ret = -2;
        }
        else{
            ret = -3;
        }
	}
    else{
        if (WAIT_TIMEOUT == ::WaitForSingleObject(m_overlapped.hEvent, timeout)){
            // recv data timeout this time
            ret = -4;
        }
        else{
            readRet = ::GetOverlappedResult(handle, &m_overlapped, &recvLen, TRUE);
            if (!readRet){
                errCode = ::GetLastError();
                cout << "GetOverlappedResult error:x%x" << std::hex << errCode << endl;
                ret = -3;
            }
            else if(recvLen > 1){
                msgLen = recvLen - 1;
                // remove hardware report ID 
                for (int idx = 0; idx < (int)(recvLen - 1); idx++){
                    buf[idx] = buf[idx + 1];
                }
#if 1
                // save recv data for debug
                SYSTEMTIME sysTime;
                ::GetLocalTime(&sysTime);
                char szTime[32] = { 0 };
                sprintf_s(szTime, "%02d/%02d-%02d:%02d:%02d-%03d\t", sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
                std::ofstream out;
                out.open("recvData.dat", std::ios::app);
                string log = "";
                for (UINT idx = 0; idx < recvLen; idx++){
                    char szHex[3] = { 0 };
                    sprintf_s(szHex, "%02X", buf[idx]);
                    log += szHex + string(" ");
                }
                if (0xF0 != buf[4] && 0xF1 != buf[4]){
                    cout << "[R]: " << log << endl;
                    log += "\n";
                    out.write(szTime, strlen(szTime));
                    out.write(log.data(), log.size());
                }

                out.close();
#endif 
            }
            else{
                // something may be wrong...
                msgLen = 0;
            }
        }
    }
	return ret;
}

// Calcute the check sum
UCHAR CHIDAccess::GetCheckSum(UCHAR* data, UINT dataLen)
{
	UCHAR uSum = 0;
	for (UCHAR idx = 0; idx < dataLen; idx++){
		uSum = uSum + data[idx];
	}
	uSum = (~uSum) + 1;
	return uSum;
}