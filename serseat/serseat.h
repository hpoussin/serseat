#include <wdm.h>
#include <hidport.h>

#define REPORT_NEVER 0
#define REPORT_ALWAYS 1
#define REPORT_AUTO 2

#define DRIVERNAME "SERSEAT"
#define LDRIVERNAME L"SERSEAT"

#define ISPDO           0x00000001

#define DEVICE_SCREEN   0x00000001
#define DEVICE_KEYBOARD 0x00000002

#define NUMBER_OF_COLUMNS 80
#define NUMBER_OF_ROWS    25

#define REPORTID_DISPLAY_ATTRIBUTES 1
#define REPORTID_DISPLAY_CONTROL    2
#define REPORTID_CHARACTER          3
#define REPORTID_DISPLAY_STATUS     4
#define REPORTID_CURSOR_POSITION    5
#define REPORTID_KEYBOARD           7

typedef struct _COMMON_DEVICE_EXTENSION
{
    ULONG Flags;
} COMMON_DEVICE_EXTENSION, *PCOMMON_DEVICE_EXTENSION;

typedef struct _DEVICE_EXTENSION
{
    COMMON_DEVICE_EXTENSION;
    PDRIVER_OBJECT DriverObject;
    PDEVICE_OBJECT Fido;
    PDEVICE_OBJECT LowerDeviceObject;
    PDEVICE_OBJECT Pdo;
    IO_REMOVE_LOCK RemoveLock;
    PDEVICE_OBJECT SeatPdo;
    ULONG ReportSeat;
    ULONG AvailableDevices;
    ULONG NumberColumns;
    ULONG NumberRows;
    ULONG CurrentColumn;
    ULONG CurrentRow;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

typedef struct _PDO_EXTENSION
{
    COMMON_DEVICE_EXTENSION;
    PDEVICE_OBJECT Fido;
    PDEVICE_OBJECT Pdo;

    PIO_WORKITEM WorkItem;
    BOOLEAN FidoOpened;
    BOOLEAN NeedReleaseKey;

    UCHAR InBuffer[1];
    IO_STATUS_BLOCK IoStatusBlock;
    PIRP InitialIrp;
} PDO_EXTENSION, *PPDO_EXTENSION;


