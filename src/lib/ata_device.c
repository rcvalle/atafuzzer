/** @file */

#include "ata_device.h"

#include "ata.h"
#include "bus_master.h"
#include "pci_device.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

struct _ata_device {
    pci_device_t *pci_device;
    int region_num;
    int timeout;
    uint8_t error;
    uint8_t features[2];
    uint8_t sector_count[2];
    uint8_t lba_low[2];
    uint8_t lba_mid[2];
    uint8_t lba_high[2];
    uint8_t device[2];
    uint8_t status;
    uint16_t *identify_data;
};

static ata_device_error_handler_t *error_handler = NULL;

int ata_device_command_dma(ata_device_t *restrict ata_device, uint16_t command);
int ata_device_command_non_data(ata_device_t *restrict ata_device, uint16_t command);
int ata_device_command_pio_data_in(ata_device_t *restrict ata_device, uint16_t command, uint16_t *data, uint32_t count);
int ata_device_command_pio_data_out(
        ata_device_t *restrict ata_device, uint16_t command, const uint16_t *data, uint32_t count);
void ata_device_error(ata_device_t *restrict ata_device, int status, int error, const char *restrict format, ...);
void ata_device_set_features(ata_device_t *restrict ata_device, uint8_t features);
void ata_device_set_lba(ata_device_t *restrict ata_device, uint32_t lba);
void ata_device_set_lba48(ata_device_t *restrict ata_device, uint64_t lba);
void ata_device_set_sector_count(ata_device_t *restrict ata_device, uint8_t sectors);
void ata_device_set_sector_count16(ata_device_t *restrict ata_device, uint16_t sectors);
void ata_device_software_reset(ata_device_t *restrict ata_device);

int
ata_device_command_execute_device_diagnostic(ata_device_t *restrict ata_device)
{
    return ata_device_command_non_data(ata_device, ATA_EXECUTE_DEVICE_DIAGNOSTIC);
}

int
ata_device_command_flush_cache(ata_device_t *restrict ata_device)
{
    return ata_device_command_non_data(ata_device, ATA_FLUSH_CACHE);
}

int
ata_device_command_flush_cache_ext(ata_device_t *restrict ata_device)
{
    return ata_device_command_non_data(ata_device, ATA_FLUSH_CACHE_EXT);
}

int
ata_device_command_identify_device(ata_device_t *restrict ata_device)
{
    return ata_device_command_pio_data_in(ata_device, ATA_IDENTIFY_DEVICE, ata_device->identify_data, 256);
}

int
ata_device_command_read_dma(ata_device_t *restrict ata_device, uint8_t sectors, uint32_t lba)
{
    ata_device_set_sector_count(ata_device, sectors);
    ata_device_set_lba(ata_device, lba);
    return ata_device_command_dma(ata_device, ATA_READ_DMA);
}

int
ata_device_command_read_dma_ext(ata_device_t *restrict ata_device, uint16_t sectors, uint64_t lba)
{
    ata_device_set_sector_count16(ata_device, sectors);
    ata_device_set_lba48(ata_device, lba);
    return ata_device_command_dma(ata_device, ATA_READ_DMA_EXT);
}

int
ata_device_command_read_multiple(
        ata_device_t *restrict ata_device, uint8_t sectors, uint32_t lba, uint16_t *data, uint32_t count)
{
    ata_device_set_sector_count(ata_device, sectors);
    ata_device_set_lba(ata_device, lba);
    return ata_device_command_pio_data_in(ata_device, ATA_READ_MULTIPLE, data, count);
}

int
ata_device_command_read_multiple_ext(
        ata_device_t *restrict ata_device, uint16_t sectors, uint64_t lba, uint16_t *data, uint32_t count)
{
    ata_device_set_sector_count16(ata_device, sectors);
    ata_device_set_lba48(ata_device, lba);
    return ata_device_command_pio_data_in(ata_device, ATA_READ_MULTIPLE_EXT, data, count);
}

