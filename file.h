#ifndef FILE_H
#define FILE_H

#include <filesystem>
#include <utility>

#include <QString>

std::string double_to_pretty_string(double value);
std::pair<double, QString> process_bytes(size_t bytes);

class File
{
private:

    std::filesystem::path path;
    size_t size = 0;
    bool removable = false;

public:

    File() = default;
    File(std::filesystem::path arg_path, size_t arg_size, bool arg_removable);

    const std::filesystem::path& get_path() const;
    size_t get_size() const;
    bool is_removable() const;

    void set_removable(bool arg);

    std::pair<size_t, QString> process_size();

    friend bool operator<(const File& a, const File& b);

    ~File() = default;

};

#endif // FILE_H
