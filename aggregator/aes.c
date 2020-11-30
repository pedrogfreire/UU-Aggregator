#include <openssl/aes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char indata[AES_BLOCK_SIZE]; //buffer of chunk of message to encrypt 
unsigned char outdata[AES_BLOCK_SIZE]; //buffer of chunk of message encrypted
unsigned char decryptdata[AES_BLOCK_SIZE];
unsigned char userkey[] = "\x09\x8F\x6B\xCD\x46\x21\xD3\x73\xCA\xDE\x4E\x83\x26\x27\xB4\xF6";
unsigned char IV[] = "\x0A\x91\x72\x71\x6A\xE6\x42\x84\x09\x88\x5B\x8B\x82\x9C\xCB\x05";

AES_KEY key;

/***********************************************************************************
*  copy_vector																	   *
* Copy a vector without considering the null termination						   *
* 																				   *
* Arguments: char *message, int length 											   *
* Return: void																	   *
*		  																		   *
************************************************************************************/
void copy_vector_str (char* destination, char* source, int length){
	int i;
	for (i=0; i<length; i++){
		destination[i] = source[i];		
	}	
}

/***********************************************************************************
* aes_encrypt_message   														   *
* Encrypt message (variable message) with AES algorithn. Write result in the	   *
* pointer (char *encrypted_message)												   *
* Arguments: char *message, char *encrypted_message								   *
* Return: unsigned int encryption message lenght		       					   *
*		  																		   *
************************************************************************************/
unsigned int aes_encrypt_message(char *message, char *encrypted_message) {   
    int postion = 0;
    int bytes_read, bytes_write;
	int count_parts=0;
	int encrypt_len=0;
	int chunk_size;
	AES_set_encrypt_key(userkey, 128, &key);
    while (1) {
        unsigned char ivec[AES_BLOCK_SIZE];
        memcpy(ivec, IV, AES_BLOCK_SIZE);
		chunk_size = strlen((char *) &message[count_parts*AES_BLOCK_SIZE]);
		memset(indata,0,AES_BLOCK_SIZE);
		if(chunk_size<AES_BLOCK_SIZE){
			bytes_read = strlen((char *) &message[count_parts*AES_BLOCK_SIZE]);
			strncpy(indata, (char *) &message[count_parts*AES_BLOCK_SIZE], bytes_read);
		}
		else if(chunk_size==0){
			break;
		}
		else{
			bytes_read = AES_BLOCK_SIZE;
			strncpy(indata, (char *) &message[count_parts*AES_BLOCK_SIZE], AES_BLOCK_SIZE);
		}
        encrypt_len += bytes_read;
        AES_cfb128_encrypt(indata, &encrypted_message[count_parts*AES_BLOCK_SIZE], bytes_read, &key, ivec, &postion, AES_ENCRYPT);		
		count_parts++;
        if (bytes_read < AES_BLOCK_SIZE)
            break;
    }
	return encrypt_len;
	
}
/***********************************************************************************
* aes_decrypt_message   														   *
* Decrypt message (variable message) with AES algorithn. Write result in the	   *
* pointer (char *decrypted_message)												   *
* Arguments: char *message, char *decrypted_message, int length					   *
* Return: void											       					   *
*		  																		   *
************************************************************************************/
void aes_decrypt_message(char *message, char *decrypted_message, int length) {
    FILE *ifp, *ofp;
	AES_set_encrypt_key(userkey, 128, &key);
    int postion = 0;
	int count_parts=0;
    int bytes_read, bytes_write;
	int chunk_size;
    while (1) {
        unsigned char ivec[AES_BLOCK_SIZE];
        memcpy(ivec, IV, AES_BLOCK_SIZE);
		chunk_size = length-(count_parts*AES_BLOCK_SIZE);
		memset(indata,0,AES_BLOCK_SIZE);		
		if(chunk_size<AES_BLOCK_SIZE){//last part
			bytes_read = chunk_size;
			copy_vector_str(indata, (char *) &message[count_parts*AES_BLOCK_SIZE], bytes_read);
			memset(&indata[bytes_read],0,(AES_BLOCK_SIZE-bytes_read));
		}
		else if(chunk_size==0){
			break;
		}
		else{
			bytes_read = AES_BLOCK_SIZE;
			copy_vector_str(indata, (char *) &message[count_parts*AES_BLOCK_SIZE], bytes_read);
		}
        AES_cfb128_encrypt(indata, &decrypted_message[count_parts*AES_BLOCK_SIZE], bytes_read, &key, ivec, &postion, AES_DECRYPT);
        count_parts++;
        if (bytes_read < AES_BLOCK_SIZE)
            break;
    }
}
