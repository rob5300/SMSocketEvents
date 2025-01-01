#pragma once
#include <string>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>

class SignatureHelper
{
	public:
	SignatureHelper(const std::string& publicPEMKey);
	~SignatureHelper();

	//Returns if the given signature is valid for the given message
	bool IsValid(const char* message, size_t length, const unsigned char* signature, size_t signatureLength);

	private:
	EVP_PKEY* publicKey;
};