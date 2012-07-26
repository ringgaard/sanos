#define CONFIG_TCCDIR "/usr"
#define CONFIG_TCC_CRT_PREFIX "/usr/lib"
#define GCC_MAJOR 2
#define HOST_I386 1
#define TCC_VERSION "0.9.24"
#define TCC_TARGET_PE 
#undef _WIN32

#if defined(__linux)
#define TCC_PLATFORM "Linux"
#elif defined(_MSC_VER)
#define TCC_PLATFORM "Windows"
#else
#define TCC_PLATFORM "Sanos"
#endif

