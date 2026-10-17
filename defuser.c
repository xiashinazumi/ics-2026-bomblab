#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t hi = (uint32_t)(challenge >> 32);
    uint32_t lo = (uint32_t)challenge;
    uint32_t step1 = ((uint32_t)(challenge >> 16) + 0x5482) ^ hi;
    uint32_t step0 = (hi ^ 0x393) + lo * 8;
    uint32_t step2 = 3 * (step0 - step1) + 0xe00b;
    uint32_t values[MAX_FIELDS] = {step2};
    size_t count = 1;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    values[0] = ((uint32_t)(challenge * 2) & 0x1e) + 0x15;
    unsigned shift = 0x17;
    for (size_t i = 1; i < 6; ++i) {
        values[i] = (((uint32_t)(challenge >> shift) & 0x3f) + values[i - 1]) ^ (uint32_t)i;
        shift += 3;
    }
    size_t count = 6;
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t index = (uint32_t)challenge & 7;
    uint32_t y = (uint32_t)(challenge >> 3) ^ (uint32_t)(challenge >> 32);
    uint32_t result;
    switch (index) {
    case 0: result = 3 * y + 0xadd1;           break;  /* p3_case_3 */
    case 1: result = 5 * y + 0x49e7b;          break;  /* p3_case_7 */
    case 2: result = 4 * y ^ 0x4a9e;           break;  /* p3_case_1 */
    case 3: result = 5 * y + 0x6dbf;           break;  /* p3_case_2 */
    case 4: result = 3 * y + 0xb823;           break;  /* p3_case_4 */
    case 5: result = (y >> 2) + (y ^ 0xc191);  break;  /* p3_case_5 */
    case 6: result = (y >> 1) + (y ^ 0x8ab8);  break;  /* p3_case_0 */
    default: result = 4 * y ^ 0x37b8;          break;  /* p3_case_6 */
    }
    values[0] = index;
    values[1] = result;
    size_t count = 2;
    return write_answer(out, cap, values, count);
}

static uint32_t p4_walk(uint32_t target, uint32_t lo, uint32_t hi, uint32_t key) {
    uint32_t mid = ((hi - lo) >> 1) + lo;
    if (mid == target) {
        return (mid + 0x5b) ^ key;
    }
    uint32_t temp = mid + key;
    if (target < mid) {
        uint32_t result = p4_walk(target, lo, mid - 1, key);
        return ((result * 3) ^ temp) ^ 0x1;
    } else {
        uint32_t result = p4_walk(target, mid + 1, hi, key);
        return ((result * 3) ^ temp) ^ 0x5;
    }
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t target = (uint32_t)challenge & 0x7f;
    uint32_t key = (uint32_t)(challenge >> 32) ^ (uint32_t)(challenge >> 7);
    values[0] = p4_walk(target, 0, 0x7f, key);
    size_t count = 1;
    return write_answer(out, cap, values, count);
}

/* p5_nodes 的静态拷贝：value, next_index, next（next 用数组下标表示，-1 = NULL） */
struct p5_node { uint32_t value; uint32_t next_index; int next; };
static const struct p5_node p5_nodes[] = {
    {49873,  632,  7},   /* A 0x405ba0 */
    { 4794, 1582, 11},   /* B 0x405bb0 */
    {58963, 2951,  5},   /* C 0x405bc0 */
    {47108, 1848, -1},   /* D 0x405bd0 (链尾) */
    {37661, 2648,  3},   /* E 0x405be0 */
    {43898, 1138,  8},   /* F 0x405bf0 */
    {36594,  277,  4},   /* G 0x405c00 */
    { 7167,   87,  6},   /* H 0x405c10 */
    {50758, 1328,  0},   /* I 0x405c20 */
    {49658, 2162,  1},   /* J 0x405c30 */
    {41251, 2376,  9},   /* K 0x405c40 */
    {63295,  783,  2},   /* L 0x405c50 */
};
static const int p5_heads[] = {10, 9, 1, 11, 2, 5, 8, 0};

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    int node = p5_heads[challenge & 0x7];
    uint32_t result = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    result = (result ^ 0x26d9) + p5_nodes[node].value;
    result = result * 5;
    values[1] = p5_nodes[node].next_index;
    result ^= p5_nodes[node].next_index;
    uint32_t remain = (uint32_t)(challenge >> 3) & 0x3;
    for (uint32_t i = 1; i <= remain; ++i) {
        node = p5_nodes[node].next;
        result += p5_nodes[node].value;
        result *= 5;
        values[1] = p5_nodes[node].next_index;
        result ^= p5_nodes[node].next_index;
    }
    values[0] = result;
    size_t count = 2;
    return write_answer(out, cap, values, count);
}

/* p6_ops 函数指针表：index -> op 函数（表项顺序由编译器决定，按 .rodata 实际内容映射） */
static uint32_t p6_op(uint32_t p, uint32_t op) {
    switch (op) {
    case 0:
    case 1: return (p + 0x84f1) ^ (p >> 5);              /* p6_op_4 */
    case 2: return (p >> 1) | (p << 31);                  /* p6_op_5: ror 1 */
    case 3:
    case 7: return (p << 7) + (p ^ 0x480c);               /* p6_op_2 */
    case 4: return ((p >> 5) ^ p) + 0x3665;               /* p6_op_1 */
    case 5: return ((p << 7) | (p >> 25)) ^ 0x254b;       /* p6_op_3: rol 7 */
    default: return (p * 4 + 0x3882) ^ (p * 7);           /* index 6: p6_op_0 */
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t p = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    uint32_t r = p6_op(p, (uint32_t)challenge & 7);
    r = p6_op(r ^ 0xbd7c, (uint32_t)(challenge >> 3) & 7);
    values[0] = r + 0xe486;
    size_t count = 1;
    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t input1 = (uint32_t)context & 0x3;
    uint32_t para2  = (uint32_t)challenge ^ (uint32_t)(challenge >> 32);
    uint32_t r8     = (uint32_t)(context >> 32) ^ (uint32_t)context;
    uint32_t mix    = (7 * (para2 ^ r8) + 0x5c88) ^ 0x9826;
    uint32_t input2 = mix ^ (input1 * 0x5c88 ^ (uint32_t)(context >> 16));
    uint32_t input3 = input2 ^ (r8 + input1 + 0x9826 + (para2 >> 3));
    /* secret_gate 的行格式：32 字节任意前缀 + "OPEN"（偏移 32）+ 3 个 %u（偏移 36 起），总长 36~163 */
    int n = snprintf(out, cap,
                     "7e4231978addb6aa7d96974d19006f3cOPEN %" PRIu32 " %" PRIu32 " %" PRIu32,
                     input1, input2, input3);
    if (n < 0 || (size_t)n >= cap) return 1;
    return 0;
}
