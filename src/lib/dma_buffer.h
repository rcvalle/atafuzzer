/** @file */

#ifndef DMA_BUFFER_H
#define DMA_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct _dma_buffer dma_buffer_t; /**< DMA buffer. */

typedef void dma_buffer_error_handler_t(int status, int error, const char *restrict format, va_list ap);
/**
 * Creates an DMA buffer.
 *
 * @param [in] size Size.
 * @return A DMA buffer.
 */
dma_buffer_t *dma_buffer_create(size_t size);

/**
 * Destroys the DMA buffer.
 *
 * @param [in] dma_buffer DMA buffer.
 */
void dma_buffer_destroy(dma_buffer_t *restrict dma_buffer);

/**
 * Returns the size of the DMA buffer.
 *
 * @param [in] dma_buffer DMA buffer.
 * @return Size.
 */
size_t dma_buffer_get_size(dma_buffer_t *restrict dma_buffer);

/**
 * Returns the physical address of the DMA buffer.
 *
 * @param [in] dma_buffer DMA buffer.
 * @return Physical address.
 */
uint64_t dma_buffer_get_phys_addr(dma_buffer_t *restrict dma_buffer);

/**
 * Returns whether the DMA buffer is enabled.
 *
 * @return Returns true if the DMA buffer is enabled; otherwise, returns false
 *   if the DMA buffer is not enabled.
 */
bool dma_buffer_is_enabled();

/**
 * Maps the DMA buffer into memory.
 *
 * @param [in] dma_buffer DMA buffer.
 * @param [in] prot Protection flags.
 * @return Address of mapped DMA buffer.
 */
void *dma_buffer_map(dma_buffer_t *restrict dma_buffer, int prot);

/**
 * Sets the error handler for the DMA buffer.
 *
 * @param [in] handler Error handler.
 * @return Previous error handler.
 */
dma_buffer_error_handler_t *dma_buffer_set_error_handler(dma_buffer_error_handler_t *handler);

/**
 * Unmaps the DMA buffer from memory.
 *
 * @param [in] dma_buffer DMA buffer.
 * @param [in] addr Address of mapped DMA buffer.
 * @return Address of mapped DMA buffer.
 */
void dma_buffer_unmap(dma_buffer_t *restrict dma_buffer, void *addr);

#ifdef __cplusplus
}
#endif

#endif /* DMA_BUFFER_H */
