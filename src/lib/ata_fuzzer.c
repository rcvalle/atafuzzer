/** @file */

#include "ata_fuzzer.h"

#include "ata_controller.h"
#include "input.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_DATA (sizeof(uint16_t) * UINT16_MAX)

struct _ata_fuzzer {
    ata_controller_t *ata_controller;
    int device_num;
    ata_fuzzer_log_handler_t *log_handler;
    FILE *log_stream;
};

static ata_fuzzer_error_handler_t *error_handler = NULL;

void ata_fuzzer_error(ata_fuzzer_t *restrict ata_fuzzer, int status, int error, const char *restrict format, ...);
void ata_fuzzer_log(ata_fuzzer_t *restrict ata_fuzzer, const char *restrict format, ...);

ata_fuzzer_t *
ata_fuzzer_create(ata_controller_t *restrict ata_controller, int device_num)
{
    ata_fuzzer_t *ata_fuzzer = (ata_fuzzer_t *)calloc(1, sizeof(*ata_fuzzer));
    if (ata_fuzzer == NULL) {
        ata_fuzzer_error(ata_fuzzer, 0, errno, __func__);
        return NULL;
    }

    if (device_num < 0 || device_num > 1) {
        errno = EINVAL;
        ata_fuzzer_error(ata_fuzzer, 0, errno, __func__);
        return NULL;
    }

    ata_fuzzer->ata_controller = ata_controller;
    ata_fuzzer->device_num = device_num;
    return ata_fuzzer;
}

void
ata_fuzzer_destroy(ata_fuzzer_t *restrict ata_fuzzer)
{
    if (ata_fuzzer == NULL) {
        return;
    }

    free(ata_fuzzer);
}

void
ata_fuzzer_error(ata_fuzzer_t *restrict ata_fuzzer, int status, int error, const char *restrict format, ...)
{
    if (error_handler == NULL) {
        return;
    }

    va_list ap;
    va_start(ap, format);
    (*error_handler)(status, error, format, ap);
    va_end(ap);
}

