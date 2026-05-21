#if __cplusplus >= 201703L
    #define PRYSMA_MAYBE_UNUSED [[maybe_unused]]
#elif defined(__GNUC__) || defined(__clang__)
    #define PRYSMA_MAYBE_UNUSED __attribute__((unused))
#else
    #define PRYSMA_MAYBE_UNUSED
#endif