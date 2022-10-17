#include "Client.h"
#include "Buffer.h"
#include "Common.h"
#include <conio.h>
#include <iostream>
#include <string>
#include <windows.h>

#define BACKSPACE 8
#define ENTER 13
#define ESC 27

enum MessageType {
	Join = 1,
	Leave = 2,
	Send = 3
};

Client client;
std::string message = " ";
std::string name = "";

void SetConsoleColor(int);

void ProcessMessage() {}

void ProcessKeyboardInput() {
	if (_kbhit) {
		int key = _getch();
		switch (key)
		{
		case BACKSPACE:
			if (!message.empty())
				message.pop_back();
			break;
		case ENTER:
			printf("Send message to server! \n");
			message.clear();
			break;
		case ESC:
			SetConsoleColor(15);
			printf("Exiting program . . . \n");
			exit(client.ShutDown());
		default:
			message += (char)key;
			printf("%c", (char)key);
			break;
		}
	}
}

void SetConsoleColor(int color) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

int main(int argc, char* argv) {
	printf("##################################################################\n");
	printf("#                       DEMO CHAT                                #\n");
	printf("#               Enter username to start                          #\n");
	printf("#               Enter 'Join roomname' to Join room               #\n");
	printf("#               Enter 'Leave roomname' to Leave room             #\n");
	printf("##################################################################\n");

	Header h;
	h.messageId = 10;
	h.packetLength = 20;

	std::cout << "Message Id: " << h.messageId << " Packet length: " << h.packetLength << std::endl;

	Buffer uIntTest = Buffer(2);
	uIntTest.WriteUInt32(4294967294);
	uint32_t g = uIntTest.ReadUInt32(0);
	std::cout << "Assert: 4294967295 equals: " << g << std::endl;

	const int recvBufLen = 128;
	char recvBuf[recvBufLen];

	client = Client();
	client.Initialize();

	int success = 0;

	if (client.Connect() != success)
		return 1;

	if (client.ManageSocket() != success)
		return 1;

	SetConsoleColor(1);
	printf("\n");
	while (name.length() == 0) {
		printf("Please enter username: ");
		std::cin >> name;
	}
	while (true) {
		ProcessKeyboardInput();
		client.Receive(recvBuf, recvBufLen);
	}

	system("Pause");

	//const int recvBufLen = 128;
	//char recvBuf[recvBufLen];

	const char* buf = "Hello!";
	int buflen = 6;

	client.Send(buf, buflen);

	bool retry = true;
	while (retry)
	{
		int response = client.Receive(recvBuf, recvBufLen);
		if (response == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAEWOULDBLOCK) {
				retry = true;
			}
			else {
				return 1;
			}
		}
		else {
			retry = false;
		}
	}
	client.ShutDown();
	return 0;
}