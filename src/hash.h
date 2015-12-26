#pragma once

#include <string.h>

#include <inttypes.h>

	uint32_t hash_32(const void *key, uint32_t size, uint32_t seed = 0);
	void hash_128(uint32_t hash[4], const void *key, uint32_t size, uint32_t seed = 0);

	struct Hash_key
	{
		uint32_t hash;

		Hash_key(uint32_t hash = 0);
		Hash_key(const char* key);
		
		Hash_key(const Hash_key& src) = default;
		Hash_key& operator=(const Hash_key& src) = default;

		operator uint32_t() const;
		
		bool operator==(const Hash_key& src) const;
		bool operator!=(const Hash_key& src) const;

#ifndef FINAL
		char skey[32];
		operator const char*() const;
#endif
	};


	////////////////////////////////////////////////////////////////
	inline Hash_key::Hash_key(uint32_t hash) : hash(hash)
	{
#ifndef FINAL
		skey[0] = 0;
#endif
	}


	inline Hash_key::Hash_key(const char* key) : hash(hash_32(key, strlen(key)))
	{
#ifndef FINAL
		strcpy(skey, key);
#endif
	}


	inline Hash_key::operator uint32_t() const
	{
		return hash;
	}


	inline bool Hash_key::operator==(const Hash_key& src) const
	{
		return hash == src.hash;
	}


	inline bool Hash_key::operator!=(const Hash_key& src) const
	{
		return hash != src.hash;
	}


#ifndef FINAL
	inline Hash_key::operator const char*() const
	{
		return skey;
	}
#endif
