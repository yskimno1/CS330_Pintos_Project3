#include <stdint.h>
#include <stdbool.h>
#include "threads/palloc.h"

#ifndef VM_FRAME_H
#define VM_FRAME_H

struct list frame_table;
struct lock lock_frame;

struct frame_table_entry
{
	uint32_t* frame;
	struct thread* owner;
	struct sup_page_table_entry* spte;

	struct list_elem* elem_table_list;
};

void frame_init (void);
bool allocate_frame (void *addr, enum palloc_flags flag);

#endif /* vm/frame.h */
