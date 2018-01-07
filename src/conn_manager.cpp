// #include <fstream>
#include <iostream>
#include <regex>
// #include <netinet/in.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <netdb.h>
// #include <sys/types.h>

#include <conn_manager.hpp>
#include <parser.hpp>

using namespace std;

ConnectionManager::ConnectionManager() {}

ConnectionManager::~ConnectionManager() {
// 	acceptor.join();
	for(auto &reader : reader_threads)
		reader.join();
// 	for(auto &download : downloads)
// 		download.join();
// 	for(auto &upload : uploads)
// 		upload.join();

	close(local_sock);
}

int ConnectionManager::start_server(string port) {

	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *result, *rp;
	int s = getaddrinfo(NULL, port.c_str(), &hints, &result);
	if(s != 0) return -1;

	int optval = 1;
	int sock;
	for(rp = result; rp != NULL; rp = rp->ai_next) {
		sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(sock == -1) continue;

		s = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
		if(s == -1) {
			close(sock);
			freeaddrinfo(result);
			return -1;
		}

		if(bind(sock, rp->ai_addr, rp->ai_addrlen) == 0) break;

		close(sock);
	}

	s = listen(sock, BACKLOG);
	if(s == -1) {
		freeaddrinfo(result);
		return -1;
	}

	freeaddrinfo(result);

	if(rp != NULL) {
		acceptor = thread(&ConnectionManager::accept_new_peer, this);
		local_sock = sock;
		return sock;
	}

	return -1;
}

void ConnectionManager::connect_to_rendezvouz_server(string server_host,
                                                    string server_port,
                                                    string local_id,
                                                    string local_host,
                                                    string local_port) {
    cout << "(" << server_host << ":" << server_port << ")" << endl;
	while(rendezvouz_server_sock == -1) {
		rendezvouz_server_sock = connect_to(server_host, server_port);
		if(rendezvouz_server_sock == -1) {
			cout << "Rendezvouz server is unrecheable..." << endl;
			sleep(10);
		}
		else {
            break;
        }

    }

	string adv = local_id + ":" + local_host + ":" + local_port;
	send_data(rendezvouz_server_sock, adv);
	receive_peer_list();

	//alive_keeper = thread(&ConnectionManager::answer_alive, this, rendezvouz_server_sock);

}

int ConnectionManager::connect_to_peer(string host, string port) {
	int peer = connect_to(host, port);

	peers.push_back(peer);
	thread reader = thread(&ConnectionManager::read_data, this, peer);
	reader_threads.push_back(move(reader));
}

int ConnectionManager::connect_to(string host, string port) {
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	struct addrinfo *result, *rp;
	getaddrinfo(host.c_str(), port.c_str(), &hints, &result);

	int sock;
	for(rp = result; rp != NULL; rp = rp->ai_next) {
		sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

		if(connect(sock, rp->ai_addr, rp->ai_addrlen) != -1) break; // SUCCESS

		close(sock);
	}

	freeaddrinfo(result);

	return (rp == NULL) ? -1 : sock;
}

void ConnectionManager::receive_peer_list() {
	char buffer[1024];
	memset(buffer, 0, 1024);
	int bytes_read = -1;
	while(bytes_read == -1) {
		bytes_read = recv(rendezvouz_server_sock, buffer, 1024, 0);
		sleep(1);
	}

	string buffer_str = buffer;

	if(buffer_str == "no_peers") return;

	vector<string> peer_list;
    Parser parser;
    peer_list = parser.parse(buffer_str, '|');
	//boost::split(peer_list, buffer_str, boost::is_any_of("|"));
	for(auto &p : peer_list) {
		vector<string> peer_detail;
        peer_detail = parser.parse(p, ':');
		//boost::split(peer_detail, p, boost::is_any_of(":"));
		connect_to_peer(peer_detail[1], peer_detail[2]);
	}
}

void ConnectionManager::accept_new_peer() {
	while(not should_close) {
		struct sockaddr_in peer_addr;
		socklen_t peer_addr_len;
		peer_addr_len = sizeof(peer_addr);

		int peer_sock = accept(local_sock, (struct sockaddr*)&peer_addr, &peer_addr_len);
		if(peer_sock > 0) {
			cout << "New peer connected: " << peer_sock << endl;
			string msg = "HELLO, PEER!\n";
			write(peer_sock, msg.c_str(), msg.size());

			peers.push_back(peer_sock);
			thread reader = thread(&ConnectionManager::read_data, this, peer_sock);
			reader_threads.push_back(move(reader));
		}
	}
}

// void ConnectionManager::file_sender(int port, string filename) {
// 	int send_file_sock = socket(AF_INET, SOCK_STREAM, 0);
// 	int opt = 1;
// 	setsockopt(send_file_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

// 	struct sockaddr_in local_addr;
// 	local_addr.sin_family = AF_INET;
// 	local_addr.sin_addr.s_addr = INADDR_ANY;
// 	local_addr.sin_port = htons(port);

// 	bind(send_file_sock, (struct sockaddr*)&local_addr, sizeof(local_addr));
// 	listen(send_file_sock, 5);

