#include "pag0page.h"
#include <cstring>


/** The page infimum and supremum of an empty page in ROW_FORMAT=REDUNDANT */
static const char infimum_supremum_redundant[] = {
	/* the infimum record */
	0x08/*end offset*/,
	0x01/*n_owned*/,
	0x00, 0x00/*heap_no=0*/,
	0x03/*n_fields=1, 1-byte offsets*/,
	0x00, 0x74/* pointer to supremum */,
	'i', 'n', 'f', 'i', 'm', 'u', 'm', 0,
	/* the supremum record */
	0x09/*end offset*/,
	0x01/*n_owned*/,
	0x00, 0x08/*heap_no=1*/,
	0x03/*n_fields=1, 1-byte offsets*/,
	0x00, 0x00/* end of record list */,
	's', 'u', 'p', 'r', 'e', 'm', 'u', 'm', 0
};

/** The page infimum and supremum of an empty page in ROW_FORMAT=COMPACT */
static const char infimum_supremum_compact[] = {
	/* the infimum record */
	0x01/*n_owned=1*/,
	0x00, 0x02/* heap_no=0, REC_STATUS_INFIMUM */,
	0x00, 0x0d/* pointer to supremum */,
	'i', 'n', 'f', 'i', 'm', 'u', 'm', 0,
	/* the supremum record */
	0x01/*n_owned=1*/,
	0x00, 0x0b/* heap_no=1, REC_STATUS_SUPREMUM */,
	0x00, 0x00/* end of record list */,
	's', 'u', 'p', 'r', 'e', 'm', 'u', 'm'
};



/******************************************************//**
The following function is used to get the number of records owned by the
previous directory record.
@return number of owned records */
ulint rec_get_n_owned_new(
	/*================*/
	const byte*	rec)	/*!< in: new-style physical record */
{
	return(rec_get_bit_field_1(rec, REC_NEW_N_OWNED,
		REC_N_OWNED_MASK, REC_N_OWNED_SHIFT));
}


/******************************************************//**
The following function is used to get the number of records owned by the
previous directory record.
@return number of owned records */
ulint rec_get_n_owned_old(
	/*================*/
	const byte*	rec)	/*!< in: old-style physical record */
{
	return(rec_get_bit_field_1(rec, REC_OLD_N_OWNED,
		REC_N_OWNED_MASK, REC_N_OWNED_SHIFT));
}

/******************************************************//**
Gets a bit field from within 1 byte. */
ulint rec_get_bit_field_1(
	/*================*/
	const byte*	rec,	/*!< in: pointer to record origin */
	ulint		offs,	/*!< in: offset from the origin down */
	ulint		mask,	/*!< in: mask used to filter bits */
	ulint		shift)	/*!< in: shift right applied after masking */
{

	return((mach_read_from_1(rec - offs) & mask) >> shift);
}



/************************************************************//**
		TRUE if the record is on a page in compact format.
		@return nonzero if in compact format */
ulint page_is_comp(
	/*=========*/
	byte*	page)	/*!< in: index page */
{
	ulint tmp_value;
	memcpy(&tmp_value, page + PAGE_HEADER + PAGE_N_HEAP, 2);
	return(tmp_value & 0x8000);
}

/************************************************************//**
	Determine whether the page is a B-tree leaf.
	@return true if the page is a B-tree leaf (PAGE_LEVEL = 0) */
bool
page_is_leaf(
	/*=========*/
	byte*	page)	/*!< in: page */
{
	return(!*(const uint16*)(page + (PAGE_HEADER + PAGE_LEVEL)));
}


/*************************************************************//**
					Gets the number of records in the heap.
					@return number of user records */
ulint
page_dir_get_n_heap(
	/*================*/
	const byte*	page)	/*!< in: index page */
{
	uint16 tmpvalue;
	memcpy(&tmpvalue, page + PAGE_HEADER + PAGE_N_HEAP, 2);
	return(tmpvalue & 0x7fff);
}


//��ȡpage_size
int page_size_t(uint* fsp_flags)
{
	uint ssize = FSP_FLAGS_GET_PAGE_SSIZE(*fsp_flags);

	/* If the logical page size is zero in fsp_flags, then use the
	legacy 16k page size. */
	ssize = (0 == ssize) ? UNIV_PAGE_SSIZE_ORIG : ssize;

	/* Convert from a 'log2 minus 9' to a page size in bytes. */
	const uint	size = ((UNIV_ZIP_SIZE_MIN >> 1) << ssize);

	return size;
}