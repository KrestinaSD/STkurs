/**
* @file Client.h
* @author  Крестина С.Д.
* @version 1.0
* @date 27.02.2023
* @copyright ИБСТ ПГУ
* @brief Заголовочный файл для модуля Client
*/
#include <iostream>
#include <fstream>
#include <unistd.h> //close()
#include <arpa/inet.h>

using namespace std;

class Client
{
public:
    int Server(string str1, string str2, bool force_register = false);
    int port;
    int32_t sum;
    string msg;
    string autf_file;
    string username;
    string pswd;
};


class client_error: public invalid_argument
{
public:
    explicit client_error (const string& what_arg):
        invalid_argument(what_arg) {}
    explicit client_error (const char* what_arg):
        invalid_argument(what_arg) {}
};
