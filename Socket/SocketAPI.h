#include "stdafx.h"
#include "SocketAPI.h"


DEFINE_GUID(g_guidServiceClass, 0xb62c4e8d, 0x62cc, 0x404b, 0xbb, 0xbf, 0xbf, 0x3e, 0x3b, 0xbb, 0x13, 0x74);
WCHAR g_BthInstanceName[] = L"BthSocketInstance";

CSocketAPI::CSocketAPI()
	: m_pCSAddrInfo(nullptr)
	, m_sockListen(INVALID_SOCKET)
	, m_sockLocal(INVALID_SOCKET)
{
	StartUp();
	::InitializeCriticalSection(&m_csLog);
	::InitializeCriticalSection(&m_csSend);
}


CSocketAPI::~CSocketAPI()
{
	CleanUp();
	::DeleteCriticalSection(&m_csLog);
	::DeleteCriticalSection(&m_csSend);
}


// WSAStartup
int CSocketAPI::StartUp()
{
	WSADATA WSAData = { 0 };
	int ret = ::WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (0 != ret) {
		ret = ::WSAGetLastError();
        cout << "WSAStartup err:" << ret << endl;
	}
	return ret;
}

// WSACleanup
int CSocketAPI::CleanUp()
{
	int ret = ::WSACleanup();
	if (0 != ret){
		ret = ::WSAGetLastError();
        cout << "WSACleanup err:" << ret << endl;
	}
	return ret;
}

// socket
int CSocketAPI::InitSocket(SOCKET& sock, int socketType)
{
	int ret = 0;
	switch (socketType){
	case SOCKET_TCP:
		sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		break;
	case SOCKET_UDP:
		sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        EnableBlocking(sock, false);
		break;
	case SOCKET_BTH:
		sock = ::socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
		break;
	default:
		ret = -2;
		break;
	}
	if (0 == ret && INVALID_SOCKET == sock){
        ret = ::WSAGetLastError();
        cout << "socket err:" << ret << endl;
	}

	return ret;
}

// bind
int CSocketAPI::BindSocket(SOCKET& sock, int socketType, USHORT port)
{
	int ret = 0;
	switch (socketType){
	case SOCKET_TCP:
	case SOCKET_UDP:{
		SOCKADDR_IN sockAddr;
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = htons(port);
		sockAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		ret = ::bind(sock, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr));
		break;
	}
	case SOCKET_BTH:{
		SOCKADDR_BTH sockAddr;
		sockAddr.addressFamily = AF_BTH;
		sockAddr.port = BT_PORT_ANY;
		ret = ::bind(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR_BTH));
		break;
	}
	default:
		ret = -2;
		break;
	}

	if (SOCKET_ERROR == ret){
		ret = ::WSAGetLastError();
        cout << "bind err:" << ret << endl;
	}
	return ret;
}

// connect
int CSocketAPI::ConnectSocket(SOCKET& sock, int socketType, char* addr, USHORT port, int timeout)
{
	int ret = 0;
	switch (socketType){
	case SOCKET_TCP:
	case SOCKET_UDP:{
		sockaddr_in sockAddr;
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_addr.s_addr = inet_addr(addr);
		sockAddr.sin_port = htons(port);
		EnableBlocking(sock, false);
		ret = ::connect(sock, (sockaddr*)&sockAddr, sizeof(sockAddr));
		break;
	}
	case SOCKET_BTH:{
		SOCKADDR_BTH sockAddr;
		int iAddrLen = sizeof(sockAddr);
		ret = WSAStringToAddressA(addr, AF_BTH, NULL, (LPSOCKADDR)&sockAddr, &iAddrLen);
		if (0 != ret){
			wprintf(L"Failed in WSAStringToAddressA. err:%d\n", ::WSAGetLastError());
			ret = ::WSAGetLastError();
			break;
		}
		sockAddr.addressFamily = AF_BTH;
		sockAddr.serviceClassId = g_guidServiceClass;
		sockAddr.port = 0;
		EnableBlocking(sock, false);
		ret = ::connect(sock, (sockaddr*)&sockAddr, sizeof(sockAddr));
		break;
	}
	default:
		ret = -2;
		break;
	}
	if (SOCKET_ERROR == ret){
		int errCode = ::WSAGetLastError();
		if (WSAEWOULDBLOCK == errCode){
			struct timeval tm;
			tm.tv_sec = timeout;
			tm.tv_usec = 0;
			FD_SET readSet;
			FD_SET writeSet;
			FD_ZERO(&readSet);
			FD_ZERO(&writeSet);
			FD_SET(sock, &readSet);
			FD_SET(sock, &writeSet);
			ret = select(0, &readSet, &writeSet, nullptr, &tm);
			if (SOCKET_ERROR == ret){
                ret = ::WSAGetLastError();
                cout << "connect err:" << ret << endl;
			}
			else if (0 == ret){
				ret = -1;		// timeout
			}
			else if (FD_ISSET(sock, &writeSet)){
				ret = 0;		// OK
			}
			else{
				ret = 0;		// exceptions
			}
		}
		else{
			ret = errCode;
            cout << "WSACleanup err:" << ret << endl;
		}
	}
	return ret;
}

