//
// Threading Building Blocks demo 01 - parallel_for
//
#define _USE_MATH_DEFINES
#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/blocked_range2d.h>
#include <cstdlib>
#include <math.h>
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


// Example 1 - Square a 1D array of integers using the Map access pattern
void tbb_example1() {

    cout << "\n\nTBB example 1...\n\n";

    // Demo array size
    const int n = 100;

    // Setup random number generator and distribution [0, n]
    random_device rd;
    mt19937 mt(rd());
    uniform_int_distribution<int> iDist(0, n);


    vector<int>		input(n); // Input array
    vector<int>		output(n); // Output array


    // Populate input with random numbers between 0 and n (this is done sequentially for now)
    for (int i = 0; i<n; ++i)
        input[i] = iDist(mt);



    // Square numbers in the array (no interdependencies exist between the elements in the array so each element can be read, processed and the result written independently.  This is an example use of the Map data access pattern.

    // Sequential version
    for (int i = 0; i<n; i++)
        output[i] = input[i] * input[i];


    // parallel_for version - simple form for 1D arrays only where the parameters are (start index, end index, lambda)
    parallel_for(0, n, [&](int i) { output[i] = input[i] * input[i]; });


    // parallel_for blocked_range version
    tbb::parallel_for(blocked_range<int>(0, n), [&](const blocked_range<int>& range) {

        for (int i=range.begin(); i<range.end(); ++i)
            output[i] = input[i] * input[i];
    });
}


// Example 2 - Use the Stencil access pattern to implement a simple 1D blur filter
void tbb_example2() {

    cout << "\n\nTBB example 2...\n\n";

    // Demo array size
    const int n = 100;

    // Setup input array and populate with example data
    vector<float> floatInput(n);

    for (int i = 0; i<n; ++i)
        floatInput[i] = (sinf(float(i) / float(n) * 3.142f * 220.0f) + 1.0f) / 2.0f * (cosf(float(i) / float(n) * 3.142f * 150.0f) + 1.0f) / 2.0f + 0.4f;

    fipImage floatOutputImage;
    floatOutputImage = fipImage(FIT_FLOAT, n, 1, 32);
    float *floatOutput = (float*)floatOutputImage.accessPixels();

    // Use the Stencil access pattern to blur the data.
    tbb::parallel_for(blocked_range<int>(1, n-1), [&](const blocked_range<int>& range) {

        for (int i=range.begin(); i!=range.end(); i++)
            floatOutput[i] = floatInput[i-1] * 0.33f + floatInput[i] * 0.33f + floatInput[i+1] * 0.33f;
    });

    // Save result
    floatOutputImage.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
    floatOutputImage.convertTo24Bits();
    floatOutputImage.save("tbb_example2.bmp");
}


// Extend example 2 to invert 2D dataset (load image from disk to populate input array).  This is also based on the Map access pattern.
void tbb_example3() {

    cout << "\n\nTBB example 3...\n\n";

    // Setup and load input image dataset
    fipImage inputImage;
    inputImage.load("../Images/fungus_highres.jpg");
    inputImage.convertToFloat();

    auto width = inputImage.getWidth();
    auto height = inputImage.getHeight();
    const float* const inputBuffer = (float*)inputImage.accessPixels();

    // Setup output image array
    fipImage outputImage;
    outputImage = fipImage(FIT_FLOAT, width, height, 32);
    float *outputBuffer = (float*)outputImage.accessPixels();

    // Get total array size
    uint64_t numElements = width * height;

    // Divide processing into 4 blocks by default
    uint64_t stepSize = numElements >> 2;


    // Test sequential vs. parallel_for versions - run test multiple times and track speed-up
    double meanSpeedup = 0.0f;
    uint64_t numTests = 50;

    for (uint64_t testIndex = 0; testIndex < numTests; ++testIndex) {

        // Sequential version (base line for comparison)

        auto sLambda = [=](uint64_t i)->void {

            outputBuffer[i] = 1.0f - inputBuffer[i];
        };

        auto st1 = high_resolution_clock::now();

        for (uint64_t i = 0; i < numElements; ++i)
            sLambda(i);

        auto st2 = high_resolution_clock::now();
        auto st_dur = duration_cast<microseconds>(st2 - st1);
        std::cout << "sequential negative operation took = " << st_dur.count() << "\n";



        // parallel_for version

        auto pt1 = high_resolution_clock::now();

        // 1D parallel_for version
        tbb::parallel_for<uint64_t>(0, numElements, stepSize, [=](uint64_t i) {

            for (uint64_t j = i; j < i + stepSize; ++j)
                outputBuffer[j] = 1.0f - inputBuffer[j];
        });



        // 1D Blocked-range
/*		tbb::parallel_for(blocked_range<uint64_t>(0, numElements, stepSize), [=](const blocked_range<uint64_t>& range) {
			
			auto j1 = range.begin();
			auto j2 = range.end();
			
			for (uint64_t j = j1; j != j2; ++j)
				outputBuffer[j] = 1.0f - inputBuffer[j];
		});*/


        // blocked_range2D - basic version
/*		tbb::parallel_for(blocked_range2d<uint64_t, uint64_t>(0, height, 8, 0, width, width>>2), [=](const blocked_range2d<uint64_t, uint64_t>& r) {
			
			auto y1 = r.rows().begin();
			auto y2 = r.rows().end();
			auto x1 = r.cols().begin();
			auto x2 = r.cols().end();
			
			for (uint64_t y = y1; y < y2; ++y) {
				
				for (uint64_t x = x1; x < x2; ++x)
					outputBuffer[y * width + x] = inputBuffer[y*width + x];
			}
		});*/


        // blocked_range2D - pointer version
/*		tbb::parallel_for(blocked_range2d<uint64_t, uint64_t>(0, height, stepSize, 0, width, width), [=](const blocked_range2d<uint64_t, uint64_t>& r) {

			auto baseX = r.cols().begin();
			auto baseY = r.rows().begin();

			auto numRows = r.rows().end() - baseY;
			auto rowSpan = r.cols().end() - baseX;

			float *inputBase = (float*)(inputBuffer)+(baseY * width) + baseX;
			float *outputBase = outputBuffer + (baseY * width) + baseX;

			for (auto i = 0U; i<numRows; ++i, inputBase += width, outputBase += width) {

				for (auto x = 0U; x < rowSpan; ++x)
					outputBase[x] = 1.0f - inputBase[x];
			}
		});*/


        auto pt2 = high_resolution_clock::now();
        auto pt_dur = duration_cast<microseconds>(pt2 - pt1);
        std::cout << "parallel negative operation took = " << pt_dur.count() << "\n";

        double speedup = double(st_dur.count()) / double(pt_dur.count());
        std::cout << "Test " << testIndex << " speedup = " << speedup << endl;

        meanSpeedup += speedup;
    }

    meanSpeedup /= double(numTests);
    std::cout << "Mean speedup = " << meanSpeedup << endl;


    std::cout << "Saving tbb_example3 image...\n";

    outputImage.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
    outputImage.convertTo24Bits();
    outputImage.save("tbb_example3.bmp");

    std::cout << "...done\n\n";
}


