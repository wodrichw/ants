#pragma once

#include <string>

// trim from start (in place)
inline void ltrim(std::string &s);

// trim from end (in place)
inline void rtrim(std::string &s);

// trim from both ends (in place)
inline void trim(std::string &s);

inline long trimmed_size(std::string const& s);

inline bool trimmed_empty(std::string const& s);

// trim from start (copying)
inline std::string ltrim_copy(std::string s);

// trim from end (copying)
inline std::string rtrim_copy(std::string s);

// trim from both ends (copying)
std::string trim_copy(std::string s);
