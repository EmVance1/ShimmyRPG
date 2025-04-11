#include <vector>
#include <string>


std::string read_to_string(const std::string& filename);
void write_to_file(const std::string& filename, const std::string& content);

std::vector<std::string> split_string(const std::string& str, char delimiter);
std::string trim_string(const std::string& str);

