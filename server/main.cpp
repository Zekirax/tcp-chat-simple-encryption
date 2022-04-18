#include <iostream>
#include<thread>

#include<string>
#include<string.h>
#include<list>
#include<vector>
// socket includes
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../encryption.hpp"

using namespace std;

#define BUFF_LENGTH 4096 // for recv msg

// static thing. So these elements can be accessed by other threads
list<int> sockets;

int listening;
int socket_index = 0;

in_addr_t get_socket_ip(const int &fd){
    sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    getpeername(fd, (struct sockaddr *)&addr, &addr_size);

    return addr.sin_addr.s_addr;
}

char *get_socket_string_ip(const int &fd){
    sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    getpeername(fd, (struct sockaddr *)&addr, &addr_size);

    return inet_ntoa(addr.sin_addr);
}

int main(int, char**) {
    
    // server config
    #define SAME_IP_LIMITE 1 // -1 is unlimited
    #define MAX_CLIENT 64
    #define IP "127.0.0.1"
    #define PORT 2556

    sockaddr_in hints;
    hints.sin_family = AF_INET;
    hints.sin_port = htons(PORT);
    inet_aton(IP, (in_addr *) &hints.sin_addr.s_addr);

    // create listening socket
    listening = socket(hints.sin_family, SOCK_STREAM, 0);
    if (listening == -1)
    {
        cerr << "Failed to create listening socket" << strerror(errno) << endl;
        return -1;
    }

    // bind socket
    if (bind(listening, (sockaddr *) &hints, sizeof(hints)) == -1)
    {
        cerr << "Failed to bind to the listening socket " << strerror(errno) << endl;
        return -1;
    }
    
    if(listen(listening, SOMAXCONN))
    {
        cerr << "Failed to listen" << strerror(errno) << endl;
        return -1;
    }

    cout << "Server is bonded to port : " << PORT << " | IP : " << IP << endl; 

    // accept sockets thread
    thread([]
        {    
            // declare some var used in the while
            #if SAME_IP_LIMITE != -1
            int same_ip_count = 0;
            bool too_many_same_ip;
            #endif
            int new_client_fd;

            while (true)
            {
                new_client_fd = accept(listening, nullptr, nullptr);
                if (new_client_fd != 1)
                {
                    if (sockets.size() == MAX_CLIENT)
                    {
                        close(new_client_fd);
                    }
                    else
                    {
                        #if SAME_IP_LIMITE != -1
                            same_ip_count = 0;
                            for (auto &fd : sockets)
                            {
                                if (get_socket_ip(fd) == get_socket_ip(new_client_fd))
                                {
                                    same_ip_count++;

                                    if (same_ip_count > SAME_IP_LIMITE)
                                    {
                                        too_many_same_ip = true;

                                        cout << "Connection from " << get_socket_string_ip(new_client_fd) << " was refused for exceed SAME_IP_LIMITE == " << SAME_IP_LIMITE << endl;

                                        close(new_client_fd);

                                        break;
                                    }
                                }
                            }
                        #endif

                        #if SAME_IP_LIMITE != -1
                        if (too_many_same_ip)
                        {
                            too_many_same_ip = false;
                        }
                        else
                        {
                        #endif
                            socket_index++;
                            
                            cout << get_socket_string_ip(new_client_fd) << " is connected. ID : " << socket_index << endl; 
                            
                            string key = generate_key();

                            // send key
                            char buf[CRYPTE_KEY_SIZE];
                            send(new_client_fd, &buf, CRYPTE_KEY_SIZE, 0);
                            
                            sockets.push_back(new_client_fd);

                            // recv thread
                            thread([fd = new_client_fd, key, index = socket_index]
                            { 
                                char buf[BUFF_LENGTH];
                                while (true)
                                {
                                    memset(&buf, 0, sizeof(buf));
                                    int bytesReceived = recv(fd, buf, BUFF_LENGTH, 0);
                                    if (bytesReceived > 0)
                                    {
                                        string msg = string(buf, 0, bytesReceived);
                                        crypte(msg, key);

                                        cout << get_socket_string_ip(fd) << "/" << index << "> " << msg << endl; 
                                    }
                                    else
                                    {
                                        sockets.remove(fd);

                                        cout << get_socket_string_ip(fd) << "/" << index << " disconnected" << endl;

                                        close(fd);

                                        break;                                    
                                    }
                                }                                    
                            }).detach();
                        #if SAME_IP_LIMITE != -1
                        }
                        #endif
                    }
                }            
            }
        }
    ).detach();
    cout << "Server is ready to accept new clients" << endl;

    // wait
    cin.get();

    // end
    close(listening);

    for (auto &fd : sockets)
    {
        close(fd);
    }

    cout << "Server closed" << endl;

    return 0;
}