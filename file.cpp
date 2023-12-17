#include "file.h"

#include <sstream>

// ================================================================
// | Free Functions                                               |
// ================================================================

std::string double_to_pretty_string(double value)
{
    std::stringstream ss;
    int front_numbers = 0;

    for (double temp = value; temp > 1.0; temp *= 0.1)
        ++front_numbers;

    ss << std::setprecision(front_numbers + 2) << value;

    return ss.str();
}

std::pair<double, QString> process_bytes(size_t bytes)
{
    static const double gib = 1024.0 * 1024.0 * 1024.0;
    static const double mib = 1024.0 * 1024.0;
    static const double kib = 1024.0;
    double converted_bytes = static_cast<double>(bytes);

    if (converted_bytes > gib) return { converted_bytes / gib, "GiB" };
    else if (converted_bytes > mib) return { converted_bytes / mib, "MiB" };
    else if (converted_bytes > kib) return { converted_bytes / kib, "KiB" };
    return { converted_bytes, "B" };
}

// ================================================================
// | File Class - Constructors                                    |
// ================================================================

File::File(std::filesystem::path arg_path, size_t arg_size, bool arg_removable) :
    path(arg_path),
    size(arg_size),
    removable(arg_removable) {}

// ================================================================
// | File Class - Public                                          |
// ================================================================

const std::filesystem::path& File::get_path() const
{
    return path;
}

size_t File::get_size() const
{
    return size;
}

bool File::is_removable() const
{
    return removable;
}

std::pair<size_t, QString> File::process_size()
{
    return process_bytes(size);
}

void File::set_removable(bool arg)
{
    removable = arg;
}

// ================================================================
// | File Class - Friend Functions                                |
// ================================================================

bool operator<(const File& a, const File& b)
{
    if (a.size == b.size)
        return a.path.filename().stem() < b.path.filename().stem();
    return a.size < b.size;
}
