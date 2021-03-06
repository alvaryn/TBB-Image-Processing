#include <iostream>
//#include <vector>
//Thread building blocks library
#include <tbb/task_scheduler_init.h>
#include <tbb/blocked_range2d.h>
#include <tbb/parallel_for.h>
#include <tbb/tick_count.h>
//Free Image library
#include <FreeImagePlus.h>
//Thread library
#include <thread>

using namespace std;
using namespace tbb;

void part1_Stage1_Top()
{
    fipImage inputImage_top_1;
    fipImage inputImage_top_2;
    inputImage_top_1.load("../Images/render_top_1.png");
    inputImage_top_2.load("../Images/render_top_2.png");

    inputImage_top_1.convertToFloat();
    inputImage_top_2.convertToFloat();

    auto width = inputImage_top_1.getWidth();
    auto height = inputImage_top_1.getHeight();

    // Get total array size
    uint64_t numElements = width * height;

    float* floatInputImage1 = (float*)inputImage_top_1.accessPixels();    //Input arrays not sure if this should be constant.
    float* floatInputImage2 = (float*)inputImage_top_2.accessPixels();

    //setup output image array
    fipImage outputImage;
    outputImage = fipImage(FIT_FLOAT, width, height, 32);
    float* floatOutputImage = (float*)outputImage.accessPixels();   //output array

    // Sequential version (base line for comparison)

    auto sLambda = [=](uint64_t i) -> void{
        if(floatInputImage1[i] == floatInputImage2[i])   // if pixels are the same of the 2 input images then...
        {
            floatOutputImage[i] = 0.0f; // make output picture's pixel black
        }
        else
        {   // if pixels are different.
            floatOutputImage[i] = 1.0f; // make output picture's pixel white
        }
    };


    for (uint64_t i = 0; i < numElements; ++i)  //loop thru entire pixels
    {
        sLambda(i);
    }
    printf("Saving image...\n");

    outputImage.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
    outputImage.convertTo24Bits();
    outputImage.save("stage1_top.png");

    printf("...done\n\n");
};

void part1_Stage1_Bottom()
{
    fipImage inputImage_bottom_1, inputImage_bottom_2;
    inputImage_bottom_1.load("../Images/render_bottom_1.png");
    inputImage_bottom_2.load("../Images/render_bottom_2.png");

    inputImage_bottom_1.convertToFloat();
    inputImage_bottom_2.convertToFloat();

    auto width1 = inputImage_bottom_1.getWidth();
    auto height1 = inputImage_bottom_1.getHeight();

    // Get total array size
    uint64_t numElements = width1 * height1;

    float* floatInputImage1 = (float*)inputImage_bottom_1.accessPixels();    //Input arrays buffer
    float* floatInputImage2 = (float*)inputImage_bottom_2.accessPixels();

    //setup output image array
    fipImage outputImage;
    outputImage = fipImage(FIT_FLOAT, width1, height1, inputImage_bottom_1.getBitsPerPixel());
    float* floatOutputImage = (float*)outputImage.accessPixels();   //output array

    // Sequential version (base line for comparison)

    auto sLambda = [=](uint64_t i) -> void{
        if(floatInputImage1[i] == floatInputImage2[i])   // if pixels are the same of the 2 input images then...
        {
            floatOutputImage[i] = 0.0f; // make output picture's pixel black
        }
        else
        {   // if pixels are different.
            floatOutputImage[i] = 1.0f; // make output picture's pixel black
        }
    };

    //auto st1 = high_resolution_clock::now();

    for (uint64_t i = 0; i < numElements; ++i)  //loop thru entire pixels
    {
        sLambda(i);
    }


    printf("Saving image...\n");

    outputImage.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
    outputImage.convertTo24Bits();
    outputImage.save("stage1_bottom.png");

    printf("...done\n\n");
};

