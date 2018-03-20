/**
 * @author See Contributors.txt for code contributors and overview of BadgerDB.
 *
 * @section LICENSE
 * Copyright (c) 2012 Database Group, Computer Sciences Department, University of Wisconsin-Madison.
 */

#include <memory>
#include <iostream>
#include "buffer.h"
#include "exceptions/buffer_exceeded_exception.h"
#include "exceptions/page_not_pinned_exception.h"
#include "exceptions/page_pinned_exception.h"
#include "exceptions/bad_buffer_exception.h"
#include "exceptions/hash_not_found_exception.h"

//uint32_t buff_size; //holds size of table
namespace badgerdb { 

BufMgr::BufMgr(std::uint32_t bufs)
	: numBufs(bufs) {
	bufDescTable = new BufDesc[bufs];

  for (FrameId i = 0; i < bufs; i++) 
  {
  	bufDescTable[i].frameNo = i;
  	bufDescTable[i].valid = false;
  }

  bufPool = new Page[bufs];

	int htsize = ((((int) (bufs * 1.2))*2)/2)+1;
  hashTable = new BufHashTbl (htsize);  // allocate the buffer hash table
  clockHand = bufs - 1; //hand starts from last page
  //buff_size = bufs;


BufMgr::~BufMgr() {
//flush out all dirty
//deallocate buffer pool and bufferdesk table

}

void BufMgr::advanceClock()
{
  //clock hand needs to rotate around a virtual circle from 0 to numBufs - 1
  clockHand++; 
  if(clockHand == hashTable.HTSIZE)  //should this work?
    clockHand = 0;
}

void BufMgr::allocBuf(FrameId & frame) 
{
//refbit is set to true
//for every clock advance
  //if refbit is set, unset it
  //if refbit is not set, page is selected for replacement
    //assumed that page is not pinned
    //if dirty, write back to disk 
    //clear frame
    //new page is read to location

 bool found = false; 
  for(FrameId i = 0; i < numBufs && !found; i++) {
    advanceClock(); //next frame
    
    //found invalid page
    if(!bufDescTable[clockHAnd].valid) {
      frame = clockHand; 
      found = true; 
      continue
    }
    //clear ref bit
    if (bufDescTable[clockHand].refbit) {
      bufDescTable[clockHand].refbit = true; 
      continue
    }
   //page is pinned
   if(bufDescTable[clockHand].pinCnt > 0) {
     continue
   }
   //this page can be used, check if it needs to be written 
   if(bufDescTable[clockHand].dirty) {
     //write page to memory by sending the address in bufPool that corresponds to our frame 
     bufDescTable[clockHand].file->writePage(&bufPool[clockHand]);  
   } 

    //clear entries
    bufDescTable[i].clear();
    frame = clockHand;
    found = true; 
    
 }
  if(!found) //not found throw exception 
    throw BufferExceededException();
}

	
void BufMgr::readPage(File* file, const PageId pageNo, Page*& page)
{

}


void BufMgr::unPinPage(File* file, const PageId pageNo, const bool dirty) 
{
}

void BufMgr::flushFile(const File* file) 
{
}

void BufMgr::allocPage(File* file, PageId &pageNo, Page*& page) 
{
}

void BufMgr::disposePage(File* file, const PageId PageNo)
{
    
}

void BufMgr::printSelf(void) 
{
  BufDesc* tmpbuf;
	int validFrames = 0;
  
  for (std::uint32_t i = 0; i < numBufs; i++)
	{
  	tmpbuf = &(bufDescTable[i]);
		std::cout << "FrameNo:" << i << " ";
		tmpbuf->Print();

  	if (tmpbuf->valid == true)
    	validFrames++;
  }

	std::cout << "Total Number of Valid Frames:" << validFrames << "\n";
}

}
