#ifndef __MD5_H__
#define __MD5_H__

/*
===============================================================================

	Calculates a checksum for a block of data
	using the MD5 message-digest algorithm.

===============================================================================
*/

// Techyon RB: 64 bit fix, changed long to int
unsigned int MD5_BlockChecksum( const void* data, int length );
// Techyon END

#endif /* !__MD5_H__ */
