#include "Point3D.h"

std::ostream& operator<<(std::ostream& out, const LocationPoint3D& point)
{
    return ( out << std::fixed << std::setprecision(4)
                 << "(" << point.x << ";" << point.y << ";" << point.z << ")" );
}