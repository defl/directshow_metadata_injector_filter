#pragma once

#include <wtypes.h>

double GetPrivateProfileDouble(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    double default,
    LPCWSTR lpFileName);