void part1_stage1_Combine()
{
    //combines the 2 stage

    fipImage stage1_bottom, stage1_top;
    stage1_bottom.load("../cmake-build-debug/stage1_bottom.png");
    stage1_top.load("../cmake-build-debug/stage1_top.png");

    stage1_bottom.convertToFloat();   //converts to float
    stage1_top.convertToFloat();

    auto width1 = stage1_bottom.getWidth();
    auto height1 = stage1_bottom.getHeight();
    auto width2 = stage1_top.getWidth();
    auto height2 = stage1_top.getHeight();

    // Get total array size
    uint64_t numElements = width1 * height1;

    const float* floatInputImage1 = (float*)stage1_bottom.accessPixels();    //Input arrays that are constant. We never change this buffer. Any changes made is stored into outputBuffer.
    const float* floatInputImage2 = (float*)stage1_top.accessPixels();

    //setup output image array
    fipImage outputImage,outputImage_top;
    outputImage = fipImage(FIT_FLOAT, width1, height1, stage1_bottom.getBitsPerPixel());
    float* floatOutputImage = (float*)outputImage.accessPixels();

    // Sequential version (base line for comparison)

    auto sLambda = [=](uint64_t i) -> void{
        floatOutputImage[i] = floatInputImage1[i] + floatInputImage2[i]; // add the two pixels of the input picture. this automatically also takes 50% of the pixel value.
    };

    //auto st1 = high_resolution_clock::now();

    for (uint64_t i = 0; i < numElements; ++i)
    {
        sLambda(i);
    }


    printf("Saving image...\n");

    outputImage.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
    outputImage.convertTo24Bits();
    outputImage.save("stage1_combined.png");

    printf("...done\n\n");
}

void part2_Blur_Sequential()
{
    fipImage inputImage;
    inputImage.load("../cmake-build-debug/stage1_combined.png");

    inputImage.convertToFloat();   //converts to float

    auto width = inputImage.getWidth();
    auto height = inputImage.getHeight();
    uint64_t numElements = width * height; // area or number of pixels = length * breadth

    const float* floatInputImage = (float*)inputImage.accessPixels();   //input picture buffer, ensures pixel to pixel access of the image.

    //output image
    fipImage outputImage = fipImage(FIT_FLOAT, width, height, inputImage.getBitsPerPixel());
    float* floatOutputImage = (float*)outputImage.accessPixels();     //output picture buffer, ensures pixel to pixel access of the image.

    //sobel operator

    auto y2 = width - 2;
    auto x2 = height - 2;

    for (uint64_t y = 2; y < y2; ++y) {

        for (uint64_t x = 2; x < x2; ++x)
            floatOutputImage[y * width + x] = floatInputImage[(y-2) * width + (x-2)] * 0.01f + floatInputImage[(y-2) * width + (x-1)] * 0.04f + floatInputImage[(y-2) * width + x] * 0.06f + floatInputImage[(y-2) * width + (x+1)] * 0.04f + floatInputImage[(y-2) * width + (x+2)] * 0.01f       //used hardcoded kernel values
                                              +floatInputImage[(y-1) * width + (x-2)] * 0.04f + floatInputImage[(y-1) * width + (x-1)] * 0.16f + floatInputImage[(y-1) * width + x] * 0.24f + floatInputImage[(y-1) * width + (x+1)] * 0.16f + floatInputImage[(y-1) * width + (x+2)] * 0.04f
                                              +floatInputImage[y * width + (x-2)] * 0.06f + floatInputImage[y * width+ (x-1)] * 0.24f + floatInputImage[y * width + x] * 0.36f + floatInputImage[y * width + (x-1)] * 0.24f + floatInputImage[y * width + (x+2)] * 0.06f
                                              +floatInputImage[(y+1) * width + (x-2)] * 0.04f + floatInputImage[(y+1) * width+ (x-1)] * 0.16f + floatInputImage[(y+1) * width + x] * 0.24f + floatInputImage[(y+1) * width + (x+1)] * 0.16f + floatInputImage[(y+1) * width + (x+2)] * 0.04f
                                              +floatInputImage[(y+2) * width + (x-2)] * 0.01f + floatInputImage[(y+2) * width+ (x-1)] * 0.04f + floatInputImage[(y+2) * width + x] * 0.06f + floatInputImage[(y+2) * width+ (x+1)] * 0.04f + floatInputImage[(y+2) * width+ (x+2)] * 0.01f;
    }

    //auto st1 = high_resolution_clock::now();

    printf("Saving image...\n");

    outputImage.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
    outputImage.convertTo24Bits();
    outputImage.save("stage2_blurred.png");

    printf("...done\n\n");
}

