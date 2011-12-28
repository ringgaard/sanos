/****************************  library.h   ********************************
* Author:        Agner Fog
* Date created:  2006-07-15
* Last modified: 2007-02-14
* Project:       objconv
* Module:        library.h
* Description:
* Header file defining classes for reading and writing UNIX and OMF style 
* libraries.
*
* Copyright 2007-2008 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/

#ifndef LIBRARY_H
#define LIBRARY_H


// Make big-endian numbers for library
uint32 EndianChange(uint32);           // Convert little-endian to big-endian number, or vice versa


// Define UNIX library member header
struct SUNIXLibraryHeader {
   char Name[16];                      // Member name
   char Date[12];                      // Member date, seconds, decimal ASCII
   char UserID[6];                     // Member User ID, decimal ASCII
   char GroupID[6];                    // Member Group ID, decimal ASCII
   char FileMode[8];                   // Member file mode, octal
   char FileSize[10];                  // Member file size, decimal ASCII
   char HeaderEnd[2];                  // "`\n"
};


// Class for extracting members from library or building a library
class CLibrary : public CFileBuffer {
public:
   CLibrary();                         // Constructor
   void Go();                          // Do whatever the command line says
   void Dump();                        // Print contents of library
   static char *TruncateMemberName(char const*); // Remove path and truncate object file name to 15 characters
   const char * GetModuleName(uint32 Index); // Get name of module from index or page index
protected:
   // Properties for UNIX input libraries only
   uint32 LongNames;                   // Offset to long names member
   uint32 LongNamesSize;               // Size of long names member
   uint32 AlignBy;                     // Member alignment

   // Properties for OMF input libraries only
   uint32 PageSize;                    // Alignment of members
   uint32 DictionaryOffset;            // Offset to hash table
   uint32 DictionarySize;              // Dictionary size, in 512 bytes blocks

   // Methods and properties for reading library:
   void DumpUNIX();                    // Print contents of UNIX style library
   void DumpOMF();                     // Print contents of OMF style library
   void CheckOMFHash(CMemoryBuffer &stringbuf, CSList<SStringEntry> &index);// Check if OMF library hash table has correct entries for all symbol names
   void StartExtracting();             // Initialize before ExtractMember()
   const char * ExtractMember(CFileBuffer*); // Extract next library member from input library
   const char * ExtractMemberUNIX(CFileBuffer*); // Extract member of UNIX style library
   const char * ExtractMemberOMF(CFileBuffer*);  // Extract member of OMF style library
   uint32 NextHeader(uint32 Offset);   // Loop through library headers
   CConverter MemberBuffer;            // Buffer containing single library member
   uint32 CurrentOffset;               // Offset to current member
   uint32 CurrentNumber;               // Number of current member
   int MemberFileType;                 // File type of members
   // Methods and properties for modifying or writing library
   void Rebuild();                     // Make member names unique and without path. Rebuild symbol table
   void RebuildUNIX();                 // Rebuild UNIX style library
   void RebuildOMF();                  // Rebuild OMF style library
   void InsertMember(CFileBuffer*);    // Add next library member to output library
   void InsertMemberUNIX(CFileBuffer*);// Add member to UNIX library
   void InsertMemberOMF(CFileBuffer*); // Add member to OMF library
   void MakeBinaryFile();              // Combine string index and members into binary file
   void MakeBinaryFileUNIX();          // Make UNIX library
   void MakeBinaryFileOMF();           // Make OMF library
   char * FixMemberNameUNIX(char const *name); // Truncate library member name to 15 characters and make unique
   char * FixMemberNameOMF(char const *name);  // Truncate library member name to 15 characters and make unique
   void SortStringTable();             // Sort the string table
   void MakeSymbolTableUnix();         // Make symbol table for COFF, ELF or MACHO library
   CFileBuffer OutFile;                // Buffer for building output file
   CSList<SStringEntry> StringEntries; // String table using SStringEntry
   CMemoryBuffer StringBuffer;         // Buffer containing strings
   CMemoryBuffer DataBuffer;           // Buffer containing raw members
   CSList<uint32> Indexes;             // Buffer containing indexes into DataBuffer
   int MemberNameExistsUNIX(char * name);// Check if DataBuffer contains a member with this name
   int RepressWarnings;                // Repress warnings when rebuilding library
};


// Definitions for OMF library hash table:

#define OMFNumBuckets   37             // Number of buckets per block

#define OMFBlockSize    512            // Size of each block

// Structure of hash table block
union SOMFHashBlock {
   struct {
      uint8 Buckets[OMFNumBuckets];    // Indicators for each bucket
      uint8 FreeSpace;                 // Pointer to free space
      uint8 Data[OMFBlockSize-OMFNumBuckets-1]; // Contains strings and module indices
   } b;
   uint8 Strings[OMFBlockSize];        // Start of each string = length
};


// Hash table handler
class COMFHashTable {
public:
   void Init(SOMFHashBlock * blocks, uint32 NumBlocks); // Initialize
   void MakeHash(int8 * name);         // Compute hash
   int  FindString(uint32 & ModulePage, uint32 & Conflicts); // Search for string. Get number of occurrences, module, number of conflicting strings
   int  InsertString(uint16 & ModulePage); // Insert string in hash table. Return 0 if success
   void MakeHashTable(CSList<SStringEntry> & StringEntries, CMemoryBuffer & StringBuffer, CMemoryBuffer & HashTable, CLibrary * Library); // Make hash table
protected:
   uint8 * String;                     // String to search for or insert
   uint32 StringLength;                // Length of string
   SOMFHashBlock * blocks;             // Pointer to blocks
   uint32 NumBlocks;                   // Number of blocks
   uint16 StartBlock;                  // Start block for search
   uint16 StartBucket;                 // Start bucket for search
   uint16 BlockD;                      // Block step size in search
   uint16 BucketD;                     // Bucket step size in search
};

#endif // #ifndef LIBRARY_H