void
ata_fuzzer_iterate(ata_fuzzer_t *restrict ata_fuzzer, FILE *restrict stream)
{
    uint16_t data[MAX_DATA];
    ata_controller_device_reset(ata_fuzzer->ata_controller);
    ata_controller_device_select(ata_fuzzer->ata_controller, ata_fuzzer->device_num);
    switch (input_derive_range(stream, 0, 24)) {
    case 0: {
        ata_fuzzer_log(ata_fuzzer, "s", "command", "EXECUTE DEVICE DIAGNOSTIC");
        ata_controller_command_execute_device_diagnostic(ata_fuzzer->ata_controller);
        break;
    }

    case 1: {
        ata_fuzzer_log(ata_fuzzer, "s", "command", "FLUSH CACHE");
        ata_controller_command_flush_cache(ata_fuzzer->ata_controller);
        break;
    }

    case 2: {
        ata_fuzzer_log(ata_fuzzer, "s", "command", "FLUSH CACHE EXT");
        ata_controller_command_flush_cache_ext(ata_fuzzer->ata_controller);
        break;
    }

    case 3: {
        ata_fuzzer_log(ata_fuzzer, "s", "command", "IDENTIFY DEVICE");
        ata_controller_command_identify_device(ata_fuzzer->ata_controller);
        break;
    }

    case 4: {
        if (!ata_controller_is_dma_enabled(ata_fuzzer->ata_controller)) {
            return;
        }

        uint8_t sectors = input_derive_range(stream, 0, 128);
        uint32_t lba = input_read32(stream);
        uint16_t count = input_read16(stream);
        ata_fuzzer_log(ata_fuzzer, "suupu", "command", "READ DMA", "sectors", sectors, "lba", lba, "data", data,
                "count", count);
        ata_controller_command_read_dma(ata_fuzzer->ata_controller, sectors, lba, data, count);
        break;
    }

    case 5: {
        if (!ata_controller_is_dma_enabled(ata_fuzzer->ata_controller)) {
            return;
        }

        uint16_t sectors = input_derive_range(stream, 0, 128);
        uint64_t lba = input_read64(stream);
        uint16_t count = input_read16(stream);
        ata_fuzzer_log(ata_fuzzer, "suqpu", "command", "READ DMA EXT", "sectors", sectors, "lba", lba, "data", data,
                "count", count);
        ata_controller_command_read_dma_ext(ata_fuzzer->ata_controller, sectors, lba, data, count);
        break;
    }

    case 6: {
        uint8_t sectors = input_derive_range(stream, 0, 128);
        uint32_t lba = input_read32(stream);
        uint16_t count = input_read16(stream);
        ata_fuzzer_log(ata_fuzzer, "suupu", "command", "READ MULTIPLE", "sectors", sectors, "lba", lba, "data", data,
                "count", count);
        ata_controller_command_read_multiple(ata_fuzzer->ata_controller, sectors, lba, data, count);
        break;
    }

    case 7: {
        uint16_t sectors = input_derive_range(stream, 0, 128);
        uint64_t lba = input_read64(stream);
        uint16_t count = input_read16(stream);
        ata_fuzzer_log(ata_fuzzer, "suqpu", "command", "READ MULTIPLE EXT", "sectors", sectors, "lba", lba, "data",
                data, "count", count);
        ata_controller_command_read_multiple_ext(ata_fuzzer->ata_controller, sectors, lba, data, count);
        break;
    }

    case 8: {
        uint8_t sectors = input_derive_range(stream, 0, 128);
        uint32_t lba = input_read32(stream);
        uint16_t count = input_read16(stream);
        ata_fuzzer_log(ata_fuzzer, "suupu", "command", "READ SECTOR(S)", "sectors", sectors, "lba", lba, "data", data,
                "count", count);
        ata_controller_command_read_sectors(ata_fuzzer->ata_controller, sectors, lba, data, count);
        break;
    }

    case 9: {
        uint16_t sectors = input_derive_range(stream, 0, 128);
        uint64_t lba = input_read64(stream);
        uint16_t count = input_read16(stream);
        ata_fuzzer_log(ata_fuzzer, "suqpu", "command", "READ SECTOR(S) EXT", "sectors", sectors, "lba", lba, "data",
                data, "count", count);
        ata_controller_command_read_sectors_ext(ata_fuzzer->ata_controller, sectors, lba, data, count);
        break;
    }

    case 10: {
        uint8_t sectors = input_derive_range(stream, 0, 128);
        uint32_t lba = input_read32(stream);
        ata_fuzzer_log(ata_fuzzer, "suu", "command", "READ VERIFY SECTOR(S)", "sectors", sectors, "lba", lba);
        ata_controller_command_read_verify_sectors(ata_fuzzer->ata_controller, sectors, lba);
        break;
    }

    case 11: {
        uint16_t sectors = input_derive_range(stream, 0, 128);
        uint64_t lba = input_read64(stream);
        ata_fuzzer_log(ata_fuzzer, "suq", "command", "READ VERIFY SECTOR(S) EXT", "sectors", sectors, "lba", lba);
        ata_controller_command_read_verify_sectors_ext(ata_fuzzer->ata_controller, sectors, lba);
        break;
    }

    case 12: {
        uint32_t lba = input_read32(stream);
        ata_fuzzer_log(ata_fuzzer, "su", "command", "SEEK", "lba", lba);
        ata_controller_command_seek(ata_fuzzer->ata_controller, lba);
        break;
    }

    case 13: {
        uint8_t code = input_read8(stream);
        uint8_t specific[4];
        input_read_string8(stream, specific, 4);
        ata_fuzzer_log(ata_fuzzer, "sup", "command", "SET FEATURES", "code", code, "specific", specific);
        ata_controller_command_set_features(ata_fuzzer->ata_controller, code, specific);
        break;
    }

    case 14: {
        uint8_t sectors = input_derive_range(stream, 0, 128);
        ata_fuzzer_log(ata_fuzzer, "su", "command", "SET MULTIPLE MODE", "sectors", sectors);
        ata_controller_command_set_multiple_mode(ata_fuzzer->ata_controller, sectors);
        break;
    }

    case 15: {
        if (!ata_controller_is_dma_enabled(ata_fuzzer->ata_controller)) {
            return;
        }

        uint8_t sectors = input_derive_range(stream, 0, 128);
        uint32_t lba = input_read32(stream);
        uint16_t count = input_read16(stream);
        ata_fuzzer_log(ata_fuzzer, "suupu", "command", "WRITE DMA", "sectors", sectors, "lba", lba, "data", data,
                "count", count);
        ata_controller_command_write_dma(ata_fuzzer->ata_controller, sectors, lba, data, count);
        break;
    }

    case 16: {
        if (!ata_controller_is_dma_enabled(ata_fuzzer->ata_controller)) {
            return;
        }

        uint16_t sectors = input_derive_range(stream, 0, 128);
        uint64_t lba = input_read64(stream);
        uint16_t count = input_read16(stream);
        ata_fuzzer_log(ata_fuzzer, "suqpu", "command", "WRITE DMA EXT", "sectors", sectors, "lba", lba, "data", data,
                "count", count);
        ata_controller_command_write_dma_ext(ata_fuzzer->ata_controller, sectors, lba, data, count);
        break;
    }

    case 17: {
        uint8_t sectors = input_derive_range(stream, 0, 128);
        uint32_t lba = input_read32(stream);
        uint16_t count = input_read16(stream);
        input_read_string16(stream, data, count);
        ata_fuzzer_log(ata_fuzzer, "suupu", "command", "WRITE MULTIPLE", "sectors", sectors, "lba", lba, "data", data,
                "count", count);
        ata_controller_command_write_multiple(ata_fuzzer->ata_controller, sectors, lba, data, count);
        break;
    }

    case 18: {
        uint16_t sectors = input_derive_range(stream, 0, 128);
        uint64_t lba = input_read64(stream);
        uint16_t count = input_read16(stream);
        input_read_string16(stream, data, count);
        ata_fuzzer_log(ata_fuzzer, "suqpu", "command", "WRITE MULTIPLE EXT", "sectors", sectors, "lba", lba, "data",
                data, "count", count);
        ata_controller_command_write_multiple_ext(ata_fuzzer->ata_controller, sectors, lba, data, count);
        break;
    }

    case 19: {
        uint8_t sectors = input_derive_range(stream, 0, 128);
        uint32_t lba = input_read32(stream);
        uint16_t count = input_read16(stream);
        input_read_string16(stream, data, count);
        ata_fuzzer_log(ata_fuzzer, "suupu", "command", "WRITE SECTOR(S)", "sectors", sectors, "lba", lba, "data", data,
                "count", count);
        ata_controller_command_write_sectors(ata_fuzzer->ata_controller, sectors, lba, data, count);
        break;
    }

    case 20: {
        uint16_t sectors = input_derive_range(stream, 0, 128);
        uint64_t lba = input_read64(stream);
        uint16_t count = input_read16(stream);
        input_read_string16(stream, data, count);
        ata_fuzzer_log(ata_fuzzer, "suqpu", "command", "WRITE SECTOR(S) EXT", "sectors", sectors, "lba", lba, "data",
                data, "count", count);
        ata_controller_command_write_sectors_ext(ata_fuzzer->ata_controller, sectors, lba, data, count);
        break;
    }

    case 21: {
        uint8_t code = input_read8(stream);
        uint8_t sectors = input_derive_range(stream, 0, 128);
        uint16_t count = input_read16(stream);
        input_read_string16(stream, data, count);
        ata_fuzzer_log(ata_fuzzer, "suupu", "command", "DOWNLOAD MICROCODE", "code", code, "sectors", sectors, "data",
                data, "count", count);
        ata_controller_command_download_microcode(ata_fuzzer->ata_controller, code, sectors, data, count);
        break;
    }

    case 22: {
        uint8_t code = input_read8(stream);
        ata_fuzzer_log(ata_fuzzer, "su", "command", "NOP", "code", code);
        ata_controller_command_nop(ata_fuzzer->ata_controller, code);
        break;
    }

    case 23: {
        uint16_t count = input_read16(stream);
        ata_fuzzer_log(ata_fuzzer, "spu", "command", "READ BUFFER", "data", data, "count", count);
        ata_controller_command_read_buffer(ata_fuzzer->ata_controller, data, count);
        break;
    }

    case 24: {
        uint16_t count = input_read16(stream);
        input_read_string16(stream, data, count);
        ata_fuzzer_log(ata_fuzzer, "spu", "command", "WRITE BUFFER", "data", data, "count", count);
        ata_controller_command_write_buffer(ata_fuzzer->ata_controller, data, count);
        break;
    }

    default:
        abort();
    }
}

void
ata_fuzzer_log(ata_fuzzer_t *restrict ata_fuzzer, const char *restrict format, ...)
{
    if (ata_fuzzer->log_handler == NULL) {
        return;
    }

    va_list ap;
    va_start(ap, format);
    (*ata_fuzzer->log_handler)(ata_fuzzer->log_stream, format, ap);
    va_end(ap);
}

ata_fuzzer_error_handler_t *
ata_fuzzer_set_error_handler(ata_fuzzer_error_handler_t *handler)
{
    ata_fuzzer_error_handler_t *previous_handler = error_handler;
    error_handler = handler;
    return previous_handler;
}

ata_fuzzer_log_handler_t *
ata_fuzzer_set_log_handler(ata_fuzzer_t *restrict ata_fuzzer, ata_fuzzer_log_handler_t *handler)
{
    ata_fuzzer_log_handler_t *previous_handler = ata_fuzzer->log_handler;
    ata_fuzzer->log_handler = handler;
    return previous_handler;
}

FILE *
ata_fuzzer_set_log_stream(ata_fuzzer_t *restrict ata_fuzzer, FILE *stream)
{
    FILE *previous_stream = ata_fuzzer->log_stream;
    ata_fuzzer->log_stream = stream;
    return previous_stream;
}
