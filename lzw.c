#include <stdio.h>
#include <stdlib.h>
#include "lzw.h"

// root and tail of dictionary list
struct dict_node *dict_root;
struct dict_node *tail; 

/*
 * LZW encoding, takes a pointer to input chunk location and outputs 
 * a pointer to the encoded chunk
 */
int lzw(unsigned char *in_chunk, unsigned char *out_chunk, int chunk_length)
{
	int ret;
	int index = 0; //index incoming chunk
	int i = -1; // index dict_root list
	int prefix;
	unsigned int code; // code for current string 
	unsigned char current_code_len; // length of the current code 
	unsigned int next_code; // next available code index 
	unsigned char c; // character to add to string 

	// validate arguments 
	if ((in_chunk == 0) || (out_chunk == 0) || (chunk_length == 0)) {
		printf("Invalid argument to lzw\n");
		return -1;
	}
	
	// Initialize dict_root
	ret = dict_init(dict_root);
	if (ret != 0) {
		printf("Allocation error dict_root roots\n");
		return -1;
	}

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
		
		// look for code + c in the dict_root
		//node = find_dict_root_entry(dict_root, code, c);
		i = dictionary_lookup(prefix, c);
		if (i != -1) 
			prefix = i;
		else {
			// add to dict_root
			dictionary_add(prefix, c, next_code++);


			if (prefix >= CURRENT_MAX_CODES(current_code_len)) {
				currentCodeLen++;
			}

			//encode to output stream
			write_code_word(out_chunk, prefix, current_code_len);

			prefix = c;
		}
	}
	
	// no more input.  write out last of the code
	write_code_word(out_chunk, code, current_code_len);
	
	// free the dict_root
	//free_tree(dict_root);
	
	return 0;
}

/* 
 * Initialize dict_root to contain 256 entries to the first 256 literals
 * A byte with value 27 would be encoded to 27
 */ 
int dict_init()
{
	struct dict_node *node;
	
	for (int i = 0; i < 256; i++) {
		node = malloc(sizeof(struct dict_node));
		node->prefix_code = -1;
		node->suffix_char = (unsigned char)i;
		append_node(node);
	}
	
	return 0;
}

void append_node(struct dict_node *node)
{
	if (dict_root != 0)
		tail->next = node;
	else
		dict_root = node;
	
	tail = node;
	node->next = 0;
}

int dictionary_lookup(int prefix, unsigned char character)
{
	struct dict_node *node;
	for (node = dict_root; node != NULL; node = node->next) { // ...traverse forward
		if (node->prefix_code == prefix && node->suffix_char == character)
			return node->value;
	}
	return -1;
}

// add prefix + character to the dict_root
void dictionary_add(int prefix, unsigned char character, int value)
{
	struct dict_node *node;
	node = malloc(sizeof(struct dict_node));
	node->value = value;
	node->prefix_code = prefix;
	node->suffix_char = character;
	//printf("\n(%i) = (%i) + (%i)\n", node->value, node->prefix, node->character);
	append_node(node);
}

/*
 * This function writes a code word to the output stream
 * Code words are output big endian. The beginning of the stream will be:
 * If a 9b code has the binary value x8,x7,x6,x5,x4,x3,x2,x1,x0, tream will result 
 * in two consecutive bytes with values x8,x7,x6,x5,x4,x3,x2,x1 and x0,0,0,0,0,0,0,0
 * The next code word will be packed into the stream changing the second byte to:
 * x8,x7,x6,x5,x4,x3,x2,x1 and x0,y8,y7,y6,y5,y4,y3,y2 and so it on
 */
int write_code_word(unsigned char *out_stream, int code, const unsigned char code_len)
{
	static int out_index = 0;
	out_stream[out_index++] = code;
	
	return 0;
}
