/** @file */

#ifndef BUS_MASTER_H
#define BUS_MASTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* The Physical Region Descriptor Table (PRDT) must be aligned on a 4-byte
   boundary, and the table cannot cross a 64K boundary in memory. */
#define BM_IDE_MAX_PRDT_SIZE 0x10000
#define BM_IDE_MAX_NUM_PRDS (BM_IDE_MAX_PRDT_SIZE / sizeof(struct prd))

/** Bus Master IDE Registers */
enum
{
    BM_IDE_COMMAND0 = 0,
    BM_IDE_STATUS0 = 2,
    BM_IDE_PRDT0 = 4,
    BM_IDE_COMMAND1 = 8,
    BM_IDE_STATUS1 = 10,
    BM_IDE_PRDT1 = 12,
};

/** Bus Master IDE Command Register */
enum
{
    BM_IDE_START = (1 << 0),
    BM_IDE_WRITE = (1 << 3),
};

/** Bus Master IDE Status Register */
enum
{
    BM_IDE_ACTIVE = (1 << 0),
    BM_IDE_ERROR = (1 << 1),
    BM_IDE_INTERRUPT = (1 << 2),
    BM_IDE_DRIVE0_DMA = (1 << 5),
    BM_IDE_DRIVE1_DMA = (1 << 6),
    BM_IDE_SIMPLEX = (1 << 7),
};

/** Physical Region Descriptor (PRD) */
struct prd {
    uint32_t address;
    uint16_t count;
    uint16_t reserved;
};

#ifdef __cplusplus
}
#endif

#endif /* BUS_MASTER_H */
