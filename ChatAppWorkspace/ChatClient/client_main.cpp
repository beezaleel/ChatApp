#include "Client.h"
#include <conio.h>
#include <iostream>
#include <string>
#include <windows.h>

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
		case 8:
			if (!message.empty())
				message.pop_back();
			break;
		case 13:
			printf("Send message to server! \n");
			message.clear();
			break;
		case 27:
			SetConsoleColor(15);
			printf("Exiting program . . . \n");
			exit(0);
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
	printf("#               Enter /J to Join room                            #\n");
	printf("#               Enter /L to Leave room                           #\n");
	printf("##################################################################\n");

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