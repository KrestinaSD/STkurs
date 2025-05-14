/**
* @file communicator.h
* @author Крестина С.Д.
* @version 1.0
* @brief Заголовочный файл для модуля communacator, отвечающий за связь с клиентом
*/

#pragma once
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <map>

#include <cryptopp/cryptlib.h>
#include <cryptopp/hex.h>
#include <cryptopp/files.h>
#include <cryptopp/filters.h>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/md5.h>

#include <fstream>
#include <random>
#include <sstream>
#include <exception>
#include <typeinfo>
#include "programmerror.h"
#include "logger.h"
#include "userbase.h"
#include "interface.h"

#include <thread>
#include <mutex>
#include "filelogger.h"


class communicator
{
private:
    
    struct sockaddr_in addr; ///< Структура sockaddr_in
    int sckt; ///< Основной сокет 
    std::string password; ///< Пароль клиента
    
    std::string strHash; ///< Хэш
    
     // Многопоточные атрибуты
    static std::mutex clients_mutex;
    static int active_clients;
    static const int MAX_CLIENTS = 3;
 
    void client_handler(int client_socket, sockaddr_in client_addr, DB& user_db);
    
public:
	
	communicator(){};
    void conversation(const std::string& address, unsigned int port, DB& user_db);
    std::string GenSALT();
    std::string GenHash(const std::string& password, const std::string& salt);
    bool CompareHashes(std::string ClientHash, const std::string& salt, const std::string& password);
};
