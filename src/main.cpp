#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <cstring>
#include <cstdint>
#include <sys/mman.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include <optional>
#include <array>

#define LOG_TAG "NoHurtCam"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static bool g_enabled = true;

static std::optional<std::array<float, 3>> (*g_tryGetDamageBob_orig)(
    void**,
    void*,
    float
) = nullptr;

static std::optional<std::array<float, 3>>
VanillaCameraAPI_tryGetDamageBob_hook(
    void** self,
    void* traits,
    float a
) {
    if (g_enabled) {
        return std::nullopt;
    }
    return g_tryGetDamageBob_orig(self, traits, a);
}

static bool parseMapsLine(
    const std::string& line,
    uintptr_t& start,
    uintptr_t& end
) {
    return sscanf(line.c_str(), "%lx-%lx", &start, &end) == 2;
}

static bool findAndHookVanillaCameraAPI() {
    void* mcLib = dlopen("libminecraftpe.so", RTLD_NOLOAD);
    if (!mcLib) mcLib = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (!mcLib) return false;

    constexpr const char* RTTI_NAME = "16VanillaCameraAPI";
    const size_t RTTI_LEN = strlen(RTTI_NAME);

    uintptr_t rttiNameAddr = 0;
    uintptr_t typeinfoAddr = 0;
    uintptr_t vtableAddr = 0;

    std::ifstream maps;
    std::string line;

    maps.open("/proc/self/maps");
    while (std::getline(maps, line)) {
        if (line.find("libminecraftpe.so") == std::string::npos) continue;
        if (line.find("r--p") == std::string::npos &&
            line.find("r-xp") == std::string::npos) continue;

        uintptr_t start, end;
        if (!parseMapsLine(line, start, end)) continue;

        for (uintptr_t p = start; p < end - RTTI_LEN; ++p) {
            if (memcmp((void*)p, RTTI_NAME, RTTI_LEN) == 0) {
                rttiNameAddr = p;
                break;
            }
        }
        if (rttiNameAddr) break;
    }
    maps.close();

    if (!rttiNameAddr) return false;

    maps.open("/proc/self/maps");
    while (std::getline(maps, line)) {
        if (line.find("libminecraftpe.so") == std::string::npos) continue;
        if (line.find("r--p") == std::string::npos) continue;

        uintptr_t start, end;
        if (!parseMapsLine(line, start, end)) continue;

        for (uintptr_t p = start; p < end - sizeof(void*); p += sizeof(void*)) {
            if (*(uintptr_t*)p == rttiNameAddr) {
                typeinfoAddr = p - sizeof(void*);
                break;
            }
        }
        if (typeinfoAddr) break;
    }
    maps.close();

    if (!typeinfoAddr) return false;

    maps.open("/proc/self/maps");
    while (std::getline(maps, line)) {
        if (line.find("libminecraftpe.so") == std::string::npos) continue;
        if (line.find("r--p") == std::string::npos) continue;

        uintptr_t start, end;
        if (!parseMapsLine(line, start, end)) continue;

        for (uintptr_t p = start; p < end - sizeof(void*); p += sizeof(void*)) {
            if (*(uintptr_t*)p == typeinfoAddr) {
                vtableAddr = p + sizeof(void*);
                break;
            }
        }
        if (vtableAddr) break;
    }
    maps.close();

    if (!vtableAddr) return false;

    void** slot = (void**)(vtableAddr + 2 * sizeof(void*));
    g_tryGetDamageBob_orig =
        (decltype(g_tryGetDamageBob_orig))(*slot);

    if (!g_tryGetDamageBob_orig) return false;

    uintptr_t page = (uintptr_t)slot & ~4095UL;
    if (mprotect((void*)page, 4096, PROT_READ | PROT_WRITE) != 0) return false;

    *slot = (void*)VanillaCameraAPI_tryGetDamageBob_hook;
    mprotect((void*)page, 4096, PROT_READ);

    return true;
}

extern "C" {
    __attribute__((visibility("default")))
    void LeviMod_Load() {
        LOGI("LeviMod_Load()");
        if (!findAndHookVanillaCameraAPI()) return LOGE("!findAndHookVanillaCameraAPI()");
        LOGI("Successfully Activated Mod: NoHurtCam!");
    }
}