#include "vm/page.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include "threads/thread.h"
#include "vm/frame.h"
#include "userprog/pagedir.h"


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
page_insert(struct sup_page_table_entry* spt_e){
    struct thread* curr = thread_current();
    list_push_back(&curr->sup_page_table, &spt_e->elem);
    //list_insert_ordered(&curr->sup_page_table, &spt_e->elem, list_less, 0);
}

/*
 * Make new supplementary page table entry for addr 
 */
struct sup_page_table_entry* 
allocate_page (void* addr, bool access){
    struct sup_page_table_entry* spt_e = malloc(sizeof(struct sup_page_table_entry));
    if(spt_e == NULL) return NULL;
    spt_e->user_vaddr = addr;
    spt_e->accessed = access;
    
    return spt_e;
}

void*
free_page(struct list_elem* e){
    struct sup_page_table_entry* spt_e = list_entry(e, struct sup_page_table_entry, elem);
    if(spt_e->accessed){
        struct thread* curr = thread_current();
        free_frame(pagedir_get_page(curr->pagedir, spt_e->user_vaddr));
        pagedir_clear_page(curr->pagedir, spt_e->user_vaddr);
    }
    free(spt_e);
}

void* 
grow_stack(void* addr){
    struct sup_page_table_entry* spt_e = allocate_page(addr, false);
    if(spt_e == NULL) return NULL;
    
    uint8_t* frame_addr = allocate_frame(spt_e, PAL_USER|PAL_ZERO);
    spt_e->accessed = true;

    if(frame_addr == NULL){
        free(spt_e);
        return NULL;
    }
    page_insert(spt_e);

    return frame_addr;
}
