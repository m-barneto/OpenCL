#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#define CL_TARGET_OPENCL_VERSION 220
#define MAX_SOURCE_SIZE (0x100000)

#include <CL/cl.h>

struct Kernel {
    cl_program program;
    cl_kernel kernel;
};

class OpenCLHelper {
private:
    cl_platform_id platformID;
    cl_device_id deviceID;
    cl_uint numDevices, numPlatforms;
    cl_context context;
    cl_command_queue commandQueue;
    std::vector<Kernel> kernels;
    std::vector<cl_mem> buffers;
public:
    OpenCLHelper() {
        platformID, deviceID = NULL;
        // Get platform and device information
        clGetPlatformIDs(1, &platformID, &numPlatforms);
        clGetDeviceIDs(platformID, CL_DEVICE_TYPE_DEFAULT, 1, &deviceID, &numDevices);

        // Create an OpenCL context
        context = clCreateContext(NULL, 1, &deviceID, NULL, NULL, NULL);

        // Create a command queue
        commandQueue = clCreateCommandQueueWithProperties(context, deviceID, 0, NULL);
    }

    ~OpenCLHelper() {
        #pragma region Cleanup OpenCL
        // Clean up
        clFlush(commandQueue);
        clFinish(commandQueue);
        for (int i = 0; i < kernels.size(); ++i) {
            clReleaseKernel(kernels[i].kernel);
            clReleaseProgram(kernels[i].program);
        }
        for (int i = 0; i < buffers.size(); ++i) {
            clReleaseMemObject(buffers[i]);
        }
        clReleaseCommandQueue(commandQueue);
        clReleaseContext(context);
        #pragma endregion
    }

    cl_int createKernel(const char* file, const char* kernel, Kernel& kOut) {
        // Create a program from the kernel source
        cl_program prog = createProgram(file);
        kOut.program = prog;
        // Build the program
        cl_int err = clBuildProgram(kOut.program, 1, &deviceID, NULL, NULL, NULL);
        if (err != 0) {
            return err;
        }
        // Set the main method of the kernel
        kOut.kernel = clCreateKernel(kOut.program, kernel, NULL);

        kernels.push_back(kOut);
        return 0;
    }

    template <class T>
    cl_int writeArg(Kernel& kernel, cl_uint argIndex, T arg) {
        return clSetKernelArg(kernel.kernel, argIndex, sizeof(T), (void*)&arg);
    }

    template <class T>
    cl_int readArg(size_t argIndex, cl_mem buffer, void* dataPtr, size_t bufferLength = 1) {
        bufferLength *= sizeof(T);
        return clEnqueueReadBuffer(commandQueue, buffer, CL_TRUE, 0, bufferLength, dataPtr, 0, NULL, NULL);
    }

    cl_int runKernel(Kernel& kernel, size_t length, size_t chunkSize) {
        return clEnqueueNDRangeKernel(commandQueue, kernel.kernel, 1, NULL, &length, &chunkSize, 0, NULL, NULL);
    }

    template <class T>
    cl_mem createBuffer(std::vector<T> data, cl_mem_flags flag, cl_bool blocking = CL_TRUE) {
        cl_int err;
        cl_mem buffer = clCreateBuffer(context, flag, sizeof(T) * data.size(), NULL, &err);
        buffers.push_back(buffer);
        return buffer;
    }

    template <class T>
    cl_int writeToBuffer(Kernel& kernel, std::vector<T> data, cl_mem buffer) {
        return clEnqueueWriteBuffer(commandQueue, buffer, CL_TRUE, 0, sizeof(T) * data.size(), data.data(), 0, NULL, NULL);
    }
private:
    cl_program createProgram(const char* file) {
        // Load the kernel source code into the array source_str
        FILE* fp;
        char* source_str;
        size_t source_size;

        fp = fopen(file, "r");
        if (!fp) {
            fprintf(stderr, "Failed to load kernel.\n");
            exit(1);
        }
        source_str = (char*)malloc(MAX_SOURCE_SIZE);
        if (source_str) {
            source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
        }
        else {
            fprintf(stderr, "Failed to load source code from file.\n");
            exit(1);
        }
        fclose(fp);

        return clCreateProgramWithSource(context, 1, (const char**)&source_str, (const size_t*)&source_size, NULL);
    }

    template<typename T> struct isVector : public std::false_type {};

    template<typename T, typename A>
    struct isVector<std::vector<T, A>> : public std::true_type {};
};

/*
template <class T>
class Argument {
private:
    OpenCLHelper ocl;
    bool isVec;
public:
    T val;
    cl_mem buffer;
    cl_uint argIndex;
    Argument(T& val, cl_uint argIndex, OpenCLHelper& ocl, cl_mem_flags flag) {
        this->ocl = ocl;
        // Check if value is vector here?
        this->val = val;
        ocl.createBuffer<T>({ val }, flag);
    }

    T read(cl_uint argIndex) {
        ocl.readArg(argIndex, buffer, &val[0], val.size());
    }

    void write(Kernel& kernel, cl_uint argIndex) {
        ocl.writeToBuffer(kernel, val, buffer);
        ocl.writeArg(kernel, argIndex, val);
    }
};
*/


