#include <openssl/aes.h>
#include <openssl/md5.h>
#include <openssl/rand.h>

#include <iostream>
#include <sstream>
#include <stdio.h>

#include "../include/crypto.hpp"
#include "../include/base64.hpp"

using namespace std;

Crypto::Crypto() {
	aes_encrypt_ctx = new EVP_CIPHER_CTX;
	aes_decrypt_ctx = new EVP_CIPHER_CTX;
	rsa_encrypt_ctx = new EVP_CIPHER_CTX;
	rsa_decrypt_ctx = new EVP_CIPHER_CTX;
	key_pair = new EVP_PKEY;

	//RAND_bytes(aes_key, 32);
	//RAND_bytes(aes_iv, 32);

	unsigned char *aesKey = (unsigned char*)malloc(32);
	unsigned char *aesIV = (unsigned char*)malloc(32);
}

Crypto::Crypto(string pass) {
	aes_encrypt_ctx = new EVP_CIPHER_CTX;
	aes_decrypt_ctx = new EVP_CIPHER_CTX;
	rsa_encrypt_ctx = new EVP_CIPHER_CTX;
	rsa_decrypt_ctx = new EVP_CIPHER_CTX;
	key_pair = new EVP_PKEY;

	//RAND_bytes(aes_key, 32);
	//RAND_bytes(aes_iv, 32);

	unsigned char *aesKey = (unsigned char*)malloc(32);
	unsigned char *aesIV = (unsigned char*)malloc(32);

	string salt = "CASACASA";

	EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), (unsigned char*)salt.c_str(), (unsigned char*)pass.c_str(), pass.size()+1, 100, aesKey, aesIV);

	//gen_rsa_keypair();
	//write_key_to_file();

	read_rsa_public();
	read_rsa_private();

	md5(pass);
}

Crypto::~Crypto() {
	delete aes_encrypt_ctx;
	delete aes_decrypt_ctx;
	delete rsa_encrypt_ctx;
	delete rsa_decrypt_ctx;
}

string Crypto::md5(string word) {
	unsigned char digest[MD5_DIGEST_LENGTH];
	MD5((unsigned char*)word.c_str(), word.size(), digest);

	stringstream ss;
	ss << digest;
	aes_key = ss.str();
	aes_iv = ss.str();
	
	ss.str(string());
	ss.clear();
	for(auto &c : digest) 
		ss << hex << (int)c;
	
	return ss.str();
}

void Crypto::encrypt(string filename) {
	unsigned char *file;
	int file_size = read_file(filename, &file);

	//unsigned char *encrypted_file;
	//int encrypted_file_size = aes_encrypt(file, file_size, &encrypted_file);

	unsigned char *encrypted_file;
	int encrypted_file_size = rsa_encrypt(file, file_size, &session_key, &session_key_size, &iv, &encrypted_file);

	//string result = base64_encode(file, file_size);
	
	filename = "enc_" + filename;
	int bytes_writen = write_file(filename, encrypted_file, encrypted_file_size);
}

void Crypto::decrypt(string filename) {
	unsigned char *file;
	int file_size = read_file(filename, &file);

	//unsigned char *decrypted_file;
	//int decrypted_file_size = aes_decrypt(file, file_size, &decrypted_file);

	unsigned char *decrypted_file;
	int decrypted_file_size = rsa_decrypt(file, file_size, session_key, session_key_size, iv, iv_size, &decrypted_file);

	//string str(reinterpret_cast<char*>(file));
	//cout << str << endl;
	//string result = base64_decode(str);

	filename = "dec_" + filename;
	int bytes_writen = write_file(filename, decrypted_file, decrypted_file_size);
}

void Crypto::read_rsa_public() {
	FILE *fd = fopen("pbKey", "r");
	PEM_read_PUBKEY(fd, &key_pair, NULL, NULL);
	fclose(fd);
	//PEM_write_PUBKEY(stdout, key_pair);
}


void Crypto::read_rsa_private() {
	FILE *fd = fopen("pvKey", "r");
	PEM_read_PrivateKey(fd, &key_pair, NULL, NULL);
	fclose(fd);
	//PEM_write_PrivateKey(stdout, key_pair, NULL, NULL, 0, 0, NULL);
}

bool Crypto::gen_rsa_keypair() {
	key_pair = nullptr;
	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);

	EVP_PKEY_keygen_init(ctx);
	EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, RSA_KEYLEN);
	EVP_PKEY_keygen(ctx, &key_pair);
	EVP_PKEY_CTX_free(ctx);
		
	return true;
}

