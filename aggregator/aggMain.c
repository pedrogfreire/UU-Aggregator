#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include "Base64Encode.c"
#include "Base64Decode.c"
#include "rsa_lib.c"
#include <math.h>
#include <curl/curl.h>
#include "aes.c"
#include "bluetooth_IHAP.c"
#include "time_delay.c"

#include <json-c/json.h>

#define PRINT_AGG_INFO 			
#define PRINT_ENCRYPT_RSA 	
#define PRINT_ENCRYPT_AES 	
#define PRINT_DECRYPT_AES 		
#define PRINT_ENCODED_BASE64
#define PRINT_DECODED_BASE64
#define PRINT_DECRYPTED_MESSAGE_RSA

#define MSG_BLUETOOTH_MAX_SIZE 1000



int main(){
	
	//Open public key file
	if(!(open_public_key((const char *) ("publicKey.bin")))){
		printf("Error opening public key\n");
	}
	
	//Open JSON file with Configuration info
	char fileConfigBuffer[1024];
	FILE *fileConfig = fopen("config.json", "r");
	fread(fileConfigBuffer, 1024, 1, fileConfig);
	fclose(fileConfig);
	
	struct json_object *configJsonObj;
	struct json_object *destinationConfigJson;
	char *transmissionDestination = NULL;	
	configJsonObj = json_tokener_parse(fileConfigBuffer);
	json_object_object_get_ex(configJsonObj, "destination", &destinationConfigJson); 
	transmissionDestination = malloc(json_object_get_string_len(destinationConfigJson) * sizeof(char));
	transmissionDestination = (char *) json_object_get_string(destinationConfigJson);

	
	//Open JSON file with Aggregator Info
	char fileAggregatorInfoBuffer[1024];
	FILE *fileAggregatorInfo = fopen("aggregatorInfo.json", "r");
	fread(fileAggregatorInfoBuffer, 1024, 1, fileAggregatorInfo);
	fclose(fileAggregatorInfo);	
	#ifdef PRINT_AGG_INFO
		printf("Aggregator Info: %s\n", fileAggregatorInfoBuffer);
	#endif
	
	//Open JSON file with Header
	char fileHeaderBuffer[1024];
	FILE *fileHeader = fopen("header.json", "r");
	fread(fileHeaderBuffer, 1024, 1, fileHeader);
	fclose(fileHeader);	
	
	struct json_object *parsed_json_header;	
	parsed_json_header = json_tokener_parse(fileHeaderBuffer);
	
	
	
	
	//Encrypt AES UserKey and IVEC with RSA
	
	char   *rsa_encrypted_UserKey = NULL;    // UserKey
	unsigned int rsa_encrypted_UserKey_len;		
	rsa_encrypted_UserKey = malloc(((strlen(userkey)/MESSAGE_MAX_LENGTH)+1)*ENCRYPTED_PART_LENGTH);	
	if(!(rsa_encrypted_UserKey_len = rsa_encrypt_long_message_pub(userkey, rsa_encrypted_UserKey))){
		printf("RSA UserKey Encryption Error");
	}

	char   *rsa_encrypted_IVEC = NULL;    // IVEC
	unsigned int rsa_encrypted_IVEC_len;		
	rsa_encrypted_IVEC = malloc(((strlen(IV)/MESSAGE_MAX_LENGTH)+1)*ENCRYPTED_PART_LENGTH);	
	if(!(rsa_encrypted_IVEC_len = rsa_encrypt_long_message_pub(IV, rsa_encrypted_IVEC))){
		printf("RSA IVEC Encryption Error");
	}
	
	
	//Encode encrypted UserKey and IVEC in base64
	
	char* base64EncodeUserKeyOutput = NULL; //UserKey
	int base64_UserKey_len;
	if(!(base64_UserKey_len = Base64Encode((unsigned char*)rsa_encrypted_UserKey, &base64EncodeUserKeyOutput, rsa_encrypted_UserKey_len))){
		printf("Base64 UserKeyEncoding Error");
	}		
	
	char* base64EncodeIVECOutput = NULL; //IVEC
	int base64_IVEC_len;
	if(!(base64_IVEC_len = Base64Encode((unsigned char*)rsa_encrypted_IVEC, &base64EncodeIVECOutput, rsa_encrypted_IVEC_len))){
		printf("Base64 IVEC Encoding Error");
	}		
	
	json_object_object_add(parsed_json_header, "UserKey", json_object_new_string(base64EncodeUserKeyOutput));
	json_object_object_add(parsed_json_header, "Ivec", json_object_new_string(base64EncodeIVECOutput));
	
	
	//Encrypt aggregator info in AES
	char   *AES_encrypted_message = NULL;    // Encrypted message
	unsigned int aes_encrypted_message_len;		
	AES_encrypted_message = malloc(((strlen(fileAggregatorInfoBuffer)/MESSAGE_MAX_LENGTH)+1)*ENCRYPTED_PART_LENGTH);	
	aes_encrypted_message_len = aes_encrypt_message(fileAggregatorInfoBuffer, AES_encrypted_message);
	#ifdef PRINT_ENCRYPT_AES
		print_encrypt_message(AES_encrypted_message, aes_encrypted_message_len);
		printf("AES Encrypted result: %s\n", AES_encrypted_message);
	#endif
	
	
	//Decrypt aggregator info in AES
	char   *AES_decrypted_message = NULL;    // Encrypted message
	unsigned int aes_decrypted_message_len;		
	AES_decrypted_message = malloc(((strlen(fileAggregatorInfoBuffer)/MESSAGE_MAX_LENGTH)+1)*ENCRYPTED_PART_LENGTH);	
	aes_decrypt_message(AES_encrypted_message, AES_decrypted_message, aes_encrypted_message_len);
	#ifdef PRINT_DECRYPT_AES
		printf("AES decrypted result: %s\n", AES_decrypted_message);
	#endif
	
	//Encode message in base64
	char* base64EncodeOutput = NULL;
	int base64_encrypt_len;
	if(!(base64_encrypt_len = Base64Encode((unsigned char*)AES_encrypted_message, &base64EncodeOutput, aes_encrypted_message_len))){
		printf("Base64 Encoding Error");
	}		
	#ifdef PRINT_ENCODED_BASE64
		printf("Base64 encoded result %s\n\n\n", base64EncodeOutput);
	#endif
	
	json_object_object_add(parsed_json_header, "Info", json_object_new_string(base64EncodeOutput));
			
	
	char comand_sys[2000];
	sprintf(comand_sys, "%s", json_object_to_json_string_ext(parsed_json_header, JSON_C_TO_STRING_SPACED));
	//comand_sys = (char *) json_object_to_json_string_ext(parsed_json_header, JSON_C_TO_STRING_SPACED);
	printf("Result: %s\n", comand_sys);
	//sprintf(comand_sys, "'{%cencrypted%c:'%s'}'",  '"', '"', base64EncodeOutput);
	
	if(transmissionDestination[0]=='I'){
		printf("Sending to IHAP\n");
		search_bt_uuid_service();
		int msg_len = strlen(comand_sys);
		printf("Text Lenght %d\n", msg_len);
		int num_parts = ceil((double) msg_len/MSG_BLUETOOTH_MAX_SIZE);
		char msg_with_header[MSG_BLUETOOTH_MAX_SIZE];
		char temp_buffer[MSG_BLUETOOTH_MAX_SIZE+1];
		for(int i=0; i<num_parts; i++){
			msg_with_header[0] = 'P';
			msg_with_header[1] = '1' + i;
			msg_with_header[2] = 'O';
			msg_with_header[3] = '0' + num_parts;
			msg_with_header[4] = 0;
			strncpy(temp_buffer, &comand_sys[MSG_BLUETOOTH_MAX_SIZE*i], MSG_BLUETOOTH_MAX_SIZE);
			temp_buffer[MSG_BLUETOOTH_MAX_SIZE] = 0;
			printf("Part %d, message: %s\n\n", i, temp_buffer);
			strcat(msg_with_header, temp_buffer);			
			send_message_bluetooth(msg_with_header, strlen(msg_with_header));
			delay(100);
		}				
	}
	else{
		printf("Sending to Alleato\n");
		CURL *curl;
		CURLcode res;
		
		curl_global_init(CURL_GLOBAL_ALL);
		
		curl = curl_easy_init();
		
		if(curl){
			curl_easy_setopt(curl, CURLOPT_URL, "http://tk.omsorgsportal.se/Receiver/");
			
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, comand_sys);
			
			res = curl_easy_perform(curl);
			
			if(res != CURLE_OK){
				fprintf(stderr, "curl_easy_perform() returned %s\n", curl_easy_strerror(res));			
			}
			curl_easy_cleanup(curl);		
		}
		curl_global_cleanup();
	}
}