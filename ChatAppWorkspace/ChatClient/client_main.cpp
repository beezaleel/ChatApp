#include "Client.h"

int main(int argc, char* argv) {
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