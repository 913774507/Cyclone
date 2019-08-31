/**********************************************************
 * Author        : RaKiRaKiRa
 * Email         : 763600693@qq.com
 * Create time   : 2019-08-30 23:49
 * Last modified : 2019-08-31 20:08
 * Filename      : httpStaticFile.cc
 * Description   : 
 **********************************************************/

#include "httpStaticFile.h"
#include "../base/Logging.h"
#include <unistd.h>
#include <sys/mman.h>

StaticFile::StaticFile(std::string filename):
  file_(std::move(filename)),
  closed(true)
{
  if(stat(filename.c_str(), &sbuf_) < 0)
  {
    state_ = NotFound;
    LOG_DEBUG << filename << " NotFound";
  }
  else
  {
    state_ = OK;
    LOG_DEBUG << filename << " OK";
  }
}

bool StaticFile::open()
{
  assert(closed == true);
  if(state_ == NotFound)
    return false;
  fd_ = ::open(file_.c_str(), O_RDONLY, 0);
  // 打开失败
  if(fd_ < 0)
  {
    state_ = ReadError;
    LOG_DEBUG << file_ << " ReadError";
    return false;
  }
  closed = false;
  return true;
}

bool StaticFile::writeTo(std::string& body)
{
  if(closed)
  {
    if(!open())
      return false;
  }

  assert(closed == false);
  // 还是多一层保险吧。。。。多确认确认
  if(fd_ <= 0)
    return false;
  void* mmapRet = mmap(NULL , sbuf_.st_size, PROT_READ, MAP_PRIVATE, fd_, 0);

  // 关闭fd，后面用mmap操作
  closed = true;
  ::close(fd_);

  // 若打开失败
  if(mmapRet == (void*)-1)
  {
    munmap(mmapRet, sbuf_.st_size);
    state_ = ReadError;
    return false;
  }
  char* contentAddr = static_cast<char*>(mmapRet);
  // 写入body
  body.assign(contentAddr, contentAddr + sbuf_.st_size);
  munmap(mmapRet, sbuf_.st_size);
  LOG_DEBUG << file_ <<" size is " << sbuf_.st_size <<", write to body " << body.size() ;
  return true;
}