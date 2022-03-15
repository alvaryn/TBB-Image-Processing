
//
// Threading Building Blocks demo - Reduction
//

#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>
#include <tbb/blocked_range2d.h>
#include <cstdlib>
#include <random>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <functional>
#include <vector>
#include <string>
#include <FreeImagePlus.h>


using namespace std;
using namespace std::chrono;
using namespace tbb;




int main(int argc, char **argv) {

    // Explicit initialisation of TBB with default number of threads
    int nt = task_scheduler_init::default_num_threads();
    task_scheduler_init T(nt);

    cout << "Default number of threads = " << nt << "\n\n";

    // Setup random number generator
    random_device rd;
    mt19937 mt(rd());
    uniform_int_distribution<int> iDist(0, 100);


    // Initialse array of n integers using parallel_for
    const int n = 10;
    vector<int> a(n);

    parallel_for(blocked_range<int>(0, n), [&](const blocked_range<int>& range) {

        for (int i = range.begin(); i < range.end();++i)
            a[i] = iDist(mt);
    });

    // Output numbers to sum to the console...
    for (int i = 0; i < n; ++i)
        cout << a[i] << endl;


    // Apply parallel reduce (+)
    int x = parallel_reduce(

            blocked_range<int>(0, n), // Range over the array [0, n)

            0,  // Additive identity (during reduction we have: 0 + .... )

            // Main lambda to evaluate over the given range
            [&](const blocked_range<int>& range, int initValue)->int {
                for (int i = range.begin(); i != range.end(); i++) {
                    initValue += a[i];
                }
                return initValue;
            },

            // Sum the results obtained from the sub-range lambda above (see lecture slides)
            [&](int x, int y)->int {
                return x + y;
            }
    );


    cout << "Summed values = " << x << endl;
}
