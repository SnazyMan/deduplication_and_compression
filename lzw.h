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

typedef	struct entry* Entry;
typedef struct table* Table;

#define STARTING_BITS 9
#define NUM_SPECIALS 3

int encode( int maxbits, int prune, int window, int empty );
Entry* pruneArray( Entry * table, int pos, int window, int maxcodes, int *bits, int *codesOut );
void dump( Entry* table, int numcodes );
void freeCodeArray( Entry* table, int maxcodes );

// node in dictionary list 
struct dict_node {
	unsigned int value; //position in the list
	unsigned char suffix_char;   // last char in encoded string
	unsigned int prefix_code;    // code for remaining chars in string

	struct dict_node *next;
};

Entry entryMake( unsigned char lastChar, unsigned int prefix, int accessed );
Table tableMake( int maxBits );
int tableInsert ( Entry entry, Table table );	
int tableSearch( int prefix, char lastChar, Table table );
unsigned int hash( int prefix, char lastChar, Table table );
int getSize( Table table);
Table tablePrune( Table table, int window, int pos, int *numbits );
void freeTable( Table table );
void tableSetPos( int code, int pos, Table table );

struct entry 
{
	char lastChar;
	int prefix;
	int accessed;
};

struct table
{
	int maxSize;
	int maxAllowed;
	int size;
	int *lookup;
	int maxbits;
	Entry *entries;
};

// write encoded data
int write_code_word(unsigned char *out_chunk, int code, const unsigned char code_len, int last, int *compressed_length);

// LZW encoder
int lzw(unsigned char *in_chunk, unsigned char *out_chunk, int chunk_length,int * compressed_chunk_lenghth);

// Dictionary initialization
int dict_init();

// Add new dictionary node to list
void append_node(struct dict_node *node);

// Check if current string is in dictionary
int dictionary_lookup(int prefix, unsigned char character);

// add new dictionary entry
void dictionary_add(int prefix, unsigned char character, int value);

// Cleanup method. Memory leakkiinnn
void free_dict();
