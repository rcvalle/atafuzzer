/** @file */

#include "ata_controller.h"

#include "ata.h"
#include "ata_device.h"
#include "bus_master.h"
#include "dma_buffer.h"
#include "pci_device.h"

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>

struct _ata_controller {
    int bus;
    int device;
    int function;
    bool is_dma_enabled;
    pci_device_t *pci_device;
    ata_device_t *ata_device;
    ata_device_t *ata_device0;
    ata_device_t *ata_device1;
    dma_buffer_t *dma_buffer0;
    dma_buffer_t *dma_buffer1;
    struct prd *prdt;
    void *buffer;
};

static ata_controller_error_handler_t *error_handler = NULL;

void ata_controller_error(
        ata_controller_t *restrict ata_controller, int status, int error, const char *restrict format, ...);
void ata_controller_prepare_prdt(ata_controller_t *restrict ata_controller, uint32_t count);

int
ata_controller_command_execute_device_diagnostic(ata_controller_t *restrict ata_controller)
{
    return ata_device_command_execute_device_diagnostic(ata_controller->ata_device);
}

int
ata_controller_command_flush_cache(ata_controller_t *restrict ata_controller)
{
    return ata_device_command_flush_cache(ata_controller->ata_device);
}

int
ata_controller_command_flush_cache_ext(ata_controller_t *restrict ata_controller)
{
    return ata_device_command_flush_cache_ext(ata_controller->ata_device);
}

int
ata_controller_command_identify_device(ata_controller_t *restrict ata_controller)
{
    return ata_device_command_identify_device(ata_controller->ata_device);
}

int
ata_controller_command_read_dma(
        ata_controller_t *restrict ata_controller, uint8_t sectors, uint32_t lba, uint16_t *data, uint32_t count)
{
    /* Prepare the Physical Region Descriptor Table (PRDT) */
    ata_controller_prepare_prdt(ata_controller, count);
    /* Set the PRDT Pointer to the PRDT address */
    pci_device_region_write32(
            ata_controller->pci_device, 4, BM_IDE_PRDT0, dma_buffer_get_phys_addr(ata_controller->dma_buffer0));
    /* Set the direction of the bus master transfer */
    pci_device_region_write8(ata_controller->pci_device, 4, BM_IDE_COMMAND0,
            pci_device_region_read8(ata_controller->pci_device, 4, BM_IDE_COMMAND0) | BM_IDE_WRITE);
    /* Send the DMA transfer command to the device */
    return ata_device_command_read_dma(ata_controller->ata_device, sectors, lba);
}

int
ata_controller_command_read_dma_ext(
        ata_controller_t *restrict ata_controller, uint16_t sectors, uint64_t lba, uint16_t *data, uint32_t count)
{
    /* Prepare the Physical Region Descriptor Table (PRDT) */
    ata_controller_prepare_prdt(ata_controller, count);
    /* Set the PRDT Pointer to the PRDT address */
    pci_device_region_write32(
            ata_controller->pci_device, 4, BM_IDE_PRDT0, dma_buffer_get_phys_addr(ata_controller->dma_buffer0));
    /* Set the direction of the bus master transfer */
    pci_device_region_write8(ata_controller->pci_device, 4, BM_IDE_COMMAND0,
            pci_device_region_read8(ata_controller->pci_device, 4, BM_IDE_COMMAND0) | BM_IDE_WRITE);
    /* Send the DMA transfer command to the device */
    return ata_device_command_read_dma_ext(ata_controller->ata_device, sectors, lba);
}

int
ata_controller_command_read_multiple(
        ata_controller_t *restrict ata_controller, uint8_t sectors, uint32_t lba, uint16_t *data, uint32_t count)
{
    return ata_device_command_read_multiple(ata_controller->ata_device, sectors, lba, data, count);
}

int
ata_controller_command_read_multiple_ext(
        ata_controller_t *restrict ata_controller, uint16_t sectors, uint64_t lba, uint16_t *data, uint32_t count)
{
    return ata_device_command_read_multiple_ext(ata_controller->ata_device, sectors, lba, data, count);
}

int
ata_controller_command_read_sectors(
        ata_controller_t *restrict ata_controller, uint8_t sectors, uint32_t lba, uint16_t *data, uint32_t count)
{
    return ata_device_command_read_sectors(ata_controller->ata_device, sectors, lba, data, count);
}