// accept
int CSocketAPI::AcceptSocket(SOCKET& sock, SOCKET& remoteSock)
{
	int ret = 0;
	remoteSock = ::accept(sock, nullptr, nullptr);
	if (INVALID_SOCKET == remoteSock){
        ret = ::WSAGetLastError();
        cout << "accept err:" << ret << endl;
	}
	return ret;
}

// listen
int CSocketAPI::ListenSocket(SOCKET& sock, int backLog)
{
	int ret = 0;
	int nQueSize = min(backLog, SOMAXCONN);
	ret = ::listen(sock, nQueSize);
	if (SOCKET_ERROR == ret){
        ret = ::WSAGetLastError();
        cout << "listen err:" << ret << endl;
	}
	return ret;
}

// SetService
int CSocketAPI::BthSetService(SOCKET& sock)
{
	int ret = 0;
	int sockLen = sizeof(SOCKADDR_BTH);
	SOCKADDR_BTH sockAddr;
	ret = ::getsockname(sock, (SOCKADDR*)&sockAddr, &sockLen);
	if (0 == ret){
		// CSADDR_INFO
		if (m_pCSAddrInfo){
			delete m_pCSAddrInfo;
		}
		m_pCSAddrInfo = new CSADDR_INFO;
		m_pCSAddrInfo->LocalAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
		m_pCSAddrInfo->LocalAddr.lpSockaddr = (LPSOCKADDR)&sockAddr;
		m_pCSAddrInfo->RemoteAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
		m_pCSAddrInfo->RemoteAddr.lpSockaddr = (LPSOCKADDR)&sockAddr;
		m_pCSAddrInfo->iSocketType = SOCK_STREAM;
		m_pCSAddrInfo->iProtocol = BTHPROTO_RFCOMM;

		// WSASetService
		m_wsaQuerySet.dwSize = sizeof(WSAQUERYSET);
		m_wsaQuerySet.lpServiceClassId = const_cast<LPGUID>(&g_guidServiceClass);
		m_wsaQuerySet.lpszServiceInstanceName = g_BthInstanceName;
		m_wsaQuerySet.lpszComment = L"Bluetooth Socket Connection";
		m_wsaQuerySet.dwNameSpace = NS_BTH;
		m_wsaQuerySet.dwNumberOfCsAddrs = 1;
		m_wsaQuerySet.lpcsaBuffer = m_pCSAddrInfo;
		if (SOCKET_ERROR == ::WSASetServiceW(&m_wsaQuerySet, RNRSERVICE_REGISTER, 0)) {
			ret = ::WSAGetLastError();
		}
		else{
			ret = 0;
		}
	}
	else{
		ret = ::WSAGetLastError();
	}

	return ret;
}

