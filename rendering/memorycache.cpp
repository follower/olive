#include "memorycache.h"

#include <QDateTime>

MemoryCache olive::memory_cache;

MemoryCache::MemoryCache()
{
}

void MemoryCache::Purge()
{
  buffers_.clear();
  ownerless_buffers_.clear();
}

int MemoryCache::RequestBuffer(MemoryCache::MemoryBuffer *owner)
{
  bool out_of_memory = false;

  int buffer_index;

  if (!ownerless_buffers_.isEmpty()) {

    // If we have an ownerless buffer, give one of those
    buffer_index = ownerless_buffers_.first();
    ownerless_buffers_.removeFirst();

  } else if (out_of_memory) {

    // Decommission oldest buffer (i.e. the buffer access the longest time ago)
    buffer_index = OldestBuffer();

  } else {
    // TODO do a memory check and see if we have an "oldest" buffer we can disown before creating a new buffer

    // Create a new buffer
    buffer_index = buffers_.size();
    buffers_.append(OwnedBuffer());
  }

  // Give the calling memory buffer ownership of this
  buffers_[buffer_index].owner = owner;

  return buffer_index;
}

void MemoryCache::DisownBuffer(int i)
{
  ownerless_buffers_.append(i);
}

int MemoryCache::OldestBuffer()
{
  int oldest_buffer = 0;

  for (int i=1;i<buffers_.size();i++) {
    if (buffers_.at(i).owner->LastAccessTime() < buffers_.at(oldest_buffer).owner->LastAccessTime()) {
      oldest_buffer = i;
    }
  }

  return oldest_buffer;
}

MemoryCache::MemoryBuffer::MemoryBuffer() :
  buffer_(-1)
{
}

MemoryCache::MemoryBuffer::~MemoryBuffer()
{
  Close();
}

void MemoryCache::MemoryBuffer::Close()
{
  if (BufferIsValid()) {
    olive::memory_cache.DisownBuffer(buffer_);
  }
}

FramebufferObject *MemoryCache::MemoryBuffer::Buffer()
{
  // Check if the buffer is still valid and that this is still the owner of the buffer
  if (!BufferIsValid()) {
    buffer_ = olive::memory_cache.RequestBuffer(this);
  }

  // Record access time
  access_time_ = QDateTime::currentMSecsSinceEpoch();

  // Return buffer
  return &olive::memory_cache.buffers_[buffer_].buffer;
}

qint64 MemoryCache::MemoryBuffer::LastAccessTime()
{
  return access_time_;
}

bool MemoryCache::MemoryBuffer::BufferIsValid()
{
  return buffer_ >= 0
      && buffer_ < olive::memory_cache.buffers_.size()
      && olive::memory_cache.buffers_.at(buffer_).owner == this;
}
