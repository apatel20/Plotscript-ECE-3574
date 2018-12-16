#include "threadsafequeue.hpp"

template<typename T>
void ThreadSafeQueue<T>::push(const T & value)
{
  std::unique_lock<std::mutex> lock(mutex);
  q.push(value);
  lock.unlock();
  condition.notify_one();
}

template<typename T>
bool ThreadSafeQueue<T>::empty() const
{
  std::lock_guard<std::mutex> lock(mutex);
  return q.empty();
}

template<typename T>
bool ThreadSafeQueue<T>::try_pop(T & popped)
{
  std::lock_guard<std::mutex> lock(mutex);
  if (q.empty())
  {
    return false;
  }
  popped = q.front();
  q.pop();
  return true;
}

template<typename T>
void ThreadSafeQueue<T>::wait_pop(T & poppedVal)
{
  std::unique_lock<std::mutex> lock(mutex);
  while (q.empty())
  {
    condition.wait(lock);
  }
  poppedVal = q.front();
  q.pop();
}
