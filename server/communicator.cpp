#include "communicator.h"
#include <limits>
#include <string.h>
#include <vector>
#include <memory>
#include <sstream>
#include "filelogger.h"
using namespace std;

struct sockaddr_in addr;

// Инициализация статических членов класса
std::mutex communicator::clients_mutex;
int communicator::active_clients = 0;

interface lg; 
string logfile = lg.getLogFileName();
Logger logi(logfile);

string mes = "message_from_clients.txt";
FileLogger logger(mes);


std::string communicator::GenSALT()
{
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_int_distribution<long long unsigned> distribution(0,0xFFFFFFFFFFFFFFFF);
    unsigned long long numericsalt = distribution(generator);
    std::stringstream stream;
    stream << std::hex << numericsalt;
    std::string strsalt(stream.str());
    for (long unsigned int i = 0; i <= strsalt.size(); i++) {
        strsalt[i] = toupper(strsalt[i]);
    }
    if (strsalt.size() < 16) {
        std::string strsalttemp =strsalt;
        for (unsigned i = strsalt.size(); i < 16; i++) {
            strsalttemp.insert(strsalttemp.begin(), '0');
        }
        strsalt = strsalttemp;
    }
    return strsalt;
}

std::string communicator::GenHash(const std::string& password, const std::string& salt) {
    std::string msg = salt + password;
    std::string strHash;

    CryptoPP::Weak::MD5 hash;
    CryptoPP::StringSource ss(msg, true,
        new CryptoPP::HashFilter(hash,
            new CryptoPP::HexEncoder(
                new CryptoPP::StringSink(strHash))));
    return strHash;
}

bool communicator::CompareHashes(std::string ClientHash, const std::string& salt, const std::string& password) {
 	 std::string ServerHash = GenHash(password, salt);

    if (ClientHash != ServerHash) {
        throw server_error("Invalid Hash");
    }
	std::cout<<"Kлиент: "<<ClientHash<<"\nСервер: "<<ServerHash<<" "<<std::endl;
	logi.writelog("Хеш успешно верифицирован для клиента");
    return (ClientHash.compare(ServerHash) == 0);
}


void communicator::conversation(const std::string& address, unsigned int port, DB& user_db)
{
	try{
	// Создание сокета
    int sckt = socket(AF_INET, SOCK_STREAM, 0);
    if(sckt < 0) {
        throw server_error("Socket creation error", true);
    }
    logi.writelog("Socket creation OK");
    
     // Задание параметров адреса сервера
    addr.sin_family = AF_INET; // Семейство адресов
    addr.sin_port = htons(port); // Порт, на котором будет работать сервер
    if(inet_pton(AF_INET, address.c_str(), &addr.sin_addr) <= 0) {
            throw server_error("Invalid address: " + address, true);
    }
    //addr.sin_addr.s_addr = inet_addr(Adress); // адрес

    // Привязка сокета к адресу
    int rc = bind(sckt, (struct sockaddr *)&addr, sizeof(addr));
    if(rc < 0) {
        throw server_error("Socket bind error", true);
    }
    logi.writelog("Socket bind OK");

    // Начало прослушивания входящих соединений
    int l = listen(sckt, 5);
    if (l < 0) {
    	throw server_error("Listening error", true);
    }
    std::cout << "The server started on the port " << port << " and address " << address << std::endl;
    logi.writelog("The server started on the port" + std::to_string(port) + " and adress " + address);
    
    // Многопоточный accept()
    std::vector<std::thread> client_threads;
    client_threads.reserve(MAX_CLIENTS); // Предварительно резервируем память
    socklen_t addr_len = sizeof(addr);
    
    while (true) {
    	sockaddr_in client_addr;
        int client_socket = accept(sckt, (struct sockaddr *)&client_addr, &addr_len);
    	
    	 // 4. Проверка лимита клиентов 
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            if (active_clients >= MAX_CLIENTS) {
                std::string msg = "SERVER_OVERLOAD";
    			send(client_socket, msg.c_str(), msg.size(), 0);
    			close(client_socket);
    			logi.writelog("Отклонено подключение: сервер перегружен");
    			continue;
            } else if (active_clients < MAX_CLIENTS){
            	std::string msg = "OK";
    			send(client_socket, msg.c_str(), msg.size(), 0);
            }
            active_clients++;
            cout << "[СЕРВЕР] Активных клиентов: " << active_clients << endl;
        }
    	
    	 // 5. Запуск обработчика клиента в отдельном потоке
        client_threads.emplace_back([this, client_socket, client_addr, &user_db]() {
         this->client_handler(client_socket, client_addr, user_db);
    	});
        // Отсоединяем поток, чтобы не блокировать основной
        client_threads.back().detach();
        
        }
        
    }catch (const server_error & e) {
		std::stringstream ss;
    	ss << "Error: " << e.what() << ", State: " << e.getState();
    	logi.writelog(ss.str());
          if (e.getState() == "Критическая"){
          	close(sckt);
		}
		std::cerr << e.what() << std::endl;
        
    } 
}
    
