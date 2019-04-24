#ifndef MEMORYCACHE_H
#define MEMORYCACHE_H

#include <QVector>

#include "rendering/framebufferobject.h"

/**
 * @brief The MemoryCache class
 *
 * The primary role of the MemoryCache is to manage framebuffers cached in memory while not exceeding user-defined
 * limits. Operations can request a buffer and the MemoryCache will either allocate new space in memory, or take an
 * old buffer and
 */
class MemoryCache
{
public:
  MemoryCache();

  void Purge();

  /**
   * @brief The MemoryBuffer class
   *
   * The main communication path between an object and the MemoryCache.
   *
   * TODO does this need to be locked to ensure the buffer is not disowned while in use?
   */
  class MemoryBuffer {
  public:
    MemoryBuffer();
    ~MemoryBuffer();

    void Close();

    FramebufferObject* Buffer();
    qint64 LastAccessTime();

  private:
    bool BufferIsValid();
    int buffer_;
    qint64 access_time_;
  };

  struct OwnedBuffer {
    FramebufferObject buffer;
    MemoryBuffer* owner;
  };

private:
  int RequestBuffer(MemoryBuffer* owner);
  void DisownBuffer(int i);
  int OldestBuffer();

  QVector<OwnedBuffer> buffers_;

  QVector<int> ownerless_buffers_;
};

namespace olive {
  extern MemoryCache memory_cache;
}

#endif // MEMORYCACHE_H
