#include <windows.h>
#include <process.h>
#include <streams.h>
#include <initguid.h>
#include <olectl.h>
#include <dvdmedia.h>
#include <dxva.h>
#include <stdexcept>
#include <vector>
#include <string>

#include "guids.h"
#include "lavfilters_side_data.h"
#include "utils.h"

#include "metadata_injector_filter.h"
#include "metadata_injector_properties.h"


const AMOVIESETUP_MEDIATYPE sMIPinTypes =
{
    &MEDIATYPE_Video,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};


const AMOVIESETUP_PIN sMIPPins[] =
{
    { L"Input",     // Pins string name
      FALSE,        // Is it rendered
      FALSE,        // Is it an output
      FALSE,        // Are we allowed none
      FALSE,        // And allowed many
      &CLSID_NULL,  // Connects to filter
      NULL,         // Connects to pin
      1,            // Number of types
      &sMIPinTypes  // Pin information
    },
    { L"Output",    // Pins string name
      FALSE,        // Is it rendered
      TRUE,         // Is it an output
      FALSE,        // Are we allowed none
      FALSE,        // And allowed many
      &CLSID_NULL,  // Connects to filter
      NULL,         // Connects to pin
      1,            // Number of types
      &sMIPinTypes  // Pin information
    }
};


const AMOVIESETUP_FILTER sMIPSetup =
{
    &CLSID_METADATA_INJECTOR_FILTER,  // Filter CLSID
    L"Metdata Injector",              // String name
    MERIT_DO_NOT_USE,                 // Filter merit
    2,                                // Number of pins
    sMIPPins                          // Pin information
};


// List of class IDs and creator functions for the class factory.
CFactoryTemplate g_Templates[] = {
    { L"Metadata Injector",
      &CLSID_METADATA_INJECTOR_FILTER,
      MetadataInjectorFilter::CreateInstance,
      NULL,
      &sMIPSetup
    },
    {
      L"Metadata Injector Properties",
      &CLSID_METADATA_INJECTOR_PROPERTIES,
      MetadataInjectorProperties::CreateInstance
    }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


CUnknown* MetadataInjectorFilter::CreateInstance(LPUNKNOWN punk, HRESULT* phr)
{
    MetadataInjectorFilter* pNewObject = new MetadataInjectorFilter(punk, phr);

    if (pNewObject == NULL)
    {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
}


STDMETHODIMP MetadataInjectorFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    if (riid == CLSID_METADATA_INJECTOR_INTERFACE) {
        return GetInterface((MetadataInjectorInterface*)this, ppv);

    }
    else if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages*)this, ppv);

    }
    else {
        return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}

HRESULT MetadataInjectorFilter::Transform(IMediaSample* pIn, IMediaSample* pOut)
{
    CheckPointer(pIn, E_POINTER);
    CheckPointer(pOut, E_POINTER);

    HRESULT hr;

    // Copy frame data
    hr = Copy(pIn, pOut);
    if (FAILED(hr))
        return hr;

    // Update once per second
    if ((mFrameCounter % mFramesPerSecond) == 0)
    {
        // This can fail if you have a filter behind this which does not understand side data (ie, it's not going directly to madVR)
        IMediaSideData* pMediaSideData = nullptr;
        if (FAILED(hr = pOut->QueryInterface(&pMediaSideData)))
            return hr;

        MediaSideDataHDR hdr;
        ZeroMemory(&hdr, sizeof(hdr));
        hdr.display_primaries_x[0] = mDisplayPrimaryGreenX;
        hdr.display_primaries_x[1] = mDisplayPrimaryBlueX;
        hdr.display_primaries_x[2] = mDisplayPrimaryRedX;
        hdr.display_primaries_y[0] = mDisplayPrimaryGreenY;
        hdr.display_primaries_y[1] = mDisplayPrimaryBlueY;
        hdr.display_primaries_y[2] = mDisplayPrimaryRedY;
        hdr.white_point_x = mWhitePointX;
        hdr.white_point_y = mWhitePointY;
        hdr.max_display_mastering_luminance = mMasteringLuminanceMax;
        hdr.min_display_mastering_luminance = mMasteringLuminanceMin;
        pMediaSideData->SetSideData(IID_MediaSideDataHDR, (const BYTE*)&hdr, sizeof(hdr));

        MediaSideDataHDRContentLightLevel hdrLightLevel;
        ZeroMemory(&hdrLightLevel, sizeof(hdrLightLevel));
        hdrLightLevel.MaxCLL = mMaxCLL;
        hdrLightLevel.MaxFALL = mMaxFALL;
        pMediaSideData->SetSideData(IID_MediaSideDataHDRContentLightLevel, (const BYTE*)&hdrLightLevel, sizeof(hdrLightLevel));

        pMediaSideData->Release();
    }

    ++mFrameCounter;
    return NOERROR;
}


