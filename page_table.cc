#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;

VMPool** PageTable::list_vm_pools = NULL;
int PageTable::list_index =0;

void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
    PageTable::kernel_mem_pool = _kernel_mem_pool; // setting the global parameters
   PageTable::process_mem_pool = _process_mem_pool;
   PageTable::shared_size = _shared_size;		
    Console::puts("Initialized Paging System\n");
}


PageTable::PageTable()
{
   Console::puts("Constructed Page Table directory\n") ; //we r getting a frame from the kernel memory pool
   page_directory  = (unsigned long *) (process_mem_pool->get_frames(1) * (PAGE_SIZE));// //get_frames will give the frame number of a free frame, so we the ddress of the frame = frmae_num * page_size
    
   for (unsigned int i = 1; i < 1024 ; i++) // page directory has 1024 entries , so iterating through all of them...
   page_directory[i] = 0 | 2; // we attribute all these entries to not present as we havent given any page tables to the entries so all the address in the entries are also 0

   page_directory[1023] = (unsigned long) page_directory|3;  //pointing the last entry of the page_directory to the page_directory itself

  // setting up the page_table

   unsigned long *page_table = (unsigned long *) (process_mem_pool->get_frames(1) * PAGE_SIZE);// we r getting a frame for page table[0] and address of it is obtained by multiplying with page_size
   unsigned long address = 0;
   for (unsigned int i = 0; i < 1024; i++) // since page table also has 1024 entries, we r iterating through all of them
   {  
   page_table[i] = address | 3; // marking all the page tables as present
   address = address + 4096; // since each page table is 4kb, address will be 4096 + older address 
   }

  page_directory[0] = (unsigned long) page_table;// fill the first entry of the directry with the page table we just created
  page_directory[0] |= 3;// changing the attributes of first entry to present since we filled it with one page table
	Console::puts("Constructed Page Table object\n");
/*
// initializing the list_vm_pools
  int j;
  for(j=0;j<2;j++)
  {
	list_vm_pools[j] = NULL;
  }*/
}


void PageTable::load()
{
        current_page_table = this; // assigning the pointer of this page table to current_page_table
	Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   Console::puts("Enabled paging\n");
   write_cr3((int) current_page_table->page_directory);//
   unsigned long address = read_cr3();

   Console::puts("address in CR3 : "); Console::putui(address);Console::puts("\n");
   write_cr0(read_cr0() | 0x80000000); // paging is enabled by setting the bit in cro register
    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
    unsigned long *recursive_pd = (unsigned long*)(0xFFFFF000);// appending 1023|1023|0 so MMU reaches 1st array element of page directory
    unsigned long *page_table;
    unsigned long address;
    unsigned long error = _r->err_code;

    address = read_cr2();// 32 bit address of the address that caused page fault 
    Console::puts("address that created page fault is  ");Console::putui((address));Console::puts("\n");
     Console::puts("page_directory index ");Console::putui((address>>22));Console::puts("\n");
// Console::puts("contents in page_directory index ");Console::putui((page_dir[address>>22]));Console::puts("\n"); 
	bool legitimate;
	int i=0;
	if(address != 4202496 && address != 4206592) // eliminating the addresses of array of info of allocated pools
{
	for(i=0;i<list_index;i++) //checking across all the pools if the adress that is creating page fault belongs to any one of the registered pool
	{
		legitimate = list_vm_pools[i]->is_legitimate(address);
	}
	if(!legitimate) 
	{
		Console::puts("Invalid address \n ");
		assert(false);
	}	
}
    if(recursive_pd[address>>22] & 1 == 1)//if page_table is present
    {
//	assert(false);
//       Console::puts("error code  ");Console::putui(error);Console::puts("\n");
       page_table= (unsigned long *)(0xFFC00000|((address>>22)<<12));//extracting  page table address 
 
       page_table[(address>>12) & 0x3FF] =  (process_mem_pool->get_frames(1)*PAGE_SIZE)|3;//allocating a page and setting 011 which makes it valid and gives write and read permissions
       Console::puts("page_table address ");Console::putui((unsigned long )page_table);Console::puts("\n");
    }
    else //else page_table not loaded
    {
        //load page_table into directory
//      Console::puts("error code  ");Console::putui(error);Console::puts("\n");
//	assert(false);
	recursive_pd[address>>22] = (unsigned long)((process_mem_pool->get_frames(1)*PAGE_SIZE)|3);//allocates page table from process mem
	Console::puts("page_directory index ");Console::putui((address>>22));Console::puts("\n");  
	page_table = (unsigned long *)(0x3FF|((address>>22)<<12));
        Console::puts("page_table address ");Console::putui((unsigned long )page_table);Console::puts("\n");
        
	
    }

	Console::puts("handled page fault\n");
}

void PageTable::register_pool(VMPool * _vm_pool)
{
  //  assert(false);
    list_vm_pools[list_index] = _vm_pool; // updating the list of vm pool array after evry new region is allocated
    list_index ++;   
    Console::puts("registered VM pool\n");
}

void PageTable::free_page(unsigned long _page_no) {
    	unsigned long* page_table =  (unsigned long *)(0xFFC00000|((_page_no>>22)<<12));// accessing the page table using recursive technique
	unsigned long release_frame = (page_table[(_page_no>>12) & 0x3FF]);   // calculating the frame number
	process_mem_pool->release_frames(((release_frame>>12)<<12)/4096);
	Console::puts("frame number to be released ");Console::putui((unsigned long )release_frame);Console::puts("\n");
	Console::puts("freed page ");Console::putui((unsigned long )release_frame);Console::puts("\n");page_table[(_page_no>>12) & 0x3FF] = 0|2; // invalidating page table entry 


	unsigned long address = read_cr3();
	write_cr3((int) address) ;  // flishing CR3
		
}
