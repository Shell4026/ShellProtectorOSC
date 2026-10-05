#include "PCH.h"
#include "UserKey.h"
#include "SHA256.h"
#include "ParameterObfuscator.h"

#include <memory>
#include <algorithm>

namespace
{
	// HMAC-SHA256 with the padded key already absorbed, so each MAC costs two compressions
	class PrecomputedHmac
	{
	public:
		explicit PrecomputedHmac(const std::string& key)
		{
			constexpr std::size_t BLOCK_SIZE = 64;

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
			inner.update(innerPad.data(), innerPad.size());
			outer.update(outerPad.data(), outerPad.size());
		}

		auto Compute(const uint8_t* message, std::size_t length) const -> std::array<uint8_t, 32>
		{
			SHA256 in = inner;
			in.update(message, length);
			std::unique_ptr<uint8_t[]> innerHash(in.digest());

			SHA256 out = outer;
			out.update(innerHash.get(), 32);
			std::unique_ptr<uint8_t[]> outerHash(out.digest());

			std::array<uint8_t, 32> result;
			std::copy(outerHash.get(), outerHash.get() + 32, result.begin());
			return result;
		}
	private:
		SHA256 inner;
		SHA256 outer;
	};
}

auto UserKey::Derive(const std::string& password, int keyLength, const std::string& salt) -> UserKey
{
	// Only the first keyLength bytes of the password are part of the key
	std::string truncated = password.substr(0, static_cast<std::size_t>(keyLength > 0 ? keyLength : 0));

	UserKey key;
	key.derived = Pbkdf2HmacSha256(truncated, salt, ITERATIONS);
	key.salt = salt;
	return key;
}

// PBKDF2 with a single 32-byte output block
auto UserKey::Pbkdf2HmacSha256(const std::string& password, const std::string& salt, int iterations) -> std::array<uint8_t, 32>
{
	PrecomputedHmac hmac(password);

	std::string first = salt;
	first.append({ 0, 0, 0, 1 }); // Big-endian block index 1

	auto u = hmac.Compute(reinterpret_cast<const uint8_t*>(first.data()), first.size());
	auto result = u;
	for (int i = 1; i < iterations; ++i)
	{
		u = hmac.Compute(u.data(), u.size());
		for (std::size_t j = 0; j < result.size(); ++j)
			result[j] ^= u[j];
	}
	return result;
}

bool UserKey::IsValidSalt(const std::string& salt)
{
	if (salt.size() != SALT_HEX_LENGTH)
		return false;
	return std::all_of(salt.begin(), salt.end(), [](char c)
		{
			return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
		});
}

auto UserKey::GetKeyByte(int index) const -> uint8_t
{
	return (std::min)(derived[index], MAX_KEY_BYTE);
}

auto UserKey::ObfuscateParameter(const std::string& name) const -> std::string
{
	std::string nameKey(reinterpret_cast<const char*>(derived.data()) + 16, 16);
	return ParameterObfuscator(nameKey).Obfuscate(name);
}

auto UserKey::GetSalt() const -> const std::string&
{
	return salt;
}