// Set socket blocking or not
int CSocketAPI::EnableBlocking(SOCKET& sock, bool bBlock)
{
    ULONG mode = 0;
    if (!bBlock){
        mode = 1;
    }

    int ret = ::ioctlsocket(sock, FIONBIO, &mode);
    if (SOCKET_ERROR == ret) {
        ret = ::WSAGetLastError();
        cout << "ioctlsocket error:" << ret << endl;
    }
    return ret;
}

// Set Recv/Send Timeout
int CSocketAPI::SetSocketTimeout(SOCKET& sock, int type, int timeout)
{
    int ret = 0;
    switch (type){
        case SOCKET_RECV:
            ret = ::setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));
            break;
        case SOCKET_SEND:
            ret = ::setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));
            break;
        default:
            ret = -2;
            break;
    }
    if (SOCKET_ERROR == ret){
        ret = ::WSAGetLastError();
        cout << "setsockopt error:" << ret << endl;
    }
    return ret;
}


/***************************************************************************************
*                                   APIs                                               *
***************************************************************************************/
// Start ServerSocket
int CSocketAPI::StartServerSocket(USHORT port, int maxClientCnt, int socketType)
{
    int m_nSocketType = socketType;
    int ret = 0;
    if (SOCKET_UDP == socketType){
        ret = InitSocket(m_sockLocal, socketType);
        if (0 == ret){
            ret = BindSocket(m_sockLocal, socketType, port);
        }
    }
    else{
        // TCP or BTH
        ret = InitSocket(m_sockListen, socketType);
        if (0 == ret){
            ret = BindSocket(m_sockListen, socketType, port);
            if (0 == ret){
                if (SOCKET_BTH == socketType){
                    ret = BthSetService(m_sockListen);
                }
                if (0 == ret){
                    if (maxClientCnt <= 0){
                        maxClientCnt = SOMAXCONN;
                    }
                    m_nMaxClient = maxClientCnt;
                    ret = ListenSocket(m_sockListen, maxClientCnt);
                    m_hAcceptThread = (HANDLE)_beginthreadex(nullptr, 0, ThreadAccept, this, 0, nullptr);
                }
            }
        }
    }
    if (0 == ret){
        m_nWorkMode = WORK_MODE_SERVER;
        int m_nSocketType = socketType;
    }

    return ret;
}

// Soecket server accpet thread
UINT WINAPI CSocketAPI::ThreadAccept(LPVOID param)
{
    CSocketAPI* pSocketClass = (CSocketAPI*)param;
    if (!pSocketClass){
        return -1;
    }

    // accept loop
    while (INVALID_SOCKET != pSocketClass->m_sockListen){
        SOCKET* pClient = new SOCKET;
        if (0 == pSocketClass->AcceptSocket(pSocketClass->m_sockListen, *pClient)){
            pSocketClass->m_vecpSockRemote.push_back(pClient);
        }
        else{
            delete pClient;
        }
        while (pSocketClass->m_vecpSockRemote.size() >= pSocketClass->m_nMaxClient){
            Sleep(50);
        }
    }

    return 0;
}

// Start ClientSocket
int CSocketAPI::StartClientSocket(char* addr, USHORT port, int timeout, int socketType)
{
    int ret = 0;

    if (SOCKET_UDP == socketType){
        // UDP Server address
        SOCKADDR_IN* pSockAddr = new SOCKADDR_IN;
        pSockAddr->sin_port = htons(port);
        pSockAddr->sin_family = AF_INET;
        pSockAddr->sin_addr.S_un.S_addr = inet_addr(addr);
        m_vecpSockAddr.push_back(pSockAddr);

        ret = InitSocket(m_sockLocal, socketType);
    }
    else{
        // TCP or BTH connect
        ret = InitSocket(m_sockLocal, socketType);
        if (0 == ret){
            ret = ConnectSocket(m_sockLocal, socketType, addr, port, timeout);
        }
    }
    if (0 == ret){
        m_nWorkMode = WORK_MODE_CLIENT;
        int m_nSocketType = socketType;
    }

    return ret;
}

