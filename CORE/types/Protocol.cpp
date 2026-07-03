#include "Protocol.h"

std::ostream& operator<<(std::ostream& out, const Protocol& protocol)
{
    switch (protocol)
    {
        case Protocol::BINARY:
            out << "BINARY";
            break;
        case Protocol::JSON:
            out << "JSON";
            break;
        default:
            out << "UNKNOWN";
    }
    return out;
}
