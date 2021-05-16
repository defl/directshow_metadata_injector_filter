#pragma once

#include <strsafe.h>
#include <string>

#include "metadata_injector_interface.h"


class MetadataInjectorProperties:
    public CBasePropertyPage
{

public:

    static CUnknown* WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT* phr);

private:

    INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    HRESULT OnConnect(IUnknown* pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();

    void    GetControlValues();

    MetadataInjectorProperties(LPUNKNOWN lpunk, HRESULT* phr);

    BOOL mIsInitialized;
    std::string mConfigFilename;
    MetadataInjectorInterface* mDMIFInterface;

};