// TCP server send msg
int CSocketAPI::SendMsg(UINT sockIdx, char* sendBuf, int totalSendSize)
{
	int ret = 0;
    if (sockIdx >= m_vecpSockRemote.size()){
        cout << "SendMsg sockIdx[%d][%d] error" << sockIdx << m_vecpSockRemote.size() << endl;
        return -2;
    }
    SOCKET sock = *m_vecpSockRemote[sockIdx];
    if (INVALID_SOCKET == sock || !sendBuf || totalSendSize <= 0){
        cout << "SendMsg sock invalid param" << endl;
        ret = -2;
    }
    else{
        ::EnterCriticalSection(&m_csSend);
        EnableBlocking(sock, true);
        int sendSize = ::send(sock, sendBuf, totalSendSize, 0);
        if (SOCKET_ERROR == sendSize){
            ret = ::WSAGetLastError();
            cout << "SendMsg error:" << ret << endl;
        }
        EnableBlocking(sock, false);
        ::LeaveCriticalSection(&m_csSend);
    }

	return ret;
}

// TCP send msg
int CSocketAPI::SendMsg(char* sendBuf, int totalSendSize)
{
	int ret = 0;
	if (INVALID_SOCKET == m_sockLocal || !sendBuf || totalSendSize <= 0){
        cout << "SendMsg invalid param" << endl;
		ret = -2;
	}

	::EnterCriticalSection(&m_csSend);
	EnableBlocking(m_sockLocal, true);
	int sendSize = ::send(m_sockLocal, sendBuf, totalSendSize, 0);
	if (SOCKET_ERROR == sendSize){
		ret = ::WSAGetLastError();
        cout << "SendMsg error:%d" << ret << endl;
	}
	EnableBlocking(m_sockLocal, false);
	::LeaveCriticalSection(&m_csSend);

	return ret;
}

// UDP send
int CSocketAPI::SendMsg(sockaddr_in* remoteSockAddr, char* sendBuf, int sendSize)
{
    if (INVALID_SOCKET == m_sockLocal || !sendBuf || sendSize <= 0){
        cout << "SendMsg sock invalid param" << endl;
        return -2;
    }
    int ret = 0;

    ::EnterCriticalSection(&m_csSend);
    //EnableBlocking(m_sockLocal, true);
    int realSendSize = ::sendto(m_sockLocal, sendBuf, sendSize, 0, (SOCKADDR*)remoteSockAddr, sizeof(sockaddr_in));
    if (SOCKET_ERROR == realSendSize){
        ret = ::WSAGetLastError();
        cout << "SendMsg error:" << ret << endl;
    }
    cout << "Fin sendto:" << ntohs(remoteSockAddr->sin_port) << " " << (UINT)remoteSockAddr->sin_addr.S_un.S_un_b.s_b1 << "."
        << (UINT)remoteSockAddr->sin_addr.S_un.S_un_b.s_b2 << "." << (UINT)remoteSockAddr->sin_addr.S_un.S_un_b.s_b3 << "."
        << (UINT)remoteSockAddr->sin_addr.S_un.S_un_b.s_b4 << endl;
    //EnableBlocking(m_sockListen, false);
    ::LeaveCriticalSection(&m_csSend);

    return ret;
}

// server recv msg
int CSocketAPI::RecvMsg(UINT sockIdx, char* recvBuf, int& recvSize, int maxRetryCnt)
{
	int ret = 0;
	if (sockIdx >= m_vecpSockRemote.size() || sockIdx >= m_vecpSockRemote.size()){
		char buf[32] = { 0 };
		sprintf_s(buf, "RecvMsg err:sockIdx[%d][%d]", sockIdx, m_vecpSockRemote.size());
        cout << "RecvMsg error:" << sockIdx << " " << m_vecpSockRemote.size() << endl;
		return -2;
	}
	SOCKET sock = *m_vecpSockRemote[sockIdx];
	if (INVALID_SOCKET == sock || !recvBuf){
        cout << "RecvMsg invalid param" << endl;
        return -2;
	}
	// tcp or bluetooth
	char* pRecv = recvBuf;
	int totalRecvCnt = 0;
	int retryCnt = 0;

	// recv loop
	while (retryCnt < maxRetryCnt && totalRecvCnt < recvSize){
		int recvCnt = ::recv(sock, pRecv, recvSize - totalRecvCnt, 0);
		if (0 == recvCnt){
			ret = -1;
			break;
		}
		else if (SOCKET_ERROR == recvCnt){
			int errCode = ::WSAGetLastError();
			if (WSAEWOULDBLOCK == errCode){
				retryCnt++;
				Sleep(SOCKET_RECV_RETRY_SLEEP);
				continue;
			}
			else{
                if (WSAEINVAL == errCode){
                    break;
                }
				// error has occured
				ret = errCode;
                cout << "RecvMsg error:" << errCode << endl;
				break;
			}
		}
		else{
			// ok
			totalRecvCnt += recvCnt;
			pRecv += recvCnt;
		}
	}

	return ret;
}

