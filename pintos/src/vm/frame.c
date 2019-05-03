#include <stdint.h>
#include <stdbool.h>
#include "vm/frame.h"
#include "threads/thread.h"
#include "threads/palloc.h"

/*
 * Initialize frame table
 */
void 
frame_init (void)
{
    list_init(&frame_table);
    /* Needs frame lock */
    lock_init(&lock_frame);
}

void
insert_frame_table(struct frame_table_entry* fte){
    list_push_back(&frame_table, fte->elem_table_list);
}

struct frame_table_entry*
create_frame_table_entry(void* frame){
    struct frame_table_entry* fte = malloc(sizeof(struct frame_table_entry));
    fte->frame = frame;
    fte->owner = thread_current();
    /* spte should be initialized! */

    return fte;
}

/* 
 * Make a new frame table entry for addr.
 */
bool
allocate_frame (void *addr, enum palloc_flags flag)
{
    ASSERT(flag & PAL_USER);
    void* frame = palloc_get_page(flag);
    struct frame_table_entry* fte = create_frame_table_entry(frame);
    lock_acquire(&lock_frame);
    insert_frame_table(fte);
    lock_release(&lock_frame);
    return true;
}

