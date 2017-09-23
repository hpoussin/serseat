#include "serseat.h"
// FIXME: IoAcquireRemoveLock. quand?
// FIXME devpower, syspower ? p58
// FIXME: MmProbeAndLockPages protected with SEH (p72)

// WRITE: Vertical Scroll:
// 0: characters received after the cursor reaches the right-most column (Column = Columns) will overwrite each other
// 1: if the cursor is on the last character of a row (Column =
//    Columns) other than the last row (Row != Rows), the next character received will cause the vertical
//    cursor position to be incremented (Row++) and the horizontal cursor position to be set to 0 (Column
//    = 0). If the cursor is on the last character (Column = Columns) of the last row (Row = Rows), the
//    next character will cause all rows to be scrolled up, the last row to be cleared, and the horizontal
//    cursor position to be set to 0 (Column = 0).
// => On peut l'enlever si on ne supporte que le mode 0.
#if 0

DO_BUFFERED_IO/DO_DIRECT_IO?

PDO->Characteristics:
FILE_DEVICE_SECURE_OPEN
#endif

static UCHAR HidReportDescriptorScreen[] =
{
    0x05, 0x14,                    // USAGE_PAGE (Alphanumeric Display)
    0x09, 0x01,                    // USAGE (Alphanumeric Display)
    0xa1, 0x02,                    // COLLECTION (Logical)
    0x09, 0x20,                    //   USAGE (Display Attributes Report)
    0xa1, 0x02,                    //   COLLECTION (Logical)
    0x85, REPORTID_DISPLAY_ATTRIBUTES,//  REPORT_ID (1)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x09, 0x21,                    //     USAGE (ASCII Character Set)
    0xb1, 0x03,                    //     FEATURE (Cnst,Var,Abs)
    0xc0,                          //   END_COLLECTION
    0x09, 0x24,                    //   USAGE (Display Control Report)
    0xa1, 0x02,                    //   COLLECTION (Logical)
    0x85, REPORTID_DISPLAY_CONTROL,//     REPORT_ID (2)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x03,                    //     REPORT_COUNT (2)
    0x09, 0x25,                    //     USAGE (Clear Display)
    0x09, 0x29,                    //     USAGE (Vertical Scroll)
    0xb1, 0x02,                    //     FEATURE (Data,Var,Abs)
    0xc0,                          //   END_COLLECTION
    0x09, 0x2b,                    //   USAGE (Character Report)
    0xa1, 0x02,                    //   COLLECTION (Logical)
    0x85, REPORTID_CHARACTER,      //     REPORT_ID (3)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x09, 0x2c,                    //     USAGE (Display Data)
    0xb1, 0x02,                    //     FEATURE (Data,Var,Abs)
    0xc0,                          //   END_COLLECTION
    0x09, 0x2d,                    //   USAGE (Display Status)
    0xa1, 0x02,                    //   COLLECTION (Logical)
    0x85, REPORTID_DISPLAY_STATUS, //     REPORT_ID (4)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x03,                    //     REPORT_COUNT (1)
    0x09, 0x2f,                    //     USAGE (Stat Ready)
    0xb1, 0x02,                    //     FEATURE (Data,Var,Abs)
    0xc0,                          //   END_COLLECTION
    0x09, 0x32,                    //   USAGE (Cursor Position Report)
    0xa1, 0x02,                    //   COLLECTION (Logical)
    0x85, REPORTID_CURSOR_POSITION,//     REPORT_ID (5)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x05,                    //     REPORT_COUNT (5)
    0x09, 0x33,                    //     USAGE (Row)
    0x09, 0x34,                    //     USAGE (Column)
    0x09, 0x35,                    //     USAGE (Rows)
    0x09, 0x36,                    //     USAGE (Columns)
    0x09, 0x38,                    //     USAGE (Cursor Mode)
    0xb1, 0x02,                    //     FEATURE (Data,Var,Abs)
    0xc0,                          //   END_COLLECTION
    0xc0,                          // END_COLLECTION
};

static UCHAR HidReportDescriptorKeyboard[] =
{
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, REPORTID_KEYBOARD,       //   REPORT_ID (7)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
    0x95, 0x05,                    //   REPORT_COUNT (5)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x05, 0x08,                    //   USAGE_PAGE (LEDs)
    0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x03,                    //   REPORT_SIZE (3)
    0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0                           // END_COLLECTION
};

