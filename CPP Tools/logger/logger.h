#include <iostream>
#include <string>

using namespace std;

class Logger {
public:

    explicit Logger(ostream &out, const string &delim="[%]") : m_out(&out), m_delim(delim) {}

    template<typename H, typename... Args>
    void print(const string &format, H head, Args... tail) {
        auto newFormat = doPrint(format, head);
        if (newFormat.length() > 0) {
            print(newFormat, tail...);
        }
    }

private:
    ostream *m_out;
    const string m_delim;

    template<typename T>
    string doPrint(const string &format, T arg) {
        int idx = format.find(m_delim);
        string substr;
        string newFormat;
        if (idx != string::npos) {
            substr = format.substr(0, idx);
            newFormat = format.substr(idx + m_delim.length(), format.length());
        } else {
            substr = format;
            newFormat = "";
        }

        *m_out << substr;
        if (idx != string::npos) {
            *m_out << arg;
        }

        return newFormat;
    }

    template<typename T>
    void print(const string &format, T nextArg) {
        auto newFormat = doPrint(format, nextArg);
        *m_out << newFormat;
    }
};
