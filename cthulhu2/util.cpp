#include "util.h"

std::vector<std::string> split(const std::string& str, const std::string& sep) {
    std::string temp = str;
    std::vector<std::string> out;
    size_t pos = 0;

    while ((pos = temp.find(sep)) != std::string::npos) {
        out.push_back(temp.substr(0, pos));
        temp.erase(0, pos + sep.length());
    }
    out.push_back(temp);

    return out;
}

std::string join(const std::vector<std::string>& strings, const std::string& sep) {
    std::string out;

    for (size_t i = 0; i < strings.size(); i++) {
        if (i != 0) {
            out += sep;
        }
        out += strings[i];
    }

    return out;
}

std::string trim(const std::string& str, const std::string& delim) {
    std::string out = str;
    while (out.rfind(delim, 0) == 0) {
        out.erase(0, delim.length());
    }
    return out;
}
