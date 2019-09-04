/**********************************************************************
Copyright ©2015 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

•	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
•	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/


#include "Template.hpp"


/*
 * \brief Host Initialization 
 *        Allocate and initialize memory 
 *        on the host. Print input array. 
 */
int
initializeHost(void)
{
	level				= 12;
    width               = 1;
	for(int i = 0; i<level;i++)
		width=width*level;
	walls				= 6;
    plA					= NULL;
	plB					= NULL;
	plC					= NULL;
	plD					= NULL;
	pt1x				= NULL;
	pt1y				= NULL;
	pt1z				= NULL;
	pt2x				= NULL;
	pt2y				= NULL;
	pt2z				= NULL;
	abso				= NULL;
    output              = NULL;

    /////////////////////////////////////////////////////////////////
    // Allocate and initialize memory used by host 
    /////////////////////////////////////////////////////////////////
    cl_uint sizeInBytes = walls * sizeof(cl_float);
	plA   = (cl_float *) malloc(sizeInBytes);
	plB   =	(cl_float *) malloc(sizeInBytes);
	plC   =	(cl_float *) malloc(sizeInBytes);
	plD   =	(cl_float *) malloc(sizeInBytes);
	pt1x  =	(cl_float *) malloc(sizeInBytes);
	pt1y  =	(cl_float *) malloc(sizeInBytes);
	pt1z  =	(cl_float *) malloc(sizeInBytes);
	pt2x  =	(cl_float *) malloc(sizeInBytes);
	pt2y  =	(cl_float *) malloc(sizeInBytes);
	pt2z  =	(cl_float *) malloc(sizeInBytes);
	abso   =	(cl_float *) malloc(sizeInBytes);
	
               
    if(!abso)
    {
        std::cout << "Error: Failed to allocate input memory on host\n";
        return SDK_FAILURE;
    }

    output = (cl_uint *) malloc(1 * sizeof(cl_uint));
    if(!output)
    {
        std::cout << "Error: Failed to allocate output memory on host\n";
        return SDK_FAILURE;
    }

	cl_float a = 3;
    cl_float b = 5;
    cl_float h = 2.3;
	cl_float idata0[] = {0, 0, 0, 0, 1, -1};
	cl_float idata1[] = {0, 0, 1, -1, 0, 0};
	cl_float idata2[] = {1, -1, 0, 0, 0, 0};
	cl_float idata3[] = {h, h, b, b, a, a};
	cl_float idata4[] = {-a, -a, -a, -a, -a, a};
	cl_float idata5[] = {-b, -b, -b, b, -b, -b};
	cl_float idata6[] = {-h, h, -h, -h, -h, -h};
	cl_float idata7[] = {a, a, a, a, -a, a};
	cl_float idata8[] = {b, b, -b, b, b, b};
	cl_float idata9[] = {-h, h, h, h, h, h};
	cl_float idata10[] = {0.71, 0.68, 0.75, 0.72, 0.54, 0.61};

	for(cl_uint i = 0; i < walls; i++)
    {    
		plA[i]   = 	idata0[i];
		plB[i]   =	idata1[i];
		plC[i]   =	idata2[i];
		plD[i]   =	idata3[i];
		pt1x[i]  =	idata4[i];
		pt1y[i]  =	idata5[i];
		pt1z[i]  =	idata6[i];
		pt2x[i]  =	idata7[i];
		pt2y[i]  =	idata8[i];
		pt2z[i]  =	idata9[i];
		abso[i]   =	idata10[i];
	}
	

    // print input array
    //print1DArray(std::string("Input").c_str(), input, width);
    return SDK_SUCCESS;
}

/*
 * Converts the contents of a file into a string
 */
std::string
convertToString(const char *filename)
{
    size_t size;
    char*  str;
    std::string s;

    std::fstream f(filename, (std::fstream::in | std::fstream::binary));

    if(f.is_open())
    {
        size_t fileSize;
        f.seekg(0, std::fstream::end);
        size = fileSize = (size_t)f.tellg();
        f.seekg(0, std::fstream::beg);

        str = new char[size+1];
        if(!str)
        {
            f.close();
            std::cout << "Memory allocation failed";
            return NULL;
        }

        f.read(str, fileSize);
        f.close();
        str[size] = '\0';
    
        s = str;
        delete[] str;
        return s;
    }
    else
    {
        std::cout << "\nFile containg the kernel code(\".cl\") not found. Please copy the required file in the folder containg the executable.\n";
        exit(1);
    }
    return NULL;
}

