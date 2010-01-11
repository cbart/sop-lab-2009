#include <iostream>
#include <vector>
#include <cassert>
#include <boost/foreach.hpp>

#include "sched.h"

#define foreach BOOST_FOREACH

const int THREADS_Q = 1000;
const int VECTOR_Q = 1000;

typedef std::vector<int> vi;

vi counter = vi(VECTOR_Q, 0);

void f()
{
    foreach(int& i, counter) {
        ++ i;
    }
}

int main()
{
    mysched_init();

    std::vector<mysched_thread_t> pool =
        std::vector<mysched_thread_t>(VECTOR_Q, 0);
    foreach(mysched_thread_t& thread_id, pool) {
        if((thread_id = mysched_create_thread(f, "some thread")) != 0)
            std::cout << thread_id << " ";
    }
    foreach(int& i, counter) {
        std::clog << i << ' ';
    }
    std::clog << std::endl;

    mysched_go();
}
