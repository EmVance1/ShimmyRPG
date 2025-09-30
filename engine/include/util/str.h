#include <vector>
#include <string>
#include <filesystem>


namespace shmy { namespace str {

std::string read_to_string(const std::filesystem::path& filename);
void write_to_file(const std::filesystem::path& filename, const std::string& content);

std::vector<std::string> split(const std::string& str, char delimiter);
std::vector<std::string> tok(const std::string& str, char delimiter);
std::string trim(const std::string& str);

} }