NTSTATUS NTAPI
SucceedRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)
{
    KdPrint((DRIVERNAME ": [%s] Succeeding IRP MJ=%02x MN=%02x\n",
        ((PCOMMON_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->Flags & ISPDO ? "PDO" : "Fido",
        IoGetCurrentIrpStackLocation(Irp)->MajorFunction,
        IoGetCurrentIrpStackLocation(Irp)->MinorFunction));
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS NTAPI
CompleteRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN NTSTATUS Status,
    IN ULONG_PTR Information)
{
    //KdPrint((DRIVERNAME ": [%s] Completing IRP MJ=%02x MN=%02x with status %08x and information %08x\n",
    //    ((PCOMMON_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->Flags & ISPDO ? "PDO" : "Fido",
    //    IoGetCurrentIrpStackLocation(Irp)->MajorFunction,
    //    IoGetCurrentIrpStackLocation(Irp)->MinorFunction,
    //    Status, Information));
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = Information;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}

NTSTATUS NTAPI
FailRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN NTSTATUS Status)
{
    KdPrint((DRIVERNAME ": [%s] Failing IRP MJ=%02x MN=%02x with status %08x\n",
        ((PCOMMON_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->Flags & ISPDO ? "PDO" : "Fido",
        IoGetCurrentIrpStackLocation(Irp)->MajorFunction,
        IoGetCurrentIrpStackLocation(Irp)->MinorFunction,
        Status));
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}

NTSTATUS NTAPI
IgnoreRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)
{
    NTSTATUS Status = Irp->IoStatus.Status;
    KdPrint((DRIVERNAME ": [%s] Ignoring IRP MJ=%02x MN=%02x\n",
        ((PCOMMON_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->Flags & ISPDO ? "PDO" : "Fido",
        IoGetCurrentIrpStackLocation(Irp)->MajorFunction,
        IoGetCurrentIrpStackLocation(Irp)->MinorFunction));
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}

IO_COMPLETION_ROUTINE ForwardAndWaitCompletionRoutine;
NTSTATUS NTAPI
ForwardAndWaitCompletionRoutine(
    IN PDEVICE_OBJECT Fido,
    IN PIRP Irp,
    IN PVOID Context)
{
    PKEVENT Event = Context;
    if (Irp->PendingReturned)
        KeSetEvent(Event, IO_NO_INCREMENT, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS NTAPI
ForwardAndWait(
    IN PDEVICE_EXTENSION Pdx,
    IN PIRP Irp)
{
    KEVENT Event;
    NTSTATUS Status;

    KeInitializeEvent(&Event, NotificationEvent, FALSE);
    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp, ForwardAndWaitCompletionRoutine, &Event, TRUE, TRUE, TRUE);
    Status = IoCallDriver(Pdx->LowerDeviceObject, Irp);
    if (Status == STATUS_PENDING)
    {
        KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
        Status = Irp->IoStatus.Status;
    }
    return Status;
}

DRIVER_DISPATCH DispatchAny;
NTSTATUS NTAPI
DispatchAny(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)
{
    PCOMMON_DEVICE_EXTENSION Pcx = (PCOMMON_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    PDEVICE_EXTENSION Pdx;
    NTSTATUS Status;

    if (Pcx->Flags & ISPDO)
    {
        return IgnoreRequest(DeviceObject, Irp);
    }

    Pdx = (PDEVICE_EXTENSION)Pcx;
    Status = IoAcquireRemoveLock(&Pdx->RemoveLock, Irp);
    if (!NT_SUCCESS(Status))
    {
        KdPrint((DRIVERNAME ": Failed to acquire remove lock (status %08x)\n", Status));
        return FailRequest(DeviceObject, Irp, Status);
    }

    IoSkipCurrentIrpStackLocation(Irp);
    Status = IoCallDriver(Pdx->LowerDeviceObject, Irp);
    IoReleaseRemoveLock(&Pdx->RemoveLock, Irp);
    return Status;
}

DRIVER_DISPATCH DispatchPowerFido;
NTSTATUS NTAPI
DispatchPowerFido(
    IN PDEVICE_OBJECT Fido,
    IN PIRP Irp)
{
    PDEVICE_EXTENSION Pdx = (PDEVICE_EXTENSION)Fido->DeviceExtension;
    NTSTATUS Status;

    PoStartNextPowerIrp(Irp);
    Status = IoAcquireRemoveLock(&Pdx->RemoveLock, Irp);
    if (!NT_SUCCESS(Status))
    {
        KdPrint((DRIVERNAME ": Failed to acquire remove lock (status %08x)\n", Status));
        return FailRequest(Fido, Irp, Status);
    }

    // FIXME: handle IRP_MN_WAIT_WAKE
    ASSERT(IoGetCurrentIrpStackLocation(Irp)->MinorFunction != IRP_MN_WAIT_WAKE);

    IoSkipCurrentIrpStackLocation(Irp);
    Status = PoCallDriver(Pdx->LowerDeviceObject, Irp);
    IoReleaseRemoveLock(&Pdx->RemoveLock, Irp);
    return Status;
}

NTSTATUS NTAPI
DefaultPowerHandler(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp)
{
    NTSTATUS Status;

    PoStartNextPowerIrp(Irp);
    Status = Irp->IoStatus.Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}

NTSTATUS NTAPI
DefaultPnpHandler(
    IN PDEVICE_OBJECT Fido,
    IN PIRP Irp)
{
    PDEVICE_EXTENSION Pdx = (PDEVICE_EXTENSION)Fido->DeviceExtension;

    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(Pdx->LowerDeviceObject, Irp);
}

NTSTATUS NTAPI
HandleQueryPower(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp)
{
    PoStartNextPowerIrp(Irp);
    return SucceedRequest(Pdo, Irp);
}

NTSTATUS NTAPI
HandleSetPower(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp)
{
    PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation(Irp);

    if (IoStack->Parameters.Power.Type == DevicePowerState)
    {
        // nothing to do
        PoSetPowerState(Pdo, DevicePowerState, IoStack->Parameters.Power.State);
    }
    PoStartNextPowerIrp(Irp);
    return SucceedRequest(Pdo, Irp);
}

DRIVER_DISPATCH DispatchPowerPdo;
NTSTATUS NTAPI
DispatchPowerPdo(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp)
{
    PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation(Irp);

    switch (IoStack->MinorFunction)
    {
    case IRP_MN_QUERY_POWER:
        return HandleQueryPower(Pdo, Irp);
    case IRP_MN_SET_POWER:
        return HandleSetPower(Pdo, Irp);
    case IRP_MN_WAIT_WAKE:
        // we don't support waking up the system, so we should't received this Irp
        KdPrint((DRIVERNAME ": Received IRP_MN_WAIT_WAKE, but we don't support waking up the system\n"));
        ASSERT(FALSE);
        return DefaultPowerHandler(Pdo, Irp);
    case IRP_MN_POWER_SEQUENCE:
    default:
        return DefaultPowerHandler(Pdo, Irp);
    }
}

DRIVER_DISPATCH DispatchPower;
NTSTATUS NTAPI
DispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)
{
    PCOMMON_DEVICE_EXTENSION Pcx = (PCOMMON_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    if (Pcx->Flags & ISPDO)
        return DispatchPowerPdo(DeviceObject, Irp);
    else
        return DispatchPowerFido(DeviceObject, Irp);
}

NTSTATUS NTAPI
HandleHidGetDeviceDescriptor(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp)
{
    PPDO_EXTENSION Ppx = (PPDO_EXTENSION)Pdo->DeviceExtension;
    PDEVICE_EXTENSION Pdx = (PDEVICE_EXTENSION)Ppx->Fido->DeviceExtension;
    PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation(Irp);
    PHID_DESCRIPTOR p;

    if (IoStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(HID_DESCRIPTOR))
        return FailRequest(Pdo, Irp, STATUS_BUFFER_TOO_SMALL);

    p = Irp->UserBuffer;
    RtlZeroMemory(p, sizeof(HID_DESCRIPTOR));
    p->bLength = sizeof(HID_DESCRIPTOR);
    p->bDescriptorType = HID_HID_DESCRIPTOR_TYPE;
    p->bcdHID = HID_REVISION;
    p->bCountry = 0;
    p->bNumDescriptors = 1;
    p->DescriptorList[0].bReportType = HID_REPORT_DESCRIPTOR_TYPE;
    p->DescriptorList[0].wReportLength = 0;
    if (Pdx->AvailableDevices & DEVICE_SCREEN)
        p->DescriptorList[0].wReportLength += sizeof(HidReportDescriptorScreen);
    if (Pdx->AvailableDevices & DEVICE_KEYBOARD)
        p->DescriptorList[0].wReportLength += sizeof(HidReportDescriptorKeyboard);
    return CompleteRequest(Pdo, Irp, STATUS_SUCCESS, sizeof(HID_DESCRIPTOR));
}

NTSTATUS NTAPI
HandleHidGetReportDescriptor(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp)
{
    PPDO_EXTENSION Ppx = (PPDO_EXTENSION)Pdo->DeviceExtension;
    PDEVICE_EXTENSION Pdx = (PDEVICE_EXTENSION)Ppx->Fido->DeviceExtension;
    PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG RequiredSize = 0;
    PUCHAR p;

    if (Pdx->AvailableDevices & DEVICE_SCREEN)
        RequiredSize += sizeof(HidReportDescriptorScreen);
    if (Pdx->AvailableDevices & DEVICE_KEYBOARD)
        RequiredSize += sizeof(HidReportDescriptorKeyboard);

    if (IoStack->Parameters.DeviceIoControl.OutputBufferLength < RequiredSize)
        return FailRequest(Pdo, Irp, STATUS_BUFFER_TOO_SMALL);

    p = Irp->UserBuffer;
    if (Pdx->AvailableDevices & DEVICE_SCREEN)
    {
        RtlCopyMemory(p, HidReportDescriptorScreen, sizeof(HidReportDescriptorScreen));
        p += sizeof(HidReportDescriptorScreen);
    }
    if (Pdx->AvailableDevices & DEVICE_KEYBOARD)
    {
        RtlCopyMemory(p, HidReportDescriptorKeyboard, sizeof(HidReportDescriptorKeyboard));
    }
    return CompleteRequest(Pdo, Irp, STATUS_SUCCESS, RequiredSize);
}

NTSTATUS
SendSyncRequest(
    IN PDEVICE_OBJECT Fido,
    IN UCHAR MajorFunction)
{
    PDEVICE_EXTENSION Pdx = (PDEVICE_EXTENSION)Fido->DeviceExtension;
    PIRP Irp;
    PIO_STACK_LOCATION IoStack;
    NTSTATUS Status;

    Irp = IoAllocateIrp(Fido->StackSize, FALSE);
    if (!Irp)
    {
        KdPrint((DRIVERNAME ": Failed to allocate Irp\n"));
        return STATUS_NO_MEMORY;
    }

    IoStack = IoGetNextIrpStackLocation(Irp);
    IoStack->MajorFunction = MajorFunction;

    Status = ForwardAndWait(Pdx, Irp);
    return Status;
}

IO_COMPLETION_ROUTINE OnReadComplete;
NTSTATUS NTAPI
OnReadComplete(
    IN PDEVICE_OBJECT Fido,
    IN PIRP Irp,
    IN PVOID Context)
{
    PPDO_EXTENSION Ppx = Context;
    NTSTATUS Status = Ppx->IoStatusBlock.Status;
    PIRP InitialIrp = Ppx->InitialIrp;
    PUCHAR InBuffer = Irp->AssociatedIrp.SystemBuffer;
    PUCHAR OutBuffer = InitialIrp->UserBuffer;

#if 0
    ObDereferenceObjectWithTag(Irp->Tail.Overlay.Thread, 'SRES');
    if ((Fido->Flags & DO_DIRECT_IO) == DO_DIRECT_IO)
    {
        MmUnlockPages(Irp->MdlAddress);
    }
    IoFreeMdl(Irp->MdlAddress);
    IoFreeIrp(Irp);
#endif

    if (!NT_SUCCESS(Status))
    {
        KdPrint((DRIVERNAME ": Failed IRP_MJ_READ with status %08x\n", Status));
        FailRequest(Fido, InitialIrp, Status);
        return STATUS_SUCCESS;
    }
    Ppx->NeedReleaseKey = TRUE;

    RtlZeroMemory(OutBuffer, 9);
    OutBuffer[0] = REPORTID_KEYBOARD;
    if (InBuffer[0] >= 'a' && InBuffer[0] <= 'z')
    {
        OutBuffer[3] = InBuffer[0] - 'a' + 4;
    }
    else if (InBuffer[0] >= 'A' && InBuffer[0] <= 'Z')
    {
        OutBuffer[3] = InBuffer[0] - 'A' + 4;
    }
    else if (InBuffer[0] == 0x0d) /* return */
    {
        OutBuffer[3] = 0x28;
    }
    else if (InBuffer[0] == '\b') /* backspace */
    {
        OutBuffer[3] = 0x2a;
    }
    else if (InBuffer[0] == ' ')
    {
        OutBuffer[3] = 0x2c;
    }
    else
    {
        // unknown key, do nothing
        KdPrint((DRIVERNAME ": Unknown key 0x%02x\n", InBuffer[0]));
        Ppx->NeedReleaseKey = FALSE;
        //ASSERT(FALSE); // FIXME: do another read?
    }

    CompleteRequest(Fido, InitialIrp, STATUS_SUCCESS, 9);
    return STATUS_SUCCESS;
}

IO_WORKITEM_ROUTINE HidReadReportWorkItem;
VOID NTAPI
HidReadReportWorkItem(
    IN PDEVICE_OBJECT Pdo,
    IN PVOID Context)
{
    PPDO_EXTENSION Ppx = (PPDO_EXTENSION)Pdo->DeviceExtension;
    PDEVICE_OBJECT Fido = Ppx->Fido;
    PIRP Irp = Context;
    LARGE_INTEGER zero;
    PIRP SubIrp;
    NTSTATUS Status;

    if (!Ppx->FidoOpened)
    {
        Status = SendSyncRequest(Fido, IRP_MJ_CREATE);
        if (!NT_SUCCESS(Status))
        {
            KdPrint((DRIVERNAME ": Failed to send IRP_MJ_CREATE (status %08x)\n", Status));
            FailRequest(Pdo, Irp, Status);
            return;
        }
        Ppx->FidoOpened = TRUE;
    }

    RtlZeroMemory(&zero, sizeof(zero));
    SubIrp = IoBuildAsynchronousFsdRequest(IRP_MJ_READ, Fido, Ppx->InBuffer, sizeof(Ppx->InBuffer), &zero, &Ppx->IoStatusBlock);
    if (!SubIrp)
    {
        KdPrint((DRIVERNAME ": Failed to allocate subIrp\n"));
        FailRequest(Pdo, Irp, STATUS_NO_MEMORY);
        return;
    }

    Status = ObReferenceObjectByPointer(SubIrp->Tail.Overlay.Thread, 0, *PsThreadType, KernelMode);
    if (!NT_SUCCESS(Status))
    {
        KdPrint((DRIVERNAME ": Failed to reference current thread\n"));
        IoFreeIrp(SubIrp);
        FailRequest(Pdo, Irp, Status);
        return;
    }

    Ppx->InitialIrp = Irp;
    IoSetCompletionRoutine(SubIrp, OnReadComplete, Ppx, TRUE, TRUE, TRUE);
    IoCallDriver(Fido, SubIrp);
}

NTSTATUS NTAPI
HandleHidReadReport(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp)
{
    PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation(Irp);
    PPDO_EXTENSION Ppx = (PPDO_EXTENSION)Pdo->DeviceExtension;
    KdPrint((DRIVERNAME ": METHOD_BUFFERED/METHOD_IN_DIRECT Irp->AssociatedIrp.SystemBuffer %p\n", Irp->AssociatedIrp.SystemBuffer));
    KdPrint((DRIVERNAME ": METHOD_OUT_DIRECT Irp->MdlAddress %p\n", Irp->MdlAddress));
    KdPrint((DRIVERNAME ": METHOD_NEITHER Parameters.DeviceIoControl.Type3InputBuffer %p Irp->UserBuffer %p\n", IoStack->Parameters.DeviceIoControl.Type3InputBuffer, Irp->UserBuffer));

    if (IoStack->Parameters.DeviceIoControl.OutputBufferLength < 9)
    {
        KdPrint((DRIVERNAME ": Not enough room to fill read report (given %d)\n", IoStack->Parameters.DeviceIoControl.OutputBufferLength));
        return FailRequest(Pdo, Irp, STATUS_BUFFER_TOO_SMALL);
    }

    if (Ppx->NeedReleaseKey)
    {
        PUCHAR OutBuffer = Irp->UserBuffer;
        Ppx->NeedReleaseKey = FALSE;
        RtlZeroMemory(OutBuffer, 9);
        OutBuffer[0] = REPORTID_KEYBOARD;
        return CompleteRequest(Pdo, Irp, STATUS_SUCCESS, 9);
    }

    // FIXME: race condition if 2 threads go there at the same time. Protect WorkItem!

    // We may be at DISPATCH_LEVEL. Use a work item to go back to PASSIVE_LEVEL
    Ppx->WorkItem = IoAllocateWorkItem(Pdo);
    if (!Ppx->WorkItem)
    {
        KdPrint((DRIVERNAME ": Failed to allocate work item\n"));
        return FailRequest(Pdo, Irp, STATUS_NO_MEMORY);
    }

    // FIXME: it's forbidden to queue twice the same work item. In that case, fail with STATUS_SHARING_VIOLATION?
    IoQueueWorkItem(Ppx->WorkItem, HidReadReportWorkItem, DelayedWorkQueue, Irp);

    IoMarkIrpPending(Irp);
    return STATUS_PENDING;
}

NTSTATUS NTAPI
HandleHidGetFeature(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp)
{
    PHID_XFER_PACKET XferPacket = Irp->UserBuffer;
    PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation(Irp);
    PUCHAR OutputBuffer = XferPacket->reportBuffer;
    PPDO_EXTENSION Ppx = (PPDO_EXTENSION)Pdo->DeviceExtension;
    PDEVICE_EXTENSION Pdx = (PDEVICE_EXTENSION)Ppx->Fido->DeviceExtension;

    KdPrint((DRIVERNAME ": Want to get feature %u\n", XferPacket->reportId));
    ASSERT(FALSE);

    switch (XferPacket->reportId)
    {
    case REPORTID_DISPLAY_ATTRIBUTES:
    {
        if (IoStack->Parameters.DeviceIoControl.OutputBufferLength < 2)
        {
            KdPrint((DRIVERNAME ": Not enough room to fill get feature report (given %d)\n", IoStack->Parameters.DeviceIoControl.OutputBufferLength));
            return FailRequest(Pdo, Irp, STATUS_BUFFER_TOO_SMALL);
        }
        OutputBuffer[1] = 1; /* ASCII Character Set */
        return CompleteRequest(Pdo, Irp, STATUS_SUCCESS, 2);
    }
    case REPORTID_DISPLAY_CONTROL:
    {
        if (IoStack->Parameters.DeviceIoControl.OutputBufferLength < 3)
        {
            KdPrint((DRIVERNAME ": Not enough room to fill get feature report (given %d)\n", IoStack->Parameters.DeviceIoControl.OutputBufferLength));
            return FailRequest(Pdo, Irp, STATUS_BUFFER_TOO_SMALL);
        }
        OutputBuffer[1] = 0; /* Clear Display */
        OutputBuffer[2] = 0; /* Vertical Scroll */
        return CompleteRequest(Pdo, Irp, STATUS_SUCCESS, 3);
    }
    case REPORTID_CHARACTER:
    {
        ASSERT(FALSE);
        return FailRequest(Pdo, Irp, STATUS_NOT_IMPLEMENTED);
    }
    case REPORTID_DISPLAY_STATUS:
    {
        if (IoStack->Parameters.DeviceIoControl.OutputBufferLength < 2)
        {
            KdPrint((DRIVERNAME ": Not enough room to fill get feature report (given %d)\n", IoStack->Parameters.DeviceIoControl.OutputBufferLength));
            return FailRequest(Pdo, Irp, STATUS_BUFFER_TOO_SMALL);
        }
        OutputBuffer[1] = 0x2f; /* Stat Ready */
        return CompleteRequest(Pdo, Irp, STATUS_SUCCESS, 2);
    }
    case REPORTID_CURSOR_POSITION:
    {
        if (IoStack->Parameters.DeviceIoControl.OutputBufferLength < 6)
        {
            KdPrint((DRIVERNAME ": Not enough room to fill get feature report (given %d)\n", IoStack->Parameters.DeviceIoControl.OutputBufferLength));
            return FailRequest(Pdo, Irp, STATUS_BUFFER_TOO_SMALL);
        }
        ASSERT(FALSE);
        OutputBuffer[1] = Pdx->CurrentRow; /* Row */
        OutputBuffer[2] = Pdx->CurrentColumn; /* Column */
        OutputBuffer[3] = Pdx->NumberRows; /* Rows */
        OutputBuffer[4] = Pdx->NumberColumns; /* Columns */
        OutputBuffer[5] = 1; /* Cursor Mode */
        return CompleteRequest(Pdo, Irp, STATUS_SUCCESS, 4);
    }
    default:
        return IgnoreRequest(Pdo, Irp);
    }
}

NTSTATUS NTAPI
HandleHidSetFeature(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp)
{
    PHID_XFER_PACKET XferPacket = Irp->UserBuffer;
    PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation(Irp);
    PUCHAR InputBuffer = XferPacket->reportBuffer;

    KdPrint((DRIVERNAME ": Want to get feature %u\n", XferPacket->reportId));
    ASSERT(FALSE);

    switch (XferPacket->reportId)
    {
    case REPORTID_DISPLAY_CONTROL:
    {
        if (IoStack->Parameters.DeviceIoControl.InputBufferLength < 3)
        {
            KdPrint((DRIVERNAME ": Not enough room to fill set feature report (given %d)\n", IoStack->Parameters.DeviceIoControl.InputBufferLength));
            return FailRequest(Pdo, Irp, STATUS_BUFFER_TOO_SMALL);
        }
        KdPrint((DRIVERNAME ": ClearDisplay=%02x VerticalScroll=%02x\n", InputBuffer[1], InputBuffer[2]));
        ASSERT(FALSE);
        return SucceedRequest(Pdo, Irp);
    }
    case REPORTID_CHARACTER:
    {
        if (IoStack->Parameters.DeviceIoControl.InputBufferLength < 2)
        {
            KdPrint((DRIVERNAME ": Not enough room to fill set feature report (given %d)\n", IoStack->Parameters.DeviceIoControl.InputBufferLength));
            return FailRequest(Pdo, Irp, STATUS_BUFFER_TOO_SMALL);
        }
        KdPrint((DRIVERNAME ": received char %02x\n", InputBuffer[1]));
        ASSERT(FALSE);
        return SucceedRequest(Pdo, Irp);
    }
    case REPORTID_CURSOR_POSITION:
    {
        ASSERT(FALSE);
        return FailRequest(Pdo, Irp, STATUS_NOT_IMPLEMENTED);
    }
    default:
        return IgnoreRequest(Pdo, Irp);
    }
}

DRIVER_DISPATCH DispatchInternIoctlPdo;
NTSTATUS NTAPI
DispatchInternIoctlPdo(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp)
{
    PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation(Irp);

    switch (IoStack->Parameters.DeviceIoControl.IoControlCode)
    {
    case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
        return HandleHidGetDeviceDescriptor(Pdo, Irp);
    case IOCTL_HID_GET_REPORT_DESCRIPTOR:
        return HandleHidGetReportDescriptor(Pdo, Irp);
    case IOCTL_HID_READ_REPORT:
        return HandleHidReadReport(Pdo, Irp);
    case IOCTL_HID_GET_FEATURE:
        return HandleHidGetFeature(Pdo, Irp);
    case IOCTL_HID_SET_FEATURE:
        return HandleHidSetFeature(Pdo, Irp);
    default:
        return IgnoreRequest(Pdo, Irp);
    }
}

DRIVER_DISPATCH DispatchInternIoctl;
NTSTATUS NTAPI
DispatchInternIoctl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)
{
    PCOMMON_DEVICE_EXTENSION Pcx = (PCOMMON_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    if (Pcx->Flags & ISPDO)
        return DispatchInternIoctlPdo(DeviceObject, Irp);
    else
        return DispatchAny(DeviceObject, Irp);
}

IO_COMPLETION_ROUTINE UsageNotificationCompletionRoutine;
NTSTATUS NTAPI
UsageNotificationCompletionRoutine(
    IN PDEVICE_OBJECT Fido,
    IN PIRP Irp,
    IN PVOID Context)
{
    PDEVICE_EXTENSION Pdx = Context;
    if (Irp->PendingReturned)
        IoMarkIrpPending(Irp);
    if (!(Pdx->LowerDeviceObject->Flags & DO_POWER_PAGABLE))
        Fido->Flags &= ~DO_POWER_PAGABLE;
    IoReleaseRemoveLock(&Pdx->RemoveLock, Irp);
    return STATUS_SUCCESS;
}

NTSTATUS NTAPI
CreateChild(
    IN PDEVICE_EXTENSION Pdx,
    IN ULONG Flags,
    OUT PDEVICE_OBJECT* pPdo)
{
    PDEVICE_OBJECT Pdo;
    PPDO_EXTENSION Ppx;
    NTSTATUS Status;

    Status = IoCreateDevice(Pdx->DriverObject, sizeof(PDO_EXTENSION), NULL, FILE_DEVICE_UNKNOWN, FILE_AUTOGENERATED_DEVICE_NAME, FALSE, &Pdo);
    if (!NT_SUCCESS(Status))
    {
        KdPrint((DRIVERNAME ": Failed IoCreateDevice() for PDO: status %08x\n", Status));
        return Status;
    }

    Ppx = (PPDO_EXTENSION)Pdo->DeviceExtension;
    Ppx->Flags = ISPDO | Flags;
    Ppx->Pdo = Pdo;
    Ppx->Fido = Pdx->Fido;
    Pdo->Flags &= ~DO_DEVICE_INITIALIZING;

    *pPdo = Pdo;
    return STATUS_SUCCESS;
}

NTSTATUS NTAPI
StartDevice(IN PDEVICE_OBJECT Fido)
{
    PDEVICE_EXTENSION Pdx = (PDEVICE_EXTENSION)Fido->DeviceExtension;
    PDEVICE_OBJECT SeatPdo;
    NTSTATUS Status;

    if (Pdx->SeatPdo)
        return STATUS_SUCCESS;

    Status = CreateChild(Pdx, 0, &SeatPdo);
    if (!NT_SUCCESS(Status))
    {
        KdPrint((DRIVERNAME ": Failed to create the seat PDO: status %08x\n", Status));
        return Status;
    }

    Pdx->SeatPdo = SeatPdo;
    return STATUS_SUCCESS;
}

VOID NTAPI
RemoveDevice(IN PDEVICE_OBJECT Fido)
{
    PDEVICE_EXTENSION Pdx = (PDEVICE_EXTENSION)Fido->DeviceExtension;

    if (Pdx->LowerDeviceObject)
        IoDetachDevice(Pdx->LowerDeviceObject);
    if (Pdx->SeatPdo)
        IoDeleteDevice(Pdx->SeatPdo);
    IoDeleteDevice(Fido);
}

NTSTATUS NTAPI
HandleQueryRelationsFido(
    IN PDEVICE_OBJECT Fido,
    IN PIRP Irp)
{
    PDEVICE_EXTENSION Pdx = (PDEVICE_EXTENSION)Fido->DeviceExtension;
    PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_RELATIONS OldRelations, NewRelations;
    BOOLEAN ReportPDOs;
    ULONG Length;
    NTSTATUS Status;

    KdPrint((DRIVERNAME ": [FDO] HandleQueryRelationsPdo(Type=%d)\n", IoStack->Parameters.QueryDeviceRelations.Type));
    if (IoStack->Parameters.QueryDeviceRelations.Type != BusRelations)
        return DefaultPnpHandler(Fido, Irp);

    Status = ForwardAndWait(Pdx, Irp);
    if (Status == STATUS_NOT_SUPPORTED)
    {
        KdPrint((DRIVERNAME ": IRP_MN_QUERY_DEVICE_RELATIONS/BusRelations failed with STATUS_NOT_SUPPORTED. serenum.sys not loaded? Activating HACK\n"));
        Status = STATUS_SUCCESS;
    }
    if (!NT_SUCCESS(Status))
    {
        KdPrint((DRIVERNAME ": Failed to forward IRP_MN_QUERY_DEVICE_RELATIONS/BusRelations: status %08x\n", Status));
        return Status;
    }

    OldRelations = (PDEVICE_RELATIONS)Irp->IoStatus.Information;
    if (Pdx->ReportSeat == REPORT_ALWAYS)
        ReportPDOs = TRUE;
    else if (Pdx->ReportSeat == REPORT_NEVER)
        ReportPDOs = FALSE;
    else
        ReportPDOs = !OldRelations || OldRelations->Count == 0;

    if (ReportPDOs)
    {
        // allocate some memory
        Length = sizeof(DEVICE_RELATIONS) + ((OldRelations ? OldRelations->Count : 0) + 1) * sizeof(PDEVICE_OBJECT);
        NewRelations = ExAllocatePoolWithTag(NonPagedPool, Length, 'SRES');
        if (!NewRelations)
        {
            if (OldRelations)
                ExFreePool(OldRelations);
            return FailRequest(Fido, Irp, STATUS_NO_MEMORY);
        }

        if (OldRelations)
        {
            // copy already reported device objects
            RtlCopyMemory(NewRelations->Objects,
                          OldRelations->Objects,
                          OldRelations->Count * sizeof(PDEVICE_OBJECT));
            NewRelations->Count = OldRelations->Count;
        }
        else
        {
            NewRelations->Count = 0;
        }

        // copy new object
        ObReferenceObject(Pdx->SeatPdo);
        NewRelations->Objects[NewRelations->Count++] = Pdx->SeatPdo;

        if (OldRelations)
            ExFreePool(OldRelations);

        Irp->IoStatus.Information = (ULONG_PTR)NewRelations;
        Irp->IoStatus.Status = STATUS_SUCCESS;
    }

    return DefaultPnpHandler(Fido, Irp);

       // Status = CompleteRequest(Irp, STATUS_SUCCESS, NewRelations);
            //end of chapter 5;
            //add some new PDOs;
//page 308;
    //}

    //return Status;
}

IO_COMPLETION_ROUTINE StartDeviceCompletionRoutine;
NTSTATUS NTAPI
StartDeviceCompletionRoutine(
    IN PDEVICE_OBJECT Fido,
    IN PIRP Irp,
    IN PVOID Context)
{
    PDEVICE_EXTENSION Pdx = (PDEVICE_EXTENSION)Fido->DeviceExtension;

    if (Irp->PendingReturned)
        IoMarkIrpPending(Irp);
    if (Pdx->LowerDeviceObject->Characteristics & FILE_REMOVABLE_MEDIA)
        Fido->Characteristics |= FILE_REMOVABLE_MEDIA;
    IoReleaseRemoveLock(&Pdx->RemoveLock, Irp);
    return STATUS_SUCCESS;
}

NTSTATUS NTAPI
HandleStartDevice(
    IN PDEVICE_OBJECT Fido,
    IN PIRP Irp)
{
    PDEVICE_EXTENSION Pdx = (PDEVICE_EXTENSION)Fido->DeviceExtension;
    NTSTATUS Status;

    Status = StartDevice(Fido);
    if (!NT_SUCCESS(Status))
    {
        KdPrint((DRIVERNAME ": StartDevice() failed (status %08x)\n", Status));
        return FailRequest(Fido, Irp, Status);
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp, StartDeviceCompletionRoutine, Pdx, TRUE, TRUE, TRUE);
    return IoCallDriver(Pdx->LowerDeviceObject, Irp);
}

NTSTATUS NTAPI
HandleRemoveDevice(
    IN PDEVICE_OBJECT Fido,
    IN PIRP Irp)
{
    PDEVICE_EXTENSION Pdx = (PDEVICE_EXTENSION)Fido->DeviceExtension;
    NTSTATUS Status;
    Status = DefaultPnpHandler(Fido, Irp);
    IoReleaseRemoveLockAndWait(&Pdx->RemoveLock, Irp);
    RemoveDevice(Fido);
    return Status;
}

DRIVER_DISPATCH DispatchPnpFido;
NTSTATUS NTAPI
DispatchPnpFido(
    IN PDEVICE_OBJECT Fido,
    IN PIRP Irp)
{
    PDEVICE_EXTENSION Pdx = (PDEVICE_EXTENSION)Fido->DeviceExtension;
    PIO_STACK_LOCATION IoStack;
    NTSTATUS Status;

    Status = IoAcquireRemoveLock(&Pdx->RemoveLock, Irp);
    if (!NT_SUCCESS(Status))
    {
        KdPrint((DRIVERNAME ": Failed to acquire remove lock (status %08x)\n", Status));
        return FailRequest(Fido, Irp, Status);
    }

    IoStack = IoGetCurrentIrpStackLocation(Irp);
    switch (IoStack->MinorFunction)
    {
    case IRP_MN_DEVICE_USAGE_NOTIFICATION:
        if (!Fido->AttachedDevice || Fido->AttachedDevice->Flags & DO_POWER_PAGABLE)
        {
            Fido->Flags |= DO_POWER_PAGABLE;
        }
        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoSetCompletionRoutine(Irp, UsageNotificationCompletionRoutine, Pdx, TRUE, TRUE, TRUE);
        return IoCallDriver(Pdx->LowerDeviceObject, Irp);
    case IRP_MN_START_DEVICE:
        return HandleStartDevice(Fido, Irp);
    case IRP_MN_REMOVE_DEVICE:
        return HandleRemoveDevice(Fido, Irp);
    case IRP_MN_QUERY_DEVICE_RELATIONS:
        Status = HandleQueryRelationsFido(Fido, Irp);
        IoReleaseRemoveLock(&Pdx->RemoveLock, Irp);
        return Status;
    default:
        IoSkipCurrentIrpStackLocation(Irp);
        Status = IoCallDriver(Pdx->LowerDeviceObject, Irp);
        IoReleaseRemoveLock(&Pdx->RemoveLock, Irp);
        return Status;
    }
}

IO_COMPLETION_ROUTINE OnRepeaterComplete;
NTSTATUS NTAPI
OnRepeaterComplete(
    IN PDEVICE_OBJECT Tdo,
    IN PIRP SubIrp,
    IN PVOID NeedsVote)
{
    PIO_STACK_LOCATION SubStack = IoGetCurrentIrpStackLocation(SubIrp);
    PIRP Irp = (PIRP)SubStack->Parameters.Others.Argument1;

    ObDereferenceObject(Tdo);
    if (SubIrp->IoStatus.Status == STATUS_NOT_SUPPORTED)
    {
        if (NeedsVote)
            Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
    }
    else
    {
        Irp->IoStatus = SubIrp->IoStatus;
    }

    IoFreeIrp(SubIrp);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS NTAPI
RepeatRequest(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp,
    IN BOOLEAN NeedsVote)
{
    PPDO_EXTENSION Ppx = (PPDO_EXTENSION)Pdo->DeviceExtension;
    PDEVICE_OBJECT Fido = Ppx->Fido;
    PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_OBJECT Tdo;
    PIRP SubIrp;
    PIO_STACK_LOCATION SubStack;

    Tdo = IoGetAttachedDeviceReference(Fido);
    if (!Tdo)
    {
        KdPrint((DRIVERNAME ": Failed to get top of stack Device Object\n"));
        return STATUS_UNSUCCESSFUL;
    }
    SubIrp = IoAllocateIrp(Tdo->StackSize + 1, FALSE);
    if (!SubIrp)
    {
        KdPrint((DRIVERNAME ": Failed to allocate subIrp\n"));
        return STATUS_NO_MEMORY;
    }
    SubStack = IoGetNextIrpStackLocation(SubIrp);
    SubStack->DeviceObject = Tdo;
    SubStack->Parameters.Others.Argument1 = (PVOID)Irp;

    IoSetNextIrpStackLocation(SubIrp);
    SubStack = IoGetNextIrpStackLocation(SubIrp);
    RtlCopyMemory(SubStack, IoStack,
                  FIELD_OFFSET(IO_STACK_LOCATION, CompletionRoutine));
    SubStack->Control = 0;

    IoSetCompletionRoutine(SubIrp, OnRepeaterComplete, UlongToPtr(NeedsVote), TRUE, TRUE, TRUE);

    SubIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    IoMarkIrpPending(Irp);
    IoCallDriver(Tdo, SubIrp);
    return STATUS_PENDING;
}

NTSTATUS NTAPI
HandleQueryId(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp)
{
    PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation(Irp);
    PCWSTR IdString;
    PWSTR Id;
    ULONG nChars, nBytes;

    switch (IoStack->Parameters.QueryId.IdType)
    {
    case BusQueryInstanceID:
        IdString = L"0000";
        break;
    case BusQueryDeviceID:
        IdString = LDRIVERNAME L"\\Seat";
        break;
    case BusQueryHardwareIDs:
        IdString = L"*" LDRIVERNAME;
        break;
    case BusQueryCompatibleIDs:
        IdString = L"SERIAL\\HID_DEVICE";
        break;
    default:
        return FailRequest(Pdo, Irp, STATUS_NOT_SUPPORTED);
    }

    nChars = wcslen(IdString);
    nBytes = nChars * sizeof(WCHAR) + 2 * sizeof(UNICODE_NULL);
    Id = ExAllocatePoolWithTag(PagedPool, nBytes, 'SRES');
    if (!Id)
    {
        KdPrint((DRIVERNAME ": Failed to allocate memory to handle IRP_MN_QUERY_ID\n"));
        return FailRequest(Pdo, Irp, STATUS_NO_MEMORY);
    }
    wcscpy(Id, IdString);
    Id[nChars + 1] = UNICODE_NULL;
    return CompleteRequest(Pdo, Irp, STATUS_SUCCESS, (ULONG_PTR)Id);
}

NTSTATUS NTAPI
HandleQueryRelationsPdo(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp)
{
    PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_RELATIONS Relations;

    KdPrint((DRIVERNAME ": [PDO] HandleQueryRelationsPdo(Type=%d)\n", IoStack->Parameters.QueryDeviceRelations.Type));
    if (IoStack->Parameters.QueryDeviceRelations.Type != TargetDeviceRelation)
        return IgnoreRequest(Pdo, Irp);

    Relations = ExAllocatePool(PagedPool, sizeof(DEVICE_RELATIONS));
    if (!Relations)
    {
        KdPrint((DRIVERNAME ": Failed to allocate memory to handle IRP_MN_QUERY_DEVICE_RELATIONS/TargetDeviceRelation\n"));
        return FailRequest(Pdo, Irp, STATUS_NO_MEMORY);
    }
    Relations->Count = 1;
    Relations->Objects[0] = Pdo;
    ObReferenceObject(Pdo);
    return CompleteRequest(Pdo, Irp, STATUS_SUCCESS, (ULONG_PTR)Relations);
}

DRIVER_DISPATCH DispatchPnpPdo;
NTSTATUS NTAPI
DispatchPnpPdo(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp)
{
    PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation(Irp);

    switch (IoStack->MinorFunction)
    {
    case IRP_MN_START_DEVICE:
    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_REMOVE_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:
    case IRP_MN_STOP_DEVICE:
    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
    case IRP_MN_QUERY_RESOURCES:
    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
    case IRP_MN_QUERY_DEVICE_TEXT:
    case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
    case IRP_MN_SURPRISE_REMOVAL:
        return SucceedRequest(Pdo, Irp);
    case IRP_MN_QUERY_DEVICE_RELATIONS:
        return HandleQueryRelationsPdo(Pdo, Irp);
    case IRP_MN_READ_CONFIG:
    case IRP_MN_WRITE_CONFIG:
    case IRP_MN_EJECT:
    case IRP_MN_SET_LOCK:
    case IRP_MN_QUERY_BUS_INFORMATION:
    //case IRP_MN_QUERY_LEGACY_BUS_INFORMATION: // FIXME: add it
        return RepeatRequest(Pdo, Irp, TRUE);
    case IRP_MN_QUERY_CAPABILITIES:
    case IRP_MN_DEVICE_USAGE_NOTIFICATION:
        return RepeatRequest(Pdo, Irp, FALSE);
    case IRP_MN_QUERY_ID:
        return HandleQueryId(Pdo, Irp);
    case IRP_MN_QUERY_PNP_DEVICE_STATE:
        // FIXME: maybe add a PNP_DEVICE_DONT_DISPLAY_IN_UI?
        return RepeatRequest(Pdo, Irp, FALSE);
    case IRP_MN_QUERY_INTERFACE:
    default:
        return IgnoreRequest(Pdo, Irp);
    }
}

DRIVER_DISPATCH DispatchPnp;
NTSTATUS NTAPI
DispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)
{
    PCOMMON_DEVICE_EXTENSION Pcx = (PCOMMON_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    if (Pcx->Flags & ISPDO)
        return DispatchPnpPdo(DeviceObject, Irp);
    else
        return DispatchPnpFido(DeviceObject, Irp);
}

ULONG NTAPI
GetDeviceTypeToUse(IN PDEVICE_OBJECT Pdo)
{
    PDEVICE_OBJECT Tdo = IoGetAttachedDeviceReference(Pdo);
    ULONG DeviceType;

    if (!Tdo)
        return FILE_DEVICE_UNKNOWN;
    DeviceType = Tdo->DeviceType;
    ObDereferenceObject(Tdo);
    return DeviceType;
}

NTSTATUS NTAPI
ReadParameters(
    IN PDEVICE_OBJECT Pdo,
    IN ULONG DevInstKeyType,
    IN PDEVICE_EXTENSION Pdx)
{
    RTL_QUERY_REGISTRY_TABLE ParametersTable[5];

    HANDLE hKey;
    NTSTATUS Status;
    ULONG DefaultReportSeat = REPORT_AUTO;
    ULONG DefaultAvailableDevices = DEVICE_SCREEN | DEVICE_KEYBOARD;
    ULONG DefaultNumberColumns = NUMBER_OF_COLUMNS;
    ULONG DefaultNumberRows = NUMBER_OF_ROWS;

    Status = IoOpenDeviceRegistryKey(Pdo, DevInstKeyType, KEY_READ, &hKey);
    if (!NT_SUCCESS(Status))
        return Status;

    RtlZeroMemory(ParametersTable, sizeof(ParametersTable));

    ParametersTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    ParametersTable[0].Name          = L"ReportSeat";
    ParametersTable[0].EntryContext  = &Pdx->ReportSeat;
    ParametersTable[0].DefaultType   = REG_DWORD;
    ParametersTable[0].DefaultData   = &DefaultReportSeat;
    ParametersTable[0].DefaultLength = sizeof(DefaultReportSeat);

    ParametersTable[1].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    ParametersTable[1].Name          = L"AvailableDevices";
    ParametersTable[1].EntryContext  = &Pdx->AvailableDevices;
    ParametersTable[1].DefaultType   = REG_DWORD;
    ParametersTable[1].DefaultData   = &DefaultAvailableDevices;
    ParametersTable[1].DefaultLength = sizeof(DefaultAvailableDevices);

    ParametersTable[2].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    ParametersTable[2].Name          = L"NumberColumns";
    ParametersTable[2].EntryContext  = &Pdx->NumberColumns;
    ParametersTable[2].DefaultType   = REG_DWORD;
    ParametersTable[2].DefaultData   = &DefaultNumberColumns;
    ParametersTable[2].DefaultLength = sizeof(DefaultNumberColumns);

    ParametersTable[3].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    ParametersTable[3].Name          = L"NumberRows";
    ParametersTable[3].EntryContext  = &Pdx->NumberRows;
    ParametersTable[3].DefaultType   = REG_DWORD;
    ParametersTable[3].DefaultData   = &DefaultNumberRows;
    ParametersTable[3].DefaultLength = sizeof(DefaultNumberRows);

    Status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE, hKey, ParametersTable, NULL, NULL);

    ZwClose(hKey);
    return Status;
}

DRIVER_ADD_DEVICE AddDevice;
NTSTATUS NTAPI
AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo)
{
    PDEVICE_EXTENSION Pdx;
    PDEVICE_OBJECT Fido, Fdo;
    NTSTATUS Status;

    Status = IoCreateDevice(DriverObject,
                            sizeof(DEVICE_EXTENSION),
                            NULL,
                            GetDeviceTypeToUse(Pdo),
                            0,
                            FALSE,
                            &Fido);
    if (!NT_SUCCESS(Status))
    {
        KdPrint((DRIVERNAME ": Failed IoCreateDevice() for Fido: status %08x\n", Status));
        return Status;
    }

    Pdx = (PDEVICE_EXTENSION)Fido->DeviceExtension;

    Pdx->DriverObject = DriverObject;
    IoInitializeRemoveLock(&Pdx->RemoveLock, 'SRES', 0, 0);
    Pdx->Fido = Fido;
    Pdx->Pdo = Pdo;
    Fdo = IoAttachDeviceToDeviceStack(Fido, Pdo);
    if (!Fdo)
    {
        KdPrint((DRIVERNAME ": Failed to attach Fido to device stack: status %08x\n", Status));
        IoDeleteDevice(Fido);
        return STATUS_DEVICE_REMOVED;
    }
    Pdx->LowerDeviceObject = Fdo;
    Fido->Flags |= Fdo->Flags & (DO_DIRECT_IO | DO_BUFFERED_IO | DO_POWER_PAGABLE);

    // read configuration
    Status = ReadParameters(Pdo, PLUGPLAY_REGKEY_DEVICE, Pdx);
    if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
        Status = ReadParameters(Pdo, PLUGPLAY_REGKEY_DRIVER, Pdx);
    if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
        Pdx->ReportSeat = REPORT_AUTO;
    else if (!NT_SUCCESS(Status))
    {
        KdPrint((DRIVERNAME ": Failed to read registry configuration: status %08x\n", Status));
        IoDeleteDevice(Fido);
        return Status;
    }

    Fido->Flags &= ~DO_DEVICE_INITIALIZING;
    return STATUS_SUCCESS;
}

DRIVER_UNLOAD DriverUnload;
VOID NTAPI
DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
    /* nothing to do */
}

NTSTATUS NTAPI
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath)
{
    ULONG i;

    DriverObject->DriverExtension->AddDevice = AddDevice;
    DriverObject->DriverUnload = DriverUnload;

    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
    {
        DriverObject->MajorFunction[i] = DispatchAny;
    }
    DriverObject->MajorFunction[IRP_MJ_PNP] = DispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER] = DispatchPower;
    //DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = DispatchWmi;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = DispatchInternIoctl;

    return STATUS_SUCCESS;
}
