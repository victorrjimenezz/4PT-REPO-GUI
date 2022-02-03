//
// Created by Víctor Jiménez Rugama on 1/30/22.
//

//BINARY OPERATIONS
#define LE16(x) (x)
#define LE32(x) (x)

#define SWAP32(x) \
	((uint32_t)( \
		(((uint32_t)(x) & UINT32_C(0x000000FF)) << 24) | \
		(((uint32_t)(x) & UINT32_C(0x0000FF00)) <<  8) | \
		(((uint32_t)(x) & UINT32_C(0x00FF0000)) >>  8) | \
		(((uint32_t)(x) & UINT32_C(0xFF000000)) >> 24) \
	))
#define SWAP64(x) \
	((uint64_t)( \
		(uint64_t)(((uint64_t)(x) & UINT64_C(0x00000000000000FF)) << 56) | \
		(uint64_t)(((uint64_t)(x) & UINT64_C(0x000000000000FF00)) << 40) | \
		(uint64_t)(((uint64_t)(x) & UINT64_C(0x0000000000FF0000)) << 24) | \
		(uint64_t)(((uint64_t)(x) & UINT64_C(0x00000000FF000000)) <<  8) | \
		(uint64_t)(((uint64_t)(x) & UINT64_C(0x000000FF00000000)) >>  8) | \
		(uint64_t)(((uint64_t)(x) & UINT64_C(0x0000FF0000000000)) >> 24) | \
		(uint64_t)(((uint64_t)(x) & UINT64_C(0x00FF000000000000)) >> 40) | \
		(uint64_t)(((uint64_t)(x) & UINT64_C(0xFF00000000000000)) >> 56) \
	))

#define BE32(x) SWAP32(x)
#define BE64(x) SWAP64(x)

//PACKAGE FILE INFO
#define PKG_INITIAL_BUFFER_SIZE 16000
#define PKG_SIZE_OFFSET 0x430
#define PKG_CONTENT_ID_SIZE 0x24
#define SIZEOF_PKG_TABLE_ENTRY 0x20
#define PKG_ENTRY_COUNT 0x10
#define PKG_ENTRY_TABLE_OFFSET 0x18
#define PKG_ENTRY_OFFSET 0x10
#define PKG_ENTRY_SIZE 0x14

//APP TYPES
#define APP_TYPES 7

//SFO FILE
#define SFO_ENTRY_COUNT 0x10
#define SFO_KEY_TABLE_OFFSET 0x08
#define SFO_VALUE_TABLE_OFFSET 0x0C

#define SFO_ENTRY_FORMAT 0x02
#define SFO_ENTRY_VALUE_SIZE 0x04
#define SFO_ENTRY_MAX_SIZE 0x08
#define SFO_ENTRY_VALUE_OFFSET 0x0C

#define SIZEOF_SFO_TABLE_ENTRY 0x10
#define SIZEOF_SFO_HEADER 0x14

#define PKGFolder "pkg"
#define repoIconName "icon.png"

