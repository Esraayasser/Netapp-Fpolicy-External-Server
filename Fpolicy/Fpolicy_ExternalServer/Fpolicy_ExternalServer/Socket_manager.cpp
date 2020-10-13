#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Socket_manager.h"

Socket_manager::Socket_manager()
{
	if (Manage_connection())
		cout << "Connection to the ONTAP fpolicy engine is established successfully.\n";
	else
		cout << "Failed to connect to the ONTAP engine.\n";
}

Socket_manager::~Socket_manager()
{
	closesocket(listening_sock);
	closesocket(sending_sock);
	WSACleanup();
}

bool Socket_manager::Manage_connection()
{
	// Intializing Winsock

	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);
	int wsOK = WSAStartup(ver, &wsData);

	if (wsOK){
		cout << "Intializing Winsock, failed!\n";
		return false;
	}

	// Intializing the listening socket.
	listening_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listening_sock == INVALID_SOCKET) {
		cout << "Intializing listening socket, failed!\n";
		return false;
	}

	sockaddr_in server_sockaddr;
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(PORT);
	server_sockaddr.sin_addr.S_un.S_addr = INADDR_ANY;

	if (bind(listening_sock, (sockaddr*)&server_sockaddr, sizeof server_sockaddr)) {
		cout << "Binding with the destination engine, failed with error: " << WSAGetLastError() << endl;
		return false;
	}
	
	cout << "Listening for connection...\n";
	if (listen(listening_sock, SOMAXCONN) == SOCKET_ERROR) {
		cout << "Listening for connection, Failed with error: " << WSAGetLastError() << endl;
		return false;
	}

	// Accepting incoming connection requests.
	sockaddr_in client_sockaddr;
	int clientaddr_size = sizeof client_sockaddr;
	sending_sock = accept(listening_sock, (sockaddr*)&client_sockaddr, &clientaddr_size);
	cout << "Connection request from the fpolicy engine is accepted!\n";
	struct hostent* cp;
	cp = gethostbyaddr((char*)&client_sockaddr.sin_addr, sizeof(struct in_addr), AF_INET);
	printf("Connection is from %s\n", ((cp && cp->h_name) ? cp->h_name : inet_ntoa(client_sockaddr.sin_addr)));

	if (sending_sock == INVALID_SOCKET) {
		cout << "Accepting connection, Failed with error: " << WSAGetLastError() << endl;
	}

	return true;
}

void Socket_manager::Read_msg(char* msgBuffer, int msg_len)
{
	int msgLen;
	string msg;
	int32_t  numBytesRead = 0;
	int32_t  retval = 0;
	memset(msgBuffer, '\0', sizeof msgBuffer);

	struct timeval   endTime = {};
	struct timeval   currentTime = {};

	timer.gettimeofday(&endTime, NULL);
	endTime.tv_sec += READ_TIMEOUT;
	int err = 0;
	do {
		retval = recv(sending_sock, msgBuffer + numBytesRead, msg_len - numBytesRead, 0);
		if (retval < 0)
			cout << WSAGetLastError() << endl;

		numBytesRead += retval;

		if (numBytesRead >= msg_len || retval == 0)
			break;

		timer.gettimeofday(&currentTime, NULL);
	} while (timercmp(&endTime, &currentTime, > ));

	return;
}

void Socket_manager::Send_msg(char* responseBuffer, int response_len)
{
	int32_t  numBytesSent = 0;
	int32_t  retval = 0;

	struct timeval   endTime = {};
	struct timeval   currentTime = {};

	timer.gettimeofday(&endTime, NULL);
	endTime.tv_sec += WRITE_TIMEOUT;

	do {
		retval = send(sending_sock, responseBuffer + numBytesSent, response_len - numBytesSent, 0);

		if (retval < 0)
			cout << WSAGetLastError() << endl;

		numBytesSent += retval;

		if (numBytesSent >= response_len || retval == 0)
			break;

		timer.gettimeofday(&currentTime, NULL);
	} while (timercmp(&endTime, &currentTime, > ));

	cout << "The response was sent successfully!\n\n" << endl;

	return;
}

