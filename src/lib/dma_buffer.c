/** @file */

#include "dma_buffer.h"

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

static int buffer_num = -1;
static const int num_buffers = 7;

struct _dma_buffer {
    int fd;
    size_t size;
    uint64_t phys_addr;
};

static dma_buffer_error_handler_t *error_handler = NULL;

void dma_buffer_error(dma_buffer_t *restrict dma_buffer, int status, int error, const char *restrict format, ...);

dma_buffer_t *
dma_buffer_create(size_t size)
{
    dma_buffer_t *dma_buffer = (dma_buffer_t *)calloc(1, sizeof(*dma_buffer));
    if (dma_buffer == NULL) {
        dma_buffer_error(dma_buffer, 0, errno, __func__);
        return NULL;
    }

    ++buffer_num;
    if (buffer_num > num_buffers) {
        errno = ENOMEM;
        dma_buffer_error(dma_buffer, 0, errno, __func__);
        goto err;
    }

    char path[PATH_MAX];
    sprintf(path, "/dev/udmabuf%d", buffer_num);
    dma_buffer->fd = open(path, O_RDWR);
    if (dma_buffer->fd == -1) {
        errno = ENOMEM;
        dma_buffer_error(dma_buffer, 0, errno, __func__);
        goto err;
    }

    sprintf(path, "/sys/class/u-dma-buf/udmabuf%d/size", buffer_num);
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        dma_buffer_error(dma_buffer, 0, errno, __func__);
        goto err;
    }

    char buf[1024] = {0};
    ssize_t nbytes = read(fd, buf, sizeof(buf));
    if (nbytes == -1) {
        dma_buffer_error(dma_buffer, 0, errno, __func__);
        close(fd);
        goto err;
    }

    close(fd);
    sscanf(buf, "%zd", &dma_buffer->size);
    if (dma_buffer->size < size) {
        errno = ENOMEM;
        dma_buffer_error(dma_buffer, 0, errno, __func__);
        goto err;
    }

    sprintf(path, "/sys/class/u-dma-buf/udmabuf%d/phys_addr", buffer_num);
    fd = open(path, O_RDONLY);
    if (fd == -1) {
        dma_buffer_error(dma_buffer, 0, errno, __func__);
        goto err;
    }

    memset(buf, 0, sizeof(buf));
    nbytes = read(fd, buf, sizeof(buf));
    if (nbytes == -1) {
        dma_buffer_error(dma_buffer, 0, errno, __func__);
        close(fd);
        goto err;
    }

    close(fd);
    sscanf(buf, "%lx", &dma_buffer->phys_addr);
    return dma_buffer;

err:
    dma_buffer_destroy(dma_buffer);
    return NULL;
}

void
dma_buffer_destroy(dma_buffer_t *restrict dma_buffer)
{
    if (dma_buffer == NULL) {
        return;
    }

    close(dma_buffer->fd);
    --buffer_num;
    free(dma_buffer);
}

void
dma_buffer_error(dma_buffer_t *restrict dma_buffer, int status, int error, const char *restrict format, ...)
{
    if (error_handler == NULL) {
        return;
    }

    va_list ap;
    va_start(ap, format);
    (*error_handler)(status, error, format, ap);
    va_end(ap);
}

size_t
dma_buffer_get_size(dma_buffer_t *restrict dma_buffer)
{
    return dma_buffer->size;
}

uint64_t
dma_buffer_get_phys_addr(dma_buffer_t *restrict dma_buffer)
{
    return dma_buffer->phys_addr;
}

bool
dma_buffer_is_enabled()
{
    return (access("/dev/udmabuf0", F_OK) == 0);
}

void *
dma_buffer_map(dma_buffer_t *restrict dma_buffer, int prot)
{
    void *addr = mmap(NULL, dma_buffer->size, prot, MAP_FILE | MAP_SHARED, dma_buffer->fd, 0);
    if (addr == MAP_FAILED) {
        return NULL;
    }

    return addr;
}

dma_buffer_error_handler_t *
dma_buffer_set_error_handler(dma_buffer_error_handler_t *handler)
{
    dma_buffer_error_handler_t *previous_handler = error_handler;
    error_handler = handler;
    return previous_handler;
}

void
dma_buffer_unmap(dma_buffer_t *restrict dma_buffer, void *addr)
{
    if (addr == NULL) {
        return;
    }

    munmap(addr, dma_buffer->size);
}
