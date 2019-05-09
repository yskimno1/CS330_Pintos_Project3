#include "vm/page.h"
#include "vm/frame.h"
#include "devices/disk.h"

#include "lib/kernel/bitmap.h"

void swap_init (void);
bool swap_in (void *frame_addr, disk_sector_t sector_num);
disk_sector_t swap_out (void *frame_addr);
void read_from_disk (void *frame_addr, disk_sector_t sector_num);
void write_to_disk (void *frame_addr, disk_sector_t sector_num);

/* The swap device */
static struct disk *swap_device;

/* Tracks in-use and free swap slots */
static struct bitmap *swap_table;

/* Protects swap_table */
static struct lock swap_lock;

#ifndef VM_SWAP_H
#define VM_SWAP_H



#endif /* vm/swap.h */