int
ata_device_command_read_sectors(
        ata_device_t *restrict ata_device, uint8_t sectors, uint32_t lba, uint16_t *data, uint32_t count)
{
    ata_device_set_sector_count(ata_device, sectors);
    ata_device_set_lba(ata_device, lba);
    return ata_device_command_pio_data_in(ata_device, ATA_READ_SECTORS, data, count);
}

int
ata_device_command_read_sectors_ext(
        ata_device_t *restrict ata_device, uint16_t sectors, uint64_t lba, uint16_t *data, uint32_t count)
{
    ata_device_set_sector_count16(ata_device, sectors);
    ata_device_set_lba48(ata_device, lba);
    return ata_device_command_pio_data_in(ata_device, ATA_READ_SECTORS_EXT, data, count);
}

int
ata_device_command_read_verify_sectors(ata_device_t *restrict ata_device, uint8_t sectors, uint32_t lba)
{
    ata_device_set_sector_count(ata_device, sectors);
    ata_device_set_lba(ata_device, lba);
    return ata_device_command_non_data(ata_device, ATA_READ_VERIFY_SECTORS);
}

int
ata_device_command_read_verify_sectors_ext(ata_device_t *restrict ata_device, uint16_t sectors, uint64_t lba)
{
    ata_device_set_sector_count16(ata_device, sectors);
    ata_device_set_lba48(ata_device, lba);
    return ata_device_command_non_data(ata_device, ATA_READ_VERIFY_SECTORS_EXT);
}

int
ata_device_command_seek(ata_device_t *restrict ata_device, uint32_t lba)
{
    ata_device_set_lba(ata_device, lba);
    return ata_device_command_non_data(ata_device, ATA_SEEK);
}

int
ata_device_command_set_features(ata_device_t *restrict ata_device, uint8_t code, const uint8_t specific[])
{
    ata_device_set_features(ata_device, code);
    ata_device_set_sector_count(ata_device, specific[0]);
    ata_device_set_lba(ata_device, (specific[3] << 16) | (specific[2] << 8) | specific[1]);
    return ata_device_command_non_data(ata_device, ATA_SET_FEATURES);
}

int
ata_device_command_set_multiple_mode(ata_device_t *restrict ata_device, uint8_t sectors)
{
    ata_device_set_sector_count(ata_device, sectors);
    return ata_device_command_non_data(ata_device, ATA_SET_MULTIPLE_MODE);
}

int
ata_device_command_write_dma(ata_device_t *restrict ata_device, uint8_t sectors, uint32_t lba)
{
    ata_device_set_sector_count(ata_device, sectors);
    ata_device_set_lba(ata_device, lba);
    return ata_device_command_dma(ata_device, ATA_WRITE_DMA);
}

int
ata_device_command_write_dma_ext(ata_device_t *restrict ata_device, uint16_t sectors, uint64_t lba)
{
    ata_device_set_sector_count16(ata_device, sectors);
    ata_device_set_lba48(ata_device, lba);
    return ata_device_command_dma(ata_device, ATA_WRITE_DMA_EXT);
}

int
ata_device_command_write_multiple(
        ata_device_t *restrict ata_device, uint8_t sectors, uint32_t lba, const uint16_t *data, uint32_t count)
{
    ata_device_set_sector_count(ata_device, sectors);
    ata_device_set_lba(ata_device, lba);
    return ata_device_command_pio_data_out(ata_device, ATA_WRITE_MULTIPLE, data, count);
}

int
ata_device_command_write_multiple_ext(
        ata_device_t *restrict ata_device, uint16_t sectors, uint64_t lba, const uint16_t *data, uint32_t count)
{
    ata_device_set_sector_count16(ata_device, sectors);
    ata_device_set_lba48(ata_device, lba);
    return ata_device_command_pio_data_out(ata_device, ATA_WRITE_MULTIPLE_EXT, data, count);
}

int
ata_device_command_write_sectors(
        ata_device_t *restrict ata_device, uint8_t sectors, uint32_t lba, const uint16_t *data, uint32_t count)
{
    ata_device_set_sector_count(ata_device, sectors);
    ata_device_set_lba(ata_device, lba);
    return ata_device_command_pio_data_out(ata_device, ATA_WRITE_SECTORS, data, count);
}

