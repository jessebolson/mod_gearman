#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <crypt.h>
#include "common.h"

int encryption_initialized = 0;
unsigned char key[KEYLENGTH(KEYBITS)];


/* initialize encryption */
void mod_gm_blowfish_init(char * password) {

    /* pad key till keysize */
    int i;
    for (i = 0; i < 32; i++)
        key[i] = *password != 0 ? *password++ : 0;

    encryption_initialized = 1;
}


/* encrypt text with given key */
int mod_gm_blowfish_encrypt(unsigned char ** encrypted, char * text) {

    assert(encryption_initialized == 1);

    unsigned long rk[RKLENGTH(KEYBITS)];
    int nrounds;
    nrounds = rijndaelSetupEncrypt(rk, key, KEYBITS);
    int size = strlen(text);
    int totalsize = size + BLOCKSIZE-size%BLOCKSIZE;
    int i = 0;
    int k = 0;
    unsigned char *enc;
    enc = (unsigned char *) malloc(sizeof(unsigned char)*totalsize);
    while(size > 0) {
        unsigned char plaintext[BLOCKSIZE];
        unsigned char ciphertext[BLOCKSIZE];
        int j;
        for (j = 0; j < BLOCKSIZE; j++) {
            int c = text[i];
            if(c == 0)
                break;
            plaintext[j] = c;
            i++;
        }

        for (; j < BLOCKSIZE; j++)
            plaintext[j] = ' ';
        rijndaelEncrypt(rk, nrounds, plaintext, ciphertext);
        for (j = 0; j < BLOCKSIZE; j++)
            enc[k++] = ciphertext[j];
        size -=BLOCKSIZE;
    }

    *encrypted = enc;
    return totalsize;
}


/* decrypt text with given key */
void mod_gm_blowfish_decrypt(char ** text, unsigned char * encrypted, int size) {

    assert(encryption_initialized == 1);

    char decr[GM_BUFFERSIZE];
    decr[0] = '\0';
    unsigned long rk[RKLENGTH(KEYBITS)];
    int nrounds;
    nrounds = rijndaelSetupDecrypt(rk, key, KEYBITS);
    int i = 0;
    while(1) {
        unsigned char plaintext[BLOCKSIZE];
        unsigned char ciphertext[BLOCKSIZE];
        int j;
        for (j = 0; j < BLOCKSIZE; j++) {
            int c = encrypted[i];
            ciphertext[j] = c;
            i++;
        }
        rijndaelDecrypt(rk, nrounds, ciphertext, plaintext);
        strncat(decr, (char*)plaintext, BLOCKSIZE);
        size -= BLOCKSIZE;
        if(size < BLOCKSIZE)
            break;
    }

    strcpy(*text, decr);
    return;
}