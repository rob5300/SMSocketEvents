#include "SignatureHelper.h"
#include <openssl/conf.h>
#include <openssl/pem.h>
#include <iostream>
#include "socket_extension.h"

SignatureHelper::SignatureHelper(const std::string& publicPEMKey)
{
    if(publicPEMKey.empty()) return;

    OPENSSL_init_crypto(0, nullptr);
    
    //Load pem key and read into a EVP_PKEY
    BIO* bio = BIO_new_mem_buf(publicPEMKey.c_str(), -1);
    publicKey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
}

bool SignatureHelper::IsValid(const char* message, size_t length, const unsigned char* signature, size_t signatureLength)
{
    if (publicKey == nullptr)
    {
        SocketExtension::PrintError("A signature check was attempted but no public key is loaded.");
        return false;
    }

    const auto ctx = EVP_MD_CTX_new();
    bool success = EVP_DigestVerifyInit(ctx, NULL, EVP_sha256(), NULL, publicKey);
    if(!success) return false;

    success = EVP_DigestVerifyUpdate(ctx, message, length);
    if(!success) return false;

    const int sigResult = EVP_DigestVerifyFinal(ctx, signature, signatureLength);
    EVP_MD_CTX_free(ctx);

    if (sigResult != 1)
    {
        ERR_print_errors_fp(stderr);
    }
    return sigResult;
}

SignatureHelper::~SignatureHelper()
{
    EVP_PKEY_free(publicKey);
}
