#pragma once

class FileClass;
class StringClass;



unsigned long SCRIPTS_API CRC_Memory(const unsigned char* data, unsigned long length, unsigned long crc = 0);
unsigned long SCRIPTS_API CRC_String(const char *data,unsigned long crc);
unsigned long SCRIPTS_API CRC_Stringi(char  const*, unsigned long = 0);
unsigned long SCRIPTS_API CRC_File(FileClass *f, uint32 seed = 0);
