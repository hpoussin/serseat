#include "hidserial.h"

NTSTATUS NTAPI
AddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT Fdo)
{
    /* nothing to do */
    return STATUS_SUCCESS;
}

VOID NTAPI
DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
    /* nothing to do */
}

NTSTATUS NTAPI
DispatchInternIoctl(IN PDEVICE_OBJECT Fdo, IN PIRP Irp)
{
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

    switch (IrpStack->Parameters.DeviceIoControl.IoControlCode)
    {
    case IOCTL_GET_PHYSICAL_DESCRIPTOR:
    case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
    case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
    case IOCTL_HID_GET_FEATURE:
    case IOCTL_HID_GET_INDEXED_STRING:
    case IOCTL_HID_GET_STRING:
    case IOCTL_HID_GET_REPORT_DESCRIPTOR:
    case IOCTL_HID_READ_REPORT:
    case IOCTL_HID_SEND_IDLE_NOTIFICATION_REQUEST:
    case IOCTL_HID_SET_FEATURE:
    case IOCTL_HID_WRITE_REPORT:
        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver(LDO(Fdo), Irp);
    default:
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_NOT_SUPPORTED;
    }
}

NTSTATUS NTAPI
DispatchPnp(IN PDEVICE_OBJECT Fdo, IN PIRP Irp)
{
    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(LDO(Fdo), Irp);
}

NTSTATUS NTAPI
DispatchPower(IN PDEVICE_OBJECT Fdo, IN PIRP Irp)
{
    IoCopyCurrentIrpStackLocationToNext(Irp);
    return PoCallDriver(LDO(Fdo), Irp);
}

NTSTATUS NTAPI
DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    HID_MINIDRIVER_REGISTRATION reg;

    DriverObject->DriverExtension->AddDevice = AddDevice;
    DriverObject->DriverUnload = DriverUnload;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = DispatchInternIoctl;
    DriverObject->MajorFunction[IRP_MJ_PNP] = DispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER] = DispatchPower;

    RtlZeroMemory(&reg, sizeof(reg));
    reg.Revision = HID_REVISION;
    reg.DriverObject = DriverObject;
    reg.RegistryPath = RegistryPath;
    reg.DevicesArePolled = FALSE;

    return HidRegisterMinidriver(&reg);
}
