#include <stdio.h>
#include <stdlib.h>
#include "lzw.h"
#include <math.h>

// declare the dictionary
struct dict_node dic[MAX_DICT_SIZE];
int empty = -1;
int gDictInit = 0;

/*
 * LZW encoding, takes a pointer to input chunk location and outputs
 * a pointer to the encoded chunk
 */
#pragma SDS data mem_attribute(in_chunk:PHYSICAL_CONTIGUOUS, out_chunk:PHYSICAL_CONTIGUOUS)
void lzw(unsigned char in_chunk[MAX_DICT_SIZE], unsigned char out_chunk[MAX_DICT_SIZE], int chunk_length, int *compressed_chunk_lenghth)
{
    int index = 0; //index incoming chunk
    int i = -1; // index dict_root list
    int prefix; // code for current string
    unsigned char current_code_len; // length of the current code
    unsigned int next_code; // next available code index
    unsigned char c; // character to add to string
    int last = 0; //false
    
    // Initialize dictionary
    dict_init();
    
    // start MIN_CODE_LEN bit code words
    // Which is 9 initially because the dict_root has 256 entries
    // but the first character is guarenteed to be in dict_root so we will get
    // 257 entries log2(257) = 8.0005 which means we need 9 bits of codeword
    current_code_len = MIN_CODE_LEN;
    
    next_code = 256;  // code for next (first) string
    
    // read the first char, guarenteed to be in dict_root
    prefix = in_chunk[index++];

    // now encode normally
    while (index != chunk_length) {
        // read the next byte from the chunk
        c = in_chunk[index++];

        //printf("lzw index = %d\n", index);

        // look for code + c in the dict_root
        i = dictionary_lookup(prefix, c);
        if (i != -1) {
            prefix = i;
        }
        else {
            // add to dict_root
            //if (next_code < MAX_DICT_SIZE) {
            if(empty !=-1){
            	dictionary_add(prefix, c, next_code);
            }
            next_code++; // This is incorrect, 
            
            // Check to see if dictionary size can be represented in code length
            if ((next_code - 1) >= CURRENT_MAX_CODES(current_code_len)) {
                current_code_len++;
            }
	    
            //encode to output stream
            write_code_word(out_chunk, prefix, current_code_len, last, compressed_chunk_lenghth);

            prefix = c;
        }
    }

    // no more input.  write out last of the code
    write_code_word(out_chunk, prefix, current_code_len, last = 1, compressed_chunk_lenghth);
}

/*
 * Initialize dict_root to contain 256 entries to the first 256 literals
 * A byte with value 27 would be encoded to 27
 */
void dict_init()
{
    //initialize the first 256 entries.
    // This needs to only happen once, hence the global variable
    if (!gDictInit) {	
        for (int i = 0; i < 256; i++) {
	    dic[i].value = i;
            dic[i].prefix_code = -1;
            dic[i].suffix_char = (unsigned char)i;
	    dic[i].valid = 1;
	}
	gDictInit = 1;
    }
    //initialize the other entries
    for (int j = 256; j < MAX_DICT_SIZE; j++) {
        dic[j].value = -3;
        dic[j].prefix_code = -3;
        dic[j].suffix_char = (unsigned char)(-3);
        dic[j].valid = 0;
    }
}

