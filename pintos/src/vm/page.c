#include "vm/page.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include "threads/thread.h"
#include "vm/frame.h"


/*
 * Initialize supplementary page table
 */
void hash_action_before_destroy (struct hash_elem *e, void *aux){
    /* temporary made */
}

unsigned
hash_func(struct hash_elem* e, void* aux){
    struct sup_page_table_entry* spt_e = hash_entry(e, struct sup_page_table_entry, elem);
    return hash_int(spt_e->user_vaddr);
}

/* true if A is less than B */
bool
hash_less(const struct hash_elem* a, const struct hash_elem* b, void* aux){
    struct sup_page_table_entry* spt_e_1 = hash_entry(a, struct sup_page_table_entry, elem);
    struct sup_page_table_entry* spt_e_2 = hash_entry(b, struct sup_page_table_entry, elem);
    return (spt_e_1->user_vaddr < spt_e_1->user_vaddr); // not using hash_func b/c all user_vaddr are different
}

void 
page_init (void)
{
    struct thread* curr = thread_current();
    hash_init(curr->sup_page_table, hash_func, hash_less, 0);
}

/*
 * Make new supplementary page table entry for addr 
 */
struct sup_page_table_entry* 
allocate_page (void* addr){
    struct sup_page_table_entry* spt_e = malloc(sizeof(struct sup_page_table_entry));
    if(spt_e == NULL) return NULL;
    spt_e->user_vaddr = addr;
    
    return spt_e;
}

bool
grow_stack(void* addr){
    struct sup_page_table_entry* spt_e = allocate_page(addr);
    if(spt_e == NULL) return false;
    
    uint8_t* frame_addr = allocate_frame(spt_e, PAL_USER);
    
    if(frame_addr == NULL){
        free(spt_e);
        return false;
    }
    return true;
}

void
page_done(void)
{
    struct thread* curr = thread_current();
    hash_destroy(curr->sup_page_table, hash_action_before_destroy);
}