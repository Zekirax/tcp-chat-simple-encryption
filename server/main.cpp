#include <iostream>
#include<ctime>

#define FD_SETSIZE 128 // will be set to 64 in winsock2
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

fd_set master;
int same_ip_limite = 1; // -1 is illimited

void crypte(string &text, char* &key, int &key_size){
    for (size_t i = 0; i < text.length(); i++)
    {
        text[i] = ~(text[i] + key[i % key_size]);
    }
}

char* generate_key(const int &size){  // size in bytes
    char* key = new char[size];

    if(size % 4 != 0)
        throw invalid_argument("Key size must be a multiple of 4 !");

    for (size_t i = 0; i < size / 4; i++)
    {
        memset(key + 4 * i, rand(), 4);
    }
    
    return key;
}

int get_socket_ip(SOCKET &socket){
	sockaddr_in addr;
	socklen_t len = sizeof(sockaddr_in);
	getsockname(socket, (sockaddr *)&addr, &len);

	return addr.sin_addr.S_un.S_addr;
}

string get_socket_string_ip(SOCKET &socket){
	int ip = get_socket_ip(socket);
	char string_ip[INET_ADDRSTRLEN];

	return string( inet_ntop(AF_INET, &ip, string_ip, INET_ADDRSTRLEN) );
}

void close_socket(SOCKET &socket)
{
	cout << get_socket_string_ip(socket) << " is disconnected" << endl;

	FD_CLR(socket, &master); // drop client

	closesocket(socket);
}

int main(int, char**) 
{
	srand(time(nullptr)); // to generate random key
	// Initialze winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		cerr << "Can't Initialize winsock! Quitting" << endl;
		return EXIT_FAILURE;
	}
	
	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cerr << "Can't create a socket! Quitting" << endl;
		return EXIT_FAILURE;
	}

	// Bind the ip address and port to a socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY; // Could also use inet_pton .... 
	
	bind(listening, (sockaddr*)&hint, sizeof(hint));

	// Tell Winsock the socket is for listening 
	listen(listening, SOMAXCONN);

	// Create the master file descriptor set and zero it
	FD_ZERO(&master);

	FD_SET(listening, &master);

	int key_size = 16; // 128 bits
	char *key = generate_key(key_size);
	while (true)
	{
		fd_set copy = master;

		int socket_count = select(0, &copy, nullptr, nullptr, nullptr); // select destroy copy. so copy master. do it
		for (int i = 0; i < socket_count; i++)
		{
			SOCKET socket = copy.fd_array[i];
			if (socket == listening)
			{				
				SOCKET client = accept(socket, nullptr, nullptr);
				int client_ip = get_socket_ip(client);

				// same ip check
				bool too_many_same_ip;
				if (same_ip_limite != -1)
				{
					too_many_same_ip = false;
					
					int same_ip_count = 0;
					for (int j = 1; j < master.fd_count; j++) // not check listening socket
					{
						if (get_socket_ip(master.fd_array[j]) == client_ip)
						{
							same_ip_count++;

							if (same_ip_count > same_ip_limite)
							{
								too_many_same_ip = true;
								break;
							}
							
						}
					}
				}

				if (too_many_same_ip)
				{
					cout << "another connexion from " << client_ip << " was refused. Limite same ip is set to " << same_ip_limite << endl;

					close_socket(client);
					continue;
				}

				FD_SET(client, &master); // add to list of connected clients

				cout << get_socket_string_ip(client) <<" is connected" <<endl;

				send(client, key, key_size, 0); // send secure key
			}
			else
			{
				// recieve msg
				char buf[4096];
				ZeroMemory(buf, 4096);

				int bytesReceived = recv(socket, buf, 4096, 0);
				if (bytesReceived > 0)
				{
					string msg = string(buf, 0, bytesReceived); // decrypte
					crypte(msg, key, key_size);

					cout << get_socket_string_ip(socket) << "> " << msg << endl; 
				}
				else
				{
					close_socket(socket);
				}
			}
		}
		
	}
	
}