#include "Client.h"
#include <conio.h>
#include <string>

std::string message = "";
std::string name = "";

void ProcessMessage() {}

void ProcessKeyboardInput() {
	if (_kbhit) {
		int key = _getch();
		switch (key)
		{
		case 13:
			printf("Send message to server! \n");
			message.clear();
			break;
		case 27:
			printf("Exiting program . . . \n");
			exit(0);
		case 8:
			if (!message.empty())
				message.pop_back();
			break;
		default:
			message += (char)key;
			printf("%c", (char)key);
			break;
		}
	}
}

int main(int argc, char* argv) {
	while (true) {
		ProcessKeyboardInput();
	}
	int success = 0;
	Client client = Client();
	client.Initialize();

	if (client.Connect() != success)
		return 1;

	if (client.ManageSocket() != success)
		return 1;

	system("Pause");

	const int recvBufLen = 128;
	char recvBuf[recvBufLen];

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