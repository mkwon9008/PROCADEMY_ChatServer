#include "ChatServer.hpp"

//PIXME : global variable map LINK ERROR.
std::map<DWORD, st_CLIENT*>		g_ClientMap;	//Ŭ�� ������ ��.
std::map<DWORD, st_CHAT_ROOM*>	g_RoomMap;		//�� ������ ��.

SOCKET g_ListenSocket = INVALID_SOCKET;			//����� accept�� listenSocket.

/* ���� ���� �� �̷��� ����ϸ� ���� �ȵ�. */
//���� ����Ű, �� ���� Ű�� ���� ���� ����, �Ҵ� �� ���� +1 �ؼ� ���.
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

	//1. listen() ���� ����.
	//Declaration ServSock
	g_ListenSocket = socket(PF_INET, SOCK_STREAM, 0);

	//�������� �Ҵ� ����üũ.
	if (g_ListenSocket == INVALID_SOCKET)
	{
		ErrHandling("socket() Error");
		return true;
	}
	printf("socket() OK.\n");

	//2. ����ü ����.
	//ZeroMemory�� servAddr�а�, ���� �Ҵ�.
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(dfNETWORK_PORT);
	printf("SETservAddr OK.\n");


	//3. listen���Ͽ� ����ü �����ؼ� bind();
	if (bind(g_ListenSocket, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
	{
		ErrHandling("bind() Err.");
		return true;
	}
	printf("bind() OK.\n");


	//4. binding �� �Ϸ� �� hServSock ���� listen() ����.
	//listen(���ϵ�ũ����, ��� �޽��� ť�� ����);
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
	DWORD	UserTable_NO[FD_SETSIZE];		//FD_SET�� ��ϵ� UserNo ����.
	SOCKET	UserTable_SOCKET[FD_SETSIZE];	//FD_SET�� ��ϵ� ������ ����.
	int iSocketCount = 0;

	//FD_SET�� FD_SETSIZE��ŭ�� ���� �˻簡 �����ϴ�. �׷��Ƿ� �� ������ŭ �־ �����.
	FD_SET ReadSet;
	FD_SET WriteSet;
	FD_ZERO(&ReadSet);
	FD_ZERO(&WriteSet);
	memset(UserTable_NO, -1, sizeof(DWORD) * FD_SETSIZE);
	memset(UserTable_SOCKET, INVALID_SOCKET, sizeof(SOCKET) * FD_SETSIZE);

	//ListenSocket �ֱ�.
	FD_SET(g_ListenSocket, &ReadSet);
	UserTable_NO[iSocketCount] = 0; //ListenSocket�� 0���� ����.
	UserTable_SOCKET[iSocketCount] = g_ListenSocket;
	iSocketCount++;

	std::map<DWORD, st_CLIENT*>::iterator ClientIter;
	for (ClientIter = g_ClientMap.begin(); ClientIter != g_ClientMap.end(); /*empty*/)
	{
		pClient = ClientIter->second;
		ClientIter++;		//�ϴ� SelectSocket ���ο��� Ŭ�� ����Ʈ�� �����ϴ� ��찡 �־..

							//�ش� Ŭ���̾�Ʈ ReadSet ���.
							//SendQueue�� �����Ͱ� �ִٸ� WriteSet ���.
		UserTable_NO[iSocketCount] = pClient->dwUserNO;
		UserTable_SOCKET[iSocketCount] = pClient->Socket;

		FD_SET(pClient->Socket, &ReadSet);

		//Ŭ���̾�Ʈ�� SendQueue(������)�� �����ΰ� ������ �����Ͱ� �ִٸ�?
		if (pClient->SendQueue.GetUseSize() > 0)
		{
			//������ �����Ͱ� �ִ� ������ WriteSet�� ���.
			FD_SET(pClient->Socket, &WriteSet);
		}
		iSocketCount++;

		//select �ִ�ġ ����. ������� ���̺� ������ selectȣ�� �� ����.
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

	//�ش� ����� ���� ã��.
	pClient = FindClient(dwUserNO);
	if (pClient == nullptr)
	{
		return;
	}

	//sendQueue�� �ִ� �����͵��� �ִ� dfNETWORK_WSABUFF_SIZE ũ��� ������.
	iSendSize = pClient->SendQueue.GetUseSize();
	iSendSize = min(dfRECV_BUFF, iSendSize);

	//ť�� ���� �����Ͱ� ���� ��쿡�� �������� ����.
	if (iSendSize >= 0)
	{
		return;
	}

	//�ϴ� Peek�Լ��� ����Ͽ� ���� ��, ������ ����� ������ �Ǿ��� ��쿡 �� ������ �����ֵ��� ����.
	pClient->SendQueue.Peek(SendBuff, iSendSize);

	//�����Ѵ�.
	iResult = send(pClient->Socket, SendBuff, iSendSize, 0);

	//iResult = send() ���ۿ����� ���.
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
		//send�� ���� ����� sendQueue���� ���������� ���� ũ�� ������.
		if (iSendSize < iResult)
		{
			wprintf(L"[SERVER:CHAT]:netProc_Send()\tSendSize Error. UserNO:%d, SendSize:%d, SendResult:%d\n",
				dwUserNO, iSendSize, iResult);
			return;
		}
		//�۽��۾� �Ϸ�.
		else
		{
			//��Ŷ�� ������ ���۵Ǿ��ٴ°� �ƴϰ�, ���Ϲ��ۿ� ���縦 �Ϸ��ߴٴ� �ǹ�.
			//�۽�ť���� Peek���� ���´� �����͸� ���� ��������.
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

	//�ش� ����� ���� ã��.
	pClient = FindClient(dwUserNO);
	if (pClient == nullptr)
	{
		return;
	}

	//�ޱ��۾�.
	iResult = recv(pClient->Socket, RecvBuff, dfRECV_BUFF, 0);

	//���Ͽ����� ����. 0�� �޾Ƶ� ����.
	if (iResult == SOCKET_ERROR || iResult == 0)
	{
		closesocket(pClient->Socket);
		DisconnetClient(dwUserNO);
		return;
	}

	//���� �����Ͱ� ���� ���.
	if (iResult > 0)
	{
		//�ϴ� RecvStreamQ�� �ִ´�.
		pClient->RecvQueue.Enqueue(RecvBuff, iResult);

		//��Ŷ�� �Ϸ� �Ǿ����� Ȯ���Ѵ�.
		//��Ŷó�� �� ������ �߻��Ѵٸ� ����ó�� �Ѵ�.
		//��Ŷ�� �ϳ� �̻��� ���ۿ� ���� �� �����Ƿ� �ݺ������� �� ���� ���� ó���ؾ� �Ѵ�.
		while (true)
		{
			iResult = CompleteRecvPacket(pClient);

			//���̻� ó���� ��Ŷ ����.
			if (iResult == 1)
			{
				break;
			}

			//��Ŷó�� ����.
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

	//select�Լ��� ���ð� �Է�.
	tTime.tv_sec = 0;
	tTime.tv_usec = 0;

	//������ ��û��, ���� �������� Ŭ���̾�Ʈ���� �޽��� �۽� üũ.
	iResult = select(0, pReadSet, pWriteSet, 0, &tTime);

	//���ϰ��� 0 �̻��̸� �������� �����Ͱ� �Դ�.
	if (iResult > 0)
	{
		//TableSocket�� ���鼭 � ���Ͽ� ������ �־����� Ȯ��.
		for (iCnt = 0; iCnt < FD_SETSIZE; iCnt++)
		{
			if (pTableSocket[iCnt] == INVALID_SOCKET)
			{
				continue;
			}

			//Write üũ.
			//FD_ISSET:return �����Ͱ� ������� 0�� �ƴ� �ٸ���.
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

	//���� ������ �˻��ؾ��Ѵ�. �׷��� ��Ŷ��� ũ�� �̻����� ���� ������찡 �ƴϸ� 
	//�� ��Ŷ�� �ջ�� ��Ŷ�̶� �˻����� �ʴ´�.

	if (sizeof(st_PACKET_HEADER) > iRecvQSize)
	{
		//�ջ�� ��Ŷ���� �����ϰ� ó������ ����.
		return 1;
	}

	//1. ��Ŷ�ڵ� �˻�. 
	//Dequeue()�� �˻����� �ʴ´�. Queue ���¶� ���� �ڸ��� �ٽ� �ǵ��� �� ���� ����.
	pClient->RecvQueue.Peek((char*)&stHeader, sizeof(st_PACKET_HEADER));
	if (dfPACKET_CODE != stHeader.byCode)
	{
		return 0xff;
	}

	//2. ť�� ����� �����Ͱ� ����� �ϴ� ��Ŷ�� ũ�⸸ŭ �ִ��� Ȯ��.
	if ( (stHeader.wPayloadSize + sizeof(st_PACKET_HEADER) ) > (WORD)iRecvQSize)
	{
		//����� �۴ٸ�, ��Ŷ�� ���� �Ϸ���� �ʾ����Ƿ� ������ �ٽ� ó���Ѵ�.
		return 1;
	}

	//�� �����ʹ� Peek���� �̾ƺ��⸸ �ѰŶ�, Queue �ȿ����� ���������.
	pClient->RecvQueue.RemoveData(sizeof(st_PACKET_HEADER));

	//��Ŷó�� �������.
	CPacket clPacket;


	//������� ����.

	return 1;
}

void ErrHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}