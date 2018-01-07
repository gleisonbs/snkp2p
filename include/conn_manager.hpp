#ifndef CONN_MANAGER_HPP
#define CONN_MANAGER_HPP

#include <queue>
#include <string>
#include <thread>
#include <vector>

#define BACKLOG 50

class ConnectionManager
{
public:
	ConnectionManager();
	~ConnectionManager();
	int start_server(std::string);
 	int connect_to_peer(std::string, std::string); // return socket to peer or -1 in case of failure
 	void connect_to_rendezvouz_server(std::string, std::string, std::string, std::string, std::string);
	void receive_peer_list();
 	void finish();

// 	std::string receive_data(int);
	void send_data(int, std::string);
	std::string get_external_ip();
	std::string list_peers();

	std::vector<int> peers;
	std::queue<std::string> requests;

private:
 	int connect_to(std::string, std::string); // return socket to peer or -1 in case of failure
	void accept_new_peer();
	void answer_alive(int);
// 	void connect_to_peer(std::string, int);
// 	void file_sender(int, std::string);
	void read_data(int);
// 	void receive_file(std::string, int, int);
// 	void send_file(int, std::string);

	int rendezvouz_server_sock = -1;
	int local_sock;
// 	int server_port;
// 	int upload_port = 5005;

// 	//std::thread reader;
	std::vector<std::thread> reader_threads;
// 	std::vector<std::thread> downloads;
// 	std::vector<std::thread> uploads;

	std::thread acceptor;
	std::thread alive_keeper;

 	bool should_close = false;
	// bool keep_accepting = true;
};

#endif //CONN_MANAGER_HPP