// 	struct sockaddr_in peer_addr;
// 	socklen_t peer_addr_len;
// 	peer_addr_len = sizeof(peer_addr);

// 	int peer_sock = accept(send_file_sock, (struct sockaddr*)&peer_addr, &peer_addr_len);

// 	if(peer_sock > 0) {

// 		unsigned char *file;
// 		int file_size = fm.get_file(filename, &file);

// 		int bytes_writen = write(peer_sock, file, file_size);
// 		cout << "bytes_writen: " << bytes_writen << endl;
// 	}

// 	close(peer_sock);
// 	close(send_file_sock);
// }

// void ConnectionManager::connect_to_peer(string host, int port) {

// 	int peer_sock = socket(AF_INET, SOCK_STREAM, 0);

// 	struct hostent *server;
// 	server = gethostbyname(host.c_str());

// 	struct sockaddr_in serv_addr;
// 	bzero((char *) &serv_addr, sizeof(serv_addr));
// 	serv_addr.sin_family = AF_INET;

// 	bcopy((char *)server->h_addr,
// 		(char *)&serv_addr.sin_addr.s_addr,
// 		server->h_length);

// 	serv_addr.sin_port = htons(port);
// 	connect(peer_sock,(struct sockaddr *) &serv_addr,sizeof(serv_addr));

// 	peers.push_back(peer_sock);
// 	thread reader = thread(&ConnectionManager::read_data, this, peer_sock);
// 	reader_threads.push_back(move(reader));
// }

void ConnectionManager::read_data(int peer) {
	fcntl(peer, F_SETFL, fcntl(peer, F_GETFL) | O_NONBLOCK);
	char buffer[256];
	while(not should_close) {
		memset(buffer, 0, 256);
		int bytes_read = read(peer, buffer, 256);

		if(bytes_read > 0) {
			string request = buffer;
			//handle_peer_request(peer, msg);
			requests.push(request);
		}
		sleep(1);
	}
}

// void ConnectionManager::receive_file(string filename, int file_size, int port) {
// 	int peer_sock = socket(AF_INET, SOCK_STREAM, 0);

// 	struct hostent *server;
// 	server = gethostbyname("localhost");

// 	struct sockaddr_in serv_addr;
// 	bzero((char *) &serv_addr, sizeof(serv_addr));
// 	serv_addr.sin_family = AF_INET;

// 	bcopy((char *)server->h_addr,
// 		(char *)&serv_addr.sin_addr.s_addr,
// 		server->h_length);

// 	serv_addr.sin_port = htons(port);
// 	connect(peer_sock,(struct sockaddr *) &serv_addr,sizeof(serv_addr));

// 	unsigned char *buffer = (unsigned char*)malloc(file_size);
// 	//unsigned char buffer[512];// = (unsigned char*)malloc(file_size);
// 	cout << file_size << endl;
// 	int bytes_read = 0;
// 	while(bytes_read < file_size) {
// 		//memset(buffer, 0, 512);
// 		bytes_read += recv(peer_sock, buffer, file_size, MSG_WAITALL);
// 		cout << "bytes_read: " << bytes_read << endl;
// 	}
// 	cout << filename << ": " << bytes_read << " bytes" << endl;
// 	//fm.save_file(filename, bytes_read, buffer);
// 	free(buffer);
// 	close(peer_sock);
// }

// string ConnectionManager::get_peer_list() {
// 	string peers_str {};
// 	for(auto &peer : peers)
// 		peers_str += to_string(peer) + "\n";
// 	return peers_str;
// }



void ConnectionManager::answer_alive(int sock) {
	while(not should_close) {
		char answer[4];
		memset(answer, 0, 4);
		recv(sock, answer, 4, MSG_DONTWAIT);

		string answer_str = answer;
		if(answer_str == "PONG") {
			cout << answer << endl;

			string ping = "PING";
			send_data(sock, ping);
		}
	}
}

void ConnectionManager::send_data(int peer, string data) {
    cout << "send_data: " << data << endl;
	send(peer, data.c_str(), data.size(), 0);
}

string ConnectionManager::get_external_ip() {
	string host = "canihazip.com";
	string port = "80";
	int peer = connect_to(host, port);

	string http_get =
		"GET /s HTTP/1.1\r\n"
		"Host: canihazip.com\r\n"
		"User-Agent: snkp2p/0.1\r\n"
		"Accept: */*\r\n\r\n";
	send_data(peer, http_get);

	char buffer[512];
	recv(peer, buffer, 512, 0);
	string ip = buffer;

	regex rg_exp("(?:[0-9]{1,3}\\.){3}[0-9]{1,3}");

	auto match_beg = sregex_iterator(ip.begin(), ip.end(), rg_exp);
	auto match_end = sregex_iterator();

	for(auto i = match_beg; i != match_end; ++i) {
		smatch match = *i;
		return match.str();
	}
	return "";
}

string ConnectionManager::list_peers() {
	string peers_str {};
	for(auto &peer : peers)
		peers_str += to_string(peer) + "\n";
	return peers_str;
}

void ConnectionManager::finish() {
	should_close = true;
}
