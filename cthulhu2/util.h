#pragma once

#include <vector>
#include <string>

template<typename T, typename E>
struct Result {
    union Data {
        Data(T value) : value(value) { }
        Data(E error) : error(error) { }
        ~Data() { }

        T value;
        E error;
    };

    Result(T value) : ok(true), data(value) { }
    Result(E error) : ok(false), data(error) { }
    ~Result() { if (ok) data.value.~T(); else data.error.~E(); }

    operator bool() const { return ok; }
    bool valid() const { return ok; }

    T& value() const { return data.value; }
    E& error() const { return data.error; }

private:
    bool ok;
    Data data;
};

std::vector<std::string> split(const std::string& str, const std::string& sep);
std::string join(const std::vector<std::string>& strings, const std::string& sep);
std::string trim(const std::string& str, const std::string& delim);
