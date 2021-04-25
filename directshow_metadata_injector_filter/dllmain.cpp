// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN  

#include <windows.h>

#include <streams.h>


STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);

}


STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);
}


extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);


BOOL APIENTRY DllMain(HANDLE hModule, DWORD  dwReason, LPVOID lpReserved)
{
    return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}
