#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MEM_SIZE 16384  // MUST equal PAGE_SIZE * PAGE_COUNT
#define PAGE_SIZE 256  // MUST equal 2^PAGE_SHIFT
#define PAGE_COUNT 64
#define PAGE_SHIFT 8  // Shift page number this much

// Simulated RAM
unsigned char mem[MEM_SIZE];

//
// Convert a page,offset into an address
//
int get_address(int page, int offset)
{
    return (page << PAGE_SHIFT) | offset;
}

//
// Initialize RAM
//
void initialize_mem(void){
    memset(mem, 0, MEM_SIZE);            // Zero every byte of physical memory in the mem array.
    mem[0] = 1;                          // Mark as 0 page as used/1, it should always be reserved.
}

void set_page_table_entry(int page_table, int vpage, int page){
    int pt_addr = get_address(page_table, vpage);
    mem[pt_addr] = page;
}
//
// Allocate a physical page
//
// Returns the number of the page, or 0xff if no more pages available
//
unsigned char get_page(void){            // AllocatePage():
    for (int i=0; i < PAGE_COUNT; ++i){  // For each page_number in the Used Page array in zero page:
        if (mem[i]==0){                  // If it's unused (if it's 0):
            mem[i] = 1;                  // mem[page_number] = 1 // mark used
            return i;                    // return the page_number
        }
    }
    return 0xff;                         // indicating no free page
}

//
// Allocate pages for a new process
//
// This includes the new process page table and page_count data pages.
//
void new_process(int proc_num, int page_count){   // NewProcess(proc_num, page_count):
    int page_table = get_page();                  // Get the page table page
    if (page_table == 0xff){                      // If the initial page table allocation fails (due to out-of-memory/0xff)
        printf("OOM: proc %d: page table\n", proc_num);
        return;                                   // Return early, End function
    }
    mem[64 + proc_num] = page_table;              // Set this process's page table pointer in zero page

    for (int i=0; i < page_count; ++i){          // For i from 0 to page_count:
        int new_page = get_page();
        if (new_page == 0xff) {                  // Subsequent page allocations fail (due to out-of-memory/0xff)
            printf("OOM: proc %d data page\n", proc_num);
            return;                              // Return early, End function
        }
        // Set the page table to map virt -> phys
        // Virtual page number is i
        // Physical page number is new_page
        set_page_table_entry(page_table, i, new_page);
    }
}

//
// Get the page table for a given process
//
unsigned char get_page_table(int proc_num)
{
    return mem[proc_num + 64];
}

//
// Print the free page map
//
void print_page_free_map(void)
{
    printf("--- PAGE FREE MAP ---\n");

    for (int i = 0; i < 64; i++) {
        int addr = get_address(0, i);

        printf("%c", mem[addr] == 0? '.': '#');

        if ((i + 1) % 16 == 0)
            putchar('\n');
    }
}

//
// Print the address map from virtual pages to physical
//
void print_page_table(int proc_num)
{
    printf("--- PROCESS %d PAGE TABLE ---\n", proc_num);

    // Get the page table for this process
    int page_table = get_page_table(proc_num);

    // Loop through, printing out used pointers
    for (int i = 0; i < PAGE_COUNT; i++) {
        int addr = get_address(page_table, i);

        int page = mem[addr];

        if (page != 0) {
            printf("%02x -> %02x\n", i, page);
        }
    }
}

//
// Main -- process command line
//
int main(int argc, char *argv[]){
    assert(PAGE_COUNT * PAGE_SIZE == MEM_SIZE);

    if (argc == 1) {
        fprintf(stderr, "usage: ptsim commands\n");
        return 1;
    }
    
    initialize_mem();

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "np") == 0) {
            int proc_num = atoi(argv[++i]);
            int pages = atoi(argv[++i]);
            new_process(proc_num, pages);
        }
        else if (strcmp(argv[i], "pfm") == 0) {
            print_page_free_map();
        }
        else if (strcmp(argv[i], "ppt") == 0) {
            int proc_num = atoi(argv[++i]);
            print_page_table(proc_num);
        }
    }
}