int
ata_controller_command_read_sectors_ext(
        ata_controller_t *restrict ata_controller, uint16_t sectors, uint64_t lba, uint16_t *data, uint32_t count)
{
    return ata_device_command_read_sectors_ext(ata_controller->ata_device, sectors, lba, data, count);
}

int
ata_controller_command_read_verify_sectors(ata_controller_t *restrict ata_controller, uint8_t sectors, uint32_t lba)
{
    return ata_device_command_read_verify_sectors(ata_controller->ata_device, sectors, lba);
}

int
ata_controller_command_read_verify_sectors_ext(
        ata_controller_t *restrict ata_controller, uint16_t sectors, uint64_t lba)
{
    return ata_device_command_read_verify_sectors_ext(ata_controller->ata_device, sectors, lba);
}

int
ata_controller_command_seek(ata_controller_t *restrict ata_controller, uint32_t lba)
{
    return ata_device_command_seek(ata_controller->ata_device, lba);
}

int
ata_controller_command_set_features(ata_controller_t *restrict ata_controller, uint8_t code, const uint8_t specific[])
{
    return ata_device_command_set_features(ata_controller->ata_device, code, specific);
}

int
ata_controller_command_set_multiple_mode(ata_controller_t *restrict ata_controller, uint8_t sectors)
{
    return ata_device_command_set_multiple_mode(ata_controller->ata_device, sectors);
}

int
ata_controller_command_write_dma(
        ata_controller_t *restrict ata_controller, uint8_t sectors, uint32_t lba, const uint16_t *data, uint32_t count)
{
    /* Copy data to the DMA buffer */
    memcpy(ata_controller->buffer, data, count);
    /* Prepare the Physical Region Descriptor Table (PRDT) */
    ata_controller_prepare_prdt(ata_controller, count);
    /* Set the PRDT Pointer to the PRDT address */
    pci_device_region_write32(
            ata_controller->pci_device, 4, BM_IDE_PRDT0, dma_buffer_get_phys_addr(ata_controller->dma_buffer0));
    /* Set the direction of the bus master transfer */
    pci_device_region_write8(ata_controller->pci_device, 4, BM_IDE_COMMAND0,
            pci_device_region_read8(ata_controller->pci_device, 4, BM_IDE_COMMAND0) & ~BM_IDE_WRITE);
    /* Send the DMA transfer command to the device */
    return ata_device_command_write_dma(ata_controller->ata_device, sectors, lba);
}

int
ata_controller_command_write_dma_ext(
        ata_controller_t *restrict ata_controller, uint16_t sectors, uint64_t lba, const uint16_t *data, uint32_t count)
{
    /* Copy data to the DMA buffer */
    memcpy(ata_controller->buffer, data, count);
    /* Prepare the Physical Region Descriptor Table (PRDT) */
    ata_controller_prepare_prdt(ata_controller, count);
    /* Set the PRDT Pointer to the PRDT address */
    pci_device_region_write32(
            ata_controller->pci_device, 4, BM_IDE_PRDT0, dma_buffer_get_phys_addr(ata_controller->dma_buffer0));
    /* Set the direction of the bus master transfer */
    pci_device_region_write8(ata_controller->pci_device, 4, BM_IDE_COMMAND0,
            pci_device_region_read8(ata_controller->pci_device, 4, BM_IDE_COMMAND0) & ~BM_IDE_WRITE);
    /* Send the DMA transfer command to the device */
    return ata_device_command_write_dma_ext(ata_controller->ata_device, sectors, lba);
}

int
ata_controller_command_write_multiple(
        ata_controller_t *restrict ata_controller, uint8_t sectors, uint32_t lba, const uint16_t *data, uint32_t count)
{
    return ata_device_command_write_multiple(ata_controller->ata_device, sectors, lba, data, count);
}

int
ata_controller_command_write_multiple_ext(
        ata_controller_t *restrict ata_controller, uint16_t sectors, uint64_t lba, const uint16_t *data, uint32_t count)
{
    return ata_device_command_write_multiple_ext(ata_controller->ata_device, sectors, lba, data, count);
}

int
ata_controller_command_write_sectors(
        ata_controller_t *restrict ata_controller, uint8_t sectors, uint32_t lba, const uint16_t *data, uint32_t count)
{
    return ata_device_command_write_sectors(ata_controller->ata_device, sectors, lba, data, count);
}

