#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "limits.h"

#include <CL/cl.h>

#define CHUNK_SIZE 1000000
#define HASH_TABLE_SIZE 15000
#define MULTIPLICATIVE_INCREASE 2;
#define VECTOR_SIZE 16

struct cl_state {
    cl_device_id device_id;
    cl_context context;
    cl_command_queue commands;
    cl_program program;
    cl_kernel kernel;
    cl_platform_id* platforms;
    size_t workgroup_size;
    cl_uint num_platforms;
    cl_mem hash_mem;

    double benchmark;
};

struct dynamic_array {
    long* data;
    size_t array_size;
    size_t current_index;
};

struct bignum {
    ulong data[VECTOR_SIZE];
};

struct dynamic_array* da_init() {
    struct dynamic_array* arr = malloc(sizeof(struct dynamic_array));
    arr->array_size = 1;
    arr->current_index = 0;
    arr->data = malloc(sizeof(long) * arr->array_size);
    return arr;
}

void da_append(struct dynamic_array* arr, long value) {
    if (arr->current_index >= arr->array_size) {
        size_t new_size = arr->array_size * MULTIPLICATIVE_INCREASE;
        long* new_data = malloc(sizeof(long) * new_size);
        memcpy(new_data, arr->data, arr->current_index * sizeof(long));
        free(arr->data);
        arr->data = new_data;
        arr->array_size = new_size;
    }
    arr->data[arr->current_index++] = value;
}

int da_read(struct dynamic_array* arr, int index) {
    return arr->data[index];
}

void da_print(struct dynamic_array* arr) {
    printf("[");
    for (size_t i = 0; i < arr->current_index; i++) {
        printf("%li", arr->data[i]);
        if (i != arr->current_index - 1)
            printf(", ");
    }
    printf("]\n");
}

void da_free(struct dynamic_array* arr) {
    free(arr->data);
    free(arr);
}

struct bignum* bignum_init() {
    struct bignum* num = malloc(sizeof(struct bignum));
    memset(num->data, 0, VECTOR_SIZE * sizeof(ulong));
    return num;
}

void bignum_add(struct bignum* bignum, ulong value) {
    ulong initial_value = bignum->data[0];
    bignum->data[0] += value;
    if (bignum->data[0] < initial_value) {
        for (int index = 1; index < VECTOR_SIZE; index++) {
            bignum->data[index]++;
            if (bignum->data[index] != 0)
                break;
        }
    }
}

void bignum_print(struct bignum* bignum) {
    for (int i = VECTOR_SIZE - 1; i >= 0; i--) {
        printf("%lu", bignum->data[i]);
    }
    printf("\n");
}

unsigned char* gen_rdm_bytestream(size_t num_bytes) {
    unsigned char* stream = malloc(num_bytes);
    size_t i;

    for (i = 0; i < num_bytes; i++) {
        stream[i] = rand();
    }

    return stream;
}

long string_hash(unsigned char* bytestring, uint8_t length) {
    register long x;
    register long len = length;
    unsigned char* p = bytestring;

    x = *p << 7;
    while (--len >= 0)
        x = (1000003 * x) ^ *p++;
    x ^= length;
    if (x == -1)
        return -2;
    return x;
}

char* read_file(const char* filename, size_t* file_size) {
    char* buffer = NULL;

    FILE* fp = fopen(filename, "r");
    fseek(fp, 0, SEEK_END);
    *file_size = ftell(fp);

    rewind(fp);

    buffer = (char*) malloc((*file_size + 1) * sizeof(*buffer));

    fread(buffer, *file_size, 1, fp);

    buffer[*file_size] = '\0';

    return buffer;
}

