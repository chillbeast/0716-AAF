#include "../CHardwareTimer.h"
#include <cstdlib>
#include <iostream>

#ifdef __linux__
#include <climits>
#define SEC_TO_NSEC_SCALER 1000000000
#define USEC_TO_NSEC_SCALER 1000
#endif

using namespace Ignite;

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

CHardwareTimer::CHardwareTimer(HardwareTimerCb cb, void* context)
    : _callback(cb)
    , _context(context)
{
    _id = std::rand() % 100;
}

CHardwareTimer::~CHardwareTimer()
{
#ifdef _WIN32
    shutdownDesktop();
#else
    stop();
#endif
}
void CHardwareTimer::shutdownDesktop()
{
#ifdef _WIN32
    {
        std::lock_guard<std::mutex> lock(_timerMutex);
        _activeCB = false;
        _running = false;
        ++_timerRevision;
    }
    _timerChanged.notify_all();
    if (_thread.joinable()) _thread.join();
#endif
}
bool CHardwareTimer::start(const uint16_t intervalus)
{
    if (intervalus == 0) return false;
#ifdef _WIN32
    {
        std::lock_guard<std::mutex> lock(_timerMutex);
        _intervalus = intervalus;
        _activeCB = true;
        ++_timerRevision;
        if (!_running)
        {
            _running = true;
            _thread = std::thread(&CHardwareTimer::hiResTimerThread, this);
        }
    }
    _timerChanged.notify_all();
    return true;
#endif
    _intervalus = intervalus;
#ifdef __linux__
    struct timespec ts;
    if (clock_gettime (CLOCK_MONOTONIC, &ts) < 0)
    {
        perror("clock_gettime");
        exit(1);
    }
    printf ("Start : Cur time %llu : %llu\n", ts.tv_sec, ts.tv_nsec);
    long long unsigned int currentTimeNs = (ts.tv_sec * SEC_TO_NSEC_SCALER) + ts.tv_nsec;
    long long unsigned int nextExpiryNs = currentTimeNs + (_intervalus * USEC_TO_NSEC_SCALER);
    _timepoint.tv_sec = nextExpiryNs / SEC_TO_NSEC_SCALER;
    _timepoint.tv_nsec = nextExpiryNs % SEC_TO_NSEC_SCALER;
    printf("_intervalus = %u\n", _intervalus);
    printf("Start : Next expiry %llu : %llu\n", _timepoint.tv_sec, _timepoint.tv_nsec);
    if (_running)
    {
        _activeCB = true;// thread we are not stoping
        return true;
    }
    struct sched_param param;
    int policy = SCHED_FIFO; // Use SCHED_FIFO for real-time scheduling
    int max_priority = sched_get_priority_max(policy);
    param.sched_priority = max_priority;

    if (pthread_create(&_thread, nullptr, &CHardwareTimer::loop, this) != 0)
    {
        std::cerr << "Failed to create thread" << std::endl;
        return EXIT_FAILURE;
    }
    _running = true;
    _activeCB = true;//updating below may not reach, as thread started safe to update now
    if (pthread_setschedparam(_thread, policy, &param) != 0)
    {
        std::cerr << "pthread_setschedparam failed: " << strerror(errno) << std::endl;
    }
    else
        std::cout << "pthread setschedparam success" << max_priority << std::endl;

    /* disabled CPU set - disabled bez thread timer has more distraction */
    //cpu_set_t cpuset;
    //CPU_ZERO(&cpuset);
    //CPU_SET(3, &cpuset); // Set affinity to CPU 0

    //if (pthread_setaffinity_np(_thread, sizeof(cpu_set_t), &cpuset) != 0) {
    //    std::cerr << "pthread_setaffinity_np failed: " << strerror(errno) << std::endl;
    //    return EXIT_FAILURE;
    //}
    //else {
    //    std::cout << "pthread_setaffinity_np success" << std::endl;
    //}
    pthread_detach(_thread);

#endif
     _running = true;
    _activeCB = true;
    return true;
}

bool CHardwareTimer::stop(void)
{
#ifdef _WIN32
    {
        std::lock_guard<std::mutex> lock(_timerMutex);
        _activeCB = false;
        ++_timerRevision;
    }
    _timerChanged.notify_all();
    return true;
#endif
    if (_running)
    {
    //_running = false; -- not stoping
    _activeCB = false;// disable the Call back
#ifdef __linux__
    //pthread_join(_thread, nullptr);
#endif
    }
    return true;
}

