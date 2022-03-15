//
// Threading Building Blocks demo - Cancellation
//

#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
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



struct StaffMember {

    string		name = string("Bilbo");
    int			room = 0;

public:

    StaffMember(const string& _name, const int _room) {

        name = _name;
        room = _room;
    }
};



// Return the index in the given vector of the staff member with the specified name
int findStaffMember(const vector<StaffMember>& staff, const string& searchName) {

    int returnIndex = -1;

    parallel_for(

            blocked_range<int>(0, staff.size()),

            [&](const blocked_range<int>& range) {

                for (int i = range.begin(); i != range.end(); i++) {

                    if (staff[i].name == searchName) {

                        if (task::self().cancel_group_execution()) {
                            returnIndex = i;
                            cout << "cancelling at index " << i << endl; // Report index to confirm we're cancelling!
                        }
                    }
                }
            }
    );

    return returnIndex;
}


int main(int argc, char **argv) {

    // Explicit initialisation of TBB with default number of threads
    int nt = task_scheduler_init::default_num_threads();
    task_scheduler_init T(nt);

    cout << "Default number of threads = " << nt << "\n\n";


    // Define and populate array
    vector<StaffMember> staff;

    staff.push_back(StaffMember(string("Farthing"), 125));
    staff.push_back(StaffMember(string("Davies"), 128));
    staff.push_back(StaffMember(string("Tudhope"), 230));
    staff.push_back(StaffMember(string("Angel"), 124));
    staff.push_back(StaffMember(string("Blyth"), 123));
    staff.push_back(StaffMember(string("Morris"), 124));
    staff.push_back(StaffMember(string("Wilson"), 306));
    staff.push_back(StaffMember(string("Kidner"), 311));
    staff.push_back(StaffMember(string("Cunliffe"), 311));
    staff.push_back(StaffMember(string("Verheyden"), 224));
    staff.push_back(StaffMember(string("Griffiths"), 208));
    staff.push_back(StaffMember(string("Sutherland"), 129));
    staff.push_back(StaffMember(string("Lewis"), 227));
    staff.push_back(StaffMember(string("Mulley"), 229));
    staff.push_back(StaffMember(string("Plassmann"), 150));
    staff.push_back(StaffMember(string("Ring"), 150));
    staff.push_back(StaffMember(string("Inglis"), 312));
    staff.push_back(StaffMember(string("Ware"), 303));
    staff.push_back(StaffMember(string("Griffiths"), 302));
    staff.push_back(StaffMember(string("Price"), 33));


    // parallel_for with cancellation

    string personToFind = string("Angel");

    int i = findStaffMember(staff, personToFind);

    if (i >= 0)
        cout << personToFind << " at index " << i << " is in room " << staff[i].room << endl;
    else
        cout << personToFind << " not found\n";
}
