#include <stdint.h>
#include <stdbool.h>
#include <hash.h>

#ifndef VM_PAGE_H
#define VM_PAGE_H

// struct list sup_page_table;
#define LIMIT (1 << 23)
struct sup_page_table_entry 
{
	uint32_t* user_vaddr;
	uint64_t access_time;

	uint32_t read_bytes;
	uint32_t zero_bytes;

	struct list_elem elem;

	bool dirty;
	bool accessed;

	struct file* file;
	int32_t offset;
	bool writable;
};

enum palloc_type{
    GROW_STACK=0,
	PAGE_FAULT=1,
	LOAD_SEGMENT=2
};

bool list_less(const struct list_elem* a, const struct list_elem* b, void* aux);
bool page_insert(struct sup_page_table_entry* spt_e);
void page_init (void);
struct sup_page_table_entry*  allocate_page(void *addr, bool access, enum palloc_type p_type, uint32_t read_bytes, uint32_t zero_bytes, struct file *file, int32_t offset, bool writable);
void page_done(void);
struct sup_page_table_entry* find_page(void* addr);
bool grow_stack(void* addr);
bool setup_stack_grow(void* addr);
bool page_handling(struct sup_page_table_entry* spt_e);
bool swap_handling(struct sup_page_table_entry* spt_e);
bool file_handling(struct sup_page_table_entry* spt_e);
#endif /* vm/page.h */