void part2_Blur_tbb()
{
    fipImage inputImage;
    inputImage.load("../cmake-build-debug/stage1_combined.png");

    inputImage.convertToFloat();   //converts to float

    auto width = inputImage.getWidth();
    auto height = inputImage.getHeight();
    uint64_t numElements = width * height; // area or number of pixels = length * breadth

    const float* floatInputImage = (float*)inputImage.accessPixels();   //input picture buffer, ensures pixel to pixel access of the image.

    //output image
    fipImage outputImage = fipImage(FIT_FLOAT, width, height, inputImage.getBitsPerPixel());
    float* floatOutputImage = (float*)outputImage.accessPixels();     //output picture buffer, ensures pixel to pixel access of the image.

    parallel_for(blocked_range2d<uint64_t, uint64_t>(2,height - 2, 2,width-2), [&](const blocked_range2d<uint64_t, uint64_t>& r) {          //parallelises following code

        auto y1 = r.rows().begin();
        auto y2 = r.rows().end();
        auto x1 = r.cols().begin();
        auto x2 = r.cols().end();

        for (uint64_t y = y1; y < y2; ++y) {

            for (uint64_t x = x1; x < x2; ++x)
                floatOutputImage[y * width + x] = floatInputImage[(y-2) * width + (x-2)] * 0.01f + floatInputImage[(y-2) * width + (x-1)] * 0.04f + floatInputImage[(y-2) * width + x] * 0.06f + floatInputImage[(y-2) * width + (x+1)] * 0.04f + floatInputImage[(y-2) * width + (x+2)] * 0.01f       //used hardcoded kernel values
                                                  +floatInputImage[(y-1) * width + (x-2)] * 0.04f + floatInputImage[(y-1) * width + (x-1)] * 0.16f + floatInputImage[(y-1) * width + x] * 0.24f + floatInputImage[(y-1) * width + (x+1)] * 0.16f + floatInputImage[(y-1) * width + (x+2)] * 0.04f
                                                  +floatInputImage[y * width + (x-2)] * 0.06f + floatInputImage[y * width+ (x-1)] * 0.24f + floatInputImage[y * width + x] * 0.36f + floatInputImage[y * width + (x-1)] * 0.24f + floatInputImage[y * width + (x+2)] * 0.06f
                                                  +floatInputImage[(y+1) * width + (x-2)] * 0.04f + floatInputImage[(y+1) * width+ (x-1)] * 0.16f + floatInputImage[(y+1) * width + x] * 0.24f + floatInputImage[(y+1) * width + (x+1)] * 0.16f + floatInputImage[(y+1) * width + (x+2)] * 0.04f
                                                  +floatInputImage[(y+2) * width + (x-2)] * 0.01f + floatInputImage[(y+2) * width+ (x-1)] * 0.04f + floatInputImage[(y+2) * width + x] * 0.06f + floatInputImage[(y+2) * width+ (x+1)] * 0.04f + floatInputImage[(y+2) * width+ (x+2)] * 0.01f;
        }

    });

    printf("Saving image...\n");

    outputImage.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
    outputImage.convertTo24Bits();
    outputImage.save("stage2_blurred.png");

    printf("...done\n\n");
}

void part2_BinaryThreshold_Sequential()
{

    fipImage inputImage;
    inputImage.load("../cmake-build-debug/stage2_blurred.png");


    inputImage.convertToFloat();   //converts to float

    auto width1 = inputImage.getWidth();
    auto height1 = inputImage.getHeight();


    // Get total array size
    uint64_t numElements = width1 * height1;

    const float* floatInputImage = (float*)inputImage.accessPixels();    //Input arrays that are constant. We never change this buffer. Any changes made is stored into outputBuffer.

    //setup output image array
    fipImage outputImage,outputImage_top;
    outputImage = fipImage(FIT_FLOAT, width1, height1, inputImage.getBitsPerPixel());
    float* floatOutputImage = (float*)outputImage.accessPixels();

    // Sequential version (base line for comparison)

    auto sLambda = [=](uint64_t i) -> void{
        if(floatInputImage[i] != 0.0f)  //not black
        {
            floatOutputImage[i] = 1.0f; // make output picture's pixel white
        }
    };

    //auto st1 = high_resolution_clock::now();

    for (uint64_t i = 0; i < numElements; ++i)
    {
        sLambda(i);
    }


    printf("Saving image...\n");

    outputImage.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
    outputImage.convertTo24Bits();
    outputImage.save("stage2_threshold.png");

    printf("...done\n\n");

}

