#include <iostream>

#include <file_manager.hpp>
#include <p2p.hpp>

using std::cout;

void print_usage() {
    cout << "\nUsage:\n./program port folder\n\n";
}

int main(int argc, char* argv[]){

    if (argc < 3) print_usage();

	P2P p2p(atoi(argv[1]));
	p2p.add_folder(argv[2]);
	p2p.run();

}
