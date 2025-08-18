#ifndef TEST_H_
#define TEST_H_

#define W 16
#define H 12

static int g_fail = 0;

#define ASSERT_TRUE(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "ASSERT_TRUE failed: %s (%s:%d)\n", #expr, __FILE__, __LINE__); \
        g_fail++; \
    } \
} while(0)

#define ASSERT_EQ_U32(a,b) do { \
    uint32_t _aa=(a), _bb=(b); \
    if (_aa!=_bb) { \
        fprintf(stderr, "ASSERT_EQ_U32 failed: %s=0x%08x %s=0x%08x (%s:%d)\n", \
            #a, _aa, #b, _bb, __FILE__, __LINE__); \
        g_fail++; \
    } \
} while(0)

#define ASSERT_EQ_I(a,b) do { \
    long long _aa=(long long)(a), _bb=(long long)(b); \
    if (_aa!=_bb) { \
        fprintf(stderr, "ASSERT_EQ_I failed: %s=%lld %s=%lld (%s:%d)\n", \
            #a,_aa,#b,_bb,__FILE__,__LINE__); \
        g_fail++; \
    } \
} while(0)

#endif // TEST_H_