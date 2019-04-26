/***

  Olive - Non-Linear Video Editor
  Copyright (C) 2019  Olive Team

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

***/

#include "framecache.h"


FrameCache::FrameCache()
{

}

FrameCache::~FrameCache()
{
  clear();
}

void FrameCache::lock()
{
  queue_lock.lock();
}

bool FrameCache::tryLock()
{
  return queue_lock.tryLock();
}

void FrameCache::unlock()
{
  queue_lock.unlock();
}

void FrameCache::append(AVFrame *frame)
{
  queue.append(frame);
}

AVFrame *FrameCache::at(int i)
{
  return queue.at(i);
}

AVFrame *FrameCache::first()
{
  return queue.first();
}

AVFrame *FrameCache::last()
{
  return queue.last();
}

void FrameCache::removeFirst()
{
  removeAt(0);
}

void FrameCache::removeLast()
{
  removeAt(queue.size()-1);
}

void FrameCache::removeAt(int i)
{
  av_frame_free(&queue[i]);
  queue.removeAt(i);
}

void FrameCache::clear()
{
  while (queue.size() > 0) {
    removeAt(0);
  }
}

int FrameCache::size()
{
  return queue.size();
}

bool FrameCache::isEmpty()
{
  return queue.isEmpty();
}

bool FrameCache::contains(AVFrame *frame)
{
  return queue.contains(frame);
}
