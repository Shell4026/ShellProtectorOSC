#pragma once
#include <string>
#include <array>
#include <cstdint>

// The user part of the key and the avatar parameter names, derived from the password with a per-avatar salt.
// Must match UserKey.cs on the Unity side:
//   password = UTF-8 bytes of the password, truncated to the key length
//   salt     = ASCII bytes of the 32-character lowercase hex salt string
//   derived  = PBKDF2-HMAC-SHA256(password, salt, ITERATIONS, 32 bytes)
//   key      = min(derived[i], MAX_KEY_BYTE) for i in [0, keyLength)
//   name(n)  = ParameterObfuscator(key = derived[16, 32)).Obfuscate(n)
// The avatar exposes the salt as a local-only parameter named SALT_PARAMETER_PREFIX + salt.
class UserKey
{
public:
	static auto Derive(const std::string& password, int keyLength, const std::string& salt) -> UserKey;
	static auto Pbkdf2HmacSha256(const std::string& password, const std::string& salt, int iterations) -> std::array<uint8_t, 32>;
	static bool IsValidSalt(const std::string& salt);

	auto GetKeyByte(int index) const -> uint8_t;
	auto ObfuscateParameter(const std::string& name) const -> std::string;
	auto GetSalt() const -> const std::string&;
public:
	static constexpr int ITERATIONS = 600000;
	static constexpr std::size_t SALT_HEX_LENGTH = 32;
	static constexpr const char* SALT_PARAMETER_PREFIX = "SP_SALT_";
	// Key bytes travel as synced floats, which carry only 255 distinct values over the network
	static constexpr uint8_t MAX_KEY_BYTE = 254;
private:
	std::array<uint8_t, 32> derived{};
	std::string salt;
};
