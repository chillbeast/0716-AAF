#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include "CThread.h"

namespace Ignite
{
	enum class TimeOut : uint32_t
	{
		kNoWait = 0,
		kWaitForever = 0xFFFFFFFF
	};

	template <class T>
	class CMsgQueue
	{
		public:
			CMsgQueue (uint32_t msg_count) : _size (msg_count)
			{
			}
			CMsgQueue (const CMsgQueue&) = delete; // disable copying
			CMsgQueue& operator= (const CMsgQueue&) = delete; // disable assignment
			CMsgQueue (CMsgQueue&&) = delete;
			CMsgQueue& operator= (CMsgQueue&&) = delete;

			bool put(const T& msg, TimeOut timeoutTicks = TimeOut::kWaitForever)
			{
				std::unique_lock<std::mutex> mlock(_mutex);
				if (CThread::desktopShutdownRequested()) return false;
				if (_queue.size() == _size && timeoutTicks == TimeOut::kNoWait)
				{
					return false;
				}

				while (_queue.size() >= _size)
				{
					_cond.wait_for(mlock, std::chrono::milliseconds(25));
					if (CThread::desktopShutdownRequested()) return false;
				}
				_queue.push(msg);
				mlock.unlock();
				_cond.notify_one();

				return true;
			}

			bool get (T& msg, TimeOut timeoutTicks)
			{
				std::unique_lock<std::mutex> mlock (_mutex);
				while (_queue.empty())
				{
					if (timeoutTicks == TimeOut::kNoWait || CThread::desktopShutdownRequested()) return false;
					_cond.wait_for(mlock, std::chrono::milliseconds(25));
				}
				if (CThread::desktopShutdownRequested()) return false;
				msg = _queue.front();
				_queue.pop();
				mlock.unlock();
				_cond.notify_all();
				return true;
			}

			bool reset (void)
			{
				std::lock_guard<std::mutex> lock(_mutex);
				std::queue<T> empty;
				std::swap(_queue, empty);
				_cond.notify_all();
				return true;
			}

			bool isEmpty (void)
			{
				std::lock_guard<std::mutex> lock(_mutex);
				return _queue.empty();
			}

			bool isFull (void) { return (numVacant() == 0); }

			uint32_t getSize (void) { return _size; }

			uint32_t numQueued (void) { std::lock_guard<std::mutex> lock(_mutex); return static_cast<uint32_t>(_queue.size()); }

			uint32_t numVacant (void) { std::lock_guard<std::mutex> lock(_mutex); return static_cast<uint32_t>(_size - _queue.size()); }

		private:
			std::queue<T> _queue;
			std::mutex _mutex;
			std::condition_variable _cond;
			const uint32_t _size;
	};
}; // namespace Ignite

