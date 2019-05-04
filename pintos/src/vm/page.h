#include <stdint.h>
#include <stdbool.h>
#include <hash.h>

#ifndef VM_PAGE_H
#define VM_PAGE_H

struct list sup_page_table;

struct sup_page_table_entry 
{
	uint32_t* user_vaddr;
	uint64_t access_time;

	uint32_t read_bytes;
	uint32_t zero_bytes;

	struct list_elem elem;

	bool dirty;
	bool accessed;
};


bool list_less(const struct list_elem* a, const struct list_elem* b, void* aux);
void page_init (void);
struct sup_page_table_entry*  allocate_page (void *addr);
void page_done(void);

#endif /* vm/page.h */