// client recv msg
int CSocketAPI::RecvMsg(char* recvBuf, int& recvSize, int maxRetryCnt)
{
	int ret = 0;
	if (INVALID_SOCKET == m_sockLocal || !recvBuf){
        cout << "RecvMsg invalid param" << endl;
		ret = -2;
	}
	// tcp or bluetooth
	char* pRecv = recvBuf;
	int totalRecvCnt = 0;
	int retryCnt = 0;

	// recv loop
	while (retryCnt < maxRetryCnt && totalRecvCnt < recvSize){
		int recvCnt = ::recv(m_sockLocal, pRecv, recvSize - totalRecvCnt, 0);
		if (0 == recvCnt){
			ret = -1;
			break;
		}
		else if (SOCKET_ERROR == recvCnt){
			int errCode = ::WSAGetLastError();
			if (WSAEWOULDBLOCK == errCode){
				retryCnt++;
				Sleep(SOCKET_RECV_RETRY_SLEEP);
				continue;
			}
			else{
				// error has occured
				ret = errCode;
                cout << "RecvMsg error:" << errCode << endl;
				break;
			}
		}
		else{
			// ok
			totalRecvCnt += recvCnt;
			pRecv += recvCnt;
		}
	}
	recvSize = totalRecvCnt;

	return ret;
}

// UDP recv
int CSocketAPI::RecvMsg(sockaddr_in* remoteSockAddr, char* recvBuf, int& recvSize, int timeOut)
{
    if (INVALID_SOCKET == m_sockLocal || !recvBuf){
        cout << "RecvMsg invalid param" << endl;
        return -2;
    }

    int ret = 0;
    int totalRecvCnt = 0;
    int sockLen = sizeof(SOCKADDR_IN);
    ULONGLONG startTick = ::GetTickCount64();
    while (::GetTickCount64() - startTick < timeOut){
        totalRecvCnt = ::recvfrom(m_sockLocal, recvBuf, recvSize, 0, (sockaddr*)remoteSockAddr, &sockLen);
        if (0 == totalRecvCnt){
            // socket closed
            cout << "recvfrom socket closed" << endl;
            ret = -1;
            break;
        }
        else if (SOCKET_ERROR == totalRecvCnt){
            int errCode = ::WSAGetLastError();
            if (WSAEWOULDBLOCK == errCode){
                Sleep(SOCKET_RECV_RETRY_SLEEP);
                continue;
            }
            else{
                // error has occured
                if (WSAEINVAL == errCode){
                    break;  // socket may not be initialized fully. skip this one 
                }
                ret = errCode;
                cout << "RecvMsg error:" << errCode << endl;
                break;
            }
        }
        else{
            // ok
            break;
        }
    }
    recvSize = totalRecvCnt;

    return ret;
}

// closesocket
int CSocketAPI::CloseSocket()
{
    int ret = 0;
    if (SOCKET_UDP == m_nSocketType){
        ret = ::closesocket(m_sockLocal);
    }
    else{
        // TCP or BTH
        ret = ::closesocket(m_sockListen);
        ret = ::closesocket(m_sockLocal);
        if (!m_vecpSockRemote.empty()){
            for (auto pSock : m_vecpSockRemote){
                ret = ::closesocket(*pSock);
            }
        }
    }
    return ret;
}