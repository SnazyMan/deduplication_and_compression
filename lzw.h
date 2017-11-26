/* 
 * Dictionary data structure referenced from 
 *  https://github.com/radekstepan/LZW/blob/master/dictionary.c
 * It will need to change but will serve as a placeholder, 
 * it is an unoptimal searching approach, but initial functionality
 * is more important at this point instead of speed.
 * speed comes later once all coarse grained tasks operate together
 */

#define MIN_CODE_LEN    9                   // min # bits in a code word
#define MAX_CODE_LEN    20                  // max # bits in a code word
#define MAX_DICT_SIZE   4095
#define CHAR_BITS       8

#define FIRST_CODE      0                   // value of 1st string code 
#define MAX_CODES       (1 << MAX_CODE_LEN)

#define CURRENT_MAX_CODES(bits)     ((unsigned int)(1 << (bits)))

// node in dictionary list 
struct dict_node {
    unsigned int value; //position in the list
	unsigned char suffix_char;   // last char in encoded string
	unsigned int prefix_code;    // code for remaining chars in string
};

// write encoded data
int write_code_word(unsigned char *out_chunk, int code, const unsigned char code_len, int last, int *compressed_length);

// LZW encoder
int lzw(unsigned char *in_chunk, unsigned char *out_chunk, int chunk_length,int * compressed_chunk_lenghth);

// Dictionary initialization
int dict_init();

// Check if current string is in dictionary
int dictionary_lookup(int prefix, unsigned char character);

//add to the dictionary
void dictionary_add(int prefix, unsigned char character, int value);

