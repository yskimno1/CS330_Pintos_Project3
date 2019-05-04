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


/* true if A is less than B */
bool
list_less(const struct list_elem* a, const struct list_elem* b, void* aux){
    struct sup_page_table_entry* spt_e_1 = list_entry(a, struct sup_page_table_entry, elem);
    struct sup_page_table_entry* spt_e_2 = list_entry(b, struct sup_page_table_entry, elem);
    return (spt_e_1->user_vaddr < spt_e_1->user_vaddr); // not using hash_func b/c all user_vaddr are different
}

void 
page_init (void)
{
    printf("page init start\n");
    struct thread* curr = thread_current();
    list_init(curr->sup_page_table);
    printf("page init done\n");
    return;
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
    printf("page done start\n");
    struct thread* curr = thread_current();
    printf("page done done\n");
}