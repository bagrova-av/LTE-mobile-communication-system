#include "JsonSerializer.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

BytesBuffer JsonSerializer::serialize(const LocationPoint3D& point) const
{
    json dataJSON;
    dataJSON["x"] = point.x;
    dataJSON["y"] = point.y;
    dataJSON["z"] = point.z;

    std::string serializedData = dataJSON.dump();
    return BytesBuffer(serializedData.begin(), serializedData.end());
}

std::optional<LocationPoint3D> JsonSerializer::deserialize(const BytesBuffer& dataNetworkFormat) const
{
    json dataJSON = json::parse(std::string(dataNetworkFormat.begin(), dataNetworkFormat.end()));
    
    return LocationPoint3D{ dataJSON["x"],
                            dataJSON["y"],
                            dataJSON["z"]
    };
}