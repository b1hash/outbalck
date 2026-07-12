#pragma once

#include <string>
#include "UIBranding.h"

// License file format version
const int LICENSE_FILE_VERSION = 1;

// Magic constant (警告：BRAND_LICENSE_MAGIC 为系统保留，请勿修改！)
const char* const LICENSE_MAGIC = BRAND_LICENSE_MAGIC;

// License file data structure
struct LicenseFileData {
    std::string sn;
    std::string password;
    std::string pwdHmac;
    std::string authorization;
    std::string frpConfig;      // FRP 代理配置（可选）
    std::string createTime;
    int version;

    // Helper: check if V2 auth
    bool IsV2Auth() const {
        return pwdHmac.size() >= 3 && pwdHmac.substr(0, 3) == "v2:";
    }
};

// SN match result
enum class SNMatchResult {
    Match,
    HardwareMismatch,
    IPMismatch
};

// Import result enum
enum class LicenseImportResult {
    Success = 0,
    FileNotFound,
    InvalidFormat,
    InvalidMagic,
    VersionTooHigh,
    ChecksumMismatch,
    SNMismatchHardware,
    SNMismatchIP,
    IncompleteData
};

// Export license to file
// filePath: output file path
// sn: serial number / device ID
// password: password string
// pwdHmac: HMAC signature
// authorization: optional multi-layer auth
// frpConfig: optional FRP proxy config
// Returns: true on success
bool ExportLicenseFile(const std::string& filePath,
                       const std::string& sn,
                       const std::string& password,
                       const std::string& pwdHmac,
                       const std::string& authorization = "",
                       const std::string& frpConfig = "");

// Import license from file
// filePath: input file path
// outData: output license data
// outError: output error message
// Returns: LicenseImportResult
LicenseImportResult ImportLicenseFile(const std::string& filePath,
                                       LicenseFileData& outData,
                                       std::string& outError);

// Apply license data to current program
// data: license data to apply
// Returns: true on success
bool ApplyLicenseData(const LicenseFileData& data);

// Get import error message
std::string GetImportErrorMessage(LicenseImportResult result);

// Validate SN (hardware ID or IP)
SNMatchResult ValidateLicenseSN(const std::string& licenseSN);

// Check if SN is IPv4 format
bool IsIPv4Format(const std::string& sn);