void communicator::client_handler(int client_socket, sockaddr_in client_addr, DB& user_db) {
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    
    cout << "[СЕРВЕР] Новое подключение от " << client_ip << endl;
    logi.writelog("Клиент подключен: " + std::string(client_ip));

    try {
        char buffer[1024] = {0}; //заполняем нулями
        ssize_t bytes_received;
        
        // 1. получаем команду регистрации, или аутентификации
        bytes_received = recv(client_socket, buffer, sizeof(buffer)-1, 0);
        if (bytes_received <= 0) throw server_error("Ошибка получения команды");
        buffer[bytes_received] = '\0';
        std::string command(buffer);
        
        // Отправляем подтверждение получения команды
        std::string ready_msg = "READY_FOR_LOGIN";
        send(client_socket, ready_msg.c_str(), ready_msg.size(), 0);
        
         //получаем логин
        bytes_received = recv(client_socket, buffer, sizeof(buffer)-1, 0);
        if (bytes_received <= 0) throw server_error("Ошибка получения логина");
        buffer[bytes_received] = '\0';
        std::string login(buffer);
        
        // Отправляем подтверждение получения логина
        ready_msg = "READY_FOR_PASSWORD";
        send(client_socket, ready_msg.c_str(), ready_msg.size(), 0);
        
        // 3. Получаем пароль
        bytes_received = recv(client_socket, buffer, sizeof(buffer)-1, 0);
        if (bytes_received <= 0) throw server_error("Ошибка получения пароля");
        buffer[bytes_received] = '\0';
        std::string password_data(buffer);
        
         // Обработка регистрации
         if (command == "REGIST") {
            std::lock_guard<std::mutex> lock(clients_mutex);
            
            if (user_db.user_exists(login)) {
                std::string err_msg = "LOGIN_EXISTS";
                send(client_socket, err_msg.c_str(), err_msg.size(), 0);
                throw server_error("Логин уже существует");
                close(client_socket);
        		active_clients--; // Уменьшаем счетчик
        		cout << "[СЕРВЕР] Активных клиентов: " << active_clients << endl;
        		return;
            }
            
            user_db.add_user(login, password_data);
            std::string ok_msg = "REG_OK";
            cout << "[СЕРВЕР] Клиент зарегистрирован: " << login << endl;
    		logi.writelog("Клиент зарегистрирован: " + login);
            send(client_socket, ok_msg.c_str(), ok_msg.size(), 0);
            active_clients--; // Уменьшаем счетчик
            cout << "[СЕРВЕР] Активных клиентов: " << active_clients << endl;
            close(client_socket);
            return;
        }
        // Обработка авторизации
        else if (command == "AUTH") {
            if (!user_db.user_exists(login)) {
                std::string offer_msg = "LOGIN_NOT_FOUND";
                send(client_socket, offer_msg.c_str(), offer_msg.size(), 0);
                close(client_socket);
                active_clients--; // Уменьшаем счетчик
            	cout << "[СЕРВЕР] Активных клиентов: " << active_clients << endl;
                return;
            }        
        	
	// Продолжение обычной аутентификации
       
        // 3. Генерируем и отправляем соль
        std::string salt = GenSALT();
        int rc = send(client_socket, salt.c_str(), salt.size(), 0);
         if (rc <= 0) {
            throw server_error("Ошибка отправки соли");
        }
        
        // 4. Получаем хеш от клиента
        memset(buffer, 0, sizeof(buffer)); // Очищаем буфер
        bytes_received = recv(client_socket, buffer, sizeof(buffer)-1, 0);
        if (bytes_received <= 0) throw server_error("Ошибка получения хеша");
        buffer[bytes_received] = '\0';
        std::string client_hash(buffer);
        
        // 5. Проверяем хеш
        std::string stored_password = user_db.get_password(login);
    if (!CompareHashes(client_hash, salt, stored_password)) {
        throw server_error("Invalid Hash");
    }
        
        // 6. Отправляем подтверждение
        std::string ok_msg = "AUTH_OK";
         if (send(client_socket, ok_msg.c_str(), ok_msg.size(), 0) <= 0) {
            throw server_error("Ошибка отправки подтверждения");
        }
         cout << "[СЕРВЕР] Клиент аутентифицирован, начинаем обмен сообщениями" << endl;
        
        
        // 7. Основной цикл работы
        while (true) {
        	memset(buffer, 0, sizeof(buffer)); // Очищаем буфер перед каждым чтением
        	 	// Получаем длину сообщения
    		uint32_t msg_size;    
    		ssize_t bytes = recv(client_socket, &msg_size, sizeof(msg_size), 0);
    	
    		if (bytes <= 0) {
        		if (bytes == 0) {
            		cout << "[СЕРВЕР] Клиент отключился нормально" << endl;
            		logi.writelog("Клиент отключился нормально");
        		} else {
            		cout << "[СЕРВЕР] Ошибка чтения: " << strerror(errno) << endl;
            		logi.writelog("Ошибка чтения: " + std::string(strerror(errno)));
        		}
        		break;
    		}
        			
        			
    		// Получаем само сообщение
    		std::vector<char> buffer(msg_size);
    		bytes = recv(client_socket, buffer.data(), msg_size, MSG_WAITALL);
    		if (bytes <= 0) {
        		cout << "[СЕРВЕР] Ошибка чтения сообщения" << endl;
        		break;
    		}
				
    		std::string message(buffer.begin(), buffer.end());
    		message.erase(remove(message.begin(), message.end(), '\n'), message.end());
    		
    		if (message.empty()) continue;
            		
            		// Проверка команд выхода
            		if (message == "q" || message == "quit" || message == "exit") {
            			cout << "[СЕРВЕР] Клиент " << client_ip <<" отключение: " << message << endl;
                		logi.writelog("Клиент " + std::string(client_ip) + " запросил отключение: " + message);
                		
                		std::string goodbye = "Сеанс завершен";
                  		uint32_t goodbye_size = goodbye.size();
       			 				// Сначала размер, затем сообщение
    				if (send(client_socket,  &goodbye_size, sizeof(goodbye_size), 0) <= 0 ||
        				send(client_socket, goodbye.c_str(), goodbye_size, 0) <= 0) {
        				logi.writelog("Ошибка отправки подтверждения отключения");
    				}
                		
                		//close(client_socket);   
                		cout << "[СЕРВЕР] Соединение с клиентом " << client_ip <<" закрыто" << endl;
    					logi.writelog("Клиент отключен по команде: " + message);
    					break;
            		}
            		
            		// Логируем только один раз
            		logger.write("[" + std::string(client_ip) + "]: " + message);
            		// Отправляем подтверждение о получении
            		cout << "[КЛИЕНТ]: Сообщение от " << client_ip << " <" << message << ">" << endl;
            		std::string ack = "OK";
            		cout << "[СЕРВЕР]:Отправляю ответ клиенту" << endl;
		
					uint32_t ack_size =  ack.size();
					
					if (send(client_socket, &ack_size, sizeof(ack_size), 0) <= 0 ||
    					send(client_socket, ack.c_str(), ack_size, 0) <= 0) {
                    		logi.writelog("Ошибка отправки подтверждения получения");
                    		break;
                		}
             		} // конец while(true)
        		} // конец else if (command == "AUTH")
        		else {
            		throw server_error("Неизвестная команда");
        		}
    		} catch (const server_error& e) {
        		logi.writelog("Ошибка: " + std::string(e.what()));
    		}
	
    close(client_socket);
    cout << "[СЕРВЕР] Соединение с клиентом " << client_ip <<" закрыто" << endl;
    logi.writelog("Клиент отключен: " + std::string(client_ip));

    // Уменьшаем счетчик активных клиентов
    std::lock_guard<std::mutex> lock(clients_mutex);
    active_clients--;
    cout << "[СЕРВЕР] Активных клиентов: " << active_clients << endl;
}    