int
ata_controller_command_write_sectors_ext(
        ata_controller_t *restrict ata_controller, uint16_t sectors, uint64_t lba, const uint16_t *data, uint32_t count)
{
    return ata_device_command_write_sectors_ext(ata_controller->ata_device, sectors, lba, data, count);
}

int
ata_controller_command_download_microcode(
        ata_controller_t *restrict ata_controller, uint8_t code, uint16_t sectors, const uint16_t *data, uint32_t count)
{
    return ata_device_command_download_microcode(ata_controller->ata_device, code, sectors, data, count);
}

int
ata_controller_command_nop(ata_controller_t *restrict ata_controller, uint8_t code)
{
    return ata_device_command_nop(ata_controller->ata_device, code);
}

int
ata_controller_command_read_buffer(ata_controller_t *restrict ata_controller, uint16_t *data, uint32_t count)
{
    return ata_device_command_read_buffer(ata_controller->ata_device, data, count);
}

int
ata_controller_command_write_buffer(ata_controller_t *restrict ata_controller, const uint16_t *data, uint32_t count)
{
    return ata_device_command_write_buffer(ata_controller->ata_device, data, count);
}

ata_controller_t *
ata_controller_create(int bus, int device, int function, int bus_num, int timeout)
{
    ata_controller_t *ata_controller = (ata_controller_t *)calloc(1, sizeof(*ata_controller));
    if (ata_controller == NULL) {
        ata_controller_error(ata_controller, 0, errno, __func__);
        return NULL;
    }

    if (bus_num < 0 || bus_num > 1) {
        errno = EINVAL;
        ata_controller_error(ata_controller, 0, errno, __func__);
        goto err;
    }

    ata_controller->bus = bus;
    ata_controller->device = device;
    ata_controller->function = function;
    ata_controller->pci_device
            = pci_device_create(ata_controller->bus, ata_controller->device, ata_controller->function);
    if (ata_controller->pci_device == NULL) {
        ata_controller_error(ata_controller, 0, errno, __func__);
        goto err;
    }

    /* Is the PCI device an ATA/IDE controller? */
    if (!pci_device_is_ata_controller(ata_controller->pci_device)) {
        ata_controller_error(ata_controller, 0, 0, "%s: Not an ATA/IDE controller.\n", __func__);
        goto err;
    }

    ata_controller_device_select(ata_controller, 0);
    ata_controller->ata_device0 = ata_device_create(ata_controller->pci_device, bus_num, timeout);
    if (ata_controller->ata_device0 == NULL) {
        ata_controller_error(ata_controller, 0, errno, __func__);
        goto err;
    }

    ata_controller_device_select(ata_controller, 1);
    ata_controller->ata_device1 = ata_device_create(ata_controller->pci_device, bus_num, timeout);
    if (ata_controller->ata_device1 == NULL) {
        ata_controller_error(ata_controller, 0, errno, __func__);
        goto err;
    }

    if (ata_controller->ata_device0 == NULL && ata_controller->ata_device1 == NULL) {
        ata_controller_error(ata_controller, 0, 0, "%s: No ATA device.\n", __func__);
        goto err;
    }

    ata_controller->ata_device = ata_controller->ata_device0;
    ata_controller_device_select(ata_controller, 0);
    if (ata_controller->ata_device0 == NULL) {
        ata_controller->ata_device = ata_controller->ata_device1;
        ata_controller_device_select(ata_controller, 1);
    }

    if (dma_buffer_is_enabled()) {
        ata_controller->is_dma_enabled = true;

        ata_controller->dma_buffer0 = dma_buffer_create(BM_IDE_MAX_PRDT_SIZE);
        if (ata_controller->dma_buffer0 == NULL) {
            ata_controller_error(ata_controller, 0, errno, __func__);
            goto err;
        }

        ata_controller->prdt = dma_buffer_map(ata_controller->dma_buffer0, PROT_READ | PROT_WRITE);
        if (ata_controller->prdt == NULL) {
            ata_controller_error(ata_controller, 0, errno, __func__);
            goto err;
        }

        ata_controller->dma_buffer1 = dma_buffer_create(0x10000);
        if (ata_controller->dma_buffer1 == NULL) {
            ata_controller_error(ata_controller, 0, errno, __func__);
            goto err;
        }

        ata_controller->buffer = dma_buffer_map(ata_controller->dma_buffer1, PROT_READ | PROT_WRITE);
        if (ata_controller->buffer == NULL) {
            ata_controller_error(ata_controller, 0, errno, __func__);
            goto err;
        }
    }

    return ata_controller;

err:
    ata_controller_destroy(ata_controller);
    free(ata_controller);
    return NULL;
}

