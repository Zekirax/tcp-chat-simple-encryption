// @file thread2.cpp
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

using namespace std;

struct prout
{
    int &fd;
};


int main(){ 
    vector<prout> sockets(1e3); // vector have limits because an array of 2000000 is complicated
        

    return 0;
}