int
ata_device_command_write_sectors_ext(
        ata_device_t *restrict ata_device, uint16_t sectors, uint64_t lba, const uint16_t *data, uint32_t count)
{
    ata_device_set_sector_count16(ata_device, sectors);
    ata_device_set_lba48(ata_device, lba);
    return ata_device_command_pio_data_out(ata_device, ATA_WRITE_SECTORS_EXT, data, count);
}

int
ata_device_command_download_microcode(
        ata_device_t *restrict ata_device, uint8_t code, uint16_t sectors, const uint16_t *data, uint32_t count)
{
    ata_device_set_features(ata_device, code);
    ata_device_set_sector_count(ata_device, sectors & 0xff);
    ata_device_set_lba(ata_device, (sectors >> 8) & 0xff);
    return ata_device_command_pio_data_out(ata_device, ATA_DOWNLOAD_MICROCODE, data, count);
}

int
ata_device_command_nop(ata_device_t *restrict ata_device, uint8_t code)
{
    ata_device_set_features(ata_device, code);
    return ata_device_command_non_data(ata_device, ATA_NOP);
}

int
ata_device_command_read_buffer(ata_device_t *restrict ata_device, uint16_t *data, uint32_t count)
{
    return ata_device_command_pio_data_in(ata_device, ATA_READ_BUFFER, data, count);
}

int
ata_device_command_write_buffer(ata_device_t *restrict ata_device, const uint16_t *data, uint32_t count)
{
    return ata_device_command_pio_data_out(ata_device, ATA_WRITE_BUFFER, data, count);
}

int
ata_device_command_dma(ata_device_t *restrict ata_device, uint16_t command)
{
    /* Disable interrupts */
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num + 1, ATA_DEVICE_CONTROL, ATA_nIEN);
    /* Write the command code to the Command register */
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num, ATA_COMMAND, command);
    /* Enable the bus master operation of the controller */
    pci_device_region_write8(ata_device->pci_device, 4, BM_IDE_COMMAND0,
            pci_device_region_read8(ata_device->pci_device, 4, BM_IDE_COMMAND0) | BM_IDE_START);
    /* Poll the device status/clear the interrupt pending */
    clock_t start = clock();
    for (;;) {
        /* Has the command been completed? */
        ata_device->status = pci_device_region_read8(ata_device->pci_device, ata_device->region_num, ATA_STATUS);
        if ((ata_device->status & (ATA_BSY | ATA_DRQ)) == 0) {
            break;
        }

        /* Has the command timed out? */
        clock_t end = clock();
        if ((((double)(end - start)) / CLOCKS_PER_SEC) > ata_device->timeout) {
            ata_device_software_reset(ata_device);
            break;
        }
    }

    /* Has a device fault occurred? */
    if (ata_device->status & ATA_DF) {
        goto err;
    }

    /* Has an error occurred? */
    if (ata_device->status & ATA_ERR) {
        ata_device->error = pci_device_region_read8(ata_device->pci_device, ata_device->region_num, ATA_ERROR);
        goto err;
    }

    /* Reset the direction of the bus master transfer, and disable the bus
       master operation of the controller. */
    pci_device_region_write8(ata_device->pci_device, 4, BM_IDE_COMMAND0,
            pci_device_region_read8(ata_device->pci_device, 4, BM_IDE_COMMAND0) & ~BM_IDE_START);
    return 0;

err:
    /* Reset the direction of the bus master transfer, and disable the bus
       master operation of the controller. */
    pci_device_region_write8(ata_device->pci_device, 4, BM_IDE_COMMAND0,
            pci_device_region_read8(ata_device->pci_device, 4, BM_IDE_COMMAND0) & ~BM_IDE_START);
    ata_device_software_reset(ata_device);
    return -1;
}

