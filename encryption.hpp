#include <iostream>
#include<string>
#include<string.h>
#include<vector>

using namespace std;

#define CRYPTE_KEY_SIZE 4 // CRYPTE_KEY_SIZE * 32 to have the size in bit

// key_size is * 4. 4 because key are generated with int rand()
string generate_key(){
    string key;
    key.resize(CRYPTE_KEY_SIZE * 4);

    // set random byte in the key
    for (unsigned char i = 0; i < CRYPTE_KEY_SIZE; i += 4) // 4 = sizeof int
    {
        memset(&key[i], rand(), 4);
    }

    return key;
}

void crypte(string &text, const string &key){ // when i put my key in the lambda so [key]{} in the lambda, key will be const string
    for (unsigned short i = 0; i < text.size(); i++)
    {
        text[i] = ~(text[i] + key[i % CRYPTE_KEY_SIZE]);
    }
}