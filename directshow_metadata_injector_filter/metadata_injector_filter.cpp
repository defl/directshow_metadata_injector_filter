#include <windows.h>
#include <streams.h>
#include <initguid.h>
#include <olectl.h>
#include <dvdmedia.h>
#include <dxva.h>

#include "guids.h"
#include "lavfilters_side_data.h"
#include "metadata_injector_filter.h"


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
    { L"Metdata Injector",
      &CLSID_METADATA_INJECTOR_FILTER,
      MetadataInjectorFilter::CreateInstance,
      NULL,
      &sMIPSetup 
    }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


MetadataInjectorFilter::MetadataInjectorFilter(TCHAR* tszName, LPUNKNOWN punk, HRESULT* phr):
    CTransformFilter(tszName, punk, CLSID_METADATA_INJECTOR_FILTER)
{
}


CUnknown* MetadataInjectorFilter::CreateInstance(LPUNKNOWN punk, HRESULT* phr)
{
    MetadataInjectorFilter* pNewObject = new MetadataInjectorFilter(L"Metdata Injector", punk, phr);

    if (pNewObject == NULL)
    {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
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

    // Add side data if needed
    if(!m_hasSentHDR)
    {
        IMediaSideData* pMediaSideData = nullptr;
        if (FAILED(hr = pOut->QueryInterface(&pMediaSideData)))
            return hr;
         
        // Primaries and whitepoint
        MediaSideDataHDR hdr;
        ZeroMemory(&hdr, sizeof(hdr));

        // https://en.wikipedia.org/wiki/DCI-P3#System_colorimetry
        hdr.display_primaries_x[0] = 0.265;  // G
        hdr.display_primaries_x[1] = 0.150;  // B
        hdr.display_primaries_x[2] = 0.680;  // R
        hdr.display_primaries_y[0] = 0.690;  // G
        hdr.display_primaries_y[1] = 0.060;  // B
        hdr.display_primaries_y[2] = 0.320;  // R
            
        hdr.white_point_x = 0.31271;
        hdr.white_point_y = 0.32902;

        hdr.max_display_mastering_luminance = 1000.0;
        hdr.min_display_mastering_luminance = 0.05;
        pMediaSideData->SetSideData(IID_MediaSideDataHDR, (const BYTE*)&hdr, sizeof(hdr));

        // Content light level
        MediaSideDataHDRContentLightLevel hdrLightLevel;
        hdrLightLevel.MaxCLL = 1015;
        hdrLightLevel.MaxFALL = 131;
        pMediaSideData->SetSideData(IID_MediaSideDataHDRContentLightLevel, (const BYTE*)&hdrLightLevel, sizeof(hdrLightLevel));

        pMediaSideData->Release();

        m_hasSentHDR = true;
    }

    return NOERROR;
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

    // Fake the missing fields
    pvi2->dwInterlaceFlags = 0;  // 0 is used in LAV but there is also AMINTERLACE_UNUSED
    pvi2->dwCopyProtectFlags = 0;
    pvi2->dwPictAspectRatioX = 16;
    pvi2->dwPictAspectRatioY = 9;

    // dwControlFlags is a 32bit int. With AMCONTROL_COLORINFO_PRESENT the upper 24 bits are used by DXVA_ExtendedFormat. 
    // That struct is 32 bits so it's lower member (SampleFormat) is actually overbooked with the value of dwConotrolFlags 
    // so can't be used. LAV has defined some out-of-spec but compatile with madVR values for the more modern formats, 
    // which we use as well see 
    // https://github.com/Nevcairiel/LAVFilters/blob/ddef56ae155d436f4301346408f4fdba755197d6/decoder/LAVVideo/Media.cpp

    DXVA_ExtendedFormat* colorimetry = (DXVA_ExtendedFormat*)&(pvi2->dwControlFlags);
    colorimetry->VideoTransferMatrix = (DXVA_VideoTransferMatrix)4;  // BT.2020
    //colorimetry->VideoPrimaries = (DXVA_VideoPrimaries)9;  // BT.2020
    colorimetry->VideoPrimaries = (DXVA_VideoPrimaries)11;  // DCI-P3
    colorimetry->VideoTransferFunction = (DXVA_VideoTransferFunction)15;  // SMPTE ST 2084 (PQ)
    //colorimetry->VideoTransferFunction = (DXVA_VideoTransferFunction)16;  // HLG
    colorimetry->NominalRange = DXVA_NominalRange_16_235;

    pvi2->dwControlFlags += AMCONTROL_USED;
    pvi2->dwControlFlags += AMCONTROL_COLORINFO_PRESENT;
}