void part2_BinaryThreshold_tbb()
{
    fipImage inputImage;
    inputImage.load("../cmake-build-debug/stage2_blurred.png");

    inputImage.convertToFloat();   //converts to float

    auto width = inputImage.getWidth();
    auto height = inputImage.getHeight();
    uint64_t numElements = width * height; // area or number of pixels = length * breadth

    // Divide processing into 4 blocks by default
    uint64_t stepSize = numElements >> 2;

    const float* floatInputImage = (float*)inputImage.accessPixels();   //input picture buffer, ensures pixel to pixel access of the image.

    //output image
    fipImage outputImage = fipImage(FIT_FLOAT, width, height, inputImage.getBitsPerPixel());
    float* floatOutputImage = (float*)outputImage.accessPixels();     //output picture buffer, ensures pixel to pixel access of the image.

    // 1D parallel_for version
    tbb::parallel_for<uint64_t>(0, numElements, stepSize, [=](uint64_t i) {  //parallelises following code

        for (uint64_t j = i; j < i + stepSize; ++j)
        {   if (floatInputImage[j] != 0.0f)  //not black then...
            {
                floatOutputImage[j] = 1.0f; // make output picture's pixel white
            }
        }
    });

    printf("Saving image...\n");

    outputImage.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
    outputImage.convertTo24Bits();
    outputImage.save("stage2_threshold.png");

    printf("...done\n\n");

}

void part3_CountWhitePixel_Sequential()
{

    fipImage render_top_1, stage2_threshold;
    render_top_1.load("../Images/render_top_1.png");
    stage2_threshold.load("../cmake-build-debug/stage2_threshold.png");
    auto stage2_thresholdBuffer = stage2_threshold;
    stage2_thresholdBuffer.convertToFloat();

    auto width1 = stage2_threshold.getWidth();
    auto height1 = stage2_threshold.getHeight();

    // Get total array size
    uint64_t numElements = width1 * height1;

    const float* floatStage2_threshold = (float*)stage2_thresholdBuffer.accessPixels();            //Input arrays that are constant. We never change this buffer. Any changes made is stored into outputBuffer.

    //setup output image array
    fipImage outputImage,outputImage_top;


    uint64_t whitePixelCount = 0;      //initialise to zero.
    float whitePixelPercentage = 0.0;

    //auto st1 = high_resolution_clock::now();

    RGBQUAD colourInput;
    RGBQUAD colourOutput;

    for (uint64_t i = 0; i < numElements; ++i)
    {
        if(floatStage2_threshold[i] == 1.0f)    //if pixel is white then....
        {
            whitePixelCount++;
        }
    }

    //mask filter

    for(int y = 0; y < height1; y++)
    {
        for(int x = 0; x < width1; x++)
        {
            render_top_1.getPixelColor(x,y,&colourInput);
            stage2_threshold.getPixelColor(x,y,&colourOutput);
            if(colourOutput.rgbBlue == 255 && colourOutput.rgbGreen == 255 && colourOutput.rgbRed == 255)    //if pixel is white then....
            {   //invert render top 1 pic's pixel colour
                colourInput.rgbBlue = 255 - colourInput.rgbBlue;
                colourInput.rgbGreen = 255 - colourInput.rgbGreen;
                colourInput.rgbRed = 255 - colourInput.rgbRed;
            }
            render_top_1.setPixelColor(x,y,&colourInput);
        }

    }

    //auto st1 = high_resolution_clock::now();

    whitePixelPercentage = ((float)whitePixelCount/numElements) * 100;

    cout << "Number of white pixels = " << whitePixelCount << endl << "White pixel percentage = " << whitePixelPercentage << "%" << endl;

    printf("Saving image...\n");

    render_top_1.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
    render_top_1.convertTo24Bits();
    render_top_1.save("Inversion.png");

    printf("...done\n\n");
}

