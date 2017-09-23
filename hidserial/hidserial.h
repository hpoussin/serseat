#include <wdm.h>
#include <hidport.h>

#define LDO(Fdo) (((PHID_DEVICE_EXTENSION)((Fdo)->DeviceExtension))->NextDeviceObject)

DRIVER_ADD_DEVICE AddDevice;
DRIVER_UNLOAD DriverUnload;
DRIVER_DISPATCH DispatchInternIoctl;
DRIVER_DISPATCH DispatchPnp;
DRIVER_DISPATCH DispatchPower;

