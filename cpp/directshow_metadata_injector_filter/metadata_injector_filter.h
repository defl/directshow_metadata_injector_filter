#pragma once

#include <windows.h>
#include <streams.h>

#include "metadata_injector_interface.h"


/**
 * Directshow filter which can inject/overwrite existing meta data.
 * Focused on HDR, WCG applications.
 */
class MetadataInjectorFilter:
	public CTransformFilter,
    public MetadataInjectorInterface,
    public ISpecifyPropertyPages,
    public CPersistStream
{
public:

    DECLARE_IUNKNOWN;

    // CreateInstance
    // Provide the way for COM to create a Filter object
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN punk, HRESULT* phr);

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // CTransformFilter interface functions
    HRESULT Transform(IMediaSample* pIn, IMediaSample* pOut) override;
    HRESULT CheckInputType(const CMediaType* mtIn) override;
    HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut) override;
    HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties) override;
    HRESULT GetMediaType(int iPosition, CMediaType* pMediaType) override;

    // MetadataInjectorInterface
    STDMETHODIMP get_config_filename(char* ConfigFilename);
    STDMETHODIMP set_config_filename(const char* ConfigFilename);

    // ISpecifyPropertyPages interface
    STDMETHODIMP GetPages(CAUUID* pPages);

    // CPersistStream
    STDMETHODIMP GetClassID(CLSID* pClsid);
    HRESULT ScribbleToStream(IStream* pStream);
    HRESULT ReadFromStream(IStream* pStream);

private:

    // Constructor
    MetadataInjectorFilter(LPUNKNOWN punk, HRESULT* phr);
    virtual ~MetadataInjectorFilter();

    void LoadConfig();

    // Make destination an identical copy of source
    HRESULT Copy(IMediaSample* pSource, IMediaSample* pDest) const;

    // Build fake media type
    void BuildFakeMediaType(CMediaType* pMediaType);

    std::string mConfigFilename;

    unsigned int mFramesPerSecond = 0;
    unsigned int mFrameCounter = 0;

    double mDisplayPrimaryGreenX;
    double mDisplayPrimaryBlueX;
    double mDisplayPrimaryRedX;
    double mDisplayPrimaryGreenY;
    double mDisplayPrimaryBlueY;
    double mDisplayPrimaryRedY;

    double mWhitePointX;
    double mWhitePointY;

    double mMasteringLuminanceMin;
    double mMasteringLuminanceMax;

    unsigned int mMaxCLL;
    unsigned int mMaxFALL;

    DWORD mInterlaceFlags;
    DWORD mAspectRatioX;
    DWORD mAspectRatioY;

    DXVA_VideoTransferMatrix mTransferMatrix;
    DXVA_VideoPrimaries mPrimaries;
    DXVA_VideoTransferFunction mTransferFunction;
    DXVA_NominalRange mNominalRange;
};