void
ata_controller_destroy(ata_controller_t *restrict ata_controller)
{
    if (ata_controller == NULL) {
        return;
    }

    dma_buffer_unmap(ata_controller->dma_buffer1, ata_controller->buffer);
    dma_buffer_unmap(ata_controller->dma_buffer0, ata_controller->prdt);
    dma_buffer_destroy(ata_controller->dma_buffer1);
    dma_buffer_destroy(ata_controller->dma_buffer0);
    ata_device_destroy(ata_controller->ata_device1);
    ata_device_destroy(ata_controller->ata_device0);
    pci_device_destroy(ata_controller->pci_device);
    free(ata_controller);
}

void
ata_controller_device_reset(ata_controller_t *restrict ata_controller)
{
    /* Request the devices to perform the software reset */
    pci_device_region_write8(ata_controller->pci_device, 1, ATA_DEVICE_CONTROL, ATA_nIEN | ATA_SRST);
    /* Reset Device Control SRST bit to zero after software reset */
    pci_device_region_write8(ata_controller->pci_device, 1, ATA_DEVICE_CONTROL, ATA_nIEN);
    /* Wait until the software reset has been completed */
    for (;;) {
        /* Is the device busy? */
        uint16_t status = pci_device_region_read8(ata_controller->pci_device, 0, ATA_STATUS);
        if ((status & ATA_BSY) == 0) {
            break;
        }
    }
}

void
ata_controller_device_select(ata_controller_t *restrict ata_controller, int device_num)
{
    if (device_num < 0 || device_num > 1) {
        errno = EINVAL;
        ata_controller_error(ata_controller, 0, errno, __func__);
        return;
    }

    if (ata_controller->ata_device0 == NULL || ata_controller->ata_device1 == NULL) {
        return;
    }

    ata_controller->ata_device = (device_num ? ata_controller->ata_device1 : ata_controller->ata_device0);
    if (device_num == 1) {
        pci_device_region_write8(ata_controller->pci_device, 0, ATA_DEVICE,
                pci_device_region_read8(ata_controller->pci_device, 0, ATA_DEVICE) | ATA_DEV);
    } else {
        pci_device_region_write8(ata_controller->pci_device, 0, ATA_DEVICE,
                pci_device_region_read8(ata_controller->pci_device, 0, ATA_DEVICE) & ~ATA_DEV);
    }

    /* Wait until the device select has been completed */
    for (;;) {
        /* Is the device busy? */
        uint16_t status = pci_device_region_read8(ata_controller->pci_device, 0, ATA_STATUS);
        if ((status & ATA_BSY) == 0) {
            break;
        }
    }
}

void
ata_controller_error(ata_controller_t *restrict ata_controller, int status, int error, const char *restrict format, ...)
{
    if (error_handler == NULL) {
        return;
    }

    va_list ap;
    va_start(ap, format);
    (*error_handler)(status, error, format, ap);
    va_end(ap);
}

bool
ata_controller_is_dma_enabled(ata_controller_t *restrict ata_controller)
{
    return ata_controller->is_dma_enabled;
}

void
ata_controller_prepare_prdt(ata_controller_t *restrict ata_controller, uint32_t count)
{
    uint32_t prdt_address = dma_buffer_get_phys_addr(ata_controller->dma_buffer1);
    uint32_t prdt_count = count;
    for (size_t i = 0; i < BM_IDE_MAX_NUM_PRDS; ++i) {
        /* Calculate the maximum number of bytes that can be transferred */
        uint32_t max = 0x10000 - (prdt_address & 0xffff);
        uint32_t count = (prdt_count > max) ? max : prdt_count;
        ata_controller->prdt[i].address = prdt_address;
        ata_controller->prdt[i].count = count;
        prdt_address += count;
        prdt_count -= count;
        /* Is the last PRD? */
        if (prdt_count > 0) {
            ata_controller->prdt[i].reserved = 0;
        } else {
            ata_controller->prdt[i].reserved |= 1 << 16;
            break;
        }
    }
}

ata_controller_error_handler_t *
ata_controller_set_error_handler(ata_controller_error_handler_t *handler)
{
    ata_controller_error_handler_t *previous_handler = error_handler;
    error_handler = handler;
    return previous_handler;
}
