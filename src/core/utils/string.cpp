
#include "utils/string.hpp"

#include <algorithm>
#include <cctype>
#include <locale>

// trim from start (in place)
inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
}

// trim from end (in place)
inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
            s.end());
}

// trim from both ends (in place)
inline void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}

inline long trimmed_size(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end = s.find_last_not_of(" \t\n\r");

    if(start == std::string::npos || end == std::string::npos) {
        return 0;  // String consists entirely of whitespace
    }

    return end - start + 1;
}

inline bool trimmed_empty(std::string const &s) {
    for(char ch : s) {
        if(!std::isspace(ch)) {
            return false;
        }
    }
    return true;
}

// trim from start (copying)
inline std::string ltrim_copy(std::string s) {
    ltrim(s);
    return s;
}

// trim from end (copying)
inline std::string rtrim_copy(std::string s) {
    rtrim(s);
    return s;
}

// trim from both ends (copying)
std::string trim_copy(std::string s) {
    trim(s);
    return s;
}
