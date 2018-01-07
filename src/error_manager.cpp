#include <netdb.h>
#include <stdexcept>
#include <cstring>

#include <error_manager.hpp>

using namespace std;

void throw_error(const string &func_name, int error_code) {
	string error_msg = "";
	if (func_name == "getaddrinfo") {
		error_msg = gai_strerror(error_code);
	}
	else {
		error_msg = strerror(error_code);
	}

	char error_buffer[256];
	string error_format = "%s(): %s (error code: %d)";
	snprintf(error_buffer, sizeof(error_buffer), 
		error_format.c_str(), 
			func_name.c_str(), 
			error_msg.c_str(), 
			error_code);

	throw runtime_error(error_buffer);
}