/*
 * \brief OpenCL related initialization 
 *        Create Context, Device list, Command Queue
 *        Create OpenCL memory buffer objects
 *        Load CL file, compile, link CL source 
 *		  Build program and kernel objects
 */
int
initializeCL(void)
{
    cl_int status = 0;
    size_t deviceListSize;

    //////////////////////////////////////////////////////////////////// 
    // STEP 1 Getting Platform.
    //////////////////////////////////////////////////////////////////// 
    
    /*
     * Have a look at the available platforms and pick either
     * the AMD one if available or a reasonable default.
     */

    cl_uint numPlatforms;
    cl_platform_id platform = NULL;
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if(status != CL_SUCCESS)
    {
        std::cout << "Error: Getting Platforms. (clGetPlatformsIDs)\n";
        return SDK_FAILURE;
    }
    
    if(numPlatforms > 0)
    {
        cl_platform_id* platforms = new cl_platform_id[numPlatforms];
        status = clGetPlatformIDs(numPlatforms, platforms, NULL);
        if(status != CL_SUCCESS)
        {
            std::cout << "Error: Getting Platform Ids. (clGetPlatformsIDs)\n";
            return SDK_FAILURE;
        }
        for(unsigned int i=0; i < numPlatforms; ++i)
        {
            char pbuff[100];
            status = clGetPlatformInfo(
                        platforms[i],
                        CL_PLATFORM_VENDOR,
                        sizeof(pbuff),
                        pbuff,
                        NULL);
            if(status != CL_SUCCESS)
            {
                std::cout << "Error: Getting Platform Info.(clGetPlatformInfo)\n";
                return SDK_FAILURE;
            }
            platform = platforms[i];
            if(!strcmp(pbuff, "Advanced Micro Devices, Inc."))
            {
                break;
            }
        }
        delete platforms;
    }

    if(NULL == platform)
    {
        std::cout << "NULL platform found so Exiting Application." << std::endl;
        return SDK_FAILURE;
    }


    //////////////////////////////////////////////////////////////////// 
    // STEP 2 Creating context using the platform selected
    //      Context created from type includes all available
    //      devices of the specified type from the selected platform 
    //////////////////////////////////////////////////////////////////// 
    

    /*
     * If we could find our platform, use it. Otherwise use just available platform.
     */
    cl_context_properties cps[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

    context = clCreateContextFromType(cps, 
                                      CL_DEVICE_TYPE_GPU, 
                                      NULL, 
                                      NULL, 
                                      &status);
    if(status != CL_SUCCESS) 
    {  
        std::cout << "Error: Creating Context. (clCreateContextFromType)\n";
        return SDK_FAILURE; 
    }


    //////////////////////////////////////////////////////////////////// 
    // STEP 3
    //      3.1 Query context for the device list size,
    //      3.2 Allocate that much memory using malloc or new
    //      3.3 Again query context info to get the array of device
    //              available in the created context
    //////////////////////////////////////////////////////////////////// 
    
    // First, get the size of device list data
    status = clGetContextInfo(context, 
                              CL_CONTEXT_DEVICES, 
                              0, 
                              NULL, 
                              &deviceListSize);
    if(status != CL_SUCCESS) 
    {  
        std::cout <<
            "Error: Getting Context Info \
            (device list size, clGetContextInfo)\n";
        return SDK_FAILURE;
    }

    devices = (cl_device_id *)malloc(deviceListSize);
    if(devices == 0)
    {
        std::cout << "Error: No devices found.\n";
        return SDK_FAILURE;
    }

    // Now, get the device list data
    status = clGetContextInfo(
                 context, 
                 CL_CONTEXT_DEVICES, 
                 deviceListSize, 
                 devices, 
                 NULL);
    if(status != CL_SUCCESS) 
    { 
        std::cout <<
            "Error: Getting Context Info \
            (device list, clGetContextInfo)\n";
        return SDK_FAILURE;
    }

    //////////////////////////////////////////////////////////////////// 
    // STEP 4 Creating command queue for a single device
    //      Each device in the context can have a 
    //      dedicated commandqueue object for itself
    //////////////////////////////////////////////////////////////////// 
    
    commandQueue = clCreateCommandQueue(
                       context, 
                       devices[0], 
                       0, 
                       &status);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Creating Command Queue. (clCreateCommandQueue)\n";
        return SDK_FAILURE;
    }

    /////////////////////////////////////////////////////////////////
    // STEP 5 Creating cl_buffer objects from host buffer
    //          These buffer objects can be passed to the kernel
    //          as kernel arguments
    /////////////////////////////////////////////////////////////////
    plABuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      sizeof(cl_float) * walls,
                      plA, 
                      &status);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: clCreateBuffer (plABuffer)\n";
        return SDK_FAILURE;
    }

	plBBuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      sizeof(cl_float) * walls,
                      plB, 
                      &status);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: clCreateBuffer (plBBuffer)\n";
        return SDK_FAILURE;
    }
	plCBuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      sizeof(cl_float) * walls,
                      plC, 
                      &status);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: clCreateBuffer (plCBuffer)\n";
        return SDK_FAILURE;
    }
	plDBuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      sizeof(cl_float) * walls,
                      plD, 
                      &status);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: clCreateBuffer (plDBuffer)\n";
        return SDK_FAILURE;
    }
	pt1xBuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      sizeof(cl_float) * walls,
                      pt1x, 
                      &status);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: clCreateBuffer (pt1xBuffer)\n";
        return SDK_FAILURE;
    }
	pt1yBuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      sizeof(cl_float) * walls,
                      pt1y, 
                      &status);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: clCreateBuffer (pt1yBuffer)\n";
        return SDK_FAILURE;
    }
	pt1zBuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      sizeof(cl_float) * walls,
                      pt1z, 
                      &status);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: clCreateBuffer (pt1zBuffer)\n";
        return SDK_FAILURE;
    }
	pt2xBuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      sizeof(cl_float) * walls,
                      pt2x, 
                      &status);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: clCreateBuffer (pt2xBuffer)\n";
        return SDK_FAILURE;
    }
	pt2yBuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      sizeof(cl_float) * walls,
                      pt2y, 
                      &status);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: clCreateBuffer (pt2yBuffer)\n";
        return SDK_FAILURE;
    }
	pt2zBuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      sizeof(cl_float) * walls,
                      pt2z, 
                      &status);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: clCreateBuffer (pt2zBuffer)\n";
        return SDK_FAILURE;
    }
	absoBuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      sizeof(cl_float) * walls,
                      abso, 
                      &status);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: clCreateBuffer (absoBuffer)\n";
        return SDK_FAILURE;
    }
	/*cacheBuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                      sizeof(cl_uint) * level,
                      cache, 
                      &status);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: clCreateBuffer (cacheBuffer)\n";
        return SDK_FAILURE;
    }*/

    outputBuffer = clCreateBuffer(
                       context, 
                       CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                       sizeof(cl_uint) * 1,
                       output, 
                       &status);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: clCreateBuffer (outputBuffer)\n";
        return SDK_FAILURE;
    }


    /////////////////////////////////////////////////////////////////
    // STEP 6. Building Kernel
    //      6.1 Load CL file, using basic file i/o
    //      6.2 Build CL program object
    //      6.3 Create CL kernel object
    /////////////////////////////////////////////////////////////////
    const char * filename  = "Template_Kernels.cl";
    std::string  sourceStr = convertToString(filename);
    const char * source    = sourceStr.c_str();
    size_t sourceSize[]    = { strlen(source) };

    program = clCreateProgramWithSource(
                  context, 
                  1, 
                  &source,
                  sourceSize,
                  &status);
    if(status != CL_SUCCESS) 
    { 
      std::cout <<
               "Error: Loading Binary into cl_program \
               (clCreateProgramWithBinary)\n";
      return SDK_FAILURE;
    }

    // create a cl program executable for all the devices specified
    status = clBuildProgram(program, 1, devices, NULL, NULL, NULL);
	std::cout << status << std::endl;
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: Building Program (clBuildProgram)\n";
        return SDK_FAILURE; 
    }

    // get a kernel object handle for a kernel with the given name
    kernel = clCreateKernel(program, "templateKernel", &status);
    if(status != CL_SUCCESS) 
    {  
        std::cout << "Error: Creating Kernel from program. (clCreateKernel)\n";
        return SDK_FAILURE;
    }

    return SDK_SUCCESS;
}


