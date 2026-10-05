#pragma once
#include <string>
#include <array>
#include <cstdint>

// Derives avatar parameter names from the password so they can't be read from OSC traffic.
// Must match the Unity side:
//   name = lowercase hex of HMAC-SHA256(key = UTF-8 password, message = UTF-8 original name),
//          truncated to NAME_LENGTH characters
class ParameterObfuscator
{
public:
	explicit ParameterObfuscator(std::string password);

	auto Obfuscate(const std::string& name) const -> std::string;

	static auto HmacSha256(const std::string& key, const std::string& message) -> std::array<uint8_t, 32>;
public:
	static constexpr std::size_t NAME_LENGTH = 16;
private:
	std::string password;
};
