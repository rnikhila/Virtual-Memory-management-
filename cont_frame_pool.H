/*
 File: cont_frame_pool.H
 
 Author: R. Bettati
 Department of Computer Science
 Texas A&M University
 Date  : 17/02/04 
 
 Description: Management of the CONTIGUOUS Free-Frame Pool.
 
 As opposed to a non-contiguous free-frame pool, here we can allocate
 a sequence of CONTIGUOUS frames.
 
 */

#ifndef _CONT_FRAME_POOL_H_                   // include file only once
#define _CONT_FRAME_POOL_H_

/*-------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "machine.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* C o n t F r a m e   P o o l  */
/*--------------------------------------------------------------------------*/

class ContFramePool {
    
private:
    /* -- DEFINE YOUR CONT FRAME POOL DATA STRUCTURE(s) HERE. */
    unsigned char * bitmap;
    unsigned int nFreeFrames;
    unsigned long base_frame_no;
    unsigned long nFrames;
    unsigned long info_frame_no;
    unsigned long n_info_frames;

    static ContFramePool** list_pools;
    static int index;
    void specific_release_frames(unsigned long _first_frame_no);

public:
   // The frame size is the same as the page size, duh...    
    static const unsigned int FRAME_SIZE = Machine::PAGE_SIZE; 

    ContFramePool(unsigned long _base_frame_no,
                  unsigned long _n_frames,
                  unsigned long _info_frame_no,
                  unsigned long _n_info_frames);
    unsigned long get_frames(unsigned int _n_frames);
   
    void mark_inaccessible(unsigned long _base_frame_no,
                           unsigned long _n_frames);
   
     static void release_frames(unsigned long _first_frame_no);
   
     unsigned long needed_info_frames(unsigned long _n_frames);

};
#endif
