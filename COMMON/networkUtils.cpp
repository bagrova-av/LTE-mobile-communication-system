#include "networkUtils.h"

void writeString(BytesBuffer& buffer, const std::string& str, size_t& offset)
{
    uint32_t len = static_cast<uint32_t>(str.length());
    writeValue(buffer, len, offset);
    std::memcpy(buffer.data() + offset, str.data(), len);
    offset += len;
}

std::optional<std::string> readString(const BytesBuffer& buffer, size_t& offset)
{
    uint32_t networkLen;
    size_t networkLenSize = sizeof(networkLen);
    if (offset + networkLenSize > buffer.size())
    {
        return std::nullopt;
    }

    std::memcpy(&networkLen, buffer.data() + offset, networkLenSize);
    uint32_t len = convertNetworkToHostOrder<uint32_t>(networkLen);
    offset += networkLenSize;

    if (offset + len > buffer.size())
    {
        return std::nullopt;
    }

    std::string str(buffer.begin() + offset, buffer.begin() + offset + len);
    offset += len;
    return str;
}