/*
 File: ContFramePool.C
 
 Author:
 Date  : 
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

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
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/

ContFramePool** ContFramePool::list_pools = NULL;
int ContFramePool::index =0;
ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no,
                             unsigned long _n_info_frames)
{
	base_frame_no = _base_frame_no;
	nFrames = _n_frames;
	info_frame_no = _info_frame_no;
	n_info_frames = _n_info_frames;
	nFreeFrames = _n_frames;

        assert(nFrames <= (FRAME_SIZE * 4) );

	
// gotta ask 
	if(info_frame_no == 0)
		bitmap = (unsigned char *)(base_frame_no * FRAME_SIZE);
	else
		bitmap = (unsigned char *)(info_frame_no * FRAME_SIZE);

	assert((nFrames % 4 ) == 0);
//makrking all bits as free (01)	
	for(int i=0; i*4 < nFrames; i++)
	{
		Console::puts(" marking all frames as free ");
		Console::puti(i ); 
		bitmap[i] = 0x55;
		Console::puti( bitmap[i]); Console::puts("\n");
	}
	
Console::puts("In constructor: number of free frames "); Console::puti(nFreeFrames); Console::puts("\n");
//marking the info bits as allocated
	if(_info_frame_no == 0)
	{	
         Console::puts("_info_frame_no  "); Console::puti(_info_frame_no);Console::puts("\n");

		int i,j =0;
 		int temp_count = 0;
		while(temp_count < (_n_info_frames +1))
		{	
			Console::puts(" info frame number is 0 ");
			while(j<4 && (temp_count < (_n_info_frames +1)))
			{
				bitmap[i]=bitmap[i] & (0x3F >> 2*j);
				nFreeFrames --;
				Console::puts("current byte in while loop"); Console::puti(bitmap[i]); Console::puts("\n");
				temp_count++;
				Console::puts("temp_count "); Console::puti(temp_count); Console::puts("\n");
				j++;

			}
			Console::puts("In constructr: number of free frames "); Console::puti(nFreeFrames); Console::puts("\n");	
			j =0; i++;
		}	
	}
	
	list_pools[index] = this;
	index++;
	Console::puts("Frame Pool initialized \n");
}
				


unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
	Console::puts("get_frame function call  \n");
	assert(nFreeFrames > 0);
	int i = 0;
	
	unsigned char mask = 0x40;
	int j = 0;
	while(((mask & bitmap[i]) != mask)) //chking for free frame 
	{
//		Console::puts("not free - bitmap is  ");Console::puti(bitmap[i]);Console::puts("\n");
		while(((j<4) && ((mask & bitmap[i]) != mask)))
		{
//		Console::puts("looking for free frame in the bitmap:moving horizantally \n");
		j++;
		mask = mask >> 2;
		}
		if(((mask & bitmap[i]) == mask)&&(mask != 0)) 
		{
//		Console::puts("free map found, breaking out of while loop \n");
		break;
		}
		if(j==4) {
//		Console::puts("moving vertically \n");
		i++; mask = 0x40;j=0;
		}
	}
	int head_index = i ; unsigned char head_mask = mask;
	int seq_count = 0;
//	Console::puts("current_head_index: ");Console::puti(head_index);Console::puts("\n");
// checking for the required sequence of free frames
	while((seq_count != _n_frames) && (4*i < nFrames))
	{
 //        Console::puts("started checking for required seq of free frames \n");

		while((j<4) && (seq_count != _n_frames))
		{
			if((mask & bitmap[i]) == mask)
			{
				seq_count++;
				mask >> 2;
//				Console::puts("free frames found: ");Console::puti(seq_count);Console::puts("- frames needed: ");Console::puti(_n_frames);
//				Console::puts("\n");
				
			}
			else
			{
//				Console::puts("hit a non-free frame- free frames found so far: ");Console::puti(seq_count);Console::puts("- frames needed: ");Console::puti(_n_frames);
//				Console::puts("\n");
//				Console::puts("starting over a new search \n");
				seq_count = 0;
				head_index = i;
				head_mask = head_mask >> 2;
			}
			j++;
		}
		j = 0; i++; mask = 0x40;
	}
	if(seq_count == _n_frames)
	{	
//		Console::puts("the required number of frames seq is found \n");
 //changing the mask of head of the sequence by making the required 2 bits 00 and den XORing with 10 
		j =0;
//		Console::puts("final head_mask:  ");Console::puti(head_mask);Console::puts("\n");
		while(head_mask != 0x01)
		{
//                 Console::puts("finding the shift to change the state of head of the seq \n");
			j++;	// this is to find the shift 
			head_mask = head_mask >> 2;
		}
		unsigned int shift = (3-j);
		unsigned char A = ~(0xFF << 2*j);
		unsigned char B = 0xFC << 2*j;
		mask = A | B;
		bitmap[head_index] = bitmap[head_index] & mask ;
		bitmap[head_index] = bitmap[head_index] ^ (0x02<<2*j);
//		Console::puts("bitmap with head of seq :  ");Console::puti(bitmap[head_index]);Console::puts("\n"); 
//marking rest of the bits as allocated 
		i = head_index; seq_count --;nFreeFrames--;
		while(seq_count != 0)
		{
			while(j != 0 && seq_count != 0)
			{
//				Console::puts("marking rest of the bits in the seq found as allocated \n");
				j--; seq_count --;
				A = ~(0xFF << 2*j);
				B = 0xFC << 2*j;
				mask = A|B;
				bitmap[i] = bitmap[i] & mask;
				nFreeFrames--;
//				Console::puts("bitmap after marking allocated ");Console::puti(bitmap[i]);Console::puts("\n");
			}
			if(seq_count != 0)
			{
			j = 4; i ++;
			}
		}
Console::puts("In get_frame fn: Free frames : ");Console::puti(nFreeFrames);Console::puts("\n");		
Console::puts("In get_frame fn: head frame returned ");Console::puti(( (head_index *4)+shift+base_frame_no));Console::puts("\n");	
		return( (head_index *4)+shift+base_frame_no) ;
	}
	else 
	{
Console::puts("In get_frame fn: Free frames after releasing ");Console::puti(nFreeFrames);Console::puts("\n");
		return(0) ;
 	}
	
	
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
// range check of the sequence
	assert((_base_frame_no >= base_frame_no) && (_base_frame_no + _n_frames < base_frame_no + nFrames)); 
        unsigned int bitmap_index = (_base_frame_no - base_frame_no)/4;
        unsigned char mask = 0x40 >> 2*((_base_frame_no - base_frame_no)%4);
//chking if its free or not   // chk and redo 
	assert((bitmap[bitmap_index] & mask) == mask);
//marking the first one as head of sequence		
		int j =0;
		while(mask != 0x01)
		{
			j++;	// this is to find the shift 
			mask = mask >> 2;
		}
		unsigned char A = ~(0xFF << 2*j);
		unsigned char B = 0xFC << 2*j;
		mask = A | B;
		bitmap[bitmap_index] = bitmap[bitmap_index] & mask ;
		bitmap[bitmap_index] = bitmap[bitmap_index] ^ (0x02<<2*j); 
//marking rest of the bits as allocated 
		int i = bitmap_index; j --; _n_frames --;
		while(_n_frames != 0)
		{
			while(j != 0 && _n_frames != 0)
			{
				j--; _n_frames --;
				A = ~(0xFF << 2*j);
				B = 0xFC << 2*j;
				mask = A|B;
				bitmap[i] = bitmap[i] & mask;
			}
			j = 3; i ++;
		}
	
}



void ContFramePool::release_frames(unsigned long _first_frame_no)
{
	unsigned int i =0;
	unsigned int frame_found = 0;
	for(i=0; i!=index; i++)
	{
		if((_first_frame_no >= (list_pools[i] -> base_frame_no)) && (_first_frame_no <= ((list_pools[i] -> base_frame_no) + (list_pools[i] -> nFrames))))
		{	
			frame_found = 1;
			list_pools[i] -> specific_release_frames(_first_frame_no);	
		}
	}
	if(frame_found == 0)
	{
		Console::puts("Invalid Frame number given \n");
		assert(false);
	}
}


void ContFramePool::specific_release_frames(unsigned long _first_frame_no)
{
	Console::puts("In release_frame fn:-frame_no being released:  ");Console::puti(_first_frame_no);Console::puts("\n");
	
	unsigned int bitmap_index = (_first_frame_no - base_frame_no)/4;
	unsigned char mask = 0x80 >> 2*((_first_frame_no - base_frame_no)%4);
    	int j =  ((_first_frame_no - base_frame_no)%4);
	if((bitmap[bitmap_index] & mask) != mask)
	{
	Console::puts("Error, First Frame being released is not head of the sequence\n");
	assert(false);
	}
	int reached =0;
	mask = (0x3F>>2*j)|(~(0xFF>>2*j));
	bitmap[bitmap_index] =bitmap[bitmap_index] & mask;
    	while(reached != 1 && ((4*bitmap_index) <= nFrames))
	{
		while(j<4 && (reached != 1))
		{
			mask = (0x1F>>2*j)|(~(0xFF>>2*j));
			if((bitmap[bitmap_index] & mask) == 0x01)
			{
				reached = 1 ;
				break;
			}
			else 
			{	
				mask = 0x40 >> 2*j;
				bitmap[bitmap_index]= bitmap[bitmap_index] | mask; 		    	
			}
			j++;
		}
		if(j=4)
		{		
		bitmap_index ++ ; j =0;
		}
	}
			
}
unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
   // assert(false);
	return ((_n_frames / 16384) + ((_n_frames % 16384) > 0 ? 1 : 0)); 
}
