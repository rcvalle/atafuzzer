/** @file */

#ifndef ATA_H
#define ATA_H

#ifdef __cplusplus
extern "C" {
#endif

/** Command Block registers */
enum
{
    ATA_DATA = 0,
    ATA_ERROR = 1,
    ATA_FEATURES = 1,
    ATA_SECTOR_COUNT = 2,
    ATA_SECTOR_NUMBER = 3,
    ATA_LBA_LOW = 3,
    ATA_CYLINDER_LOW = 4,
    ATA_LBA_MID = 4,
    ATA_CYLINDER_HIGH = 5,
    ATA_LBA_HIGH = 5,
    ATA_DEVICE_HEAD = 6,
    ATA_DEVICE = 6,
    ATA_STATUS = 7,
    ATA_COMMAND = 7,
};

/** Control Block registers */
enum
{
    ATA_DEVICE_CONTROL = 2,
    ATA_ALTERNATE_STATUS = 2,
};

/** Error register bits/fields */
enum
{
    ATA_ABRT = (1 << 2),
};

/** Device register bits/fields */
enum
{
    ATA_DEV = (1 << 4),
    ATA_LBA = (1 << 6),
};

/** Status register bits/fields */
enum
{
    ATA_ERR = (1 << 0),
    ATA_DRQ = (1 << 3),
    ATA_DF = (1 << 5),
    ATA_DRDY = (1 << 6),
    ATA_BSY = (1 << 7),
};

/** Device Control register bits/fields */
enum
{
    ATA_nIEN = (1 << 1),
    ATA_SRST = (1 << 2),
    ATA_HOB = (1 << 7),
};

/** General feature set */
enum
{
    ATA_EXECUTE_DEVICE_DIAGNOSTIC = 0x90,
    ATA_FLUSH_CACHE = 0xe7,
    ATA_FLUSH_CACHE_EXT = 0xea,
    ATA_IDENTIFY_DEVICE = 0xec,
    ATA_READ_DMA = 0xc8,
    ATA_READ_DMA_EXT = 0x25,
    ATA_READ_MULTIPLE = 0xc4,
    ATA_READ_MULTIPLE_EXT = 0x29,
    ATA_READ_SECTORS = 0x20,
    ATA_READ_SECTORS_EXT = 0x24,
    ATA_READ_VERIFY_SECTORS = 0x40,
    ATA_READ_VERIFY_SECTORS_EXT = 0x42,
    ATA_SEEK = 0x70,
    ATA_SET_FEATURES = 0xef,
    ATA_SET_MULTIPLE_MODE = 0xc6,
    ATA_WRITE_DMA = 0xca,
    ATA_WRITE_DMA_EXT = 0x37,
    ATA_WRITE_MULTIPLE = 0xc5,
    ATA_WRITE_MULTIPLE_EXT = 0x39,
    ATA_WRITE_SECTORS = 0x30,
    ATA_WRITE_SECTORS_EXT = 0x34,
    ATA_DOWNLOAD_MICROCODE = 0x92,
    ATA_NOP = 0x00,
    ATA_READ_BUFFER = 0xe4,
    ATA_WRITE_BUFFER = 0xe8,
};

#ifdef __cplusplus
}
#endif

#endif /* ATA_H */
