#include "vm/swap.h"
#include "devices/disk.h"
#include "threads/synch.h"
#include <bitmap.h>
#include "threads/vaddr.h"

/* The swap device */
static struct disk *swap_device;

/* Tracks in-use and free swap slots */
static struct bitmap *swap_table;

/* Protects swap_table */
static struct lock swap_lock;

/* 
 * Initialize swap_device, swap_table, and swap_lock.
 */
void 
swap_init (void)
{

    swap_device = disk_get(1,1);
    swap_table = bitmap_create(DISK_SECTOR_SIZE/PGSIZE * disk_size(swap_device)); // kys
    if(swap_table == NULL) ASSERT(0);
    lock_init(&swap_lock);
}


disk_sector_t
get_empty_sector_num(void){

    size_t bitmap_idx = bitmap_scan_and_flip(swap_table, 0, 1, false);
    if(bitmap_idx != BITMAP_ERROR) return bitmap_idx * PGSIZE/DISK_SECTOR_SIZE;
    else{
        PANIC("swap size if full!\n");
    }
}


/*
 * Reclaim a frame from swap device.
 * 1. Check that the page has been already evicted. 
 * 2. You will want to evict an already existing frame
 * to make space to read from the disk to cache. 
 * 3. Re-link the new frame with the corresponding supplementary
 * page table entry. 
 * 4. Do NOT create a new supplementray page table entry. Use the 
 * already existing one. 
 * 5. Use helper function read_from_disk in order to read the contents
 * of the disk into the frame. 
 */ 

bool 
swap_in (void *frame_addr, disk_sector_t sector_num)
{ 
    lock_acquire(&swap_lock);
    disk_sector_t bitmap_idx = sector_num * DISK_SECTOR_SIZE/PGSIZE;
    bool success = bitmap_test(swap_table, bitmap_idx);
    if(success == false) PANIC("invalid swap space!");

    bitmap_flip(swap_table, bitmap_idx);
    read_from_disk(frame_addr, sector_num);

    lock_release(&swap_lock);
    return true;
}

/* 
 * Evict a frame to swap device. 
 * 1. Choose the frame you want to evict.  -> at page.c
 * (Ex. Least Recently Used policy -> Compare the timestamps when each 
 * frame is last accessed)
 * 2. Evict the frame. Unlink the frame from the supplementray page table entry
 * Remove the frame from the frame table after freeing the frame with
 * pagedir_clear_page.  -> not yet
 * 3. Do NOT delete the supplementary page table entry. The process
 * should have the illusion that they still have the page allocated to
 * them. 
 * 4. Find a free block to write you data. Use swap table to get track
 * of in-use and free swap slots.
 */
/* frame -> swap */
disk_sector_t
swap_out (void* frame_addr)
{
    lock_acquire(&swap_lock);
    void* addr =pg_round_down(frame_addr);
    disk_sector_t sector_num = get_empty_sector_num();
    write_to_disk(frame_addr, sector_num);

    lock_release(&swap_lock);
    return sector_num;
}

/* 
 * Read data from swap device to frame. 
 * Look at device/disk.c
 */
void read_from_disk (void *frame_addr, disk_sector_t sector_num)
{
    disk_sector_t i;
    for(i=0; i<PGSIZE/DISK_SECTOR_SIZE; i++){
        void* dst = i*DISK_SECTOR_SIZE + frame_addr;
        disk_read(swap_device, i+sector_num, dst);
    }
    return;
}

/* Write data to swap device from frame */
void write_to_disk (void *frame_addr, disk_sector_t sector_num)
{
    disk_sector_t i;
    for(i=0; i<PGSIZE/DISK_SECTOR_SIZE; i++){    
        void* dst = i*DISK_SECTOR_SIZE + frame_addr; // total num is PGSIZE
        disk_write(swap_device, i+sector_num, dst);
    }
    return;
}

