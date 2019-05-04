#include <stdint.h>
#include <stdbool.h>

#include "threads/thread.h"
#include "threads/palloc.h"
#include "vm/page.h"
#include "vm/frame.h"
/*
 * Initialize frame table
 */
void 
frame_init (void)
{
    printf("frame init start\n");
    list_init(&frame_table);
    /* Needs frame lock */
    lock_init(&lock_frame);
    printf("frame init done\n");
}

void
insert_frame_table(struct frame_table_entry* fte){
    list_push_back(&frame_table, &fte->elem_table_list);
}

struct frame_table_entry*
create_frame_table_entry(void* frame, struct frame_table_entry* spt_e){
    struct frame_table_entry* fte = malloc(sizeof(struct frame_table_entry));
    if(fte==NULL) return NULL;

    fte->frame = frame;
    fte->owner = thread_current();
    /* spte should be initialized! */
    fte->spte = spt_e;

    return fte;
}

/* 
 * Make a new frame table entry for addr.
 * frame should be allocated after page allocation.
 */
void*
allocate_frame (struct sup_page_table_entry* spt_e, enum palloc_flags flag)
{
    ASSERT(flag & PAL_USER);
    void* frame = palloc_get_page(flag);
    // if(frame == NULL) PANIC("frame is full");
    struct frame_table_entry* fte = create_frame_table_entry(frame, spt_e);
    if(fte == NULL) return NULL;
    lock_acquire(&lock_frame);
    insert_frame_table(fte);
    lock_release(&lock_frame);
    return frame;
}

static struct frame_table_entry*
search_frame_table_entry (void* frame){
  struct list_elem* e;
  struct frame_table_entry* fte;
  if(!list_empty(&frame_table)){
    for(e=list_begin(&frame_table); e!=list_end(&frame_table); e = list_next(e)){
      fte = list_entry(e, struct frame_table_entry, elem_table_list);
      if(fte->frame == frame) return fte;
    }
  }
  return NULL;
}

void*
free_frame (void* frame){
    lock_acquire(&lock_frame);
    struct list_elem* e;
    struct frame_table_entry* fte;
    if(!list_empty(&frame_table)){
        for(e=list_begin(&frame_table); e!=list_end(&frame_table); e = list_next(e)){
            fte = list_entry(e, struct frame_table_entry, elem_table_list);
            if(fte->frame == frame){
                list_remove(e);
                free(fte);
                palloc_free_page(frame);
                break;
            }
        }
    }

    lock_release(&lock_frame);
}