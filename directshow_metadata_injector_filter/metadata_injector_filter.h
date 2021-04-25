#pragma once

#include <streams.h>

/**
 * Directshow filter which can inject/overwrite existing meta data.
 * Focused on HDR, WCG applications.
 */
class MetadataInjectorFilter:
	public CTransformFilter
{
public:

    DECLARE_IUNKNOWN;

    // CreateInstance
    // Provide the way for COM to create a Filter object
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN punk, HRESULT* phr);

    // CTransformFilter
    HRESULT Transform(IMediaSample* pIn, IMediaSample* pOut) override;
    HRESULT CheckInputType(const CMediaType* mtIn) override;
    HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut) override;
    HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties) override;
    HRESULT GetMediaType(int iPosition, CMediaType* pMediaType) override;


private:

    // Constructor
    MetadataInjectorFilter(TCHAR* tszName, LPUNKNOWN punk, HRESULT* phr);

    // Make destination an identical copy of source
    HRESULT Copy(IMediaSample* pSource, IMediaSample* pDest) const;

    // Build fake media type
    void BuildFakeMediaType(CMediaType* pMediaType);

    bool m_hasSentHDR = false;
};
