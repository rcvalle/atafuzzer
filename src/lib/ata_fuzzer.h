/** @file */

#ifndef ATA_FUZZER_H
#define ATA_FUZZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ata_controller.h"

#include <stdarg.h>
#include <stdio.h>

#define ATA_FUZZER_MAX_INPUT (26 + (sizeof(uint16_t) * UINT16_MAX))

typedef struct _ata_fuzzer ata_fuzzer_t; /**< ATA fuzzer. */

typedef void ata_fuzzer_error_handler_t(int status, int error, const char *restrict format, va_list ap);
typedef void ata_fuzzer_log_handler_t(FILE *restrict stream, const char *restrict format, va_list ap);

/**
 * Creates an ATA fuzzer.
 *
 * @param [in] ata_controller ATA controller.
 * @param [in] device_num ATA device number. Use 0 for Device 0, or 1 for Device
 *   1.
 * @return An ATA fuzzer.
 */
ata_fuzzer_t *ata_fuzzer_create(ata_controller_t *restrict ata_controller, int device_num);

/**
 * Destroys the ATA fuzzer.
 *
 * @param [in] ata_fuzzer ATA fuzzer.
 */
void ata_fuzzer_destroy(ata_fuzzer_t *restrict ata_fuzzer);

/**
 * Performs an iteration.
 *
 * @param [in] ata_fuzzer ATA fuzzer.
 * @param [in] stream Input stream.
 */
void ata_fuzzer_iterate(ata_fuzzer_t *restrict ata_fuzzer, FILE *restrict stream);

/**
 * Sets the error handler for the ATA fuzzer.
 *
 * @param [in] handler Error handler.
 * @return Previous error handler.
 */
ata_fuzzer_error_handler_t *ata_fuzzer_set_error_handler(ata_fuzzer_error_handler_t *handler);

/**
 * Sets the log handler for the ATA fuzzer.
 *
 * @param [in] handler Log handler.
 * @return Previous log handler.
 */
ata_fuzzer_log_handler_t *ata_fuzzer_set_log_handler(
        ata_fuzzer_t *restrict ata_fuzzer, ata_fuzzer_log_handler_t *handler);

/**
 * Sets the log stream for the ATA fuzzer.
 *
 * @param [in] stream Log stream.
 * @return Previous log stream.
 */
FILE *ata_fuzzer_set_log_stream(ata_fuzzer_t *restrict ata_fuzzer, FILE *stream);

#ifdef __cplusplus
}
#endif

#endif /* ATA_FUZZER_H */
