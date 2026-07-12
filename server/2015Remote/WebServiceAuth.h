#pragma once
// ============================================================
//  WebServiceAuth.h - WebService Authorization Module
// ============================================================
//
//  This file declares the interface for WebService authorization.
//
//  Design for future migration to private library:
//    - Keep this .h file in main project
//    - Move WebServiceAuth.cpp to private library
//    - Link private library instead
//
// ============================================================

#include <string>
#include <cstdint>

// ============================================================
//  Context structure
// ============================================================

struct WSAuthContext {
    std::string secretKey;      // Token signing key
    int tokenExpireSec;         // Token expiration (seconds)
    int maxClientsPerDevice;    // Max concurrent viewers per device

    WSAuthContext() : tokenExpireSec(86400), maxClientsPerDevice(10) {}
};

// ============================================================
//  Authorization API
// ============================================================

namespace WSAuth {

// Initialize WebService authorization
// authorizationString: raw authorization string from config (verified internally)
// Returns true if authorized, fills ctx with runtime parameters
// Returns false if not authorized (WebService should not start)
bool Init(WSAuthContext& ctx, const std::string& authorizationString);

// Generate a signed token for authenticated user
// Returns empty string on failure
std::string GenerateToken(const std::string& username,
                          const std::string& role,
                          int expireSeconds);

// Validate a token and extract user info
// Returns true if valid, fills username and role
// Returns false if invalid or expired
bool ValidateToken(const std::string& token,
                   std::string& username,
                   std::string& role);

// Verify password against stored hash
// Returns true if password matches
bool VerifyPassword(const std::string& password,
                    const std::string& storedHash,
                    const std::string& salt);

// Compute SHA256 hash of input
std::string ComputeSHA256(const std::string& input);

// Compute salted password hash for storage
std::string ComputePasswordHash(const std::string& password,
                                const std::string& salt);

// ============================================================
//  Runtime Configuration
// ============================================================

// Set token expiration time (default: 86400 = 24 hours)
void SetTokenExpireSec(int seconds);

// Get current token expiration time
int GetTokenExpireSec();

// Set max concurrent clients per device (default: 10)
void SetMaxClientsPerDevice(int maxClients);

// Get current max clients per device
int GetMaxClientsPerDevice();

} // namespace WSAuth
