// c-stdlib-zero-length-memory-slice-lab
// C11 memory function correctness evidence helper
// Does NOT execute null-pointer libc calls, overlapping memcpy, or UB.

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <math.h>

static void print_hex(const unsigned char *b, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (i) printf(" ");
        printf("%02x", b[i]);
    }
}

static void print_floats(const float *f, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (i) printf(",");
        if (f[i] == 0.0f && signbit(f[i])) printf("-0.0");
        else printf("%.6g", f[i]);
    }
}

// guarded slice copy: returns 0=success, 1=invalid_pointer
// if length==0: success, no libc call
// if length!=0 and (dst==NULL || src==NULL): invalid_pointer, no libc call
// else: memcpy, success
// out_libc_called: 0/1
static int guarded_slice_copy(void *dst, const void *src, size_t n, int *out_libc_called) {
    if (out_libc_called) *out_libc_called = 0;
    if (n == 0) return 0;
    if (dst == NULL || src == NULL) return 1;
    if (out_libc_called) *out_libc_called = 1;
    memcpy(dst, src, n);
    return 0;
}

// guarded slice compare: returns 0=success, 1=invalid_pointer, 2=invalid_output
// out_equal: 0/1
// out_libc_called: 0/1
static int guarded_slice_compare(const void *a, const void *b, size_t n, int *out_equal, int *out_libc_called) {
    if (out_libc_called) *out_libc_called = 0;
    if (out_equal == NULL) return 2;
    if (n == 0) { *out_equal = 1; return 0; }
    if (a == NULL || b == NULL) return 1;
    if (out_libc_called) *out_libc_called = 1;
    int r = memcmp(a, b, n);
    *out_equal = (r == 0) ? 1 : 0;
    return 0;
}

