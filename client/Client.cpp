/**
 * @file Client.cpp
 * @brief Файл взаимодействие с сервером
 */

#include "md5.h"
#include "Client.h"
#include <vector>
#include <iostream>

//цветной вывод
#define COLOR_GREEN "\033[32m"
#define COLOR_RESET "\033[0m"


int Client::Server(string str1, string str2, bool force_register)
{
	
	 string line;
	//Инициализация параметров
    if(str2 == "") str2 = "33333";
    
	// Подготовка сетевого адреса
    char buf[255];
    try {
        if(str1.length() >= sizeof(buf)) {
            throw runtime_error("Слишком длинный адрес");
        }
        strncpy(buf, str1.c_str(), sizeof(buf));
        buf[sizeof(buf)-1] = '\0';
    } catch (...) {
        throw client_error("fun:Server, param:buf.\nНевозможно получить адрес");
    }

    // Проверка номера порта
    try {
        port = stoi(str2);
        if(port <= 0 || port > 65535) {
            throw out_of_range("Порт вне допустимого диапазона");
        }
    } catch (...) {
        throw client_error("fun:Server, param:port.\nНевозможно получить порт (должно быть число 1-65535)");
    }


    //Подготовка сокета   
    sockaddr_in * selfAddr = new (sockaddr_in);
    selfAddr->sin_family = AF_INET;
    selfAddr->sin_port = 0;
    selfAddr->sin_addr.s_addr = 0;

    sockaddr_in * remoteAddr = new (sockaddr_in);
    remoteAddr->sin_family = AF_INET;
    remoteAddr->sin_port = htons(port);
    remoteAddr->sin_addr.s_addr = inet_addr(buf);

    int mySocket = socket(AF_INET, SOCK_STREAM, 0);
    if(mySocket == -1) {
        close(mySocket);
        throw client_error(string("fun:Server, param:mySocket.\nОшибка создания сокета"));
    }

	

	//Подключение
    int rc = bind(mySocket,(const sockaddr *) selfAddr, sizeof(sockaddr_in));
    if (rc == -1) {
        close(mySocket);
        throw client_error(string("fun:Server, param:selfAddr.\nОшибка привязки сокета с локальным адресом"));
    }

    rc = connect(mySocket, (const sockaddr*) remoteAddr, sizeof(sockaddr_in));
    if (rc == -1) {
        close(mySocket);
        throw client_error(string("fun:Server, param:remoteAddr.\nОшибка подключения сокета к удаленному серверу"));
    }

	// После подключения сразу проверяем статус перегрузки
	char overload_buffer[64];
	int bytes = recv(mySocket, overload_buffer, sizeof(overload_buffer)-1, 0);
	if (bytes > 0) {
    	overload_buffer[bytes] = '\0';
    	if (string(overload_buffer) == "SERVER_OVERLOAD") {
        	cout << "Сервер перегружен. Попробуйте позже." << endl;
        	close(mySocket);
        	return 0;
    	} else if (string(overload_buffer) == "OK"){
    		cout << "Подключение..." << endl;
    	}
	}
	  // Определяем, регистрируемся или авторизуемся
    string command;
    if (force_register) {
        command = "REGIST";
    } else {
        cout << "Вы хотите зарегистрироваться (R) или войти (L)? [R/L]: ";
        getline(cin, command);
        command = (toupper(command[0]) == 'R') ? "REGIST" : "AUTH";
    }
    // Отправка команды
   if (send(mySocket, command.c_str(), command.size(), 0) == -1) {
        close(mySocket);
        throw client_error("Ошибка отправки команды");
    }
    
    // Ждем подтверждения от сервера
    char buffer[1024];
    bytes = recv(mySocket, buffer, sizeof(buffer)-1, 0);
    if (bytes <= 0) {
        close(mySocket);
        throw client_error("Ошибка связи с сервером");
    }
    buffer[bytes] = '\0';
    
    if (string(buffer) != "READY_FOR_LOGIN") {
        close(mySocket);
        throw client_error("Ошибка протокола");
    }
    
     // Ввод логина
    cout << "Введите логин: ";
    string login;
    getline(cin, login);
    send(mySocket, login.c_str(), login.size(), 0);
    
    // Ждем подтверждения для ввода пароля
    bytes = recv(mySocket, buffer, sizeof(buffer)-1, 0);
    if (bytes <= 0) {
        close(mySocket);
        throw client_error("Ошибка связи с сервером");
    }
    buffer[bytes] = '\0';
    
    if (string(buffer) != "READY_FOR_PASSWORD") {
        close(mySocket);
        throw client_error("Ошибка протокола");
    }
    
    // Ввод пароля
    cout << "Введите пароль: ";
    string password;
    getline(cin, password);
    send(mySocket, password.c_str(), password.size(), 0);
    
    // Получаем итоговый ответ
    bytes = recv(mySocket, buffer, sizeof(buffer)-1, 0);
    if (bytes <= 0) {
        close(mySocket);
        throw client_error("Ошибка связи с сервером");
    }
    buffer[bytes] = '\0';
    string response(buffer);
    
    // Обработка ответа для REGIST
	if (command == "REGIST") {
    	if (response == "REG_OK") {
        	cout << "Регистрация прошла успешно! Чтобы войти, подключитесь еще раз." << endl;
        	close(mySocket);
        	return 0;
    	} else {
        	cout << "Ошибка регистрации: " << response << endl;
        	close(mySocket);
        	return -1;
    	}
	} 
      // Обработка ответа для AUTH
	else if (command == "AUTH") {
    	if (response == "LOGIN_NOT_FOUND") {
        	cout << "Пользователь не найден" << endl;
        	close(mySocket);
        	return -2;
    	} 
    	// Если логин найден, сервер отправит соль
    else {  	   
    	
		string salt = response;
		cout << "[КЛИЕНТ] Получаем соль: " << salt << endl;
    	string hsh = salt + password;
    	msg = MD5_hash(hsh);
    	
    	strcpy(buffer,msg.c_str());
    	rc = send(mySocket, buffer, msg.length(), 0);
    	if (rc == -1) {
        	close(mySocket);
        	throw client_error(string("fun:Server, param:msg.\nОшибка оправки хэша"));
    	}
    	cout << "[КЛИЕНТ] Отправляем хэш: " << buffer << endl;
	
	
		//Проверка аутентификации
    	rc = recv(mySocket, buffer, sizeof(buffer), 0);
    	buffer[rc] = '\0';
    	if (rc == -1) {
        	close(mySocket);
        	throw client_error(string("fun:Server, param:buffer.\nОшибка получения ответа об аунтефикации"));
    	}
    	
    	if (string(buffer) != "AUTH_OK") {
        	close(mySocket);
        	throw client_error("Ошибка аутентификации");
    	}
    	
    	cout << "Мы получаем ответ: " << buffer << endl;
     	buffer[rc] = '\0';
	
		// Отправка строк от пользователя
    	cout << "Аутентификация успешна. Вводите сообщения (enter для отправки):\n";
		
    	while(true) {
       	
    	 	// Вывод приглашения
     		cout << "> ";  // Индикатор ввода 
   			string input_line;
   			input_line.clear(); // Очистка строки ввода
   			getline(cin, input_line);
               	
        	if(input_line.empty()) continue;  // Пропускаем пустые строки
        	
        	// Подготовка вектора с данными (добавляем \n в конец)
        	vector<char> message(input_line.begin(), input_line.end());
       	message.push_back('\n');
	
        	// Отправка размера и самого сообщения
        	uint32_t msg_size = message.size();
        	if(send(mySocket, &msg_size, sizeof(msg_size), 0) == -1 ||
        	send(mySocket, message.data(), message.size(), 0) == -1) {
            	close(mySocket);
            	throw client_error("Ошибка отправки размера сообщения");
        	}
	
        	message.clear(); // Очистка буфера
        	       	
        	
        	// Получение стандартного подтверждения
        	uint32_t ack_size;
        	ssize_t bytes = recv(mySocket, &ack_size, sizeof(ack_size), 0);
        	if(bytes <= 0) {
            	cerr << "Ошибка получения размера подтверждения\n";
            	break;
        	}
        	
		  	//ack_size = ntohl(ack_size); // Добавлено преобразование порядка байт   
	
        	vector<char> ack_buffer(ack_size);
			bytes = recv(mySocket, ack_buffer.data(), ack_size, MSG_WAITALL);
			if(bytes <= 0) {
    			cerr << "Ошибка получения подтверждения\n";
    			break;
			}
			
        	string response(ack_buffer.begin(), ack_buffer.end());
        	ack_buffer.clear(); // Подготовка к следующему сообщению
          	
          	// Проверка команд выхода
        	if (input_line == "q" || input_line == "quit" || input_line == "exit") {
            	cout <<  COLOR_GREEN "[Ответ сервера]: " COLOR_RESET << response << endl;
            	break;
        	}
        	
        	cout << COLOR_GREEN "[Сервер]: " COLOR_RESET << response << endl;
        
    	}
    	} // Конец else (обработка соли)
    } // Конец else if (command == "AUTH")
	 else {
        close(mySocket);
        throw client_error("Неизвестная команда");
    }

    close(mySocket);

    delete selfAddr;
    delete remoteAddr;
    return 0;
}

