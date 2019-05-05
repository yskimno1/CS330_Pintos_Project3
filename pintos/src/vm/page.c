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
// #include "filesys/directory.h"
#include "filesys/file.h"
// #include "filesys/filesys.h"

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

bool
page_insert(struct sup_page_table_entry* spt_e){
    struct thread* curr = thread_current();
    struct list_elem* e;
    if(!list_empty(&curr->sup_page_table)){
        for(e=list_begin(&curr->sup_page_table); e!=list_end(&curr->sup_page_table); e = list_next(e)){
            if(e == &spt_e->elem) return false;
        }
    }
    printf("page insert start\n");
    list_push_back(&curr->sup_page_table, &spt_e->elem);
    return true;
    //list_insert_ordered(&curr->sup_page_table, &spt_e->elem, list_less, 0);
}

bool
add_page(void* addr, bool access, enum palloc_type p_type, uint32_t read_bytes, uint32_t zero_bytes, struct file *file, int32_t offset, bool writable){
    struct sup_page_table_entry* spt_e;
    spt_e = allocate_page(addr, access, p_type, read_bytes, zero_bytes, file, offset, writable);
    if(spt_e == NULL) return false;

    bool success = page_insert(spt_e);
    return success;
}

/*
 * Make new supplementary page table entry for addr 
 */
struct sup_page_table_entry* 
allocate_page (void* addr, bool access, enum palloc_type p_type, uint32_t read_bytes, uint32_t zero_bytes, struct file *file, int32_t offset, bool writable){
    struct sup_page_table_entry* spt_e = malloc(sizeof(struct sup_page_table_entry));
    if(spt_e == NULL) return NULL;
    if(p_type == GROW_STACK || p_type == PAGE_FAULT){
        spt_e->user_vaddr = addr;
        spt_e->accessed = access;
    }
    else if(p_type == LOAD_SEGMENT){
        spt_e->user_vaddr = addr;
        spt_e->accessed = access;
        spt_e->read_bytes = read_bytes;
        spt_e->zero_bytes = zero_bytes;
        spt_e->offset = offset;
        spt_e->file = file;
        spt_e->writable = writable;
        printf("read bytes : %d, offset %d, address %p\n", spt_e->read_bytes, spt_e->offset, spt_e->user_vaddr);
    }
    else ASSERT(0);
    return spt_e;
}

struct sup_page_table_entry*
find_page(void* addr){
    void* aligned_addr = pg_round_down(addr);
    struct sup_page_table_entry* spt_e;

    struct list_elem* e;
    struct thread* curr = thread_current();
    struct list sup_page_table = curr->sup_page_table;
    if(!list_empty(&sup_page_table)){
        for(e=list_begin(&sup_page_table); e!=list_end(&sup_page_table); e = list_next(e)){
            spt_e = list_entry(e, struct sup_page_table_entry, elem);
            if(spt_e->user_vaddr == aligned_addr) return spt_e;
        }
    }
    return NULL;
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
page_handling(struct sup_page_table_entry* spt_e){
    // return swap_handling(spt_e);
    return file_handling(spt_e);
}

bool
file_handling(struct sup_page_table_entry* spt_e){

    void* frame;

    if(spt_e->read_bytes == 0) frame = allocate_frame(spt_e, PAL_USER|PAL_ZERO);
    else frame = allocate_frame(spt_e, PAL_USER);
    ASSERT(frame);
    if(frame == NULL) return false;

    filelock_acquire();
    
    bool success = install_page(spt_e->user_vaddr, frame, spt_e->writable);
    ASSERT(success);
    if(success == false){
        free_frame(frame);
        filelock_release();
        return false;
    }

    if(spt_e->read_bytes > 0){
        file_seek (spt_e->file, spt_e->offset);
        off_t temp = file_read (spt_e->file, frame, spt_e->read_bytes);
        printf("temp : %d\n", temp);
        if (temp != (int) spt_e->read_bytes){
            printf("%d vs %d\n", temp, (int) spt_e->read_bytes);
            free_frame(frame);
            filelock_release();
            ASSERT(0);
            return false; 
        }

        memset (frame + spt_e->read_bytes, 0, spt_e->zero_bytes);
    }




    spt_e->accessed = true;
    filelock_release();
    return true;
}

bool
swap_handling(struct sup_page_table_entry* spt_e){
    return false;
}

bool
grow_stack(void* addr){

    void* page_addr = pg_round_down(addr);

    struct sup_page_table_entry* spt_e = allocate_page(page_addr, true, PAGE_FAULT, 0, 0, NULL, 0, 0);

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
    success = page_insert(spt_e);
    return success;
}

bool
setup_stack_grow(void* addr){
    printf("setup stack grow start!\n");
    struct sup_page_table_entry* spt_e = allocate_page(addr, false, GROW_STACK, 0, 0, NULL, 0, 0);
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
    bool success = page_insert(spt_e); // can insert at front kys
    ASSERT(success);

    success = install_page(addr, frame_addr, true);
    printf("Setup stack grow done!\n");
    if(success == false){
        printf("install page failed\n");
        free_frame(frame_addr);
        free(spt_e);
        return false;
    }

    return true;
}