MetadataInjectorFilter::MetadataInjectorFilter(LPUNKNOWN punk, HRESULT* phr) :
    CTransformFilter(L"Metdata Injector", punk, CLSID_METADATA_INJECTOR_FILTER),
    CPersistStream(punk, phr)
{
    // Default path is right next to DLL
    TCHAR path[1024];
    HMODULE hm = NULL;

    if (GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCWSTR)&MetadataInjectorFilter::CreateInstance, &hm) == 0)
    {
        throw std::exception("GetModuleHandle failed");
    }
    if (GetModuleFileName(hm, path, sizeof(path)) == 0)
    {
        throw std::exception("GetModuleFileName failed");
    }

    // TODO: Improve
    std::vector<char> buffer;
    int size = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
    if (size > 0)
    {
        buffer.resize(size);
        WideCharToMultiByte(CP_UTF8, 0, path, -1, &buffer[0], buffer.size(), NULL, NULL);
    }
    mConfigFilename = std::string(&buffer[0]);
    std::string::size_type pos = mConfigFilename.find_last_of("\\/");
    mConfigFilename = mConfigFilename.substr(0, pos);
    mConfigFilename += "\\directshow_metadata_injector_filter.ini";

    LoadConfig();
    //LPCTSTR path = L"C:\\users\\User\\example_config.ini";
}


MetadataInjectorFilter::~MetadataInjectorFilter()
{
}


void MetadataInjectorFilter::LoadConfig()
{
    const std::wstring stemp = std::wstring(mConfigFilename.begin(), mConfigFilename.end());

    // Defaults are P3-D65 (https://en.wikipedia.org/wiki/DCI-P3#System_colorimetry)
    mDisplayPrimaryGreenX = GetPrivateProfileDouble(L"directshow_metadata_injector_filter", L"display_primary_green_x", 0.265, stemp.c_str());
    mDisplayPrimaryGreenY = GetPrivateProfileDouble(L"directshow_metadata_injector_filter", L"display_primary_green_y", 0.690, stemp.c_str());
    mDisplayPrimaryBlueX = GetPrivateProfileDouble(L"directshow_metadata_injector_filter", L"display_primary_blue_x", 0.150, stemp.c_str());
    mDisplayPrimaryBlueY = GetPrivateProfileDouble(L"directshow_metadata_injector_filter", L"display_primary_blue_y", 0.060, stemp.c_str());
    mDisplayPrimaryRedX = GetPrivateProfileDouble(L"directshow_metadata_injector_filter", L"display_primary_red_x", 0.680, stemp.c_str());
    mDisplayPrimaryRedY = GetPrivateProfileDouble(L"directshow_metadata_injector_filter", L"display_primary_red_y", 0.320, stemp.c_str());

    // Defaults are P3-D65 (https://en.wikipedia.org/wiki/DCI-P3#System_colorimetry)
    mWhitePointX = GetPrivateProfileDouble(L"directshow_metadata_injector_filter", L"white_point_x", 0.31271, stemp.c_str());
    mWhitePointY = GetPrivateProfileDouble(L"directshow_metadata_injector_filter", L"white_point_y", 0.32902, stemp.c_str());

    // Defaults are DisplayHDR 1000	(https://en.wikipedia.org/wiki/High-dynamic-range_video#VESA_DisplayHDR)
    mMasteringLuminanceMin = GetPrivateProfileDouble(L"directshow_metadata_injector_filter", L"mastering_luminance_min", 0.05, stemp.c_str());
    mMasteringLuminanceMax = GetPrivateProfileDouble(L"directshow_metadata_injector_filter", L"mastering_luminance_max", 1000.0, stemp.c_str());

    // Defaults are for a Sony BVM-X300 V2 oled reference monitor
    mMaxCLL = GetPrivateProfileInt(L"directshow_metadata_injector_filter", L"luminance_max_cll", 1000, stemp.c_str());
    mMaxFALL = GetPrivateProfileInt(L"directshow_metadata_injector_filter", L"luminance_max_fall", 180, stemp.c_str());

    mInterlaceFlags = GetPrivateProfileInt(L"directshow_metadata_injector_filter", L"interlace_flags", 0, stemp.c_str());

    mAspectRatioX = GetPrivateProfileInt(L"directshow_metadata_injector_filter", L"aspect_ratio_x", 16, stemp.c_str());
    mAspectRatioY = GetPrivateProfileInt(L"directshow_metadata_injector_filter", L"aspect_ratio_y", 9, stemp.c_str());

    mTransferMatrix = (DXVA_VideoTransferMatrix)GetPrivateProfileInt(L"directshow_metadata_injector_filter", L"transfer_matrix", 4, stemp.c_str());  // 4=BT.2020
    mPrimaries = (DXVA_VideoPrimaries)GetPrivateProfileInt(L"directshow_metadata_injector_filter", L"primaries", 9, stemp.c_str());  // 9 = BT.2020
    mTransferFunction = (DXVA_VideoTransferFunction)GetPrivateProfileInt(L"directshow_metadata_injector_filter", L"video_transfer_function", 9, stemp.c_str());  // 15=SMPTE ST 2084 (PQ)
    mNominalRange = (DXVA_NominalRange)GetPrivateProfileInt(L"directshow_metadata_injector_filter", L"nominal_range", 2, stemp.c_str());  // 2=16-235 (wide)
}


