#ifndef CHECKSUM_H
#define CHECKSUM_H

#include "database.h"

// compute crc32 checksum of entire database
unsigned long compute_database_checksum(const StudentDatabase *db);

// compute crc32 checksum of file on disk
unsigned long compute_file_checksum(const char *filepath);

// compute crc32 checksum of a single record
unsigned long compute_record_checksum(const StudentRecord *record);

#endif // CHECKSUM_H
