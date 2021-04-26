#include <string>

#include "utils.h"

double GetPrivateProfileDouble(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    double default,
    LPCWSTR lpFileName)
{
    TCHAR buffer[32];

    // Gets a profile string called "Preferred line" and converts it to an int.
    GetPrivateProfileString(
        lpAppName,
        lpKeyName,
        L"0",
        buffer,
        sizeof(buffer),
        lpFileName
    );

    return std::stod(buffer);
}