int dictionary_lookup(int prefix, unsigned char character)
{
    int ret = -1;
    long temp = (prefix*31 + (long)character + 256);

    //4093 is the biggest prime less than 4096,hash calculation
    int address = (temp*11 & MAX_DICT_SIZE)^PRIME;

    // If the address is invalid no entry, return immediately 
    if (dic[address].valid == 0) {
    	empty = address;
        return -1;
   }
    
    //find this pair in dictionary at the exact address
    if ((dic[address].prefix_code == prefix) && (dic[address].suffix_char == character)) {    
        ret = dic[address].value;
        return ret;
    }
    else { //don't find & the address isn't empty (rehash)
        empty =-1;
        ret=-1;
        /*
        int k = address;
        int rehash = 0;
        while (k < (MAX_DICT_SIZE + address)) { // linear probe
            k++;
            rehash = k % MAX_DICT_SIZE;
            if (dic[rehash].valid == 0) {
            	empty = rehash;
            	return -1;
            }
            if ((dic[rehash].prefix_code == prefix) && (dic[rehash].suffix_char == character)) {
                ret = dic[rehash].value;
                break;
            }
            else {
            	// This case would be for if the dictionary is full
                ret = -1;
            }
        }
         */
    }

    return ret;
}
void dictionary_add(int prefix, unsigned char character, int value)
{
    dic[empty].value = value;
    dic[empty].prefix_code = prefix;
    dic[empty].suffix_char = character;
    dic[empty].valid = 1;
    empty = -1;
}


// I need to break this function into smaller functions

/*
 * This function writes a code word to the output stream
 * Code words are output big endian. The beginning of the stream will be:
 * If a 9b code has the binary value x8,x7,x6,x5,x4,x3,x2,x1,x0, tream will result
 * in two consecutive bytes with values x8,x7,x6,x5,x4,x3,x2,x1 and x0,0,0,0,0,0,0,0
 * The next code word will be packed into the stream changing the second byte to:
 * x8,x7,x6,x5,x4,x3,x2,x1 and x0,y8,y7,y6,y5,y4,y3,y2 and so it on
 */
void write_code_word(unsigned char out_stream[MAX_DICT_SIZE], int code, const unsigned char code_len, int last, int *compressed_length)
{
    // We start at 4 to give room for 32b header for each chunk
    static int out_index = 4;
    static unsigned char leftover = 0;
    static int n_leftover = 0;
    unsigned char tmp = 0;
    int header;
    
    /*
     * If there is no leftover bits, write 8 or 16 most significant bits to stream
     * save the leftover bits in leftover to go out on next iteration
     */
    if (n_leftover == 0) {
        n_leftover = code_len % CHAR_BITS;
        leftover = code << (CHAR_BITS - n_leftover);
        out_stream[out_index++] = (code >> n_leftover) & 0x000000FF;
    }
    else {
        // Move leftover bits to most significant pos in tmp
        // place code_len - n_leftover bits into stream with
        // leftover bits occupying MSB following MSB of code
        tmp = code >> (code_len - (CHAR_BITS - n_leftover));
        tmp |= leftover;
        out_stream[out_index++] = tmp;
        n_leftover = (code_len - (CHAR_BITS - n_leftover));
        if (n_leftover == 8) {
            leftover = code << (CHAR_BITS - n_leftover);
            out_stream[out_index++] = leftover;
            n_leftover = 0;
            leftover = 0;
        }
        else if (n_leftover > 8) {
            out_stream[out_index++] = code >> (n_leftover - CHAR_BITS);
            n_leftover = (n_leftover - CHAR_BITS);
            leftover = code << (CHAR_BITS - n_leftover);
        }
        else {
            leftover = code << (CHAR_BITS - n_leftover);
        }
    }
    
    // write out any leftover bits, pad chunk, add header
    if (last) {
        if (n_leftover) {
            // Padding is aready taken care of I think
            out_stream[out_index++] = leftover;
        }
        
        // Save compressed length for other functions in higher stack frames
        *compressed_length = out_index;
        
        // move from bit position 0-32 to 1-32
        header = (out_index - 4) << 1;
        // zero out bottom bit
        header &= 0xFFFFFFFE;
        out_stream[0] = (unsigned char) (header & 0x000000FF);
        out_stream[1] = (unsigned char)((header & 0x0000FF00) >> 8);
        out_stream[2] = (unsigned char)((header & 0x00FF0000) >> 16);
        out_stream[3] = (unsigned char)((header & 0xFF000000) >> 24);
        
        // Reset static variables for new chunk
        out_index = 4;
        leftover = 0;
        n_leftover = 0;
    }
}
