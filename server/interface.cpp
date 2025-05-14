#include <iostream>
#include <cstdlib> 
#include <unistd.h> 
#include <string>
#include <fstream>
#include <filesystem>
#include <optional>
#include <unordered_map>
#include "interface.h"
#include "programmerror.h"
#include "communicator.h"
#include "logger.h"

int interface::Opts(int argc, char **argv)
{
    // Создаем unordered_map для хранения аргументов командной строки
    std::unordered_map<char, std::string> args;
    int opt;
    while ((opt = getopt(argc, argv, "b:l:p:i:h")) != -1) {
        switch (opt) {
        case 'b':
        case 'l':
        case 'p':
        case 'i':
            args[opt] = std::string(optarg);
            break;
        case 'h': 
        case '?': 
        case ':': 
            usage(argv[0]);
        }
    }

    if(args.count('b')) {
        DataBaseName = args['b'];
    }
    if(args.count('l')) {
        LogFileName = args['l'];
    }
    if(args.count('p')) {
        Port = std::stoi(args['p']);
        if ((Port < 0) or (Port > 65535)){
        	usage(argv[0]);
        }
    }
     if(args.count('i')) {
        ServerAddress = args['i'];
    }
    Logger logger;
    
    try{
    	if(DataBaseName.find('.')==std::string::npos){
			throw server_error("invalid_base_path", true);
		}
		if(LogFileName.find('.')==std::string::npos){
			throw server_error("invalid_log_path", true);
		}
    	communicator main_manager;
    	logger.set_path(LogFileName);
    	DB new_db(DataBaseName);
    	main_manager.conversation(ServerAddress, Port, new_db);
    	       
	} catch (const server_error & e) {
		std::stringstream ss;
    	ss << "Error: " << e.what() << ", State: " << e.getState();
    	logger.writelog(ss.str());
          if (e.getState() == "Критическая"){
			exit(1);
		}
    }
    return 0;
}

void interface::usage(const char* progName)
{
    std::cout<<"Использование: "<<progName<<" [-b DataBaseName] [-n LogFileName] [-p Port(0;65535)] [-i Adress(=127.0.01)] \n";
    exit(1);
}



