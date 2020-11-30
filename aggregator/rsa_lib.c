#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#define KEY_LENGTH  2048
#define PUB_EXP     3
#define MESSAGE_MAX_LENGTH  210
#define ENCRYPTED_PART_LENGTH  256

size_t pri_len;            // Length of private key
size_t pub_len;            // Length of public key
char   *pri_key;           // Private key
char   *pub_key;           // Public key
char   msg[KEY_LENGTH/8];  // Message to encrypt


char   *err;               // Buffer for any error messages
RSA *keypairPriv = NULL;
RSA *keypairPub = NULL;



/***********************************************************************************
*  print_encrypt_message														   *
* Print encrypted message with decimal numbers		 							   *
* 																				   *
* Arguments: char *message, int length 												   *
* Return: void																	   *
*		  																		   *
************************************************************************************/
void print_encrypt_message (char* message, int length){
	int i;
	printf("\n");
	for (i=0; i<length; i++){
		printf(" %d ", message[i]);		
	}
	printf("\n");
		
}

/***********************************************************************************
*  copy_vector																	   *
* Copy a vector without considering the null termination						   *
* 																				   *
* Arguments: char *message, int length 											   *
* Return: void																	   *
*		  																		   *
************************************************************************************/
void copy_vector (char* destination, char* source, int length){
	int i;
	for (i=0; i<length; i++){
		destination[i] = source[i];		
	}	
}

/***********************************************************************************
*  open_private_key																   *
* Open private key file, read and store in variable 							   *
* 																				   *
* Arguments: cons_char *file_name 												   *
* Return: 1 sucessfuly read key													   *
*		  0 error opening the file												   *
************************************************************************************/
unsigned char open_private_key(const char *file_name){	
	pri_key = malloc(2*KEY_LENGTH);
	FILE *privateKey_File = fopen(file_name, "r");
	if(privateKey_File){
		fread(pri_key, sizeof(*pri_key), KEY_LENGTH, privateKey_File);
		fclose(privateKey_File);
		
		BIO *pri = BIO_new(BIO_s_mem());
		pri_len = strlen(pri_key);
		BIO_write(pri, pri_key, pri_len);
		
		
		PEM_read_bio_RSAPrivateKey(pri, &keypairPriv, NULL, NULL);
		
		return 1;
	}
	else{
		printf("Failed to open private key file\n");
		return 0;
	}
	printf("Failed to open private key file\n");
	return 0;
}

/***********************************************************************************
*  open_public_key																   *
* Open public key file, read and store in variable 							   *
* 																				   *
* Arguments: cons_char *file_name 												   *
* Return: 1 sucessfuly read key													   *
*		  0 error opening the file												   *
************************************************************************************/
unsigned char open_public_key(const char* file_name){
	
	pub_key = malloc(2*KEY_LENGTH);
	FILE *publicKey_File = fopen(file_name, "r");
	if(publicKey_File){
		fread(pub_key, sizeof(*pub_key), KEY_LENGTH, publicKey_File);
		fclose(publicKey_File);
		
		BIO *pub = BIO_new(BIO_s_mem());
		pub_len = strlen(pub_key);
		BIO_write(pub, pub_key, pub_len);
		
		
		PEM_read_bio_RSAPublicKey(pub, &keypairPub, NULL, NULL);
		
		return 1;
	}
	else{
		printf("Failed to open public key file\n");
		return 0;
	}
	printf("Failed to open private key file\n");
	return 0;
}

/***********************************************************************************
*  rsa_encrypt_message_pub															   *
* Encrypt message (variable message) with the public key. Write result in the	   *
* pointer (char *encrypted_message)												   *
* Arguments: char *message, char *encrypted_message								   *
* Return: unsigned int encryption message lenght		       					   *
*		  																		   *
************************************************************************************/
unsigned int rsa_encrypt_message_pub (char *message, char *encrypted_message){
	
	if(strlen(message)>MESSAGE_MAX_LENGTH){
		printf("String too big to encrypt\n");
		return 0;
	}
    unsigned int encrypt_len;
    err = malloc(130);
    if((encrypt_len = RSA_public_encrypt(strlen(message)+1, (unsigned char*)message, (unsigned char*)encrypted_message,
                                         keypairPub, RSA_PKCS1_OAEP_PADDING)) == -1) {
        ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        fprintf(stderr, "Error encrypting message: %s\n", err);
		return 0;
    }
	else{		
		return encrypt_len;
	}	
	return 0;
}

