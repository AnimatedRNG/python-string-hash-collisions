#define INPUT_SIZE 16
#define BYTES_IN_LONG 8
#define SZ INPUT_SIZE * BYTES_IN_LONG

long string_hash(unsigned char* bytestring) {
    long x;
    unsigned char* p = bytestring;

    x = *p << 7;
    #pragma unroll
    for (int i = 0; i < SZ; i++)
        x = (1000003 * x) ^ *p++;
    x ^= SZ;
    if (x == -1)
        return -2;
    return x;
}

void bignum_add(ulong bignum[16], ulong value) {
    ulong initial_value = bignum[0];
    bignum[0] += value;
    if (bignum[0] < initial_value) {
#pragma unroll
        for (int index = 1; index < INPUT_SIZE; index++) {
            bignum[index]++;
            if (bignum[index] != 0)
                break;
        }
    }
}

__kernel void hash(
    __constant ulong* offset,
    __constant ulong* cmp,
    __global long* output) {
    int array_id = get_global_id(0);

    ulong offset_cpy[INPUT_SIZE];
#pragma unroll
    for (int i = 0; i < INPUT_SIZE; i++) {
        offset_cpy[i] = offset[i];
    }
    bignum_add(offset_cpy, array_id);
    uchar* key = (unsigned char *) (&offset_cpy);

    ulong d_mask = *cmp;
    long hash_value = string_hash(key);
    if ((hash_value & d_mask) == d_mask)
       output[array_id] = hash_value;
    else
       output[array_id] = 0;
}
