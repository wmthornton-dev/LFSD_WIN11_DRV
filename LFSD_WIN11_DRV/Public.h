/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_LFSDWIN11DRV,
    0x12b41bd5,0x150b,0x4239,0xad,0xe3,0x20,0xae,0x4e,0x55,0x19,0xe8);
// {12b41bd5-150b-4239-ade3-20ae4e5519e8}
