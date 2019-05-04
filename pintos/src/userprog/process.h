#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H
#define USERPROG

#include "threads/thread.h"
#include "threads/synch.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);
static bool install_page (void *upage, void *kpage, bool writable);
#endif /* userprog/process.h */