// Show recursive decomposition of blocked_range2d when processing a 2D dataset
void tbb_example4() {

    cout << "\n\nTBB example 4...\n\n";

    // Setup and load input image dataset
    fipImage inputImage;
    inputImage.load("../Images/fungus_lowres.jpg");
    inputImage.convertToFloat();

    auto width = inputImage.getWidth();
    auto height = inputImage.getHeight();
    const float* const inputBuffer = (float*)inputImage.accessPixels();

    // Setup output image array
    fipImage outputImage;
    outputImage = fipImage(FIT_FLOAT, width, height, 32);
    float *outputBuffer = (float*)outputImage.accessPixels();

    parallel_for(blocked_range2d<uint64_t, uint64_t>(0, height, 8, 0, width, 8), [=](const blocked_range2d<uint64_t, uint64_t>& r) {

        auto y1 = r.rows().begin();
        auto y2 = r.rows().end();
        auto x1 = r.cols().begin();
        auto x2 = r.cols().end();

        for (auto y = y1; y < y2; ++y) {

            for (auto x = x1; x < x2; ++x) {

                if (x == x1 || y == y1)
                    outputBuffer[y * width + x] = 0.0f;
                else
                    outputBuffer[y * width + x] = 1.0f;
            }
        }
    });


    std::cout << "Saving image...\n";

    outputImage.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
    outputImage.convertTo24Bits();
    outputImage.save("blocked_range2d.bmp");

    std::cout << "...done\n\n";
}


// Example 5 - generateImage revisited - use parallel_for to generate images using TBB
void tbb_example5() {

    cout << "\n\nTBB example 5...\n\n";

    const int width = 64;
    const int height = 64;

    fipImage outputImage;
    outputImage = fipImage(FIT_FLOAT, width, height, 32);
    float *outputBuffer = (float*)outputImage.accessPixels();

    float sigma = 10.0f;

    parallel_for(blocked_range2d<int, int>(0, height, 0, width), [=](const blocked_range2d<int, int>& r) {

        for (auto y = r.rows().begin(); y < r.rows().end(); ++y) {

            for (auto x = r.cols().begin(); x < r.cols().end(); ++x) {

                float u = float(-(width >> 1) + x);
                float v = float(-(height >> 1) + y);

                outputBuffer[y * width + x] = 1.0f / (2.0f * float(M_PI) * sigma * sigma) * exp(-((u * u + v * v) / (2.0f * sigma * sigma)));
            }
        }
    });


    std::cout << "Saving image...\n";

    outputImage.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
    outputImage.convertTo24Bits();
    outputImage.save("gaussian2D.jpg");

    std::cout << "...done\n\n";

}


int main(int argc, char **argv) {

    int nt = task_scheduler_init::default_num_threads();
    task_scheduler_init T(nt);

    //tbb_example1();
    //tbb_example2();
    //tbb_example3();
    //tbb_example4();
    //tbb_example5();

    return 0;
}
