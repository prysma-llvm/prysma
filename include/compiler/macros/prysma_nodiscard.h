#if __cplusplus >= 201703L
    #define PRYSMA_NODISCARD [[nodiscard]]
#elif defined(__GNUC__) || defined(__clang__)
    #define PRYSMA_NODISCARD __attribute__((warn_unused_result))
#elif defined(_MSC_VER)
    #define PRYSMA_NODISCARD __declspec(nodiscard)
#else
    #define PRYSMA_NODISCARD
#endif