int
ata_device_command_non_data(ata_device_t *restrict ata_device, uint16_t command)
{
    /* Disable interrupts */
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num + 1, ATA_DEVICE_CONTROL, ATA_nIEN);
    /* Write the command code to the Command register */
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num, ATA_COMMAND, command);
    /* Poll the device status/clear the interrupt pending */
    clock_t start = clock();
    for (;;) {
        /* Has the command been completed? */
        ata_device->status = pci_device_region_read8(ata_device->pci_device, ata_device->region_num, ATA_STATUS);
        if ((ata_device->status & (ATA_BSY | ATA_DRQ)) == 0) {
            break;
        }

        /* Has the command timed out? */
        clock_t end = clock();
        if ((((double)(end - start)) / CLOCKS_PER_SEC) > ata_device->timeout) {
            ata_device_software_reset(ata_device);
            break;
        }
    }

    /* Has a device fault occurred? */
    if (ata_device->status & ATA_DF) {
        goto err;
    }

    /* Has an error occurred? */
    if (ata_device->status & ATA_ERR) {
        ata_device->error = pci_device_region_read8(ata_device->pci_device, ata_device->region_num, ATA_ERROR);
        goto err;
    }

    return 0;

err:
    ata_device_software_reset(ata_device);
    return -1;
}

int
ata_device_command_pio_data_in(ata_device_t *restrict ata_device, uint16_t command, uint16_t *data, uint32_t count)
{
    /* Disable interrupts */
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num + 1, ATA_DEVICE_CONTROL, ATA_nIEN);
    /* Write the command code to the Command register */
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num, ATA_COMMAND, command);
    /* Poll the device status/clear the interrupt pending */
    clock_t start = clock();
    /* Don't use the Sector Count to try to discover any out-of-bounds reads and
       writes. Read until the device clears the DRQ bit. */
    for (size_t i = 0;;) {
        /* Is the device ready to transfer data? */
        ata_device->status = pci_device_region_read8(ata_device->pci_device, ata_device->region_num, ATA_STATUS);
        if ((ata_device->status & (ATA_BSY | ATA_DRQ)) == ATA_DRQ) {
            /* Transfer data */
            /* @todo Investigate why Hyper-V isn't happy with REP INS/OUTS */
            for (size_t j = 0; j < 256; ++i, ++j) {
                data[i] = pci_device_region_read16(ata_device->pci_device, ata_device->region_num, ATA_DATA);
            }
        }
        /* Has the command been completed? */
        if ((ata_device->status & (ATA_BSY | ATA_DRQ)) == 0) {
            break;
        }

        /* Has the command timed out? */
        clock_t end = clock();
        if ((((double)(end - start)) / CLOCKS_PER_SEC) > ata_device->timeout) {
            ata_device_software_reset(ata_device);
            break;
        }
    }

    /* Has a device fault occurred? */
    if (ata_device->status & ATA_DF) {
        goto err;
    }

    /* Has an error occurred? */
    if (ata_device->status & ATA_ERR) {
        ata_device->error = pci_device_region_read8(ata_device->pci_device, ata_device->region_num, ATA_ERROR);
        goto err;
    }

    return 0;

err:
    ata_device_software_reset(ata_device);
    return -1;
}

int
ata_device_command_pio_data_out(
        ata_device_t *restrict ata_device, uint16_t command, const uint16_t *data, uint32_t count)
{
    /* Disable interrupts */
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num + 1, ATA_DEVICE_CONTROL, ATA_nIEN);
    /* Write the command code to the Command register */
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num, ATA_COMMAND, command);
    /* Poll the device status/clear the interrupt pending */
    clock_t start = clock();
    /* Don't use the Sector Count to try to discover any out-of-bounds reads and
       writes. Write until the device clears the DRQ bit. */
    for (size_t i = 0;;) {
        /* Is the device ready to transfer data? */
        ata_device->status = pci_device_region_read8(ata_device->pci_device, ata_device->region_num, ATA_STATUS);
        if ((ata_device->status & (ATA_BSY | ATA_DRQ)) == ATA_DRQ) {
            /* Transfer data */
            for (size_t j = 0; j < 256; ++i, ++j) {
                pci_device_region_write16(ata_device->pci_device, ata_device->region_num, ATA_DATA, data[i]);
            }
        }

        /* Has the command been completed? */
        if ((ata_device->status & (ATA_BSY | ATA_DRQ)) == 0) {
            break;
        }

        /* Has the command timed out? */
        clock_t end = clock();
        if ((((double)(end - start)) / CLOCKS_PER_SEC) > ata_device->timeout) {
            ata_device_software_reset(ata_device);
            break;
        }
    }

    /* Has a device fault occurred? */
    if (ata_device->status & ATA_DF) {
        goto err;
    }

    /* Has an error occurred? */
    if (ata_device->status & ATA_ERR) {
        ata_device->error = pci_device_region_read8(ata_device->pci_device, ata_device->region_num, ATA_ERROR);
        goto err;
    }

    return 0;

