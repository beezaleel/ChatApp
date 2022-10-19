#include "Server.h"
#include <algorithm>

#define DEFAULT_PORT "8888"

Server::Server()
{
}

Server::~Server()
{
}

void Server::Accept() {
	SOCKET clientSocket = accept(m_serverInfo.listenSocket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET) {
		printf("Accept failed with error: %d\n", WSAGetLastError());
	}
	else {
		printf("Accepted connection from client!\n");
		ClientInfo newClient;
		newClient.socket = clientSocket;
		newClient.buffer = Buffer(128);
		newClient.connected = true;
		m_serverInfo.clients.push_back(newClient);
	}
}

void Server::AddConnectedSockets() {
	// 2. Add all the connected sockets, to see if the is any information
		//    to be recieved from the connected clients.
	for (int i = 0; i < m_serverInfo.clients.size(); i++) {
		ClientInfo& client = m_serverInfo.clients[i];
		if (client.connected) {
			FD_SET(client.socket, &m_serverInfo.socketsReadyForReading);
		}
	}
}

int Server::Bind() {
	int state = -1;
	state = 
		bind(
			m_serverInfo.listenSocket, 
			m_serverInfo.info->ai_addr, 
			(int)m_serverInfo.info->ai_addrlen
		);
	if (state == SOCKET_ERROR) {
		printf("Binding failed due to Error - %d\n", WSAGetLastError());
		freeaddrinfo(m_serverInfo.info);
		closesocket(m_serverInfo.listenSocket);
		WSACleanup();
	}
	else {
		printf("Bind was successful! \n");
	}
	return state;
}

void Server::Initialize() {
	// Create and startup WSADATA
	printf("Initializing server components! \n");

	// Create WSAStartup
	Startup();

	// Create getaddrinfo
	GetServerAddrInfo();

	// Create Socket
	CreateSocket();

	// Create 3 Rooms
	Room general;
	general.roomName = "General";

	Room staff;
	staff.roomName = "Staff";

	Room student;
	student.roomName = "Student";
	
	m_serverInfo.rooms.push_back(general);
	m_serverInfo.rooms.push_back(staff);
	m_serverInfo.rooms.push_back(student);

}

int Server::Listen() {
	int state = -1;
	state = listen(m_serverInfo.listenSocket, SOMAXCONN);
	if (state == SOCKET_ERROR) {
		printf("Failed to listen. Error - %d\n", WSAGetLastError());
		freeaddrinfo(m_serverInfo.info);
		closesocket(m_serverInfo.listenSocket);
		WSACleanup();
	}
	else {
		printf("Listening on port %s \n", DEFAULT_PORT);
	}
	return state;
}

void Server::Process() {
	struct timeval tVal;
	tVal.tv_sec = 0;
	tVal.tv_usec = 200 * 1000;

	int selectResult;

	while (true) {
		FD_ZERO(&m_serverInfo.socketsReadyForReading);
		FD_SET(m_serverInfo.listenSocket, &m_serverInfo.socketsReadyForReading);

		AddConnectedSockets();

		selectResult = select(0, &m_serverInfo.socketsReadyForReading, NULL, NULL, &tVal);
		if (selectResult == SOCKET_ERROR) {
			printf("select() failed with error: %d\n", WSAGetLastError());
			return;
		}
		printf(".");

		if (FD_ISSET(m_serverInfo.listenSocket, &m_serverInfo.socketsReadyForReading)) {
			printf("\n");
			Accept();
		}

		for (int i = m_serverInfo.clients.size() - 1; i >= 0; i--) {
			ClientInfo& client = m_serverInfo.clients[i];
			if (client.connected == false)
				continue;

			if (FD_ISSET(client.socket, &m_serverInfo.socketsReadyForReading)) {

				const int buflen = 128;
				char buf[buflen];

				int dataStartIndex = client.buffer.Data.size();
				int recvResult = Receive(client, buflen, buf);
			}
		}
	}

}

