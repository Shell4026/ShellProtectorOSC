#pragma once
#include <string>
#include <array>
#include <cstdint>

// Derives avatar parameter names from a key so their role can't be read from the name.
// The key comes from UserKey. Must match the Unity side:
//   name = lowercase hex of HMAC-SHA256(key, message = UTF-8 original name),
//          truncated to NAME_LENGTH characters
class ParameterObfuscator
{
public:
	explicit ParameterObfuscator(std::string key);

	auto Obfuscate(const std::string& name) const -> std::string;

	static auto HmacSha256(const std::string& key, const std::string& message) -> std::array<uint8_t, 32>;
public:
	static constexpr std::size_t NAME_LENGTH = 16;
private:
	std::string key;
};