HRESULT MetadataInjectorFilter::Copy(IMediaSample* pSource, IMediaSample* pDest) const
{
    CheckPointer(pSource, E_POINTER);
    CheckPointer(pDest, E_POINTER);

    BYTE* pSourceBuffer, * pDestBuffer;
    long lSourceSize = pSource->GetActualDataLength();

#ifdef DEBUG
    long lDestSize = pDest->GetSize();
    ASSERT(lDestSize >= lSourceSize);
#endif

    pSource->GetPointer(&pSourceBuffer);
    pDest->GetPointer(&pDestBuffer);

    CopyMemory((PVOID)pDestBuffer, (PVOID)pSourceBuffer, lSourceSize);

    // Copy the sample times
    REFERENCE_TIME TimeStart, TimeEnd;
    if (pSource->GetTime(&TimeStart, &TimeEnd) == NOERROR) {
        pDest->SetTime(&TimeStart, &TimeEnd);
    }

    LONGLONG MediaStart, MediaEnd;
    if (pSource->GetMediaTime(&MediaStart, &MediaEnd) == NOERROR) {
        pDest->SetMediaTime(&MediaStart, &MediaEnd);
    }

    // Copy the Sync point property
    HRESULT hr = pSource->IsSyncPoint();
    switch(hr)
    {
        case S_OK:
            pDest->SetSyncPoint(TRUE);
            break;

        case S_FALSE:
            pDest->SetSyncPoint(FALSE);
            break;

        default:
            return E_UNEXPECTED;
    }

    // Copy the media type
    AM_MEDIA_TYPE* pMediaType;
    pSource->GetMediaType(&pMediaType);
    pDest->SetMediaType(pMediaType);
    DeleteMediaType(pMediaType);

    // Copy the preroll property
    hr = pSource->IsPreroll();
    switch (hr)
    {
        case S_OK:
            pDest->SetPreroll(TRUE);
            break;

        case S_FALSE:
            pDest->SetPreroll(FALSE);
            break;

        default:
            return E_UNEXPECTED;
    }

    // Copy the discontinuity property
    hr = pSource->IsDiscontinuity();
    switch (hr)
    {
        case S_OK:
            pDest->SetDiscontinuity(TRUE);
            break;

        case S_FALSE:
            pDest->SetDiscontinuity(FALSE);
            break;

        default:
            return E_UNEXPECTED;
    }

    // Copy the actual data length
    long lDataLength = pSource->GetActualDataLength();
    pDest->SetActualDataLength(lDataLength);

    return NOERROR;
}


// Check the input type is OK - return an error otherwise
HRESULT MetadataInjectorFilter::CheckInputType(const CMediaType* mtIn)
{
    CheckPointer(mtIn, E_POINTER);

    if (*mtIn->FormatType() != FORMAT_VideoInfo)
        return E_INVALIDARG;

    ASSERT(mtIn->FormatLength() == sizeof(VIDEOINFOHEADER));

    return NOERROR;
}


// Check a transform can be done between these formats
HRESULT MetadataInjectorFilter::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
    CheckPointer(mtIn, E_POINTER);
    CheckPointer(mtOut, E_POINTER);

    return NOERROR;
}


// Tell the output pin's allocator what size buffers we
// require. Can only do this when the input is connected
HRESULT MetadataInjectorFilter::DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties)
{
    if (m_pInput->IsConnected() == FALSE)
        return E_UNEXPECTED;

    CheckPointer(pAlloc, E_POINTER);
    CheckPointer(pProperties, E_POINTER);
    HRESULT hr = NOERROR;

    pProperties->cBuffers = 1;
    pProperties->cbBuffer = m_pInput->CurrentMediaType().GetSampleSize();
    ASSERT(pProperties->cbBuffer);

    // Ask the allocator to reserve us some sample memory, NOTE the function
    // can succeed (that is return NOERROR) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted
    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties, &Actual);
    if (FAILED(hr))
        return hr;

    ASSERT(Actual.cBuffers == 1);

    if (pProperties->cBuffers > Actual.cBuffers ||
        pProperties->cbBuffer > Actual.cbBuffer)
        return E_FAIL;

    return NOERROR;
}