int Server::Receive(ClientInfo& client, const int bufLen, char* buf) {
	int recvResult = recv(client.socket, (char*)&(client.buffer.Data[0]), bufLen, 0);
	if (recvResult == SOCKET_ERROR) {
		printf("Receive failed. Error - %d\n", WSAGetLastError());
	}

	if (recvResult == 0) {
		printf("Client disconnected!\n");
		client.connected = false;
	}
	else if (recvResult > 0) {
		if (client.buffer.Data.size() >= 4) {
			client.packetLength = client.buffer.ReadUInt32(0);
			unsigned int roomNameSize = client.buffer.ReadUInt32(6);
			char* roomname = client.buffer.ReadString(10);

			short messageId = client.buffer.ReadShort(4);
			Room room;
			room.roomName = roomname;
			switch (messageId)
			{
			case MessageType::Join:
				for (int i = 0; i < m_serverInfo.rooms.size(); i++) {
					if (m_serverInfo.rooms[i].roomName == "General") {
						User user;
						user.name = "Ademola";
						user.socket = client.socket;
						m_serverInfo.rooms[i].members.push_back(user);
						printf("Yes i can join a room\n");
						// Send message to users in the group
						std::string broadcast = user.name + ": joined group.";
						Buffer broadcastBuffer = Buffer();
						broadcastBuffer.WriteString((char*)broadcast.c_str());
						for (int j = 0; j < m_serverInfo.rooms[i].members.size(); j++) {
							ClientInfo roomMember;
							roomMember.socket = m_serverInfo.rooms[i].members[j].socket;
							Send(roomMember, (char*)broadcastBuffer.Data.data(), broadcast.size());
						}
					}
				}
				break;
			case MessageType::Leave:
				for (int i = 0; i < m_serverInfo.rooms.size(); i++) {
					if (m_serverInfo.rooms[i].roomName == "General") {
						for (int j = 0; j < m_serverInfo.rooms[i].members.size(); j++) {
							if (m_serverInfo.rooms[i].members[j].name == "Ademola") {
								m_serverInfo.rooms[i].members.erase(m_serverInfo.rooms[i].members.begin() + i);
							}
						}
						for (int j = 0; j < m_serverInfo.rooms[i].members.size(); j++) {
							// Send message to users in the group
							std::string broadcast = "Ademola: left group.";
							Buffer broadcastBuffer = Buffer();
							broadcastBuffer.WriteString((char*)broadcast.c_str());
							ClientInfo roomMember;
							roomMember.socket = m_serverInfo.rooms[i].members[j].socket;
							Send(roomMember, (char*)broadcastBuffer.Data.data(), broadcast.size());
						}
					}
				}
				break;
			case MessageType::Send:
				for (int i = 0; i < m_serverInfo.rooms.size(); i++) {
					if (m_serverInfo.rooms[i].roomName == "General") {
						for (int j = 0; j < m_serverInfo.rooms[i].members.size(); j++) {
							std::string broadcast = "Im testing broadcasting to everyone.";
							Buffer broadcastBuffer = Buffer();
							broadcastBuffer.WriteString((char*)broadcast.c_str());
							ClientInfo roomMember;
							roomMember.socket = m_serverInfo.rooms[i].members[j].socket;
							Send(roomMember, (char*)broadcastBuffer.Data.data(), broadcast.size());
						}
					}
				}
				break;
			default:
				break;
			}
			

			std::cout << "packagelength: " << client.packetLength << " messageId:" << messageId << " roomNameSize:" << roomNameSize << " roomname:" << roomname << std::endl;

		}
		
		printf("Bytes received: %d\n", recvResult);
		printf("Message From the client:\n%s\n", buf);
	}
	else {
		printf("Receive failed. Error - %d\n", WSAGetLastError());
	}
	return recvResult;
}

int Server::Send(ClientInfo& client, char buf[], int receiveResult) {
	printf("Sending message : %s\n", buf);
	int sendResult = send(client.socket, buf, receiveResult, 0);
	if (sendResult == SOCKET_ERROR) {
		printf("Send failed. Error - %d\n", WSAGetLastError());
		client.connected = false;
	}
	else if (sendResult > 0) {
		printf("Bytes sent: %d\n", sendResult);
	}
	else {
		printf("No response from client\n");
	}
	return sendResult;
}

void Server::ShutDown() {
	printf("Shutting down server . . .\n");
	freeaddrinfo(m_serverInfo.info);
	closesocket(m_serverInfo.listenSocket);
	WSACleanup();
}

void Server::CreateSocket() {
	m_serverInfo.listenSocket =
		socket(
			m_serverInfo.info->ai_family,
			m_serverInfo.info->ai_socktype,
			m_serverInfo.info->ai_protocol
		);
	if (m_serverInfo.listenSocket == INVALID_SOCKET) {
		printf("Failed to Create Socket. Error - %d\n", WSAGetLastError());
		freeaddrinfo(m_serverInfo.info);
		WSACleanup();
		exit(1);
	}
	else {
		printf("Socket was created successful!\n");
	}
}

void Server::GetServerAddrInfo() {
	int state = -1;

	ZeroMemory(&m_serverInfo.hints, sizeof(m_serverInfo.hints));

	m_serverInfo.hints.ai_family = AF_INET;
	m_serverInfo.hints.ai_flags = AI_PASSIVE;
	m_serverInfo.hints.ai_protocol = IPPROTO_TCP;
	m_serverInfo.hints.ai_socktype = SOCK_STREAM;

	state = getaddrinfo(NULL, DEFAULT_PORT, &m_serverInfo.hints, &m_serverInfo.info);
	if (state != 0) {
		printf("Failed to Create getaddrinfo. Error - %d\n", state);
		WSACleanup();
		exit(1);
	}
	else {
		printf("getaddrinfo was successful!\n");
	}
}

void Server::Startup() {
	int state = -1;
	WORD wVersionRequested = MAKEWORD(2, 2);

	state = WSAStartup(wVersionRequested, &m_wsaData);
	if (state != 0) {
		printf("Failed to startup WSAStartup. Error - %d\n", state);
		exit(1);
	}
	else {
		printf("WSAStartup was successful! \n");
	}
}
