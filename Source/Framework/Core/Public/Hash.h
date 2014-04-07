#ifndef __Hash_h_
#define __Hash_h_

uint32 djb2Hash(const String& name);
uint32 crc32Hash(const ubyte* data, uint32 len);

#endif // __Hash_h_