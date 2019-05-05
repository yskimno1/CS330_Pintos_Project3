#include "vm/page.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include "threads/thread.h"
#include "vm/frame.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "userprog/process.h"

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


bool
grow_stack(void* addr){
    void* page_addr = pg_round_down(addr);
    void* limit = PHYS_BASE - (1<<26);
    printf("page addr : %p, limit %p\n", page_addr, limit);
    if(page_addr < limit) return false;
    struct sup_page_table_entry* spt_e = allocate_page(page_addr, true);

    uint8_t* frame_addr = allocate_frame(spt_e, PAL_USER);
    if(frame_addr==NULL){
        printf("frame null\n");
        free(spt_e);
        return false;
    }

    bool success = install_page(page_addr, frame_addr, true);
    if(success == false){
        printf("install page failed\n");
        free_frame(frame_addr);
        free(spt_e);
        return false;
    }
    page_insert(spt_e);

    return true;
}

bool
setup_stack_grow(void* addr){
    struct sup_page_table_entry* spt_e = allocate_page(addr, false);
    if(spt_e==NULL){
        printf("spte null\n");
        return false;
    }

    uint8_t* frame_addr = allocate_frame(spt_e, PAL_USER|PAL_ZERO);
    if(frame_addr==NULL){
        printf("frame null\n");
        free(spt_e);
        return false;
    }
    spt_e->accessed = true;
    page_insert(spt_e); // can insert at front kys

    bool success = install_page(addr, frame_addr, true);
    if(success == false){
        printf("install page failed\n");
        free_frame(frame_addr);
        free(spt_e);
        return false;
    }


    return true;
}