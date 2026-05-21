#if defined(__cplusplus) && __cplusplus >= 202002L
    #define PRYSMA_UNLIKELY_BRANCH [[unlikely]]
#else
    #define PRYSMA_UNLIKELY_BRANCH
    
    #if defined(__GNUC__) || defined(__clang__)
        #define PRYSMA_UNLIKELY_FOR(x) (__builtin_expect(!!(x), 0))
    #else
        #define PRYSMA_UNLIKELY_FOR(x) (x)
    #endif
#endif