#include <connection.hpp>
#include <error_manager.hpp>

using namespace std;

Connection::Connection() {};
void Connection::listen_at(const string &port) {
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *result, *rp;
	int err = getaddrinfo(NULL, port.c_str(), &hints, &result);
	if (err != 0) {
		throw_error("getaddrinfo", err);
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sock == -1) continue;

		int optval = 1;
		err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
		if (err == -1) {
			close(sock);
			freeaddrinfo(result)
			throw_error("setsockopt", errno)
		}

		if (bind(sock, rp->ai_addr, rp->ai_addrlen) == 0)
			break;

		close(sock); // if arrives here, bind() failed
	}

	err = listen(sock, this.BACKLOG);
	freeaddrinfo(result);
	if (err == -1) {
		throw_error("listen", errno);
	}
}
