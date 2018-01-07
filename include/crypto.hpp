#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <string>

#define RSA_KEYLEN 2048

class Crypto {
public:
	Crypto();
	Crypto(std::string);
	~Crypto();

	void encrypt(std::string);
	void decrypt(std::string);
	std::string get_session_key();
	std::string get_iv();
	std::string md5(std::string); 
private:
	//unsigned char aes_key[32];
	//unsigned char aes_iv[32];

	EVP_PKEY *key_pair;
	RSA *pub;
	RSA *pri;

	std::string aes_key;
	std::string aes_iv;

	EVP_CIPHER_CTX *aes_encrypt_ctx;
	EVP_CIPHER_CTX *aes_decrypt_ctx;

	EVP_CIPHER_CTX *rsa_encrypt_ctx;
	EVP_CIPHER_CTX *rsa_decrypt_ctx;

	unsigned char *session_key;
	unsigned char *iv;
	int session_key_size;
	int iv_size;

	bool gen_rsa_keypair();
	void read_rsa_public();
	void read_rsa_private();
	int write_key_to_file();
	int rsa_encrypt(unsigned char*, int, unsigned char**, int*, unsigned char**, unsigned char**);
	int rsa_decrypt(unsigned char*, int, unsigned char*, int, unsigned char*, int, unsigned char**);
	int aes_encrypt(unsigned char*, int, unsigned char**);
	int aes_decrypt(unsigned char*, int, unsigned char**);
	int write_file(std::string, unsigned char*, int);
	int read_file(std::string, unsigned char**);
};

#endif // CRYPTO_HPP
