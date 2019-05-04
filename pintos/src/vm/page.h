#include <stdint.h>
#include <stdbool.h>
#include <hash.h>

#ifndef VM_PAGE_H
#define VM_PAGE_H

struct hash sup_page_table;

struct sup_page_table_entry 
{
	uint32_t* user_vaddr;
	uint64_t access_time;

	uint32_t read_bytes;
	uint32_t zero_bytes;

	struct hash_elem elem;

	bool dirty;
	bool accessed;
};


unsigned hash_func(struct hash_elem* e, void* aux);
bool hash_less(const struct hash_elem* a, const struct hash_elem* b, void* aux);
void page_init (void);
struct sup_page_table_entry*  allocate_page (void *addr);
void page_done(void);

#endif /* vm/page.h */