const char* getErrorString(cl_int error) {
    switch (error) {
        // run-time and JIT compiler errors
        case 0:
            return "CL_SUCCESS";
        case -1:
            return "CL_DEVICE_NOT_FOUND";
        case -2:
            return "CL_DEVICE_NOT_AVAILABLE";
        case -3:
            return "CL_COMPILER_NOT_AVAILABLE";
        case -4:
            return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
        case -5:
            return "CL_OUT_OF_RESOURCES";
        case -6:
            return "CL_OUT_OF_HOST_MEMORY";
        case -7:
            return "CL_PROFILING_INFO_NOT_AVAILABLE";
        case -8:
            return "CL_MEM_COPY_OVERLAP";
        case -9:
            return "CL_IMAGE_FORMAT_MISMATCH";
        case -10:
            return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
        case -11:
            return "CL_BUILD_PROGRAM_FAILURE";
        case -12:
            return "CL_MAP_FAILURE";
        case -13:
            return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
        case -14:
            return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
        case -15:
            return "CL_COMPILE_PROGRAM_FAILURE";
        case -16:
            return "CL_LINKER_NOT_AVAILABLE";
        case -17:
            return "CL_LINK_PROGRAM_FAILURE";
        case -18:
            return "CL_DEVICE_PARTITION_FAILED";
        case -19:
            return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

        // compile-time errors
        case -30:
            return "CL_INVALID_VALUE";
        case -31:
            return "CL_INVALID_DEVICE_TYPE";
        case -32:
            return "CL_INVALID_PLATFORM";
        case -33:
            return "CL_INVALID_DEVICE";
        case -34:
            return "CL_INVALID_CONTEXT";
        case -35:
            return "CL_INVALID_QUEUE_PROPERTIES";
        case -36:
            return "CL_INVALID_COMMAND_QUEUE";
        case -37:
            return "CL_INVALID_HOST_PTR";
        case -38:
            return "CL_INVALID_MEM_OBJECT";
        case -39:
            return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        case -40:
            return "CL_INVALID_IMAGE_SIZE";
        case -41:
            return "CL_INVALID_SAMPLER";
        case -42:
            return "CL_INVALID_BINARY";
        case -43:
            return "CL_INVALID_BUILD_OPTIONS";
        case -44:
            return "CL_INVALID_PROGRAM";
        case -45:
            return "CL_INVALID_PROGRAM_EXECUTABLE";
        case -46:
            return "CL_INVALID_KERNEL_NAME";
        case -47:
            return "CL_INVALID_KERNEL_DEFINITION";
        case -48:
            return "CL_INVALID_KERNEL";
        case -49:
            return "CL_INVALID_ARG_INDEX";
        case -50:
            return "CL_INVALID_ARG_VALUE";
        case -51:
            return "CL_INVALID_ARG_SIZE";
        case -52:
            return "CL_INVALID_KERNEL_ARGS";
        case -53:
            return "CL_INVALID_WORK_DIMENSION";
        case -54:
            return "CL_INVALID_WORK_GROUP_SIZE";
        case -55:
            return "CL_INVALID_WORK_ITEM_SIZE";
        case -56:
            return "CL_INVALID_GLOBAL_OFFSET";
        case -57:
            return "CL_INVALID_EVENT_WAIT_LIST";
        case -58:
            return "CL_INVALID_EVENT";
        case -59:
            return "CL_INVALID_OPERATION";
        case -60:
            return "CL_INVALID_GL_OBJECT";
        case -61:
            return "CL_INVALID_BUFFER_SIZE";
        case -62:
            return "CL_INVALID_MIP_LEVEL";
        case -63:
            return "CL_INVALID_GLOBAL_WORK_SIZE";
        case -64:
            return "CL_INVALID_PROPERTY";
        case -65:
            return "CL_INVALID_IMAGE_DESCRIPTOR";
        case -66:
            return "CL_INVALID_COMPILER_OPTIONS";
        case -67:
            return "CL_INVALID_LINKER_OPTIONS";
        case -68:
            return "CL_INVALID_DEVICE_PARTITION_COUNT";

        // extension errors
        case -1000:
            return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
        case -1001:
            return "CL_PLATFORM_NOT_FOUND_KHR";
        case -1002:
            return "CL_INVALID_D3D10_DEVICE_KHR";
        case -1003:
            return "CL_INVALID_D3D10_RESOURCE_KHR";
        case -1004:
            return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
        case -1005:
            return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
        default:
            return "Unknown OpenCL error";
    }
}

