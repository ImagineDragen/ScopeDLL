// intrface.h
//
// Generated by C DriverWizard 3.2.0 (Build 2485)
// Requires DDK Only
// File created on 8/13/2008
//

// Define an Interface Guid for STM32MS device class.
// This GUID is used to register (IoRegisterDeviceInterface)
// an instance of an interface so that user application
// can control the STM32MS device.
//
//  {4B6C73B2-015B-4855-8BB1-A7A6C03AFA87}
DEFINE_GUID(GUID_DEVINTERFACE_STM32MS,
    0x4B6C73B2, 0x015B, 0x4855, 0x8B, 0xB1, 0xA7, 0xA6, 0xC0, 0x3A, 0xFA, 0x87);

// GUID definition are required to be outside of header inclusion pragma to avoid
// error during precompiled headers.
//

#ifndef __INTRFACE_H__
#define __INTRFACE_H__

#define FILE_DEVICE_STM32MS  0x8000

// Define Interface reference/dereference routines for
// Interfaces exported by IRP_MN_QUERY_INTERFACE

#define STM32MS_IOCTL(index) \
    CTL_CODE(FILE_DEVICE_STM32MS, index, METHOD_BUFFERED, FILE_READ_DATA)


#endif // __INTRFACE_H__