int Crypto::write_key_to_file() {
	FILE *pvKey_file = fopen("pvKey", "w");
	PEM_write_PrivateKey(pvKey_file, key_pair, NULL, NULL, 0, 0, NULL);
	fclose(pvKey_file);
	
	FILE *pbKey_file = fopen("pbKey", "w");
	PEM_write_PUBKEY(pbKey_file, key_pair);
	fclose(pbKey_file);

	return 0;
}

int Crypto::rsa_encrypt(unsigned char *input, int input_size, unsigned char **session_key, int *session_key_size, unsigned char **iv, unsigned char **output) {
	int output_size = 0;
	int block_size = 0;

	*session_key = (unsigned char*)malloc(EVP_PKEY_size(key_pair));
	*iv = (unsigned char*)malloc(EVP_MAX_IV_LENGTH);

	*output = (unsigned char*)malloc(input_size + EVP_MAX_IV_LENGTH);
	
	EVP_CIPHER_CTX_init(rsa_encrypt_ctx);

	EVP_SealInit(rsa_encrypt_ctx, EVP_aes_256_cbc(), session_key, session_key_size, *iv, &key_pair, 1); 
	EVP_SealUpdate(rsa_encrypt_ctx, *output, &block_size, input, input_size);
	output_size += block_size;
	EVP_SealFinal(rsa_encrypt_ctx, *output + output_size, &block_size);
	output_size += block_size;

	EVP_CIPHER_CTX_cleanup(rsa_encrypt_ctx);

	return output_size;
}
int Crypto::rsa_decrypt(unsigned char *input, int input_size, unsigned char *session_key, int session_key_size, unsigned char *iv, int iv_size, unsigned char **output) {
	int output_size = 0;
	int block_size = 0;

	*output = (unsigned char*)malloc(input_size + iv_size);

	EVP_CIPHER_CTX_init(rsa_decrypt_ctx);

	EVP_OpenInit(rsa_decrypt_ctx, EVP_aes_256_cbc(), session_key, session_key_size, iv, key_pair);
	EVP_OpenUpdate(rsa_decrypt_ctx, *output, &block_size, input, input_size);
	output_size += block_size;
	EVP_OpenFinal(rsa_decrypt_ctx, *output + output_size, &block_size);
	output_size += block_size;

	EVP_CIPHER_CTX_cleanup(rsa_decrypt_ctx);

	return output_size;
}

int Crypto::aes_encrypt(unsigned char *input, int input_size, unsigned char **output) {
	int block_size = 0;
	int output_size = 0;

	*output = (unsigned char*)malloc(input_size + AES_BLOCK_SIZE);

	EVP_CIPHER_CTX_init(aes_encrypt_ctx);

	EVP_EncryptInit_ex(aes_encrypt_ctx, EVP_aes_256_cbc(), NULL, (unsigned char*)aes_key.c_str(), (unsigned char*)aes_iv.c_str());
	EVP_EncryptUpdate(aes_encrypt_ctx, *output, &block_size, input, input_size);
	output_size += block_size;
	EVP_EncryptFinal_ex(aes_encrypt_ctx, *output + output_size, &block_size);
	output_size += block_size;

	EVP_CIPHER_CTX_cleanup(aes_encrypt_ctx);

	return output_size;
}

int Crypto::aes_decrypt(unsigned char *input, int input_size, unsigned char **output) {
	int output_size = 0;
	int block_size = 0;

	*output = (unsigned char*)malloc(input_size);

	EVP_CIPHER_CTX_init(aes_decrypt_ctx);

	EVP_DecryptInit_ex(aes_decrypt_ctx, EVP_aes_256_cbc(), NULL, (unsigned char*)aes_key.c_str(), (unsigned char*)aes_iv.c_str());
	EVP_DecryptUpdate(aes_decrypt_ctx, *output, &block_size, input, input_size);
	output_size += block_size;
	EVP_DecryptFinal_ex(aes_decrypt_ctx, *output + output_size, &block_size);
	output_size += block_size;

	EVP_CIPHER_CTX_cleanup(aes_decrypt_ctx);

	return output_size;
}

int Crypto::write_file(std::string filename, unsigned char *file, int file_size) {
	FILE *fd = fopen(filename.c_str(), "wb");
	int bytes_writen = fwrite(file, 1, file_size, fd);
	fclose(fd);

	return bytes_writen;
}

int Crypto::read_file(std::string filename, unsigned char **file) {
	FILE *fd = fopen(filename.c_str(), "rb"); 

	fseek(fd, 0, SEEK_END);
	int file_size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	*file = (unsigned char*)malloc(file_size);
	int bytes_read = fread(*file, 1, file_size, fd);

	fclose(fd);

	return bytes_read;
}

string Crypto::get_session_key() {
	return base64_encode(session_key, session_key_size);
}
