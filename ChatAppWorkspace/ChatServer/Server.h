#pragma once

#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

// Need to link Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

struct ClientInfo {
	SOCKET socket;
	bool connected;
};

struct ServerInfo
{
	struct addrinfo* info = nullptr;
	struct addrinfo hints;
	SOCKET listenSocket = INVALID_SOCKET;
	fd_set activeSockets;
	fd_set socketsReadyForReading;
	std::vector<ClientInfo> clients;
};

class Server
{
public:
	Server();
	~Server();
	int Bind();
	void Initialize();
	int Listen();
	void Process();
	int Receive(ClientInfo& client, const int bufLen, char buf[]);
	int Send(ClientInfo& client, char buf[], int receiveResult);
	void ShutDown();

private:
	ServerInfo m_serverInfo;
	WSADATA m_wsaData;
	void AddConnectedSockets();
	void Accept();
	void GetServerAddrInfo();
	void PrintStatusInfo();
	void CreateSocket();
	void Startup();
};