HRESULT MetadataInjectorFilter::GetMediaType(int iPosition, CMediaType* pMediaType)
{
    if (!m_pInput->IsConnected())
        return E_UNEXPECTED;

    if (iPosition < 0)
        return E_INVALIDARG;

    if (iPosition > 0)
        return VFW_S_NO_MORE_ITEMS;

    BuildFakeMediaType(pMediaType);

    const VIDEOINFOHEADER* const pvi = (VIDEOINFOHEADER*)m_pInput->CurrentMediaType().Format();
    mFramesPerSecond = 1.0 / (pvi->AvgTimePerFrame / 10000000.0);  // time unit is 100ns

    return NOERROR;
}


STDMETHODIMP MetadataInjectorFilter::get_config_filename(char* ConfigFilename)
{
    // TODO: Build a safer interface?
    strcpy_s(ConfigFilename, 1024, mConfigFilename.c_str());

    return NOERROR;
}


STDMETHODIMP MetadataInjectorFilter::set_config_filename(const char* ConfigFilename)
{
    mConfigFilename = ConfigFilename;

    return NOERROR;
}

STDMETHODIMP MetadataInjectorFilter::GetPages(CAUUID* pPages)
{
    CheckPointer(pPages, E_POINTER);

    pPages->cElems = 1;
    pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL)
        return E_OUTOFMEMORY;

    *(pPages->pElems) = CLSID_METADATA_INJECTOR_PROPERTIES;

    return NOERROR;
}


STDMETHODIMP MetadataInjectorFilter::GetClassID(CLSID* pClsid)
{
    return CBaseFilter::GetClassID(pClsid);
}


HRESULT MetadataInjectorFilter::ScribbleToStream(IStream* pStream)
{
    HRESULT hr = pStream->Write(mConfigFilename.c_str(), mConfigFilename.length() + 1, NULL);
    if (FAILED(hr))
        return hr;

    return NOERROR;
}


HRESULT MetadataInjectorFilter::ReadFromStream(IStream* pStream)
{
    char temp[1024];
    ZeroMemory(temp, sizeof(temp));

    HRESULT hr = pStream->Read(&temp, sizeof(temp)-1, NULL);
    if (FAILED(hr))
        return hr;

    mConfigFilename = temp;

    return NOERROR;
}


void MetadataInjectorFilter::BuildFakeMediaType(CMediaType* pMediaType)
{
    ASSERT(pMediaType);

    const VIDEOINFOHEADER* const pvi = (VIDEOINFOHEADER*)m_pInput->CurrentMediaType().Format();

    // Basic copy
    *pMediaType = m_pInput->CurrentMediaType();

    // Change format type to FORMAT_VIDEOINFO2 and populate
    pMediaType->SetFormatType(&FORMAT_VIDEOINFO2);

    VIDEOINFOHEADER2* pvi2 = (VIDEOINFOHEADER2*)pMediaType->AllocFormatBuffer(sizeof(VIDEOINFOHEADER2));
    ZeroMemory(pvi2, sizeof(VIDEOINFOHEADER2));

    // Copy over all similar fields
    pvi2->rcSource = pvi->rcSource;
    pvi2->rcTarget = pvi->rcTarget;
    pvi2->dwBitRate = pvi->dwBitRate;
    pvi2->dwBitErrorRate = pvi->dwBitErrorRate;
    pvi2->AvgTimePerFrame = pvi->AvgTimePerFrame;
    pvi2->bmiHeader = pvi->bmiHeader;

    // Set the unrecoverable fields
    pvi2->dwInterlaceFlags = mInterlaceFlags;
    pvi2->dwCopyProtectFlags = 0;
    pvi2->dwPictAspectRatioX = mAspectRatioX;
    pvi2->dwPictAspectRatioY = mAspectRatioY;

    // dwControlFlags is a 32bit int. With AMCONTROL_COLORINFO_PRESENT the upper 24 bits are used by DXVA_ExtendedFormat.
    // That struct is 32 bits so it's lower member (SampleFormat) is actually overbooked with the value of dwConotrolFlags
    // so can't be used. LAV has defined some out-of-spec but compatile with madVR values for the more modern formats,
    // which we use as well see
    // https://github.com/Nevcairiel/LAVFilters/blob/ddef56ae155d436f4301346408f4fdba755197d6/decoder/LAVVideo/Media.cpp

    DXVA_ExtendedFormat* colorimetry = (DXVA_ExtendedFormat*)&(pvi2->dwControlFlags);
    colorimetry->VideoTransferMatrix = mTransferMatrix;
    colorimetry->VideoPrimaries = mPrimaries;
    colorimetry->VideoTransferFunction = mTransferFunction;
    colorimetry->NominalRange = mNominalRange;

    pvi2->dwControlFlags += AMCONTROL_USED;
    pvi2->dwControlFlags += AMCONTROL_COLORINFO_PRESENT;
}
