string CFaceVerify::GetFaceDeviceSerial()
{
	string serialNum = "NULL";

	// Get Device path
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData = { 0 };
	DeviceInfoData.cbSize = sizeof(DeviceInfoData);

	// get device class information handle
	hDevInfo = ::SetupDiGetClassDevsA(&/*GUID_DEVCLASS_USB*/GUID_DEVINTERFACE_USB_HUB, 0, 0, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));
	if (hDevInfo == INVALID_HANDLE_VALUE){
		cout << "hDevInfo Invalid" << endl;
	}
#if 0
	// enumerute device information
	DWORD required_size = 0;
	for (int i = 0; ::SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++){
		DWORD DataT;
		char friendly_name[2046] = { 0 };
		DWORD buffersize = 2046;
		DWORD req_bufsize = 0;

		// get device description information
		if (!::SetupDiGetDeviceRegistryPropertyA(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC, &DataT, (LPBYTE)friendly_name, buffersize, &req_bufsize)){
			cout << "SetupDiGetDeviceRegistryPropertyA failed" << endl;
			continue;
		}
		cout << friendly_name << endl;
	}
#endif
	// enum HUB interface
	SP_DEVICE_INTERFACE_DATA interfaceData = { 0 };
	interfaceData.cbSize = sizeof(interfaceData);
	for (int i = 0; ::SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &GUID_DEVINTERFACE_USB_HUB, i, &interfaceData); i++){
		ULONG requiredLength = 0;
		::SetupDiGetDeviceInterfaceDetailA(hDevInfo, &interfaceData, NULL, 0, &requiredLength, NULL);
		PSP_DEVICE_INTERFACE_DETAIL_DATA_A interfaceDataDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA_A)malloc(requiredLength);
		interfaceDataDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);
		::SetupDiGetDeviceInterfaceDetailA(hDevInfo, &interfaceData, interfaceDataDetail, requiredLength, &requiredLength, NULL);
		
		// find target camera hub
		if (-1 != string(interfaceDataDetail->DevicePath).find(CAM_HUB_VID) &&
			-1 != string(interfaceDataDetail->DevicePath).find(CAM_HUB_PID)){
			cout << interfaceDataDetail->DevicePath << endl;
			string DevicePath = interfaceDataDetail->DevicePath;
			//string DevicePath = "\\\\.\\usb#vid_05e3&pid_0618#6&b7c5497&0&2#{f18a0e88-c30c-11d0-8815-00a0c906bed8}";
			//string DevicePath = "\\\\?\\usb#vid_0bda&pid_2131&mi_00#7&2be7aaf3&0&0000#{65e8773d-8f56-11d0-a3b9-00a0c9223196}\\global";
			//string DevicePath = "\\\\.\\H:";
			HANDLE hDev = ::CreateFileA(DevicePath.data(), GENERIC_READ | GENERIC_WRITE, 
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
			if (hDev == INVALID_HANDLE_VALUE){ 
				cout << "CreateFile failed" << endl;
			}
			else{
				// look for devices on 4-ports HUB
				ULONG nBytes = sizeof(USB_NODE_CONNECTION_INFORMATION) + sizeof(USB_PIPE_INFO) * 30;
				for (int idx = 1; idx < 5; idx++){
					PUSB_NODE_CONNECTION_INFORMATION connectionInfo = (PUSB_NODE_CONNECTION_INFORMATION)GlobalAlloc(GPTR, nBytes);
					connectionInfo->ConnectionIndex = idx;
					if (::DeviceIoControl(hDev, IOCTL_USB_GET_NODE_CONNECTION_INFORMATION, connectionInfo, nBytes, connectionInfo, nBytes, &nBytes, NULL)){
						PSTRING_DESCRIPTOR_NODE stringDescs = GetAllStringDescriptors(hDev, connectionInfo->ConnectionIndex, &connectionInfo->DeviceDescriptor);		
						if (stringDescs){
							// SN format: PIRA2RGB******	or	PIRA2NIR******
							if (-1 != wstring(stringDescs->Next->StringDescriptor->bString).find(L"PIRA2RGB")){
								std::wcout << "iManufactuer:" << stringDescs->Next->StringDescriptor->bString << endl;
								char* cSerial = Unicode2Ansi(stringDescs->Next->StringDescriptor->bString);
								serialNum = cSerial;
								delete[] cSerial;
							}
						}
						else{
							cout << "port:" << connectionInfo->ConnectionIndex << " NULL" << endl;
						}
					}
					else{
						cout << "DeviceIoControl failed:" << ::GetLastError() << endl;
					}
					free(connectionInfo);
				}
				//WINUSB_INTERFACE_HANDLE hUsb;
				//if (WinUsb_Initialize(hDev, &hUsb)){
				//	cout << "Init OK" << endl;
				//	USB_DEVICE_DESCRIPTOR desc;
				//	ULONG nRetLen = 0;
				//	ZeroMemory(&desc, sizeof(desc));
				//	if (WinUsb_GetDescriptor(hUsb, USB_DEVICE_DESCRIPTOR_TYPE, 0, 0, (PUCHAR)&desc, sizeof(desc), &nRetLen))
				//	{
				//		UCHAR buf[256] = { 0 };
				//		if (WinUsb_GetDescriptor(hUsb, USB_STRING_DESCRIPTOR_TYPE, desc.iManufacturer, 0x0409, buf, sizeof(buf), &nRetLen)){
				//			if (0 != nRetLen){
				//				PUSB_STRING_DESCRIPTOR pTmp = (PUSB_STRING_DESCRIPTOR)buf;
				//				MessageBox(NULL, pTmp->bString, L"", 0);
				//			}
				//		}
				//	}
				//	else{
				//		cout << "WinUsb_GetDescriptor failed" << endl;
				//	}
				//	WinUsb_Free(hUsb);
				//}
				//else{
				//	int err = GetLastError();
				//	cout << "WinUsb_Initialize failed:" << err << endl;
				//}
			}
			::CloseHandle(hDev);
		}
	}
	::SetupDiDestroyDeviceInfoList(hDevInfo);

	return serialNum;
}