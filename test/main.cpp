#include <iostream>
#include <thread>
#include <string>
#include <functional>
#include <cmath>
#include <chrono>

#include "scheduler.h"
#include "active_object.h"
#include "future.h"

using namespace std;

double dotp(vector<double>& a, vector<double>& b, int s, int e) {
    double sum = 0.0;
    for (int i = s; i < e; ++i) {
        for (int j = 0; j < 75; ++j)
            sum += sqrt(abs(a[i])) * sqrt(abs(b[i]));
    }
    return sum;
}

double dotpfj(Scheduler& sch, vector<double>& a, vector<double>& b, 
                int s, int e) 
{
    if (e - s < 10'000) {
        return dotp(a, b, s, e);
    }

    int mid = s + (e - s) / 2;
    auto f = sch.fork<double>([&sch, &a, &b, s, mid]() { 
        return dotpfj(sch, a, b, s, mid); 
    });
    double r = dotpfj(sch, a, b, mid, e);
    double l = f.get();
    return r + l;
}

double big_serial_job(double num, int len){
    for(int i = 0; i < len; ++i){
        num = sqrt(pow(i,num) + pow(num,i)) + sqrt(num*(i+num));
    }
    return num;
}

void test_scheduler2() {
    Scheduler s(8);
    vector<double> a(300'000'000);
    vector<double> b(300'000'000);
    for (int i = 0; i < a.size(); ++i) {
        a[i] = 1.0;
        b[i] = 2.0;
    }

    int start = 0;
    int end = a.size();

    cout << "starting parallel\n";
    auto beg = chrono::system_clock::now();
    auto f = s.schedule<double>([&s, &a, &b, start, end]() {
        return dotpfj(s, a, b, start, end);
    });

    double val = f.get();
    auto fin = chrono::system_clock::now();
    auto dur = chrono::duration_cast<chrono::milliseconds>(fin-beg).count();

    cout << val << " took " << dur << " ms." << endl;


    std::this_thread::sleep_for(10s);

    cout << "starting parallel\n";
    beg = chrono::system_clock::now();
    f = s.schedule<double>([&s, &a, &b, start, end]() {
        return dotpfj(s, a, b, start, end);
    });

    val = f.get();
    fin = chrono::system_clock::now();
    dur = chrono::duration_cast<chrono::milliseconds>(fin-beg).count();

    cout << val << " took " << dur << " ms." << endl;

    cout << "starting serial\n";
    beg = chrono::system_clock::now();
    val = dotp(a, b, 0, a.size());
    fin = chrono::system_clock::now();
    dur = chrono::duration_cast<chrono::milliseconds>(fin-beg).count();
    cout << val << " took " << dur << " ms." << endl;
    s.terminate();
}

void idletimer(){
    Scheduler s(8);
    vector<double> a(1'000'000'000);
    vector<double> b(1'000'000'000);
    for (int i = 0; i < a.size(); ++i) {
        a[i] = 1.0;
        b[i] = 2.0;
    }

    int start = 0;
    int end = a.size();

    cout << "starting parallel\n";
    auto beg = chrono::system_clock::now();
    auto f = s.schedule<double>([&s, &a, &b, start, end]() {
        return dotpfj(s, a, b, start, end);
    });

    auto f2 = s.schedule<double>([](){return big_serial_job(456,100'000'000);});

    double val  = f.get();
    double val2 = f2.get();
    auto fin = chrono::system_clock::now();
    double dur = chrono::duration_cast<chrono::milliseconds>(fin-beg).count();

    cout << "Both jobs took " << dur << " ms." << endl;
    cout << "The dot_product value is " << val << " and the long serial job value is " << val2 << endl; 
    auto idles = s.total_idle();

    for(int i = 0; i<8; ++i)
        cout << "thread-" << (i+1) << " idled for " << idles[i] << " ms and the worker was working " << (100*(1-(idles[i]/dur))) << "% of the time." << endl;


    s.terminate();
}



void test4() {
    Scheduler s(12);

    for (int i = 0; i < 100; ++i) {
        s.schedule<void>([i]() { 
            chrono::seconds dur(10 - i/10);
            this_thread::sleep_for(dur);
            cout << this_thread::get_id() << " hi " << i << endl; });
    }

    s.terminate();
}

int main() {
    // demonstrates work stealing, sleeping, and speed up from serial
    // test_scheduler2();

    // displays idle time and percent time working
    idletimer();
}