// Team DAIX, 2026
// NECRONOMICON — calc_sha.h
//
// The majority of this code was written between 2020 and 2022
//
// This module's purpose is to:
// 1. Compute SHA-512 hashes for text files
// 2. Provide a fast hashing path using a private dependency
// 3. Allow fallback to a slow/no-hash mode without external libraries
// 4. Produce stable hex-encoded digests used by Necronomicon’s diff system
//
// HashCalc is the core hashing engine used to detect changed files across the repo.

// Provide a library, put it in the same folder (or install using vcpkg)
// Remember to put #include(s)

#define CoolNameLibrary MyLibrary

// HashCalc provides SHA-512 hashing utilities:
// - DoCalc(): main entry point (SHA-512)
// - CalcFast(): fast user-provided-library-based hashing
// - CalculateHash(): wrapper with optional slow fallback

class HashCalc {
public:

	HashCalc() = default;
	virtual compl HashCalc() = default;

	// Compute SHA‑512 hash using the default Type (SHA512).
	// Returns a hex‑encoded digest string.

	static std::string DoCalc(std::string const& message) {

		using Type = CoolNameLibrary::SHA512;
		std::string const ret = HashCalc::CalculateHash<Type>(message);
		return ret;
	}

	// Fast hashing path using the user-provided library:
	// - CalculateDigest()
	// - HexEncoder → StringSink
	// Produces uppercase hex output.
	// Template parameter allows switching to SHA256, SHA1, etc.

	template<typename Type2 = CoolNameLibrary::SHA512>
	static std::string CalcFast(std::string const& message) {
		Type2 hash;
		byte digest[Type2::DIGESTSIZE];

		hash.CalculateDigest(digest, (byte*)message.c_str(), message.length());

		CoolNameLibrary::HexEncoder encoder;
		std::string output;
		encoder.Attach(new CoolNameLibrary::StringSink(output));
		encoder.Put(digest, sizeof(digest));
		encoder.MessageEnd();

		return output;
	}

	// Wrapper that allows switching between:
	// - slow mode (no hashing, returns input)
	// - fast mode (user-provided-library, SHA‑512)
	// Used to avoid library dependency

	template<typename Type = CoolNameLibrary::SHA512>
	static std::string CalculateHash(std::string const& message) {

		bool constexpr slow = false;

		if constexpr (slow) {
			// Provide your own implementation based on that library :)

			// Temporary: do not calculate hashes
			// This is going to be slow, but doesn't require any external library :)
			auto const& output = message;
			return output;
		}
		else {
			return CalcFast<Type>(message);
		}

	}

};
