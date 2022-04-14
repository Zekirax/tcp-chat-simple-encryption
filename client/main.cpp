#include <iostream>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <sstream>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

fd_set master;

void crypte(string &text, char* &key, int &key_size){
    for (size_t i = 0; i < text.length(); i++)
    {
        text[i] = ~(text[i] + key[i % key_size]);
    }
}

void main()
{
	string ipAddress = "127.0.0.1";			// IP Address of the server
	int port = 54000;						// Listening port # on the server

	// Initialize WinSock
	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0)
	{
		cerr << "Can't start Winsock, Err #" << wsResult << endl;
		return;
	}

	// Create socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cerr << "Can't create socket, Err #" << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	// Fill in a hint structure
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	// Connect to server
	int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		cerr << "Can't connect to server, Err #" << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return;
	}

	// Do-while loop to send and receive data
	char buf[4096];
	string userInput;

    // recv secure key
	int key_size = recv(sock, buf, 4096, 0); // we get the stop 0. so remove stop 0 from size

    char *key = new char[key_size];
    memcpy(key, &buf, key_size);
    
    cout << "Secure key recvieved with a size of " << key_size << endl;

	do
	{
		// Prompt the user for some text
		cout << "> ";
		getline(cin, userInput);

		if (userInput.size() > 0)		// Make sure the user has typed in something
		{
			// Send the text
            crypte(userInput, key, key_size); // encrypte

			int sendResult = send(sock, userInput.c_str(), userInput.size(), 0);
			/*if (sendResult != SOCKET_ERROR)
			{
				// Wait for response
				ZeroMemory(buf, 4096);
				int bytesReceived = recv(sock, buf, 4096, 0);
				if (bytesReceived > 0)
				{
                    string msg = string(buf, 0, bytesReceived); // decrypte
                    crypte(msg, key, key_size);

					cout << "SERVER> " << msg << endl;
				}
			}*/
		}
	
	} while (userInput.size() > 0);

	// Gracefully close down everything
	closesocket(sock);
	WSACleanup();
}