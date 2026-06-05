#include "CommandMove.h"

#include <cstring>

bool CommandMove::execute(const std::vector<std::string>& args)
{
    size_t countComponents = args.size();
    if (countComponents > LocationPoint3D::COUNT_COMPONENTS_IN_POINT || args.empty())
    {
        Logger::warning("Error: the MOVE command takes 1 to " 
                        + std::to_string(LocationPoint3D::COUNT_COMPONENTS_IN_POINT) 
                        + " arguments (components of the new location)");
        return false;
    }

    std::vector<CoordinatesType> numericComponents;
    numericComponents.reserve(countComponents);
    for (const auto& arg : args)
    {
        auto component = convertFromString<CoordinatesType>(arg);
        if (!component.has_value())
        {
            Logger::warning("Validation error: a location coordinate '" + arg + "' contains non-numeric characters");
            return false;
        }
        numericComponents.push_back(component.value());
    }
    
    auto [newLocation, flags] = LocationPoint3D::createUpdateFromVector(numericComponents);

    LocationPoint3D currentLocation = app.getLocation();
    LocationPoint3D tempLocation = currentLocation;
    tempLocation.updateWithFlags(newLocation, flags);
    if (tempLocation == currentLocation)
    {
        Logger::info("Location is already " + convertLocationToString(tempLocation) + ". No changes made.");
    }
    else
    {
        app.setLocation(newLocation, flags);
        Logger::info("Location updated from " + convertLocationToString(currentLocation) + " to " + convertLocationToString(app.getLocation()));
    }

    app.sendLocationToServerAsync(app.getLocation(), app.getProtocol());
    return true;
}