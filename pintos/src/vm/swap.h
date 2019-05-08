#include "vm/page.h"
#include "vm/frame.h"
#include "devices/disk.h"

void swap_init (void);
bool swap_in (void *frame_addr, disk_sector_t sector_num);
bool swap_out (void *frame_addr);
void read_from_disk (void *frame_addr, disk_sector_t sector_num);
void write_to_disk_all (void *frame_addr, disk_sector_t sector_num);
#ifndef VM_SWAP_H
#define VM_SWAP_H

#endif /* vm/swap.h */