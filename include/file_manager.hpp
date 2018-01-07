#ifndef FILE_MANANGER_HPP
#define FILE_MANANGER_HPP

#include <map>
#include <vector>
#include <string>

struct FileInfo {
	std::string filename;
	std::string path;
	int size;
};

class FileManager
{
public:
	FileManager();
	void add_folder(const std::string);

	std::string get_file_list();
	std::string get_folder_list();
	int get_file(std::string, unsigned char**);
	size_t save_file(std::string, int, unsigned char*);
	bool has_file(std::string&);
	size_t get_file_size(std::string);

private:
	void update_file_index();

	std::map<std::string, std::string> file_index;
	std::vector<std::string> folders;
};

#endif //FILE_MANANGER_HPP