/*
 * \brief Run OpenCL program 
 *
 *        Bind host variables to kernel arguments 
 *        Run the CL kernel
 */
int 
runCLKernels(void)
{
    cl_int   status;
    cl_uint maxDims;
    cl_event events[2];
    size_t globalThreads[1];
    size_t localThreads[1];
    size_t maxWorkGroupSize;
    size_t maxWorkItemSizes[3];

    //////////////////////////////////////////////////////////////////// 
    // STEP 7 Analyzing proper workgroup size for the kernel
    //          by querying device information
    //    7.1 Device Info CL_DEVICE_MAX_WORK_GROUP_SIZE
    //    7.2 Device Info CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS
    //    7.3 Device Info CL_DEVICE_MAX_WORK_ITEM_SIZES
    //////////////////////////////////////////////////////////////////// 
    
    
    /**
    * Query device capabilities. Maximum 
    * work item dimensions and the maximmum
    * work item sizes
    */ 
    status = clGetDeviceInfo(
        devices[0], 
        CL_DEVICE_MAX_WORK_GROUP_SIZE, 
        sizeof(size_t), 
        (void*)&maxWorkGroupSize, 
        NULL);
    if(status != CL_SUCCESS) 
    {  
        std::cout << "Error: Getting Device Info. (clGetDeviceInfo)\n";
        return SDK_FAILURE;
    }
    
    status = clGetDeviceInfo(
        devices[0], 
        CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, 
        sizeof(cl_uint), 
        (void*)&maxDims, 
        NULL);
    if(status != CL_SUCCESS) 
    {  
        std::cout << "Error: Getting Device Info. (clGetDeviceInfo)\n";
        return SDK_FAILURE;
    }

    status = clGetDeviceInfo(
        devices[0], 
        CL_DEVICE_MAX_WORK_ITEM_SIZES, 
        sizeof(size_t)*maxDims,
        (void*)maxWorkItemSizes,
        NULL);
    if(status != CL_SUCCESS) 
    {  
        std::cout << "Error: Getting Device Info. (clGetDeviceInfo)\n";
        return SDK_FAILURE;
    }
    
    globalThreads[0] = width;
    localThreads[0]  = 1;
    if(localThreads[0] > maxWorkGroupSize ||
        localThreads[0] > maxWorkItemSizes[0])
    {
        std::cout << "Unsupported: Device does not support requested number of work items.";
        return SDK_FAILURE;
    }

    //////////////////////////////////////////////////////////////////// 
    // STEP 8 Set appropriate arguments to the kernel
    //    8.1 Kernel Arg outputBuffer ( cl_mem object)
    //    8.2 Kernel Arg inputBuffer (cl_mem object)
    //    8.3 Kernel Arg multiplier (cl_uint)
    //////////////////////////////////////////////////////////////////// 
    
    // the output array to the kernel
    status = clSetKernelArg(
                    kernel, 
                    0, 
                    sizeof(cl_mem), 
                    (void *)&outputBuffer);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: Setting kernel argument. (output)\n";
        return SDK_FAILURE;
    }

    // the input array to the kernel
    //status = clSetKernelArg(
    //                kernel, 
    //                1, 
    //                sizeof(cl_mem), 
    //                (void *)&inputBuffer);
    //if(status != CL_SUCCESS) 
    //{ 
    //    std::cout << "Error: Setting kernel argument. (input)\n";
    //    return SDK_FAILURE;
    //}

	//plA
	status = clSetKernelArg(
                    kernel, 
                    1, 
                    sizeof(cl_mem), 
                    (void *)&plABuffer);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: Setting kernel argument. (plA)\n";
        return SDK_FAILURE;
    }

	//plB
	status = clSetKernelArg(
                    kernel, 
                    2, 
                    sizeof(cl_mem), 
                    (void *)&plBBuffer);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: Setting kernel argument. (plB)\n";
        return SDK_FAILURE;
    }

	//plC
	status = clSetKernelArg(
                    kernel, 
                    3, 
                    sizeof(cl_mem), 
                    (void *)&plCBuffer);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: Setting kernel argument. (plC)\n";
        return SDK_FAILURE;
    }

	//plD
	status = clSetKernelArg(
                    kernel, 
                    4, 
                    sizeof(cl_mem), 
                    (void *)&plDBuffer);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: Setting kernel argument. (plD)\n";
        return SDK_FAILURE;
    }

	//pt1x
	status = clSetKernelArg(
                    kernel, 
                    5, 
                    sizeof(cl_mem), 
                    (void *)&pt1xBuffer);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: Setting kernel argument. (pt1x)\n";
        return SDK_FAILURE;
    }

	//pt1y
	status = clSetKernelArg(
                    kernel, 
                    6, 
                    sizeof(cl_mem), 
                    (void *)&pt1yBuffer);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: Setting kernel argument. (plA)\n";
        return SDK_FAILURE;
    }

	//pt1z
	status = clSetKernelArg(
                    kernel, 
                    7, 
                    sizeof(cl_mem), 
                    (void *)&pt1zBuffer);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: Setting kernel argument. (pt1z)\n";
        return SDK_FAILURE;
    }

	//pt2x
	status = clSetKernelArg(
                    kernel, 
                    8, 
                    sizeof(cl_mem), 
                    (void *)&pt2xBuffer);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: Setting kernel argument. (pt2x)\n";
        return SDK_FAILURE;
    }

	//pt2y
	status = clSetKernelArg(
                    kernel, 
                    9, 
                    sizeof(cl_mem), 
                    (void *)&pt2yBuffer);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: Setting kernel argument. (pt2y)\n";
        return SDK_FAILURE;
    }

	//pt2z
	status = clSetKernelArg(
                    kernel, 
                    10, 
                    sizeof(cl_mem), 
                    (void *)&pt2zBuffer);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: Setting kernel argument. (pt2z)\n";
        return SDK_FAILURE;
    }

	//abso
	status = clSetKernelArg(
                    kernel, 
                    11, 
                    sizeof(cl_mem), 
                    (void *)&absoBuffer);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: Setting kernel argument. (abso)\n";
        return SDK_FAILURE;
    }

	//cache
	status = clSetKernelArg(
                    kernel, 
                    12, 
                    level*sizeof(cl_uint), 
                    NULL);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: Setting kernel argument. (cache)\n";
        return SDK_FAILURE;
    }



    // multiplier
    status = clSetKernelArg(
                    kernel, 
                    13, 
                    sizeof(cl_uint), 
                    (void *)&level);
    if(status != CL_SUCCESS) 
    { 
        std::cout << "Error: Setting kernel argument. (multiplier)\n";
        return SDK_FAILURE;
    }

    //////////////////////////////////////////////////////////////////// 
    // STEP 9 Enqueue a kernel run call.
    //          Wait till the event completes and release the event
    //////////////////////////////////////////////////////////////////// 
    status = clEnqueueNDRangeKernel(
                 commandQueue,
                 kernel,
                 1,
                 NULL,
                 globalThreads,
                 localThreads,
                 0,
                 NULL,
                 &events[0]);
    if(status != CL_SUCCESS) 
    { 
        std::cout <<
            "Error: Enqueueing kernel onto command queue. \
            (clEnqueueNDRangeKernel)\n";
        return SDK_FAILURE;
    }


    // wait for the kernel call to finish execution
    status = clWaitForEvents(1, &events[0]);
    if(status != CL_SUCCESS) 
    { 
        std::cout <<
            "Error: Waiting for kernel run to finish. \
            (clWaitForEvents)\n";
        return SDK_FAILURE;
    }

    status = clReleaseEvent(events[0]);
    if(status != CL_SUCCESS) 
    { 
        std::cout <<
            "Error: Release event object. \
            (clReleaseEvent)\n";
        return SDK_FAILURE;
    }

    //////////////////////////////////////////////////////////////////// 
    // STEP 10  Enqueue readBuffer to read the output back
    //  Wait for the event and release the event
    ////////////////////////////////////////////////////////////////////
	
    status = clEnqueueReadBuffer(
                commandQueue,
                outputBuffer,
                CL_TRUE,
                0,
                1 * sizeof(cl_uint),
                output,
                0,
                NULL,
                &events[1]);
    
    if(status != CL_SUCCESS) 
    { 
        std::cout << 
            "Error: clEnqueueReadBuffer failed. \
             (clEnqueueReadBuffer)\n";
        return SDK_FAILURE;
    }
    
    // Wait for the read buffer to finish execution
    status = clWaitForEvents(1, &events[1]);
    if(status != CL_SUCCESS) 
    { 
        std::cout <<
            "Error: Waiting for read buffer call to finish. \
            (clWaitForEvents)\n";
        return SDK_FAILURE;
    }
    
    status = clReleaseEvent(events[1]);
    if(status != CL_SUCCESS) 
    { 
        std::cout <<
            "Error: Release event object. \
            (clReleaseEvent)\n";
        return SDK_FAILURE;
    }
    return SDK_SUCCESS;
}


