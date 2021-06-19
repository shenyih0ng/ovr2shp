#include <iostream>
#include <sstream>


using namespace std;

enum logtype { INFO, WARN, ERROR };

extern string CURRSRC;

inline const char *logtypetoStr(logtype lt) {
    switch (lt) {
    case INFO:
        return "[INFO]";
    case WARN:
        return "[WARN]";
    case ERROR:
        return "[ERROR]";
    default:
        return "[LOG]";
    };

    return "";
}

class Log {
    ostringstream os;

  public:
    Log(){};

    Log(logtype lt) {
        os << logtypetoStr(lt);
        os << " ";
        if (!CURRSRC.empty()) {
            os << "(" << CURRSRC << ") ";
        }
    }

    ~Log() { cout << os.str().c_str() << endl; };

    template <typename T> Log &operator<<(T msg) {
        os << msg;
        return *this;
    }
};