struct cl_state* init_gpu() {
    int err;

    struct cl_state* state = malloc(sizeof(struct cl_state));

    state->platforms = malloc(sizeof(cl_platform_id) * 5);

    err = clGetPlatformIDs(5, state->platforms, &(state->num_platforms));
    if (err != CL_SUCCESS) {
        printf("Failed to query platforms! %s\n", getErrorString(err));
        return NULL;
    }

    printf("Num platforms: %u\n", state->num_platforms);

    int i = 0;
    do  {
        err = clGetDeviceIDs(state->platforms[i], CL_DEVICE_TYPE_ALL, 1,
                             &(state->device_id), NULL);
    } while (err != CL_SUCCESS && (cl_uint) i++ < state->num_platforms);

    if (err != CL_SUCCESS) {
        printf("Failed to create device group on any device! %s\n",
               getErrorString(err));
        return NULL;
    }

    state->context = clCreateContext(0, 1, &(state->device_id),
                                     NULL, NULL, &err);
    if (!state->context) {
        printf("Failed to create a context! %s\n", getErrorString(err));
        return NULL;
    }

    state->commands = clCreateCommandQueue(state->context,
                                           state->device_id,
                                           CL_QUEUE_PROFILING_ENABLE,
                                           &err);
    if (!state->commands) {
        printf("Failed to create a command queue! %s\n", getErrorString(err));
        return NULL;
    }

    size_t file_size;
    const char* hash_kernel =
        read_file("include/hash.cl", &file_size);
    state->program = clCreateProgramWithSource(state->context, 1,
                     &hash_kernel, NULL, &err);
    if (!state->program) {
        printf("Failed to create compute program! %s\n", getErrorString(err));
        return NULL;
    }