/*
 * \brief Release OpenCL resources (Context, Memory etc.) 
 */
int  
cleanupCL(void)
{
    cl_int status;

    //////////////////////////////////////////////////////////////////// 
    // STEP 11  CLean up the opencl resources used 
    //////////////////////////////////////////////////////////////////// 
    
    status = clReleaseKernel(kernel);
    if(status != CL_SUCCESS)
    {
        std::cout << "Error: In clReleaseKernel \n";
        return SDK_FAILURE; 
    }
    status = clReleaseProgram(program);
    if(status != CL_SUCCESS)
    {
        std::cout << "Error: In clReleaseProgram\n";
        return SDK_FAILURE; 
    }
    status = clReleaseMemObject(plABuffer);
    if(status != CL_SUCCESS)
    {
        std::cout << "Error: In clReleaseMemObject (plABuffer)\n";
        return SDK_FAILURE; 
    }
	status = clReleaseMemObject(plBBuffer);
    if(status != CL_SUCCESS)
    {
        std::cout << "Error: In clReleaseMemObject (plBBuffer)\n";
        return SDK_FAILURE; 
    }
	status = clReleaseMemObject(plCBuffer);
    if(status != CL_SUCCESS)
    {
        std::cout << "Error: In clReleaseMemObject (plCBuffer)\n";
        return SDK_FAILURE; 
    }
	status = clReleaseMemObject(plDBuffer);
    if(status != CL_SUCCESS)
    {
        std::cout << "Error: In clReleaseMemObject (plDBuffer)\n";
        return SDK_FAILURE; 
    }
	status = clReleaseMemObject(pt1xBuffer);
    if(status != CL_SUCCESS)
    {
        std::cout << "Error: In clReleaseMemObject (pt1xBuffer)\n";
        return SDK_FAILURE; 
    }
	status = clReleaseMemObject(pt1yBuffer);
    if(status != CL_SUCCESS)
    {
        std::cout << "Error: In clReleaseMemObject (pt1yBuffer)\n";
        return SDK_FAILURE; 
    }
	status = clReleaseMemObject(pt1zBuffer);
    if(status != CL_SUCCESS)
    {
        std::cout << "Error: In clReleaseMemObject (pt1zBuffer)\n";
        return SDK_FAILURE; 
    }
	status = clReleaseMemObject(pt2xBuffer);
    if(status != CL_SUCCESS)
    {
        std::cout << "Error: In clReleaseMemObject (pt2xBuffer)\n";
        return SDK_FAILURE; 
    }
	status = clReleaseMemObject(pt2yBuffer);
    if(status != CL_SUCCESS)
    {
        std::cout << "Error: In clReleaseMemObject (pt2yBuffer)\n";
        return SDK_FAILURE; 
    }
	status = clReleaseMemObject(pt2zBuffer);
    if(status != CL_SUCCESS)
    {
        std::cout << "Error: In clReleaseMemObject (pt2zBuffer)\n";
        return SDK_FAILURE; 
    }
	status = clReleaseMemObject(absoBuffer);
    if(status != CL_SUCCESS)
    {
        std::cout << "Error: In clReleaseMemObject (absoBuffer)\n";
        return SDK_FAILURE; 
    }
    status = clReleaseMemObject(outputBuffer);
    if(status != CL_SUCCESS)
    {
        std::cout << "Error: In clReleaseMemObject (outputBuffer)\n";
        return SDK_FAILURE; 
    }
    status = clReleaseCommandQueue(commandQueue);
    if(status != CL_SUCCESS)
    {
        std::cout << "Error: In clReleaseCommandQueue\n";
        return SDK_FAILURE;
    }
    status = clReleaseContext(context);
    if(status != CL_SUCCESS)
    {
        std::cout << "Error: In clReleaseContext\n";
        return SDK_FAILURE;
    }
    return SDK_SUCCESS;
}


