#pragma once

#include <string>

enum class OperatingMode
{
    LSB,
    USB,
    FM,
    OTHER // Unsupported modes
};

inline const std::string OperatingModeToString(OperatingMode mode)
{
    switch (mode)
    {
    case OperatingMode::LSB:
        return "LSB";
    case OperatingMode::USB:
        return "USB";
    case OperatingMode::FM:
        return "FM";
    case OperatingMode::OTHER:
        return "OTHER";
    default:
        return "UNKNOWN";
    }
}

inline const OperatingMode StringToOperatingMode(std::string mode)
{
    if (mode == "FM")
    {
        return OperatingMode::FM;
    }
    if (mode == "LSB")
    {
        return OperatingMode::LSB;
    }
    if (mode == "USB")
    {
        return OperatingMode::USB;
    }
    return OperatingMode::OTHER;
}