err:
    ata_device_software_reset(ata_device);
    return -1;
}

ata_device_t *
ata_device_create(pci_device_t *pci_device, int bus_num, int timeout)
{
    ata_device_t *ata_device = (ata_device_t *)calloc(1, sizeof(*ata_device));
    if (ata_device == NULL) {
        ata_device_error(ata_device, 0, errno, __func__);
        return NULL;
    }

    /* Is the PCI device an ATA/IDE controller? */
    if (!pci_device_is_ata_controller(pci_device)) {
        ata_device_error(ata_device, 0, 0, "%s: Not an ATA/IDE controller.\n", __func__);
        goto err;
    }

    if (bus_num < 0 || bus_num > 1) {
        errno = EINVAL;
        ata_device_error(ata_device, 0, errno, __func__);
        goto err;
    }

    ata_device->pci_device = pci_device;
    ata_device->region_num = (bus_num ? 2 : 0);
    ata_device->timeout = timeout;
    ata_device->identify_data = (uint16_t *)calloc(256, sizeof(*ata_device->identify_data));
    if (ata_device->identify_data == NULL) {
        ata_device_error(ata_device, 0, errno, __func__);
        goto err;
    }

    /* Is an ATA device? */
    if (ata_device_command_identify_device(ata_device) == -1) {
        ata_device_error(ata_device, 0, errno, __func__);
        goto err;
    }

    return ata_device;

err:
    ata_device_destroy(ata_device);
    return NULL;
}

void
ata_device_destroy(ata_device_t *restrict ata_device)
{
    if (ata_device == NULL) {
        return;
    }

    free(ata_device->identify_data);
    free(ata_device);
}

void
ata_device_error(ata_device_t *restrict ata_device, int status, int error, const char *restrict format, ...)
{
    if (error_handler == NULL) {
        return;
    }

    va_list ap;
    va_start(ap, format);
    (*error_handler)(status, error, format, ap);
    va_end(ap);
}

ata_device_error_handler_t *
ata_device_set_error_handler(ata_device_error_handler_t *handler)
{
    ata_device_error_handler_t *previous_handler = error_handler;
    error_handler = handler;
    return previous_handler;
}

void
ata_device_set_features(ata_device_t *restrict ata_device, uint8_t features)
{
    ata_device->features[1] = ata_device->features[0];
    ata_device->features[0] = features;
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num, ATA_SECTOR_COUNT, ata_device->features[0]);
}

void
ata_device_set_lba(ata_device_t *restrict ata_device, uint32_t lba)
{
    /* Set LBA Low to LBA bits 0 to 7 */
    ata_device->lba_low[1] = ata_device->lba_low[0];
    ata_device->lba_low[0] = lba & 0xff;
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num, ATA_LBA_LOW, ata_device->lba_low[0]);
    /* Set LBA Mid to LBA bits 8 to 15 */
    ata_device->lba_mid[1] = ata_device->lba_mid[0];
    ata_device->lba_mid[0] = (lba >> 8) & 0xff;
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num, ATA_LBA_MID, ata_device->lba_mid[0]);
    /* Set LBA High to LBA bits 16 to 23 */
    ata_device->lba_high[1] = ata_device->lba_high[0];
    ata_device->lba_high[0] = (lba >> 16) & 0xff;
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num, ATA_LBA_HIGH, ata_device->lba_high[0]);
    /* Set Device LBA bit to one to specify the address is an LBA, and set the
       bits 0 to 3 to LBA bits 24 to 27 */
    ata_device->device[1] = ata_device->device[0];
    ata_device->device[0] = (pci_device_region_read8(ata_device->pci_device, ata_device->region_num, ATA_DEVICE) & 0xf0)
                            | ATA_LBA | ((lba >> 24) & 0x0f);
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num, ATA_DEVICE, ata_device->device[0]);
}

