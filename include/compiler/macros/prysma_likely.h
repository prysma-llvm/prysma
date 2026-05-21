#if defined(__cplusplus) && __cplusplus >= 202002L
    #define PRYSMA_LIKELY_BRANCH [[likely]]
#else
    #define PRYSMA_LIKELY_BRANCH

    #if defined(__GNUC__) || defined(__clang__)
        #define PRYSMA_LIKELY_FOR(x)   (__builtin_expect(!!(x), 1))
    #else
        #define PRYSMA_LIKELY_FOR(x)   (x)
    #endif
#endif