/* 
 * \brief Releases program's resources 
 */
void
cleanupHost(void)
{
    //if(input != NULL)
    //{
    //    free(input);
    //    input = NULL;
    //}
	if(plA != NULL)
	{
		free(plA);
		plA = NULL;
	}
	if(plB != NULL)
	{
	    free(plB);
	    plB = NULL;
	}
	if(plC != NULL)
	{
	    free(plC);
	     plC = NULL;
	}
	if(plD != NULL)
	{
	    free(plD);
	    plD = NULL;
	}
	if(pt1x != NULL)
	{
	    free(pt1x);
	    pt1x = NULL;
	}
	if(pt1y != NULL)
	{
	    free(pt1y);
	    pt1y = NULL;
	}
	if(pt1z != NULL)
	{
	    free(pt1z);
	    pt1z = NULL;
	}
	if(pt2x != NULL)
	{
	    free(pt2x);
	    pt2x = NULL;
	}
	if(pt2y != NULL)
	{
	    free(pt2y);
	    pt2y = NULL;
	}
	if(pt2z != NULL)
	{
	    free(pt2z);
	    pt2z = NULL;
	}
	if(abso != NULL)
	{
	    free(abso);
	    abso = NULL;
	}
    if(output != NULL)
    {
        free(output);
        output = NULL;
    }
    if(devices != NULL)
    {
        free(devices);
        devices = NULL;
    }
}


