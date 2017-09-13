#include "ChatServer.hpp"

//PIXME : global variable map LINK ERROR.
std::map<DWORD, st_CLIENT*>		g_ClientMap;	//클라 관리용 맵.
std::map<DWORD, st_CHAT_ROOM*>	g_RoomMap;		//방 관리용 맵.

SOCKET g_ListenSocket = INVALID_SOCKET;			//사용자 accept용 listenSocket.

/* 실제 서비스 시 이렇게 사용하면 절대 안됨. */
//유저 고유키, 방 고유 키를 만들 전역 변수, 할당 시 마다 +1 해서 사용.
DWORD g_dwKey_UserNO = 1;
DWORD g_dwKey_RoomNO = 1;
/*********************************************/


bool NetworkInit(void)
{
	WSADATA wsaData;
	SOCKADDR_IN servAdr;

	//WSAStartup()
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrHandling("WSAStartup() Err.");
		return true;
	}
	printf("WSAStartup() OK.\n");

	//1. listen() 소켓 생성.
	//Declaration ServSock
	g_ListenSocket = socket(PF_INET, SOCK_STREAM, 0);

	//서버소켓 할당 에러체크.
	if (g_ListenSocket == INVALID_SOCKET)
	{
		ErrHandling("socket() Error");
		return true;
	}
	printf("socket() OK.\n");

	//2. 구조체 생성.
	//ZeroMemory로 servAddr밀고, 인자 할당.
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(dfNETWORK_PORT);
	printf("SETservAddr OK.\n");


	//3. listen소켓에 구조체 적용해서 bind();
	if (bind(g_ListenSocket, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
	{
		ErrHandling("bind() Err.");
		return true;
	}
	printf("bind() OK.\n");


	//4. binding 이 완료 된 hServSock 으로 listen() 시작.
	//listen(소켓디스크립터, 대기 메시지 큐의 갯수);
	if (listen(g_ListenSocket, 10) == SOCKET_ERROR)
	{
		ErrHandling("listen() Err.");
		return true;
	}
	printf("listen() OK\n");

	return false;
}

void NetworkProcess(void)
{
	st_CLIENT* pClient;
	DWORD	UserTable_NO[FD_SETSIZE];		//FD_SET에 등록된 UserNo 저장.
	SOCKET	UserTable_SOCKET[FD_SETSIZE];	//FD_SET에 등록된 소켓을 저장.
	int iSocketCount = 0;

	//FD_SET은 FD_SETSIZE만큼만 소켓 검사가 가능하다. 그러므로 그 개수만큼 넣어서 사용함.
	FD_SET ReadSet;
	FD_SET WriteSet;
	FD_ZERO(&ReadSet);
	FD_ZERO(&WriteSet);
	memset(UserTable_NO, -1, sizeof(DWORD) * FD_SETSIZE);
	memset(UserTable_SOCKET, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);

	//ListenSocket 넣기.
	FD_SET(g_ListenSocket, &ReadSet);
	UserTable_NO[iSocketCount] = 0; //ListenSocket을 0으로 셋팅.
	UserTable_SOCKET[iSocketCount] = g_ListenSocket;
	iSocketCount++;

	std::map<DWORD, st_CLIENT*>::iterator ClientIter;
	for (ClientIter = g_ClientMap.begin(); ClientIter != g_ClientMap.end(); /*empty*/)
	{
		pClient = ClientIter->second;
		ClientIter++;		//하단 SelectSocket 내부에서 클라 리스트를 삭제하는 경우가 있어서..

							//해당 클라이언트 ReadSet 등록.
							//SendQueue에 데이터가 있다면 WriteSet 등록.
		UserTable_NO[iSocketCount] = pClient->dwUserNO;
		UserTable_SOCKET[iSocketCount] = pClient->Socket;

		FD_SET(pClient->Socket, &ReadSet);

		//클라이언트의 SendQueue(링버퍼)에 무엇인가 전송할 데이터가 있다면?
		if (pClient->SendQueue.GetUseSize() > 0)
		{
			//전송할 데이터가 있는 소켓을 WriteSet에 등록.
			FD_SET(pClient->Socket, &WriteSet);
		}
		iSocketCount++;

		//select 최대치 도달. 만들어진 테이블 정보로 select호출 후 정리.
		if (FD_SETSIZE <= iSocketCount)
		{
			SelectSocket(UserTable_NO, UserTable_SOCKET, &ReadSet, &WriteSet);

			FD_ZERO(&ReadSet);
			FD_ZERO(&WriteSet);
			memset(UserTable_NO, -1, sizeof(DWORD) * FD_SETSIZE);
			memset(UserTable_SOCKET, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);
			iSocketCount = 0;
		}
	}//endfor.

	if (iSocketCount > 0)
	{
		SelectSocket(UserTable_NO, UserTable_SOCKET, &ReadSet, &WriteSet);
	}
}

//TODO : networkProcess_Accept. 
void netProc_Accept(void)
{

}

void netProc_Send(DWORD dwUserNO)
{
	st_CLIENT* pClient;
	char SendBuff[dfRECV_BUFF];
	int iResult;
	int iSendSize;

	//해당 사용자 세션 찾기.
	pClient = FindClient(dwUserNO);
	if (pClient == nullptr)
	{
		return;
	}

	//sendQueue에 있는 데이터들을 최대 dfNETWORK_WSABUFF_SIZE 크기로 보낸다.
	iSendSize = pClient->SendQueue.GetUseSize();
	iSendSize = min(dfRECV_BUFF, iSendSize);

	//큐에 보낼 데이터가 있을 경우에만 보내도록 하자.
	if (iSendSize >= 0)
	{
		return;
	}

	//일단 Peek함수를 사용하여 빼낸 후, 전송이 제대로 마무리 되었을 경우에 이 내용을 지워주도록 하자.
	pClient->SendQueue.Peek(SendBuff, iSendSize);

	//전송한다.
	iResult = send(pClient->Socket, SendBuff, iSendSize, 0);

	//iResult = send() 전송에러의 경우.
	if (iResult == SOCKET_ERROR)
	{
		DWORD dwError = WSAGetLastError();
		if (dwError == WSAEWOULDBLOCK)
		{
			wprintf(L"[SERVER:CHAT]:netProc_Send()\tsocket is WOULDBLOCK.\n");
			return;
		}

		wprintf(L"[SERVER:CHAT]:netProc_Send()\tsocket is unknown Error.\n");

		closesocket(pClient->Socket);
		DisconnetClient(dwUserNO);
		return;
	}
	else //iResult != SOCKET_ERROR
	{
		//send로 보낸 사이즈가 sendQueue에서 보낼사이즈 보다 크면 오류다.
		if (iSendSize < iResult)
		{
			wprintf(L"[SERVER:CHAT]:netProc_Send()\tSendSize Error. UserNO:%d, SendSize:%d, SendResult:%d\n",
				dwUserNO, iSendSize, iResult);
			return;
		}
		//송신작업 완료.
		else
		{
			//패킷이 와전히 전송되었다는건 아니고, 소켓버퍼에 복사를 완료했다는 의미.
			//송신큐에서 Peek으로 빼냈던 데이터를 이제 지워주자.
			pClient->SendQueue.RemoveData(iResult);
		}
	}

	return;
}

void netProc_Recv(DWORD dwUserNO)
{
	st_CLIENT *pClient;
	char RecvBuff[dfRECV_BUFF];
	int iResult;

	//해당 사용자 세선 찾기.
	pClient = FindClient(dwUserNO);
	if (pClient == nullptr)
	{
		return;
	}

	//받기작업.
	iResult = recv(pClient->Socket, RecvBuff, dfRECV_BUFF, 0);

	//소켓에러시 종료. 0을 받아도 종료.
	if (iResult == SOCKET_ERROR || iResult == 0)
	{
		closesocket(pClient->Socket);
		DisconnetClient(dwUserNO);
		return;
	}

	//받은 데이터가 있을 경우.
	if (iResult > 0)
	{
		//일단 RecvStreamQ에 넣는다.
		pClient->RecvQueue.Enqueue(RecvBuff, iResult);

		//패킷이 완료 되었는지 확인한다.
		//패킷처리 중 문제가 발생한다면 종료처리 한다.
		//패킷은 하나 이상이 버퍼에 있을 수 있으므로 반복문으로 한 번에 전부 처리해야 한다.
		while (true)
		{
			iResult = CompleteRecvPacket(pClient);

			//더이상 처리할 패킷 없음.
			if (iResult == 1)
			{
				break;
			}

			//패킷처리 오류.
			if (iResult == -1)
			{
				wprintf(L"[SERVER:CHAT]:netProc_Recv()\tPacket Error. UserNO:%d\n", dwUserNO);
				return;
			}
		}
	}
}

void SelectSocket(DWORD* dwpTableNO, SOCKET* pTableSocket, FD_SET* pReadSet, FD_SET* pWriteSet)
{
	timeval tTime;
	int iResult, iCnt;

	//select함수의 대기시간 입력.
	tTime.tv_sec = 0;
	tTime.tv_usec = 0;

	//접속자 요청과, 현재 접속중인 클라이언트들의 메시지 송신 체크.
	iResult = select(0, pReadSet, pWriteSet, 0, &tTime);

	//리턴값이 0 이상이면 누군가의 데이터가 왔다.
	if (iResult > 0)
	{
		//TableSocket을 돌면서 어떤 소켓에 반응이 있었는지 확인.
		for (iCnt = 0; iCnt < FD_SETSIZE; iCnt++)
		{
			if (pTableSocket[iCnt] == INVALID_SOCKET)
			{
				continue;
			}

			//Write 체크.
			//FD_ISSET:return 데이터가 있을경우 0이 아닌 다른값.
			if (FD_ISSET(pTableSocket[iCnt], pWriteSet) != false)
			{
				netProc_Send(dwpTableNO[iCnt]);
			}

			if (FD_ISSET(pTableSocket[iCnt], pReadSet) != false)
			{
				if (dwpTableNO[iCnt] == 0)
				{
					netProc_Accept(); //PIXME
				}
				else
				{
					netProc_Recv(dwpTableNO[iCnt]);
				}
			}

		}//endfor

	}
	else if (iResult == SOCKET_ERROR)
	{
		wprintf(L"select socket error.\n");
	}
}

//TODO : findClientPtr by dwUserNO.
//return(st_CLIENT*) : return st_CLIENT ptr has parameter dwUserNO.
st_CLIENT* FindClient(DWORD dwUserNO)
{
	st_CLIENT* pClient = (st_CLIENT*)1;



	return pClient;
}

//TODO: Disconnet socket find by dwUserNO.
void DisconnetClient(DWORD dwUserNO)
{

}


int CompleteRecvPacket(st_CLIENT* pClient)
{
	st_PACKET_HEADER stHeader;
	int iRecvQSize = pClient->RecvQueue.GetUseSize();

	//받은 내용을 검사해야한다. 그런데 패킷헤더 크기 이상으로 뭔가 받은경우가 아니면 
	//그 패킷은 손상된 패킷이라 검사하지 않는다.

	if (sizeof(st_PACKET_HEADER) > iRecvQSize)
	{
		//손상된 패킷으로 생각하고 처리하지 않음.
		return 1;
	}

	//1. 패킷코드 검사. 
	//Dequeue()로 검사하지 않는다. Queue 형태라서 원래 자리에 다시 되돌릴 수 없기 때문.
	pClient->RecvQueue.Peek((char*)&stHeader, sizeof(st_PACKET_HEADER));
	if (dfPACKET_CODE != stHeader.byCode)
	{
		return 0xff;
	}

	//2. 큐에 저장된 데이터가 얻고자 하는 패킷의 크기만큼 있는지 확인.
	if ( (stHeader.wPayloadSize + sizeof(st_PACKET_HEADER) ) > (WORD)iRecvQSize)
	{
		//사이즈가 작다면, 패킷이 아직 완료되지 않았으므로 다음에 다시 처리한다.
		return 1;
	}

	//위 데이터는 Peek으로 뽑아보기만 한거라서, Queue 안에서는 지워줘야함.
	pClient->RecvQueue.RemoveData(sizeof(st_PACKET_HEADER));

	//패킷처리 해줘야함.
	CPacket clPacket;


	//여기까지 했음.

	return 1;
}

void ErrHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}