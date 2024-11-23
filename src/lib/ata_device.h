/** @file */

#ifndef ATA_DEVICE_H
#define ATA_DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pci_device.h"

#include <stdarg.h>
#include <stdint.h>

typedef struct _ata_device ata_device_t; /**< ATA device. */

typedef void ata_device_error_handler_t(int status, int error, const char *restrict format, va_list ap);

/**
 * Requests the devices to perform the internal diagnostic tests.
 *
 * @param [in] ata_device ATA device.
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_execute_device_diagnostic(ata_device_t *restrict ata_device);

/**
 * Requests the device to flush the write cache.
 *
 * @param [in] ata_device ATA device.
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_flush_cache(ata_device_t *restrict ata_device);

/**
 * Requests the device to flush the write cache.
 *
 * @param [in] ata_device ATA device.
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_flush_cache_ext(ata_device_t *restrict ata_device);

/**
 * Requests identification data from the device.
 *
 * @param [in] ata_device ATA device.
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_identify_device(ata_device_t *restrict ata_device);

/**
 * Reads data using direct memory access (DMA) data transfer.
 *
 * @param [in] ata_device ATA device.
 * @param [in] sectors Number of sectors to be transferred. A value of zero
 *   specifies that 256 sectors are to be transferred.
 * @param [in] lba Logical block address (LBA).
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_read_dma(ata_device_t *restrict ata_device, uint8_t sectors, uint32_t lba);

/**
 * Reads data using direct memory access (DMA) data transfer.
 *
 * @param [in] ata_device ATA device.
 * @param [in] sectors Number of sectors to be transferred. A value of zero
 *   specifies that 65,536 sectors are to be transferred.
 * @param [in] lba 48-bit logical block address (LBA).
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_read_dma_ext(ata_device_t *restrict ata_device, uint16_t sectors, uint64_t lba);

/**
 * Reads the number of sectors specified in the Sector Count register.
 *
 * @param [in] ata_device ATA device.
 * @param [in] sectors Number of sectors to be transferred. A value of zero
 *   specifies that 256 sectors are to be transferred.
 * @param [in] lba Logical block address (LBA).
 * @param [out] data Data.
 * @param [in] count Number of bytes to be transferred.
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_read_multiple(
        ata_device_t *restrict ata_device, uint8_t sectors, uint32_t lba, uint16_t *data, uint32_t count);

/**
 * Reads the number of sectors specified in the Sector Count register.
 *
 * @param [in] ata_device ATA device.
 * @param [in] sectors Number of sectors to be transferred. A value of zero
 *   specifies that 65,536 sectors are to be transferred.
 * @param [in] lba 48-bit logical block address (LBA).
 * @param [out] data Data.
 * @param [in] count Number of bytes to be transferred.
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_read_multiple_ext(
        ata_device_t *restrict ata_device, uint16_t sectors, uint64_t lba, uint16_t *data, uint32_t count);

/**
 * Reads the number of sectors specified in the Sector Count register.
 *
 * @param [in] ata_device ATA device.
 * @param [in] sectors Number of sectors to be transferred. A value of zero
 *   specifies that 256 sectors are to be transferred.
 * @param [in] lba Logical block address (LBA).
 * @param [out] data Data.
 * @param [in] count Number of bytes to be transferred.
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_read_sectors(
        ata_device_t *restrict ata_device, uint8_t sectors, uint32_t lba, uint16_t *data, uint32_t count);

/**
 * Reads the number of sectors specified in the Sector Count register.
 *
 * @param [in] ata_device ATA device.
 * @param [in] sectors Number of sectors to be transferred. A value of zero
 *   specifies that 65,536 sectors are to be transferred.
 * @param [in] lba 48-bit logical block address (LBA).
 * @param [out] data Data.
 * @param [in] count Number of bytes to be transferred.
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_read_sectors_ext(
        ata_device_t *restrict ata_device, uint16_t sectors, uint64_t lba, uint16_t *data, uint32_t count);

/**
 * Reads the number of sectors specified in the Sector Count register without
 * transferring data.
 *
 * @param [in] ata_device ATA device.
 * @param [in] sectors Number of sectors to be transferred. A value of zero
 *   specifies that 256 sectors are to be transferred.
 * @param [in] lba Logical block address (LBA).
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_read_verify_sectors(ata_device_t *restrict ata_device, uint8_t sectors, uint32_t lba);

/**
 * Reads the number of sectors specified in the Sector Count register without
 * transferring data.
 *
 * @param [in] ata_device ATA device.
 * @param [in] sectors Number of sectors to be transferred. A value of zero
 *   specifies that 65,536 sectors are to be transferred.
 * @param [in] lba 48-bit logical block address (LBA).
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_read_verify_sectors_ext(ata_device_t *restrict ata_device, uint16_t sectors, uint64_t lba);

/**
 * Notifies the device that particular data may be requested in a subsequent
 * command.
 *
 * @param [in] ata_device ATA device.
 * @param [in] lba Logical block address (LBA).
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_seek(ata_device_t *restrict ata_device, uint32_t lba);

/**
 * Sets parameters that affect the execution of certain device features.
 *
 * @param [in] ata_device ATA device.
 * @param [in] code Subcommand code.
 * @param [in] specific Subcommand specific.
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_set_features(ata_device_t *restrict ata_device, uint8_t code, const uint8_t specific[]);

/**
 * Sets the number of sectors per block for the device to be used on all
 * subsequent READ/WRITE MULTIPLE (EXT) commands.
 *
 * @param [in] ata_device ATA device.
 * @param [in] sectors Number of sectors per block. A value of zero specifies
 *   that all subsequent READ/WRITE MULTIPLE (EXT) command are to be aborted.
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_set_multiple_mode(ata_device_t *restrict ata_device, uint8_t sectors);

/**
 * Writes data using direct memory access (DMA) data transfer.
 *
 * @param [in] ata_device ATA device.
 * @param [in] sectors Number of sectors to be transferred. A value of zero
 *   specifies that 256 sectors are to be transferred.
 * @param [in] lba Logical block address (LBA).
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_write_dma(ata_device_t *restrict ata_device, uint8_t sectors, uint32_t lba);

/**
 * Writes data using direct memory access (DMA) data transfer.
 *
 * @param [in] ata_device ATA device.
 * @param [in] sectors Number of sectors to be transferred. A value of zero
 *   specifies that 65,536 sectors are to be transferred.
 * @param [in] lba 48-bit logical block address (LBA).
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_write_dma_ext(ata_device_t *restrict ata_device, uint16_t sectors, uint64_t lba);

/**
 * Writes the number of sectors specified in the Sector Count register.
 *
 * @param [in] ata_device ATA device.
 * @param [in] sectors Number of sectors to be transferred. A value of zero
 *   specifies that 256 sectors are to be transferred.
 * @param [in] lba Logical block address (LBA).
 * @param [in] data Data.
 * @param [in] count Number of bytes to be transferred.
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_write_multiple(
        ata_device_t *restrict ata_device, uint8_t sectors, uint32_t lba, const uint16_t *data, uint32_t count);

/**
 * Writes the number of sectors specified in the Sector Count register.
 *
 * @param [in] ata_device ATA device.
 * @param [in] sectors Number of sectors to be transferred. A value of zero
 *   specifies that 65,536 sectors are to be transferred.
 * @param [in] lba 48-bit logical block address (LBA).
 * @param [in] data Data.
 * @param [in] count Number of bytes to be transferred.
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_write_multiple_ext(
        ata_device_t *restrict ata_device, uint16_t sectors, uint64_t lba, const uint16_t *data, uint32_t count);

/**
 * Writes the number of sectors specified in the Sector Count register.
 *
 * @param [in] ata_device ATA device.
 * @param [in] sectors Number of sectors to be transferred. A value of zero
 *   specifies that 256 sectors are to be transferred.
 * @param [in] lba Logical block address (LBA).
 * @param [in] data Data.
 * @param [in] count Number of bytes to be transferred.
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_write_sectors(
        ata_device_t *restrict ata_device, uint8_t sectors, uint32_t lba, const uint16_t *data, uint32_t count);

/**
 * Writes the number of sectors specified in the Sector Count register.
 *
 * @param [in] ata_device ATA device.
 * @param [in] sectors Number of sectors to be transferred. A value of zero
 *   specifies that 65,536 sectors are to be transferred.
 * @param [in] lba 48-bit logical block address (LBA).
 * @param [in] data Data.
 * @param [in] count Number of bytes to be transferred.
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_write_sectors_ext(
        ata_device_t *restrict ata_device, uint16_t sectors, uint64_t lba, const uint16_t *data, uint32_t count);

/**
 * Updates the microcode of the device.
 *
 * @param [in] ata_device ATA device.
 * @param [in] code Subcommand code.
 * @param [in] sectors Number of sectors to be transferred. A value of zero
 *   specifies that 256 sectors are to be transferred.
 * @param [in] address Logical block address (LBA).
 * @param [in] data Data.
 * @param [in] count Number of bytes to be transferred.
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_download_microcode(
        ata_device_t *restrict ata_device, uint8_t code, uint16_t sectors, const uint16_t *data, uint32_t count);

/**
 * Causes the device to respond with command aborted.
 *
 * @param [in] ata_device ATA device.
 * @param [in] code Subcommand code. For devices implementing the Overlapped
 *   feature set, a subcommand code of zero specifies that any outstanding queue
 *   are to be aborted.
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_nop(ata_device_t *restrict ata_device, uint8_t code);

/**
 * Reads the current sector buffer of the device.
 *
 * @param [in] ata_device ATA device.
 * @param [out] data Data.
 * @param [in] count Number of bytes to be transferred.
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_read_buffer(ata_device_t *restrict ata_device, uint16_t *data, uint32_t count);

/**
 * Writes the current sector buffer of the device.
 *
 * @param [in] ata_device ATA device.
 * @param [in] data Data.
 * @param [in] count Number of bytes to be transferred.
 * @return Returns zero on success; otherwise, returns -1 on failure.
 */
int ata_device_command_write_buffer(ata_device_t *restrict ata_device, const uint16_t *data, uint32_t count);

/**
 * Creates an ATA device.
 *
 * @param [in] pci_device PCI device that is an ATA controller.
 * @param [in] bus_num ATA bus number. Use 0 for primary, or 1 for secondary.
 * @param [in] timeout Timeout, in seconds, for each command.
 * @return An ATA device.
 */
ata_device_t *ata_device_create(pci_device_t *pci_device, int bus_num, int timeout);

/**
 * Destroys the ATA device.
 *
 * @param [in] ata_device ATA device.
 */
void ata_device_destroy(ata_device_t *restrict ata_device);

/**
 * Sets the error handler for the ATA device.
 *
 * @param [in] handler Error handler.
 * @return Previous error handler.
 */
ata_device_error_handler_t *ata_device_set_error_handler(ata_device_error_handler_t *handler);

#ifdef __cplusplus
}
#endif

#endif /* ATA_DEVICE_H */