/*
 * \brief Print no more than 256 elements of the given array.
 *
 *        Print Array name followed by elements.
 */
void print1DArray(
         const std::string arrayName, 
         const unsigned int * arrayData, 
         const unsigned int length)
{
    cl_uint i;
    cl_uint numElementsToPrint = (256 < length) ? 256 : length;
	int counter = 0;
    std::cout << std::endl;
    std::cout << arrayName << ":" << std::endl;
    for(i = 0; i < length; ++i)
    {
		if(arrayData[i] == i)
		{
			std::cout << arrayData[i] << " ";
			counter++;
		}
    }
    std::cout << std::endl;
	std::cout << length << " vs. " << counter << "  percent: " << (float)counter/length << "%"<<std::endl;
}

void verify()
{
    bool passed = true;
    for(unsigned int i = 0; i < width; ++i)
        //if(input[i] * multiplier != output[i])
            passed = false;

    if(passed == true)
        std::cout << "Passed!\n" << std::endl;
    else
        std::cout << "Failed!\n" << std::endl;
}

int 
main(int argc, char * argv[])
{
    // Initialize Host application 
    if(initializeHost() != SDK_SUCCESS)
        return SDK_FAILURE;

    // Initialize OpenCL resources
    if(initializeCL() != SDK_SUCCESS)
        return SDK_FAILURE;

	clock_t tstart, tend;
	tstart = clock();
    // Run the CL program
    if(runCLKernels() != SDK_SUCCESS)
        return SDK_FAILURE;
	tend = clock();

	std::cout << "Elapsed time: " << double(tend - tstart) << std::endl;

    // Print output array
    //print1DArray(std::string("Output"), output, width);

    // Verify output
    //verify();

    // Releases OpenCL resources 
    if(cleanupCL()!= SDK_SUCCESS)
        return SDK_FAILURE;

    // Release host resources
    cleanupHost();

    return SDK_SUCCESS;
}
