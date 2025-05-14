/**
* @file main.cpp
* @brief Главный модуль программы для получения параметров от пользователя
* @param str1 - адрес сервера
* @param str2 - порт сервера
* @
*/

#include "Client.h"

using namespace std;

int main (int argc, char *argv[])
{

    Client Podclychenie;

    if(argc == 1) {
        cout << "Программа клиента" << endl;
        cout << "Параметры для запуска: " << endl;
        cout << "\t-h — справка" << endl;
        cout << "\t-i — адрес сервера (обязательный)" << endl;
        cout << "\t-p — порт сервера (необязательный — по умолчанию 33333)" << endl;
        return 0;
    }

    string str1;
    string str2;

    int opt;
    while ((opt=getopt (argc,argv,"hi:p:e:s:a:"))!=-1) {

        switch(opt) {

        case 'h':
            cout << "Программа клиента" << endl;
            cout << "Параметры для запуска: " << endl;
            cout << "\t-h — справка" << endl;
            cout << "\t-i — адрес сервера (обязательный)" << endl;
            cout << "\t-p — порт сервера (необязательный — по умолчанию 33333)" << endl;
            return 0;

        case 'i':
            str1 = argv[2];

            break;

        case 'p':
                str2 = string(optarg);
            break;

        case '?':
            cout << "Праметр задан не верно или такого праметра не существует" << endl;
            return 0;
        };
    };

    Podclychenie.Server(str1, str2);
    return 0;
};