void part3_CountWhitePixel_tbb()
{

    fipImage render_top_1, stage2_threshold;
    render_top_1.load("../Images/render_top_1.png");
    stage2_threshold.load("../cmake-build-debug/stage2_threshold.png");
    auto stage2_thresholdBuffer = stage2_threshold;
    stage2_thresholdBuffer.convertToFloat();

    auto width1 = stage2_threshold.getWidth();
    auto height1 = stage2_threshold.getHeight();

    // Get total array size
    uint64_t numElements = width1 * height1;

    const float* floatStage2_threshold = (float*)stage2_thresholdBuffer.accessPixels();            //Input arrays that are constant. We never change this buffer. Any changes made is stored into outputBuffer.

    //setup output image array
    fipImage outputImage,outputImage_top;


    uint64_t whitePixelCount = 0;      //initialise to zero.
    float whitePixelPercentage = 0.0;

    //auto st1 = high_resolution_clock::now();

    RGBQUAD colourInput;
    RGBQUAD colourOutput;

    //mask filter

    // Divide processing into 4 blocks by default
    uint64_t stepSize = numElements >> 2;       //Bitwise right shift operator - right shifts numElements by 2 spaces.

    //                                                                   |row start,  row end    , col start,  col end|
    parallel_for(blocked_range2d<uint64_t, uint64_t>(2, height1 - 2 ,stepSize,2,width1 - 2, stepSize), [&](const blocked_range2d<uint64_t, uint64_t>& r) {          //parallelises following code

        auto y1 = r.rows().begin();
        auto y2 = r.rows().end();
        auto x1 = r.cols().begin();
        auto x2 = r.cols().end();

        for(int y = y1; y < y2; y++)
        {
            for(int x = x1; x < x2; x++)
            {
                render_top_1.getPixelColor(x,y,&colourInput);
                stage2_threshold.getPixelColor(x,y,&colourOutput);
                if(colourOutput.rgbBlue == 255 && colourOutput.rgbGreen == 255 && colourOutput.rgbRed == 255)    //if pixel is white then....
                {   //invert render top 1 pic's pixel colour
                    colourInput.rgbBlue = 255 - colourInput.rgbBlue;
                    colourInput.rgbGreen = 255 - colourInput.rgbGreen;
                    colourInput.rgbRed = 255 - colourInput.rgbRed;

                    //white pixel count
                    whitePixelCount++;

                }
                render_top_1.setPixelColor(x,y,&colourInput);
            }
        }
    });

    //auto st1 = high_resolution_clock::now();

    whitePixelPercentage = ((float)whitePixelCount/numElements) * 100;

    cout << "Number of white pixels = " << whitePixelCount << endl << "White pixel percentage = " << whitePixelPercentage << "%" << endl;

    printf("Saving image...\n");

    render_top_1.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
    render_top_1.convertTo24Bits();
    render_top_1.save("Inversion.png");

    printf("...done\n\n");
}

int main()
{
    int nt = task_scheduler_init::default_num_threads();
    task_scheduler_init T(nt);

    //Part 1 (Image Comparison): -----------------DO NOT REMOVE THIS COMMENT----------------------------//
    /*
     // for(int count = 0 ; count < 10 ; count ++)
 //{
     auto st1 = tick_count::now();
     part1_Stage1_Top();
     part1_Stage1_Bottom();
     part1_stage1_Combine();
     auto en1 = tick_count::now();

     cout << "Completed in " << (en1-st1).seconds() << "s." << endl;
 //}
     */
    //begin threading version below.
/*
    //for(int count = 0 ; count < 10 ; count ++)
    {
    printf("Begin Threading..\n");
    auto st2 = tick_count::now();
    thread t1(part1_Stage1_Top);
    thread t2(part1_Stage1_Bottom);

    t1.join();  //waits for thread completion
    t2.join();
    printf("End Threading..\n");

    part1_stage1_Combine();   //does this on main thread.
    auto en2 = tick_count::now();

    cout << "Completed in " << (en2-st2).seconds() << "s." << endl;
    }
    */
    //Part 2 (Blur & post-processing): -----------DO NOT REMOVE THIS COMMENT----------------------------//
/*
    for(int count = 0 ; count < 10 ; count ++)
    {
    printf("Begin sequential blur..\n");
    auto st3 = tick_count::now();
    part2_Blur_Sequential();
    part2_BinaryThreshold_Sequential();
    auto en3 = tick_count::now();
    printf("End sequential blur..\n");
    cout << "Completed in " << (en3-st3).seconds() << "s." << endl;
    }
*/
/*
    //for(int count = 0 ; count < 10 ; count ++)
    {
    printf("Begin tbb blur..\n");
    auto st4 = tick_count::now();
    part2_Blur_tbb();
    part2_BinaryThreshold_tbb();

    auto en4 = tick_count::now();
    printf("End tbb blur..\n");
    cout << "Completed in " << (en4-st4).seconds() << "s." << endl;
    }
*/

    //Part 3 (Image Mask): -----------------------DO NOT REMOVE THIS COMMENT----------------------------//
/*
    //for(int count = 0 ; count < 10 ; count ++)
    {
        auto st4 = tick_count::now();
        part3_CountWhitePixel_Sequential();
        auto en4 = tick_count::now();
        cout << "Completed in " << (en4-st4).seconds() << "s." << endl;
    }
*/

    //for(int count = 0 ; count < 10 ; count ++)
    {   auto st5 = tick_count::now();
        part3_CountWhitePixel_tbb();
        auto en5 = tick_count::now();
        cout << "Completed in " << (en5-st5).seconds() << "s." << endl;
    }

    return 0;
}
