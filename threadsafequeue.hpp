#ifndef THREADSAFEQUEUE_HPP
#define THREADSAFEQUEUE_HPP

#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <iostream>

template<typename T>
class ThreadSafeQueue
{
private:
  std::queue<T> q;
  mutable std::mutex mutex;
  std::condition_variable condition;

public:
  void push(const T & value);

  bool empty() const;

  bool try_pop(T & popped);

  void wait_pop(T & poppedVal);
};

#include "threadsafequeue.tpp"

#endif