    err = clBuildProgram(state->program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        size_t len;
        char buffer[2048];

        printf("Failed to build program executable! Compile log:\n\n");
        clGetProgramBuildInfo(state->program, state->device_id,
                              CL_PROGRAM_BUILD_LOG,
                              sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
        return NULL;
    }

    state->kernel = clCreateKernel(state->program, "hash", &err);
    if (!state->kernel || err != CL_SUCCESS) {
        printf("Failed to create hash kernel! %s\n", getErrorString(err));
        return NULL;
    }

    state->hash_mem = clCreateBuffer(state->context,
                                     CL_MEM_WRITE_ONLY,
                                     sizeof(long) * CHUNK_SIZE,
                                     NULL,
                                     NULL);
    if (!state->hash_mem) {
        printf("Failed to allocate device memory!\n %s", getErrorString(err));
        return NULL;
    }

    err = clGetKernelWorkGroupInfo(state->kernel, state->device_id,
                                   CL_KERNEL_WORK_GROUP_SIZE,
                                   sizeof(state->workgroup_size),
                                   &state->workgroup_size,
                                   NULL);
    if (err != CL_SUCCESS) {
        printf("Failed to retrieve kernel work group info! %s\n",
               getErrorString(err));
        return NULL;
    }

    printf("OpenCL successfully configured\n");

    return state;
}

void free_gpu(struct cl_state* state) {
    clReleaseMemObject(state->hash_mem);
    clReleaseProgram(state->program);
    clReleaseKernel(state->kernel);
    clReleaseCommandQueue(state->commands);
    clReleaseContext(state->context);
    clReleaseDevice(state->device_id);
}

int hash(struct bignum* offset, struct cl_state* state, long* hash_outputs,
         long d_mask) {
    int err;
    cl_event hash_task;

    cl_mem offset_mem = clCreateBuffer(state->context,
                                       CL_MEM_READ_ONLY,
                                       sizeof(struct bignum),
                                       NULL,
                                       NULL);
    err = clEnqueueWriteBuffer(state->commands, offset_mem, CL_TRUE, 0,
                               sizeof(struct bignum), &(offset->data), 0,
                               NULL, NULL);
    if (err != CL_SUCCESS) {
        printf("Failed to write offset! %s\n",
               getErrorString(err));
        return -1;
    }

    cl_mem d_mem = clCreateBuffer(state->context,
                                  CL_MEM_READ_ONLY,
                                  sizeof(long),
                                  NULL,
                                  NULL);
    err = clEnqueueWriteBuffer(state->commands, d_mem, CL_TRUE, 0,
                               sizeof(long), &d_mask, 0,
                               NULL, NULL);

    err  = clSetKernelArg(state->kernel, 0, sizeof(cl_mem), &offset_mem);
    err  = clSetKernelArg(state->kernel, 1, sizeof(cl_mem), &d_mem);
    err |= clSetKernelArg(state->kernel, 2, sizeof(cl_mem),
                          &(state->hash_mem));
    if (err != CL_SUCCESS) {
        printf("Failed to set kernel arguments! %s\n",
               getErrorString(err));
        return -1;
    }

    size_t work_size = CHUNK_SIZE;
    size_t workgroup_size = state->workgroup_size;
    printf("Workgroup size: %li\n", workgroup_size);

    err = clEnqueueNDRangeKernel(state->commands,
                                 state->kernel,
                                 1,
                                 0,
                                 &work_size,
                                 NULL,
                                 0,
                                 NULL,
                                 &hash_task);
    if (err) {
        printf("Failed to execute kernel! %s\n", getErrorString(err));
        return -1;
    }

    clWaitForEvents(1, &hash_task);

    clFinish(state->commands);

    err = clEnqueueReadBuffer(state->commands, state->hash_mem, CL_TRUE, 0,
                              sizeof(long) * work_size, hash_outputs, 0,
                              NULL, NULL);
    if (err != CL_SUCCESS) {
        printf("Failed to read output array! %s\n",
               getErrorString(err));
        return -1;
    }

    cl_ulong start_time, end_time;

    clGetEventProfilingInfo(hash_task, CL_PROFILING_COMMAND_START,
                            sizeof(start_time), &start_time, NULL);

    clGetEventProfilingInfo(hash_task, CL_PROFILING_COMMAND_END,
                            sizeof(end_time), &end_time, NULL);

    state->benchmark = end_time - start_time;

    return 0;
}

int main(int argc, char** argv) {
    size_t d = 10;
    if (argc == 2)
        d = atoi(argv[1]);

    long d_mask = (1 << d) - 1;
    struct dynamic_array* hash_table[HASH_TABLE_SIZE];
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        hash_table[i] = da_init();
    }

    struct cl_state* state = init_gpu();
    struct bignum* offset = bignum_init();

    int keep_iterating = 1;
    while (keep_iterating) {
        long hash_outputs[CHUNK_SIZE];
        memset(&hash_outputs, 0, sizeof(hash_outputs));
        if (hash(offset, state, hash_outputs, d_mask) == -1)
            return 1;
        float start = (float) clock() / CLOCKS_PER_SEC;
        for (int i = 0; i < CHUNK_SIZE; i++) {
            if (hash_outputs[i] > 0) {
                printf("Collision with %li\n", hash_outputs[i]);
                keep_iterating = 0;
            }
        }
        float end = (float) clock() / CLOCKS_PER_SEC;
        printf("Collision check time: %f\n", end - start);
        printf("Hash time: %0.5f\n", state->benchmark / 1000000000.0);
        bignum_add(offset, CHUNK_SIZE);
        bignum_print(offset);
    }

    free(offset);
    free_gpu(state);

    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        da_free(hash_table[i]);
    }
}