int main(void) {
    printf("{\n");
#ifdef __STDC_VERSION__
    printf("  \"STDC_VERSION\": %ld,\n", (long)__STDC_VERSION__);
#else
    printf("  \"STDC_VERSION\": null,\n");
#endif
    printf("  \"CHAR_BIT\": %d,\n", CHAR_BIT);
    printf("  \"sizeof_char\": %zu,\n", sizeof(char));
    printf("  \"sizeof_void_p\": %zu,\n", sizeof(void*));
    printf("  \"sizeof_size_t\": %zu,\n", sizeof(size_t));
#ifdef SIZE_MAX
    printf("  \"SIZE_MAX\": %llu,\n", (unsigned long long)SIZE_MAX);
#else
    printf("  \"SIZE_MAX\": null,\n");
#endif

    void *(*memcpy_fp)(void*, const void*, size_t) = memcpy;
    void *(*memmove_fp)(void*, const void*, size_t) = memmove;
    int (*memcmp_fp)(const void*, const void*, size_t) = memcmp;
    void *(*memset_fp)(void*, int, size_t) = memset;
    (void)memcpy_fp; (void)memmove_fp; (void)memcmp_fp; (void)memset_fp;
    printf("  \"memcpy_callable\": true,\n");
    printf("  \"memmove_callable\": true,\n");
    printf("  \"memcmp_callable\": true,\n");
    printf("  \"memset_callable\": true,\n");
    printf("  \"signbit_available\": true,\n");

    // valid_zero_length_memcpy
    {
        unsigned char src[8] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77};
        unsigned char dst[8] = {0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x88,0x99};
        unsigned char dst_before[8];
        memcpy(dst_before, dst, 8);
        void *ret = memcpy(dst, src, 0);
        printf("  \"zmemcpy_src_before\": \"");
        print_hex(src,8); printf("\",\n");
        printf("  \"zmemcpy_dst_before\": \"");
        print_hex(dst_before,8); printf("\",\n");
        printf("  \"zmemcpy_dst_after\": \"");
        print_hex(dst,8); printf("\",\n");
        printf("  \"zmemcpy_ret_eq_dst\": %s,\n", ret == (void*)dst ? "true" : "false");
    }

    // valid_zero_length_memmove
    {
        unsigned char buf[8] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77};
        unsigned char before[8]; memcpy(before, buf, 8);
        void *ret = memmove(buf+2, buf, 0);
        printf("  \"zmemmove_before\": \"");
        print_hex(before,8); printf("\",\n");
        printf("  \"zmemmove_after\": \"");
        print_hex(buf,8); printf("\",\n");
        printf("  \"zmemmove_ret_eq_dst\": %s,\n", ret == (void*)(buf+2) ? "true" : "false");
    }

    // valid_zero_length_memset
    {
        unsigned char buf[8] = {0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80};
        unsigned char before[8]; memcpy(before, buf, 8);
        void *ret = memset(buf+3, 0xff, (0));
        printf("  \"zmemset_before\": \"");
        print_hex(before,8); printf("\",\n");
        printf("  \"zmemset_after\": \"");
        print_hex(buf,8); printf("\",\n");
        printf("  \"zmemset_ret_eq_dst\": %s,\n", ret == (void*)(buf+3) ? "true" : "false");
    }

    // valid_zero_length_memcmp
    {
        unsigned char a[4] = {0x00,0x01,0x02,0x03};
        unsigned char b[4] = {0xff,0xfe,0xfd,0xfc};
        int r = memcmp(a,b,0);
        printf("  \"zmemcmp_result\": %d,\n", r);
    }

    // one_past_zero_length
    {
        unsigned char src[8] = {0,1,2,3,4,5,6,7};
        unsigned char dst[8] = {10,11,12,13,14,15,16,17};
        unsigned char src_before[8], dst_before[8];
        memcpy(src_before, src, 8); memcpy(dst_before, dst, 8);
        void *r1 = memcpy(dst+8, src+8, 0);
        void *r2 = memmove(dst+8, src+8, 0);
        int r3 = memcmp(src+8, dst+8, 0);
        void *r4 = memset(dst+8, 0, (0));
        printf("  \"onepast_src_before\": \"");
        print_hex(src_before,8); printf("\",\n");
        printf("  \"onepast_src_after\": \"");
        print_hex(src,8); printf("\",\n");
        printf("  \"onepast_dst_before\": \"");
        print_hex(dst_before,8); printf("\",\n");
        printf("  \"onepast_dst_after\": \"");
        print_hex(dst,8); printf("\",\n");
        printf("  \"onepast_memcpy_ret_eq\": %s,\n", r1 == (void*)(dst+8) ? "true" : "false");
        printf("  \"onepast_memmove_ret_eq\": %s,\n", r2 == (void*)(dst+8) ? "true" : "false");
        printf("  \"onepast_memcmp_result\": %d,\n", r3);
        printf("  \"onepast_memset_ret_eq\": %s,\n", r4 == (void*)(dst+8) ? "true" : "false");
    }

    // guarded_null_empty_copy
    {
        int libc_called = -1;
        int status = guarded_slice_copy(NULL, NULL, 0, &libc_called);
        printf("  \"guard_copy_status\": %d,\n", status);
        printf("  \"guard_copy_libc_called\": %s,\n", libc_called ? "true" : "false");
    }

    // guarded_null_empty_compare
    {
        int equal = -1;
        int libc_called = -1;
        int status = guarded_slice_compare(NULL, NULL, 0, &equal, &libc_called);
        printf("  \"guard_compare_status\": %d,\n", status);
        printf("  \"guard_compare_equal\": %d,\n", equal);
        printf("  \"guard_compare_libc_called\": %s,\n", libc_called ? "true" : "false");
    }

    // nonzero_null_rejection (5 cases)
    {
        int libc_called;
        int r1, r2, r3, r4, r5;
        int dummy_equal;
        char valid_src = 'x';
        char valid_dst;
        r1 = guarded_slice_copy(NULL, &valid_src, 1, &libc_called);
        int lc1 = libc_called;
        r2 = guarded_slice_copy(&valid_dst, NULL, 1, &libc_called);
        int lc2 = libc_called;
        r3 = guarded_slice_compare(NULL, &valid_src, 1, &dummy_equal, &libc_called);
        int lc3 = libc_called;
        r4 = guarded_slice_compare(&valid_src, NULL, 1, &dummy_equal, &libc_called);
        int lc4 = libc_called;
        r5 = guarded_slice_compare(&valid_src, &valid_src, 1, NULL, &libc_called);
        int lc5 = libc_called;
        printf("  \"guard_reject_statuses\": [%d,%d,%d,%d,%d],\n", r1,r2,r3,r4,r5);
        printf("  \"guard_reject_libc_called\": [%s,%s,%s,%s,%s],\n",
            lc1?"true":"false", lc2?"true":"false", lc3?"true":"false", lc4?"true":"false", lc5?"true":"false");
    }

    // memcpy_nonoverlap length 0..8
    {
        unsigned char src[8] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77};
        printf("  \"nonoverlap_results\": [\n");
        for (int len=0; len<=8; len++) {
            unsigned char dst[8] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
            void *ret = memcpy(dst, src, len);
            printf("    {\"len\": %d, \"dst\": \"", len);
            print_hex(dst,8);
            printf("\", \"ret_eq\": %s}%s\n", ret==(void*)dst?"true":"false", len<8?",":"");
        }
        printf("  ],\n");
    }

    // memmove forward overlap
    {
        unsigned char buf[8] = {'a','b','c','d','e','f','g','h'};
        memmove(buf+2, buf, 6);
        printf("  \"memmove_forward_out\": \"");
        print_hex(buf,8); printf("\",\n");
    }

    // memmove backward overlap
    {
        unsigned char buf[8] = {'a','b','c','d','e','f','g','h'};
        memmove(buf, buf+2, 6);
        printf("  \"memmove_backward_out\": \"");
        print_hex(buf,8); printf("\",\n");
    }

    // memcmp length_and_sign
    {
        unsigned char a[4] = {0x00,0x01,0x02,0x03};
        unsigned char b[4] = {0x00,0x01,0x04,0x03};
        printf("  \"memcmp_signs\": [");
        for (int len=0; len<=4; len++) {
            int r = memcmp(a,b,len);
            if (len) printf(",");
            printf("%d", r);
        }
        printf("],\n");
    }

    // empty_slice_concat: NULL/0 + "ab" + NULL/0 + "cd"
    {
        unsigned char out[4] = {0};
        int total = 0;
        int libc_calls[4] = {0};
        int statuses[4] = {0};
        // slice 1: NULL, 0
        statuses[0] = guarded_slice_copy(out+total, NULL, 0, &libc_calls[0]);
        // slice 2: "ab", 2
        unsigned char s2[] = {'a','b'};
        statuses[1] = guarded_slice_copy(out+total, s2, 2, &libc_calls[1]);
        if (statuses[1]==0) total += 2;
        // slice 3: NULL, 0
        statuses[2] = guarded_slice_copy(out+total, NULL, 0, &libc_calls[2]);
        // slice 4: "cd", 2
        unsigned char s4[] = {'c','d'};
        statuses[3] = guarded_slice_copy(out+total, s4, 2, &libc_calls[3]);
        if (statuses[3]==0) total += 2;
        printf("  \"concat_statuses\": [%d,%d,%d,%d],\n", statuses[0],statuses[1],statuses[2],statuses[3]);
        printf("  \"concat_libc_calls\": [%s,%s,%s,%s],\n",
            libc_calls[0]?"true":"false", libc_calls[1]?"true":"false", libc_calls[2]?"true":"false", libc_calls[3]?"true":"false");
        printf("  \"concat_total\": %d,\n", total);
        printf("  \"concat_out\": \"");
        print_hex(out,4); printf("\",\n");
    }

    // fixed_feature_row_copy
    {
        float src[4] = {0.25f, -0.0f, 1.5f, 2.0f};
        float dst[4] = {0};
        memcpy(dst, src, sizeof(src));
        printf("  \"feature_src_floats\": [0.25,-0.0,1.5,2.0],\n");
        printf("  \"feature_dst_floats\": [");
        print_floats(dst,4);
        printf("],\n");
        printf("  \"feature_nbytes\": %zu,\n", sizeof(src));
        printf("  \"feature_src_bytes\": \"");
        print_hex((unsigned char*)src, sizeof(src));
        printf("\",\n");
        printf("  \"feature_dst_bytes\": \"");
        print_hex((unsigned char*)dst, sizeof(dst));
        printf("\",\n");
        int sb = signbit(dst[1]);
        printf("  \"feature_negzero_signbit\": %s,\n", sb ? "true" : "false");
    }

    // empty_feature_batch + nonempty batch
    {
        size_t rows_empty = 0;
        size_t cols = 4;
        size_t elem_size = sizeof(float);
        int overflow_empty = (cols != 0 && rows_empty > SIZE_MAX / cols) ? 1 : 0;
        size_t elem_count_empty = overflow_empty ? 0 : rows_empty * cols;
        int overflow_bytes_empty = (elem_count_empty != 0 && elem_size > SIZE_MAX / elem_count_empty) ? 1 : 0;
        size_t byte_count_empty = overflow_bytes_empty ? 0 : elem_count_empty * elem_size;
        int libc_called_empty = -1;
        int guard_status_empty = guarded_slice_copy(NULL, NULL, byte_count_empty, &libc_called_empty);

        size_t rows_full = 2;
        size_t cols_full = 3;
        int overflow_full = (cols_full != 0 && rows_full > SIZE_MAX / cols_full) ? 1 : 0;
        size_t elem_count_full = overflow_full ? 0 : rows_full * cols_full;
        int overflow_bytes_full = (elem_count_full != 0 && elem_size > SIZE_MAX / elem_count_full) ? 1 : 0;
        size_t byte_count_full = overflow_bytes_full ? 0 : elem_count_full * elem_size;
        float batch_src[6] = {0.0f, 0.5f, 1.0f, -1.0f, 2.0f, 3.5f};
        float batch_dst[6] = {0};
        int libc_called_full = -1;
        int guard_status_full = 0;
        if (!overflow_full && !overflow_bytes_full) {
            guard_status_full = guarded_slice_copy(batch_dst, batch_src, byte_count_full, &libc_called_full);
        }

        printf("  \"batch_empty_rows\": %zu,\n", rows_empty);
        printf("  \"batch_empty_cols\": %zu,\n", cols);
        printf("  \"batch_empty_elem_count\": %zu,\n", elem_count_empty);
        printf("  \"batch_empty_byte_count\": %zu,\n", byte_count_empty);
        printf("  \"batch_empty_overflow\": %s,\n", (overflow_empty||overflow_bytes_empty)?"true":"false");
        printf("  \"batch_empty_guard_status\": %d,\n", guard_status_empty);
        printf("  \"batch_empty_libc_called\": %s,\n", libc_called_empty ? "true" : "false");

        printf("  \"batch_full_rows\": %zu,\n", rows_full);
        printf("  \"batch_full_cols\": %zu,\n", cols_full);
        printf("  \"batch_full_elem_count\": %zu,\n", elem_count_full);
        printf("  \"batch_full_byte_count\": %zu,\n", byte_count_full);
        printf("  \"batch_full_overflow\": %s,\n", (overflow_full||overflow_bytes_full)?"true":"false");
        printf("  \"batch_full_guard_status\": %d,\n", guard_status_full);
        printf("  \"batch_full_libc_called\": %s,\n", libc_called_full ? "true" : "false");
        printf("  \"batch_full_dst_floats\": [");
        print_floats(batch_dst, 6);
        printf("],\n");
    }

    printf("  \"libc_id\": null,\n");
    printf("  \"libc_unidentified\": true\n");
    printf("}\n");
    return 0;
}
