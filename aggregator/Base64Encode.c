//Encodes Base64
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

int Base64Encode(const char* message, char** buffer, int message_len) { //Encodes a string to base64
  BIO *bio, *b64;
  FILE* stream;
  int encodedSize = 4*ceil((double)message_len/3);
  *buffer = (char *)malloc(encodedSize+1);

  stream = fmemopen(*buffer, encodedSize+1, "w");
  b64 = BIO_new(BIO_f_base64());
  bio = BIO_new_fp(stream, BIO_NOCLOSE);
  bio = BIO_push(b64, bio);
  BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Ignore newlines - write everything in one line
  int size_base64 = BIO_write(bio, message, message_len);
  BIO_flush(bio);
  BIO_free_all(bio);
  fclose(stream);

  return (size_base64); //success
}