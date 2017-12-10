#ifndef ContentDefinedChunk_h
#define ContentDefinedChunk_h

//void ContentDefinedChunk(const unsigned char * Input, int * ChunkLength, int *ChunkNumber,
//			 int in_len);
//void ContentDefinedChunk(const unsigned char *Input, int PreviousLength,int *ChunkLength,int k,
//                         int *ChunkNumber, int in_len,*last);

#define MAX_CHUNK_SIZE 8192 // Bytes

int ContentDefinedChunk(const unsigned char Input[MAX_CHUNK_SIZE],int* chunk_acc, unsigned char Output[MAX_CHUNK_SIZE], char *last);

#endif /* ContentDefinedChunk_h */
