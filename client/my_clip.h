#include <windows.h>
#include <string>
#include <mutex>

namespace clip {
    using format = UINT;

    inline CRITICAL_SECTION& GetClipLock() {
        static CRITICAL_SECTION cs;
        static std::once_flag flag;
        std::call_once(flag, [] { InitializeCriticalSection(&cs); });
        return cs;
    }

    inline format text_format() {
        return CF_TEXT;
    }

    inline bool has(format f) {
        return IsClipboardFormatAvailable(f) != FALSE;
    }

    inline bool set_text(const std::string& value) {
        EnterCriticalSection(&GetClipLock());

        bool result = false;
        if (OpenClipboard(NULL)) {
            EmptyClipboard();
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, value.size() + 1);
            if (hMem) {
                char* p = (char*)GlobalLock(hMem);
                if (p) {
                    memcpy(p, value.c_str(), value.size() + 1);
                    GlobalUnlock(hMem);
                    SetClipboardData(CF_TEXT, hMem);
                    result = true;
                }
                else {
                    GlobalFree(hMem);
                }
            }
            CloseClipboard();
        }

        LeaveCriticalSection(&GetClipLock());
        return result;
    }

    inline bool get_text(std::string& value) {
        EnterCriticalSection(&GetClipLock());

        bool result = false;
        if (OpenClipboard(NULL)) {
            // 优先尝试 Unicode
            HANDLE h = GetClipboardData(CF_UNICODETEXT);
            if (h) {
                const wchar_t* pw = (const wchar_t*)GlobalLock(h);
                if (pw) {
                    int len = WideCharToMultiByte(CP_UTF8, 0, pw, -1, NULL, 0, NULL, NULL);
                    if (len > 0) {
                        value.resize(len - 1);
                        WideCharToMultiByte(CP_UTF8, 0, pw, -1, &value[0], len, NULL, NULL);
                    }
                    GlobalUnlock(h);
                    result = true;
                }
            }
            else {
                // 回退 ANSI
                h = GetClipboardData(CF_TEXT);
                if (h) {
                    const char* p = (const char*)GlobalLock(h);
                    if (p) {
                        SIZE_T size = GlobalSize(h);
                        value.assign(p, strnlen(p, size));
                        GlobalUnlock(h);
                        result = true;
                    }
                }
            }
            CloseClipboard();
        }

        LeaveCriticalSection(&GetClipLock());
        return result;
    }
} // namespace clip