void
ata_device_set_lba48(ata_device_t *restrict ata_device, uint64_t lba)
{
    /* Set LBA Low Previous to LBA bits 24 to 31 */
    ata_device->lba_low[1] = (lba >> 24) & 0xff;
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num, ATA_LBA_LOW, ata_device->lba_low[1]);
    /* Set LBA Mid Previous to LBA bits 32 to 39 */
    ata_device->lba_mid[1] = (lba >> 32) & 0xff;
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num, ATA_LBA_MID, ata_device->lba_mid[1]);
    /* Set LBA High Previous to LBA bits 40 to 47 */
    ata_device->lba_high[1] = (lba >> 40) & 0xff;
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num, ATA_LBA_HIGH, ata_device->lba_high[1]);
    /* Set LBA Low Current to LBA bits 0 to 7 */
    ata_device->lba_low[0] = lba & 0xff;
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num, ATA_LBA_LOW, ata_device->lba_low[0]);
    /* Set LBA Mid Current to LBA bits 8 to 15 */
    ata_device->lba_mid[0] = (lba >> 8) & 0xff;
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num, ATA_LBA_MID, ata_device->lba_mid[0]);
    /* Set LBA High Current to LBA bits 16 to 23 */
    ata_device->lba_high[0] = (lba >> 16) & 0xff;
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num, ATA_LBA_HIGH, ata_device->lba_high[0]);
    /* Set Device LBA bit to one to specify the address is an LBA */
    ata_device->device[1] = ata_device->device[0];
    ata_device->device[0]
            = (pci_device_region_read8(ata_device->pci_device, ata_device->region_num, ATA_DEVICE) & 0xf0) | ATA_LBA;
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num, ATA_DEVICE, ata_device->device[0]);
}

void
ata_device_set_sector_count(ata_device_t *restrict ata_device, uint8_t sectors)
{
    ata_device->sector_count[1] = ata_device->sector_count[0];
    ata_device->sector_count[0] = sectors;
    pci_device_region_write8(
            ata_device->pci_device, ata_device->region_num, ATA_SECTOR_COUNT, ata_device->sector_count[0]);
}

void
ata_device_set_sector_count16(ata_device_t *restrict ata_device, uint16_t sectors)
{
    /* Set Sector Count Previous to Sector Count bits 8 to 15 */
    ata_device->sector_count[1] = (sectors >> 8) & 0xff;
    pci_device_region_write8(
            ata_device->pci_device, ata_device->region_num, ATA_SECTOR_COUNT, ata_device->sector_count[1]);
    /* Set Sector Count Current to Sector Count bits 0 to 7 */
    ata_device->sector_count[0] = sectors & 0xff;
    pci_device_region_write8(
            ata_device->pci_device, ata_device->region_num, ATA_SECTOR_COUNT, ata_device->sector_count[0]);
}

void
ata_device_software_reset(ata_device_t *restrict ata_device)
{
    /* Request the devices to perform the software reset */
    pci_device_region_write8(
            ata_device->pci_device, ata_device->region_num + 1, ATA_DEVICE_CONTROL, ATA_nIEN | ATA_SRST);
    /* Reset Device Control SRST bit to zero after software reset */
    pci_device_region_write8(ata_device->pci_device, ata_device->region_num + 1, ATA_DEVICE_CONTROL, ATA_nIEN);
    /* Wait until the software reset has been completed */
    for (;;) {
        /* Is the device busy? */
        /* Don't use ata_device->status so the user can get the status after a
           command has been completed. */
        uint16_t status = pci_device_region_read8(ata_device->pci_device, ata_device->region_num, ATA_STATUS);
        if ((status & ATA_BSY) == 0) {
            break;
        }
    }
}
