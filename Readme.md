# CPPTimer
A simple header only c++ timer using linux timer_create and timer_settime.

## Features
* [x]  Support class-member and non-class-member function call.
* [x]  Support steady clock and wall clock time counting.
* [x]  Support run time, waitting time cycle.

```cpp
using namespace std;

static mutex os_mutex;

class TestClass {
public:
    bool test_func(int flag) {
        lock_guard<mutex> lk(os_mutex);
        cout << __FUNCTION__ << ":" << __LINE__ << ", flag: " << flag << " cnt: " << ++cnt << endl;
        return true;
    }

private:
    std::atomic<uint32_t> cnt{0};
};

bool timer_func2(int flag) {
    static std::atomic<uint32_t> cnt{0};
    lock_guard<mutex> lk(os_mutex);
    cout << __FUNCTION__ << ":" << __LINE__ << ", flag: " << flag << " cnt: " << ++cnt << endl;
    return true;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

int main() {
    TestClass test_instance;
    CPPTimer timer1(&TestClass::test_func, &test_instance, CPPTimer::Config(10), 1);  // print flag 1 for 10 times
    CPPTimer timer2(&TestClass::test_func, &test_instance, CPPTimer::Config(1), 2); // print flag 2 for 1 times
    CPPTimer timer3(&TestClass::test_func, &test_instance, CPPTimer::Config(1), 3); // print flag 3 for 1 times
    CPPTimer timer4(&timer_func2, CPPTimer::Config(4), 4);

    timer1.start();
    timer2.start();
    timer3.start();
    timer4.start();
    auto actual_cnt = 3;
    while (true) {
        if (actual_cnt > 0) {
            actual_cnt--;
        } else {
            timer1.stop(); // actual print flag 1 for 5 times
        }

        sleep(1);
    }
    return 0;
}

#pragma clang diagnostic pop

```