/***********************************************************************************
*  rsa_encrypt_long_message_pub													   *
* Break the original message in parts of MESSAGE_MAX_LENGTH to be encrypted 	   *
* and call the function rsa_encrypt_message_pub() to encrypt     				   *
* Arguments: char *message, char *encrypted_message								   *
* Return: unsigned int encryption message lenght		       					   *
*		  																		   *
************************************************************************************/
unsigned int rsa_encrypt_long_message_pub (char *message, char *complete_enc_msg){
	int parts_count=0;
	int number_parts;
	int encrypt_length=0;
	char *buffer;	
	buffer = malloc(strlen(message)+1);
	number_parts = (strlen(message)/MESSAGE_MAX_LENGTH)+1;
	while(parts_count<number_parts){		
		strcpy(buffer, &message[parts_count*MESSAGE_MAX_LENGTH]);		
		buffer[(parts_count+1)*MESSAGE_MAX_LENGTH] = 0;
		encrypt_length += rsa_encrypt_message_pub(buffer, &complete_enc_msg[parts_count*ENCRYPTED_PART_LENGTH]);			
		parts_count++;
	}
	return encrypt_length;
}

/***********************************************************************************
*  rsa_decrypt_message_pub														   *
* Decrypt message (Argument: message) with the private key. Write result in the	   *
* pointer (char *decrypted_message)												   *
*																				   *
* Arguments: char *message, char *decrypted_message, int length					   *
* Return: 1 sucessfuly decrypted												   *
*		  0 error decrypting													   *
************************************************************************************/
unsigned char rsa_decrypt_message_priv (char *message, char *decrypted_message, int length){
	//char   *decrypt = NULL;    // Decrypted message
	
	if(strlen(message)>ENCRYPTED_PART_LENGTH){
		printf("String too big to decrypt\n");
		return 0;
	}
    err = malloc(130);
    //decrypt = malloc(length);
    if(RSA_private_decrypt(length, (unsigned char*)message, (unsigned char*)decrypted_message,
                           keypairPriv, RSA_PKCS1_OAEP_PADDING) == -1) {
        ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        fprintf(stderr, "Error decrypting message: %s\n", err);
		return 0;
    }
	else{		
		return 1;
	}	
	return 0;
}

/***********************************************************************************
*  rsa_decrypt_long_message_pub													   *
* Break the encrypted message in parts of ENCRYPTED_PART_LENGTH to be decrypted    *
* and call the function rsa_decrypt_message_pub() to decrypt     				   *
*																				   *
* Arguments: char *message, char *encrypted_message								   *
* Return: 1 sucessfuly decrypted												   *
*		  0 error decrypting													   *
************************************************************************************/
unsigned char rsa_decrypt_long_message_priv (char *message, char *complete_dec_msg, int length){
	int parts_count=0;
	int number_parts;
	int encrypt_length=0;
	int part_length;
	char *buffer;	
	buffer = malloc(length+1);
	number_parts = length/ENCRYPTED_PART_LENGTH;
	if(length%ENCRYPTED_PART_LENGTH)number_parts++;	
	while(parts_count<number_parts){				
		copy_vector(buffer, &message[parts_count*ENCRYPTED_PART_LENGTH], ENCRYPTED_PART_LENGTH);			
		buffer[(parts_count+1)*ENCRYPTED_PART_LENGTH] = 0;
		
		if(parts_count<(number_parts-1)){
			part_length = ENCRYPTED_PART_LENGTH;			
		}
		else{ //last part
			part_length = length-((parts_count)*ENCRYPTED_PART_LENGTH);			
		}				
		if(!rsa_decrypt_message_priv(buffer, &complete_dec_msg[parts_count*MESSAGE_MAX_LENGTH], part_length)){
			return 0;
		}		
		parts_count++;
	}
	return 1;
}