bool CHardwareTimer::changeInterval(const uint16_t intervalus)
{
    if (intervalus == 0) return false;
#ifdef _WIN32
    {
        std::lock_guard<std::mutex> lock(_timerMutex);
        _intervalus = intervalus;
        ++_timerRevision;
    }
    _timerChanged.notify_all();
    // Changing tempo while paused must not restart playback.
    return true;
#endif
    if (!_running)
    {
        start(intervalus);
    }
    else
    {
        _intervalus = intervalus;

#ifdef __linux__
        struct timespec ts;
        if (clock_gettime (CLOCK_MONOTONIC, &ts) < 0)
        {
            perror ("clock_gettime");
            return false;
        }
        //printf ("Change : Cur time %llu : %llu\n", ts.tv_sec, ts.tv_nsec);
        long long unsigned int currentTimeNs = (ts.tv_sec * SEC_TO_NSEC_SCALER) + ts.tv_nsec;
        long long unsigned int nextExpiryNs = currentTimeNs + (_intervalus * USEC_TO_NSEC_SCALER);
        _timepoint.tv_sec = nextExpiryNs / SEC_TO_NSEC_SCALER;
        _timepoint.tv_nsec = nextExpiryNs % SEC_TO_NSEC_SCALER;
        DBG("_intervalus :" + String(_intervalus));
        //printf ("Change : Next expiry %llu : %llu\n", _timepoint.tv_sec, _timepoint.tv_nsec);
        //std::cout << _intervalus << std::endl;;
#endif
    }
    return true;
}

uint32_t CHardwareTimer::getFrequency(void) const
{
    return 1000000;
}

int8_t CHardwareTimer::getId(void) const
{
    return _id;
}


#ifdef __linux__
void* CHardwareTimer::loop(void* arg) {
    CHardwareTimer* timer = static_cast<CHardwareTimer*>(arg);
    if (arg == nullptr) {
        std::cerr << "CHardwareTimer::loop Error: arg is nullptr" << std::endl;
        return nullptr;
    }
    timer->run();
    return nullptr;
}

void CHardwareTimer::run(void)
{
    while (_running)
    {
        struct timespec ts;
        if (!_activeCB)
        {
            std::this_thread::yield();
            continue;
        }
        if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
        {
            perror("clock_gettime");
            continue;
        }
        long long unsigned int currentTimeNs = (ts.tv_sec * SEC_TO_NSEC_SCALER) + ts.tv_nsec;
        long long unsigned int expiryTimeNs = (_timepoint.tv_sec * SEC_TO_NSEC_SCALER) + _timepoint.tv_nsec;
        //std::cout << "currentTimeNs: " << currentTimeNs << ", expiryTimeNs: " << expiryTimeNs << " _activeCB " << _activeCB << std::endl;
        //if (currentTimeNs == 0) {
        //    std::cout << "currentTimeNs is zero, likely due to system boot or clock initialization." << std::endl;
        //    continue;
        //}
        if ((currentTimeNs >= expiryTimeNs) && _callback)
        {
            long long unsigned int nextExpiryNs = expiryTimeNs + (_intervalus * USEC_TO_NSEC_SCALER);
            _timepoint.tv_sec = nextExpiryNs / SEC_TO_NSEC_SCALER;
            _timepoint.tv_nsec = nextExpiryNs % SEC_TO_NSEC_SCALER;
            _callback(_context);
        }
    }
}
#endif /* __linux__ */

#ifdef _WIN32

// A steady deadline avoids the original time-critical busy loop and cumulative drift.
// Callback work runs outside _timerMutex so it can pause or change tempo itself.
void CHardwareTimer::hiResTimerThread(void)
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
    std::unique_lock<std::mutex> lock(_timerMutex);
    while (_running)
    {
        _timerChanged.wait(lock, [this] { return !_running || _activeCB; });
        if (!_running) break;
        const auto revision = _timerRevision;
        const auto interval = std::chrono::microseconds(_intervalus);
        auto deadline = std::chrono::steady_clock::now() + interval;
        while (_running && _activeCB && revision == _timerRevision)
        {
            if (_timerChanged.wait_until(lock, deadline, [this, revision]
                { return !_running || !_activeCB || revision != _timerRevision; })) break;
            lock.unlock();
            if (_callback != nullptr) _callback(_context);
            lock.lock();
            deadline += interval;
            // Do not burst thousands of old pulses after suspend or a long edit.
            const auto now = std::chrono::steady_clock::now();
            if (deadline + std::chrono::milliseconds(100) < now) deadline = now + interval;
        }
    }
}

#endif /* _WIN32 */
