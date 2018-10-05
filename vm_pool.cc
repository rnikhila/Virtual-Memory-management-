/*
 File: vm_pool.C
 
 Author: Nikhila SR
 Date  : Oct 6th
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include "vm_pool.H"
#include "page_table.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    base_address = _base_address;
    size = _size;
    frame_pool = _frame_pool;
    page_table = _page_table;
    page_table -> register_pool(this);

    number_allocated_regions = 0;
    list_allocated_regions =(allocated_region *) (frame_pool->get_frames(1)*(4096));
    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
	Console::puts("It entered allocate function in vm_pool.\n");
	unsigned long address_new;

	if(number_allocated_regions == 0) 
	{
	list_allocated_regions[0].start_addr = base_address;
	list_allocated_regions[number_allocated_regions].size = _size;
	Console::puts("it enterned the loop of 0 regions.\n");
	Console::puts("base_address of the region = ");Console::puti(list_allocated_regions[0].start_addr);Console::puts("\n");
	}
	else
	{
		Console::puts("it enterned the loop of more than 0 regions.\n");

	list_allocated_regions[number_allocated_regions].start_addr = list_allocated_regions[number_allocated_regions - 1].start_addr + list_allocated_regions[number_allocated_regions - 1].size;		list_allocated_regions[number_allocated_regions].size = _size;
	Console::puts("base_address of the region = ");Console::puti(list_allocated_regions[number_allocated_regions].start_addr);Console::puts("\n");
Console::puts("a = ");Console::puti(list_allocated_regions[number_allocated_regions - 1].start_addr);Console::puts("\n"); 
Console::puts("b = ");Console::puti(list_allocated_regions[number_allocated_regions - 1].size);Console::puts("\n");  assert(false);
	}
        
	number_allocated_regions ++; 	
	Console::puts("in vm_pool.c: size  of the region = ");Console::puti(list_allocated_regions[(number_allocated_regions-1)].size);Console::puts("\n");
	Console::puts("Allocated region of memory.\n");
Console::puts("start addr  of the region = ");Console::puti(list_allocated_regions[(number_allocated_regions-1)].start_addr);Console::puts("\n");
	return(list_allocated_regions[(number_allocated_regions-1)].start_addr);	
}

void VMPool::release(unsigned long _start_address) {
    	int i,j;
	unsigned long release_addr;
	release_addr = _start_address;
        Console::puts("_start_address in release function ");Console::puti(release_addr);Console::puts("\n");
        for(i =0;i<number_allocated_regions ;i++)
	{
		if(list_allocated_regions[i].start_addr == _start_address) break;
	}
	
	int number_of_frames_to_release = ((list_allocated_regions[i].size % 4096) > 0 ? (list_allocated_regions[i].size / 4096) +1 : (list_allocated_regions[i].size / 4096)); 
        Console::puts("number_of_frames_to release  = ");Console::puti(number_of_frames_to_release);Console::puts("\n");	
	for(j=0;j<number_of_frames_to_release;j++)
	{
	    page_table -> free_page(release_addr);
	    release_addr = release_addr + 4096;
	}
number_allocated_regions --;
	for(j =i; j<number_allocated_regions; j++)
	{
		list_allocated_regions[i] = list_allocated_regions[i+1];
	}
        	
	Console::puts("Released region of memory.\n");
	
}

bool VMPool::is_legitimate(unsigned long _address) {
    //assert(false);
    Console::puts("given address is  = ");Console::puti(_address);Console::puts("\n");
    if((_address >= base_address) || (_address < base_address + size))
	{
		Console::puts("Checked whether address is part of an allocated region.\n");
    Console::puts("base_address of the region");Console::puti(base_address);Console::puts("\n");
Console::puts("size of the region");Console::puti(size);Console::puts("\n");
 return(1);
}
    else return(0);
    }

