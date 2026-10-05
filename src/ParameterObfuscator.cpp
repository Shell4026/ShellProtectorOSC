#include "PCH.h"
#include "ParameterObfuscator.h"
#include "SHA256.h"

#include <memory>
#include <algorithm>

ParameterObfuscator::ParameterObfuscator(std::string password) :
	password(std::move(password))
{
}

auto ParameterObfuscator::Obfuscate(const std::string& name) const -> std::string
{
	static constexpr char HEX[] = "0123456789abcdef";

	const auto mac = HmacSha256(password, name);
	std::string result;
	result.reserve(NAME_LENGTH);
	for (std::size_t i = 0; result.size() < NAME_LENGTH; ++i)
	{
		result.push_back(HEX[mac[i] >> 4]);
		result.push_back(HEX[mac[i] & 0x0f]);
	}
	result.resize(NAME_LENGTH);
	return result;
}

auto ParameterObfuscator::HmacSha256(const std::string& key, const std::string& message) -> std::array<uint8_t, 32>
{
	constexpr std::size_t BLOCK_SIZE = 64;

	// Keys longer than the block size are hashed first (RFC 2104)
	std::array<uint8_t, BLOCK_SIZE> block{};
	if (key.size() > BLOCK_SIZE)
	{
		SHA256 sha;
		sha.update(key);
		std::unique_ptr<uint8_t[]> hashed(sha.digest());
		std::copy(hashed.get(), hashed.get() + 32, block.begin());
	}
	else
		std::copy(key.begin(), key.end(), block.begin());

	std::array<uint8_t, BLOCK_SIZE> innerPad, outerPad;
	for (std::size_t i = 0; i < BLOCK_SIZE; ++i)
	{
		innerPad[i] = block[i] ^ 0x36;
		outerPad[i] = block[i] ^ 0x5c;
	}

	SHA256 inner;
	inner.update(innerPad.data(), innerPad.size());
	inner.update(message);
	std::unique_ptr<uint8_t[]> innerHash(inner.digest());

	SHA256 outer;
	outer.update(outerPad.data(), outerPad.size());
	outer.update(innerHash.get(), 32);
	std::unique_ptr<uint8_t[]> outerHash(outer.digest());

	std::array<uint8_t, 32> result;
	std::copy(outerHash.get(), outerHash.get() + 32, result.begin());
	return result;
}
