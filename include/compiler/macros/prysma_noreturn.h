#if defined(_MSC_VER)
    #define PRYSMA_NORETURN __declspec(noreturn)
#elif defined(__GNUC__) || defined(__clang__)
    #define PRYSMA_NORETURN __attribute__((noreturn))
#elif __cplusplus >= 201103L
    #define PRYSMA_NORETURN [[noreturn]]
#else
    #define PRYSMA_NORETURN
#endif