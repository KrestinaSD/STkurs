#include "logger.h"

using namespace std;


void Logger::set_path(std::string LogName) {
	std::ofstream log(LogName, std::ios_base::app);
	if(!log.good()){
			throw server_error("Wrong log File Name");
	}
	log.close();
    path_to_logfile = LogName;
}
    
std::string Logger::getDateTime() {
    time_t now = time(0);
    struct tm  tstruct;
    char  buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%d.%m.%Y %X", &tstruct);
    return std::string(buf);
}

int Logger::writelog(std::string s) {
	if (s.empty()) {
        throw server_error("Empty message");
    }
    std::ofstream filelog;
    filelog.open(path_to_logfile, std::ios_base::out | std::ios_base::app);
    if(filelog.is_open()) {
        std::string time = getDateTime();
        filelog << time << ' ' << s << std::endl;
        filelog.close();
        return 0;
    } else {
    	filelog.close();
        return 1;
    }
}
