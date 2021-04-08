#include <iostream>
#include <mutex>
#include <zconf.h>
#include "CPPTimer.h"

using namespace std;

class TestClass {
public:
    bool test_func(int flag) {
        lock_guard<mutex> lk(os_mutex);
        cout << __FUNCTION__ << ":" << __LINE__ << ", flag: " << flag << " cnt: " << ++cnt << endl;
        return true;
    }

private:
    mutex os_mutex;
    std::atomic<uint32_t> cnt{0};
};


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

int main() {
    TestClass test_instance;
    CPPTimer timer1(&TestClass::test_func, &test_instance, CPPTimer::Config(), 1);  // print 1 for 10 times
    CPPTimer timer2(&TestClass::test_func, &test_instance, CPPTimer::Config(1), 2); // print 2 for 1 times
    CPPTimer timer3(&TestClass::test_func, &test_instance, CPPTimer::Config(1), 3); // print 3 for 1 times

    timer1.start();
    timer2.start();
    timer3.start();
    auto actual_cnt = 3;
    while (true) {
        if (actual_cnt > 0) {
            actual_cnt--;
        } else {
            timer1.stop();
        }

        sleep(1);
    }
    return 0;
}

#pragma clang diagnostic pop