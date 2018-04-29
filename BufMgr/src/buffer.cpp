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
}

BufMgr::~BufMgr() {
//flush out all dirty
//deallocate buffer pool and bufferdesk table
  for(uint32_t i = 0; i < numBufs; i++){
    if(bufDescTable[i].valid && bufDescTable[i].dirty) {
      //valid and needs to be written
      flushFile(bufDescTable[i].file);
    }
  }
  delete [] bufDescTable;
  delete [] bufPool;
  delete hashTable;
}

void BufMgr::advanceClock()
{
  //clock hand needs to rotate around a virtual circle from 0 to numBufs - 1
  clockHand++; 
  if(clockHand == numBufs)  //should this work?
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
    if(!bufDescTable[clockHand].valid) {
      frame = clockHand; 
      found = true; 
      continue;
    }
    //clear ref bit
    if (bufDescTable[clockHand].refbit) {
      bufDescTable[clockHand].refbit = true; 
      continue;
    }
   //page is pinned
   if(bufDescTable[clockHand].pinCnt > 0) {
     continue;
   }
   //this page can be used, check if it needs to be written 
   if(bufDescTable[clockHand].dirty) {
     //write page to memory by sending the address in bufPool that corresponds to our frame 
     bufDescTable[clockHand].file->writePage(bufPool[clockHand]);  
   } 

    //clear entries
    bufDescTable[i].Clear();
    frame = clockHand;
    found = true; 
    
 }
  if(!found) //not found throw exception 
    throw BufferExceededException();
}

	
void BufMgr::readPage(File* file, const PageId pageNo, Page*& page)
{
  FrameId frame; 
  try {
    hashTable->lookup(file, pageNo, frame); //if successful, frame is valid 
    bufDescTable[frame].refbit = true;
    bufDescTable[frame].pinCnt += 1; 
    page = &bufPool[frame]; //sets return value   
  } catch (HashNotFoundException exp) {
      allocBuf(frame); //allocates frame
      bufPool[frame] = file->readPage(pageNo); //reads page from disk to bufpool
      hashTable->insert(file, pageNo, frame);  //insert page into hashtable
      bufDescTable[frame].Set(file, pageNo); //sets the buffer description 
      page = &bufPool[frame]; //sets return value
  }
}


void BufMgr::unPinPage(File* file, const PageId pageNo, const bool dirty) 
{
  FrameId frame; 
  try {
    hashTable->lookup(file, pageNo, frame); //finds frame number
    if(bufDescTable[frame].pinCnt == 0) //check for pinCnt going to negative
      throw PageNotPinnedException(file->filename(), pageNo, frame);
    bufDescTable[frame].pinCnt -= 1; //decrements
    if (dirty) //check for dirty bit
      bufDescTable[frame].dirty = true;

  } catch (HashNotFoundException ex ) {
    //page doesnt exist
  }
    

}

void BufMgr::flushFile(const File* file) 
{
  //iterate all frames
  for(uint32_t i = 0; i < numBufs; i++){
    if(bufDescTable[i].file == file) {
      //this index belongs to file
      FrameId frameNum = bufDescTable[i].frameNo;
      PageId pageNum = bufDescTable[i].pageNo;

      if(!bufDescTable[i].valid ) {
        throw BadBufferException(frameNum, bufDescTable[i].dirty, bufDescTable[i].valid,
          bufDescTable[i].refbit ); 
      }
 
      if(bufDescTable[i].pinCnt > 0) {
        throw PagePinnedException(file->filename(), pageNum, frameNum); 
      }

     if(bufDescTable[i].dirty) {
        bufDescTable[i].file->writePage(bufPool[frameNum]);
        bufDescTable[i].dirty = false;
      }

      hashTable->remove(file, pageNum);
      bufDescTable[i].Clear();
    }
  }
}

void BufMgr::allocPage(File* file, PageId &pageNo, Page*& page) 
{
  FrameId frameNum; 
  Page newPage;
  newPage = file->allocatePage(); //new page allocated
  pageNo = newPage.page_number(); //gets page number, return value is set

  allocBuf(frameNum); //gets a valid frame
  hashTable->insert(file, pageNo, frameNum); //hashTable 

  bufDescTable[frameNum].Set(file, pageNo); //frame descriptor is set 
  bufPool[frameNum] = newPage; //page is filled
  page = &bufPool[frameNum]; //return value is set
  
}

void BufMgr::disposePage(File* file, const PageId PageNo)
{
  FrameId frameNum; 
   try {
     hashTable->lookup(file, PageNo,frameNum );
     bufDescTable[frameNum].Clear(); //descriptor cleared
     hashTable->remove(file, PageNo); //hashtable entry removed
   } catch (HashNotFoundException ex) {
     //nothing needs to be done if not found
   } 
  file->deletePage(PageNo); //file deleted
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
