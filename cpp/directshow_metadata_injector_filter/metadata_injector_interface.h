#pragma once


#ifdef __cplusplus
extern "C" {
#endif

    // {C3BD8F60-565F-404D-9B8F-D7E38BFA9D5E}
    DEFINE_GUID(
        CLSID_METADATA_INJECTOR_INTERFACE,
        0xc3bd8f60, 0x565f, 0x404d, 0x9b, 0x8f, 0xd7, 0xe3, 0x8b, 0xfa, 0x9d, 0x5e);

    DECLARE_INTERFACE_(MetadataInjectorInterface, IUnknown)
    {
        STDMETHOD(get_config_filename) (THIS_
            char* ConfigFilename
            ) PURE;

        STDMETHOD(set_config_filename) (THIS_
            const char* ConfigFilename
            ) PURE;
    };

#ifdef __cplusplus
}
#endif
