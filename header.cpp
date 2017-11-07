//例子1：以二进制模式打开并写入文件
#include <stdio.h>
#include <stdlib.h>  
#include <iostream>
#include <cstring>
#include "header.h"


using namespace std;







//获取page_size
int page_size_t(uint fsp_flags)
{
	uint	ssize = FSP_FLAGS_GET_PAGE_SSIZE(fsp_flags);

	/* If the logical page size is zero in fsp_flags, then use the
	legacy 16k page size. */
	ssize = (0 == ssize) ? UNIV_PAGE_SSIZE_ORIG : ssize;

	/* Convert from a 'log2 minus 9' to a page size in bytes. */
	const uint	size = ((UNIV_ZIP_SIZE_MIN >> 1) << ssize);

	return size;
}



//是否为-1
int check_unside(uint value)
{
	if (value == -1)
	{
		return 0;
	}
	else
		return value;
}



//读取4bytes
int read_int(FILE *file)
{
	int value;
	fread(&value, sizeof(int), 1, file);
	return value;
}

//获取簇、段的指针信息
void *read_fsp_content(Fsp_Info *fsp_info_value, char* buffer, uint offset, int type)
{
	ulint flst_len = check_unside(mach_read_from_4(buffer + offset + FLST_LEN));
	ulint flst_first_page = check_unside(mach_read_from_4(buffer + offset + FLST_FIRST + FIL_ADDR_PAGE));
	ulint flst_first_page_offset = check_unside(mach_read_from_2(buffer + offset + FLST_FIRST + FIL_ADDR_BYTE));
	ulint flst_last_page = check_unside(mach_read_from_4(buffer + offset + FLST_LAST + FIL_ADDR_PAGE));
	ulint flst_last_page_offset = check_unside(mach_read_from_2(buffer + offset + FLST_LAST + FIL_ADDR_BYTE));

	if (type == 1)
	{
		fsp_info_value->fsp_free.flst_len = flst_len;
		fsp_info_value->fsp_free.flst_first.addr_page = flst_first_page;
		fsp_info_value->fsp_free.flst_first.addr_offset = flst_first_page_offset;
		fsp_info_value->fsp_free.flst_last.addr_page = flst_last_page;
		fsp_info_value->fsp_free.flst_last.addr_offset = flst_last_page_offset;
	}
	else if (type == 2)
	{
		fsp_info_value->fsp_free_frag.flst_len = flst_len;
		fsp_info_value->fsp_free_frag.flst_first.addr_page = flst_first_page;
		fsp_info_value->fsp_free_frag.flst_first.addr_offset = flst_first_page_offset;
		fsp_info_value->fsp_free_frag.flst_last.addr_page = flst_last_page;
		fsp_info_value->fsp_free_frag.flst_last.addr_offset = flst_last_page_offset;
	}
	else if (type == 3)
	{
		fsp_info_value->fsp_full_frag.flst_len = flst_len;
		fsp_info_value->fsp_full_frag.flst_first.addr_page = flst_first_page;
		fsp_info_value->fsp_full_frag.flst_first.addr_offset = flst_first_page_offset;
		fsp_info_value->fsp_full_frag.flst_last.addr_page = flst_last_page;
		fsp_info_value->fsp_full_frag.flst_last.addr_offset = flst_last_page_offset;
	}
	else if (type == 4)
	{
		fsp_info_value->fsp_seg_inodes_free.flst_len = flst_len;
		fsp_info_value->fsp_seg_inodes_free.flst_first.addr_page = flst_first_page;
		fsp_info_value->fsp_seg_inodes_free.flst_first.addr_offset = flst_first_page_offset;
		fsp_info_value->fsp_seg_inodes_free.flst_last.addr_page = flst_last_page;
		fsp_info_value->fsp_seg_inodes_free.flst_last.addr_offset = flst_last_page_offset;
	}
	else if (type == 5)
	{
		fsp_info_value->fsp_seg_inodes_full.flst_len = flst_len;
		fsp_info_value->fsp_seg_inodes_full.flst_first.addr_page = flst_first_page;
		fsp_info_value->fsp_seg_inodes_full.flst_first.addr_offset = flst_first_page_offset;
		fsp_info_value->fsp_seg_inodes_full.flst_last.addr_page = flst_last_page;
		fsp_info_value->fsp_seg_inodes_full.flst_last.addr_offset = flst_last_page_offset;
	}

}




char* get_page_value(FILE* fp,uint* page_size)
{
	char *page;
	page = new char[*page_size];
	fread(page, *page_size, 1, fp);
	return page;
}


void Enter(char *file_name)
{
	Fsp_Info *fsp_info_value = new Fsp_Info;
	FILE *fp = fsp_info_value->fp = fopen(file_name, "rb+");   //二进制模式
	if (NULL == fp)
	{
		cout << file_name << ": No such file or directory" << endl;
		exit;
	}

	uint tmp_value;
	//page_size
	fseek(fp, FSP_HEADER_OFFSET + FSP_SPACE_FLAGS,0);
	fread(&tmp_value, 4, 1, fp);
	fsp_info_value->page_size = page_size_t(tmp_value);
	

	/* 把header页读入buffer*/
	char *buffer;
	buffer = new char[fsp_info_value->page_size];
	fseek(fp, 0, 0);
	fread(buffer, fsp_info_value->page_size, 1, fp);


	//page_ssize
	fsp_info_value->page_ssize = FSP_FLAGS_GET_PAGE_SSIZE(mach_little_read_from_4(buffer + FSP_HEADER_OFFSET + FSP_SPACE_FLAGS));

	//page_number
	fsp_info_value->page_number = mach_read_from_4(buffer + FIL_PAGE_OFFSET);

	//size_in_header
	fsp_info_value->fsp_size = mach_read_from_4(buffer + FSP_HEADER_OFFSET + FSP_SIZE);

	//space_id
	fsp_info_value->space_id = mach_read_from_4(buffer + FSP_HEADER_OFFSET);

	//id
	fsp_info_value->fsp_space_id = mach_read_from_4(buffer + FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID);

	//free_limit
	fsp_info_value->free_limit = mach_read_from_4(buffer + FSP_HEADER_OFFSET + FSP_FREE_LIMIT);

	//free_len
	fsp_info_value->free_len = mach_read_from_4(buffer + FSP_HEADER_OFFSET + FSP_FREE + FLST_LEN);

	//page_type_fsp_hdr
	fsp_info_value->fil_page_type = mach_read_from_2(buffer + FIL_PAGE_TYPE);

	//fsp_not_used
	fsp_info_value->fsp_not_used = mach_read_from_4(buffer + FSP_HEADER_OFFSET + FSP_NOT_USED);

	//fsp_flags
	fsp_info_value->fsp_space_flags = mach_little_read_from_4(buffer + FSP_HEADER_OFFSET + FSP_SPACE_FLAGS);

	//fsp_frag_n_used
	fsp_info_value->fsp_frag_n_used = mach_read_from_4(buffer + FSP_HEADER_OFFSET + FSP_FRAG_N_USED);

	//is_com 是否压缩
	fsp_info_value->is_com = page_is_comp(buffer);

	/*簇于段指针*/
	//空闲簇信息
	read_fsp_content(fsp_info_value, buffer, FSP_HEADER_OFFSET + FSP_FREE, 1);
	//半空簇信息
	read_fsp_content(fsp_info_value, buffer, FSP_HEADER_OFFSET + FSP_FREE_FRAG, 2);
	//满簇信息
	read_fsp_content(fsp_info_value, buffer, FSP_HEADER_OFFSET + FSP_FULL_FRAG, 3);

	//数据段信息
	//已满的数据页链表信息
	read_fsp_content(fsp_info_value, buffer, FSP_HEADER_OFFSET + FSP_SEG_INODES_FULL, 4);
	//半满的数据页链表信息
	read_fsp_content(fsp_info_value, buffer, FSP_HEADER_OFFSET + FSP_SEG_INODES_FREE, 5);


	delete[]buffer;
	fclose(fp);
	Print_content(fsp_info_value);
	delete[]fsp_info_value;
}

void Print_content(Fsp_Info *fsp_info_value)
{
	printf("INFO: space_id: %d, is_compact: %s, page_ssize: %d, page_size: %d, pages_used: %d ,page_number: %d\n", fsp_info_value->space_id,
		fsp_info_value->is_com, fsp_info_value->page_ssize, fsp_info_value->page_size, fsp_info_value->fsp_size,fsp_info_value->page_number);
	/*
	cout << "page_ssize: " << fsp_info_value->page_ssize << endl;
	cout << "page_size: " << fsp_info_value->page_size << "bytes" << endl;
	cout << "size_in_header: " << fsp_info_value->fsp_size << endl;
	cout << "space_id: " << fsp_info_value->space_id << endl;
	cout << "id: " << fsp_info_value->fsp_space_id << endl;
	cout << "is_compact: " << fsp_info_value->is_com << endl;
	cout << "page_number: " << fsp_info_value->page_number << endl;
	cout << "free_limit: " << fsp_info_value->free_limit << endl;
	cout << "free_len: " << fsp_info_value->free_len << endl;
	cout << "fsp_type_fsp_hdr: " << fsp_info_value->fil_page_type << endl;
	cout << "fsp_not_used: " << fsp_info_value->fsp_not_used << endl;
	cout << "fsp_flags: " << fsp_info_value->fsp_space_flags << endl;
	cout << "fsp_frag_n_used: " << fsp_info_value->fsp_frag_n_used << endl;
	*/
	printf("FSP ： fsp_flags: %d, fsp_not_used: %d, fsp_frag_n_used: %d\n", fsp_info_value->fsp_space_flags, fsp_info_value->fsp_not_used,
		fsp_info_value->fsp_frag_n_used);

	printf("     fsp_free: len: %d, firset_page_number: %d, firset_page_offset: %d, last_page_number: %d, last_page_offset: %d\n",
		fsp_info_value->fsp_free.flst_len, fsp_info_value->fsp_free.flst_first.addr_page, fsp_info_value->fsp_free.flst_first.addr_offset,
		fsp_info_value->fsp_free.flst_last.addr_page, fsp_info_value->fsp_free.flst_last.addr_offset);

	printf("     fsp_free_frag: len: %d, firset_page_number: %d, firset_page_offset: %d, last_page_number: %d, last_page_offset: %d\n",
		fsp_info_value->fsp_free_frag.flst_len, fsp_info_value->fsp_free_frag.flst_first.addr_page, fsp_info_value->fsp_free_frag.flst_first.addr_offset,
		fsp_info_value->fsp_free_frag.flst_last.addr_page, fsp_info_value->fsp_free_frag.flst_last.addr_offset);

	printf("     fsp_full_frag: len: %d, firset_page_number: %d, firset_page_offset: %d, last_page_number: %d, last_page_offset: %d\n",
		fsp_info_value->fsp_full_frag.flst_len, fsp_info_value->fsp_full_frag.flst_first.addr_page, fsp_info_value->fsp_full_frag.flst_first.addr_offset,
		fsp_info_value->fsp_full_frag.flst_last.addr_page, fsp_info_value->fsp_full_frag.flst_last.addr_offset);

	/*
	//簇信息
	cout << "fsp_free->  len: " << fsp_info_value->fsp_free.flst_len;
	cout << " first -> page_number: " << fsp_info_value->fsp_free.flst_first.addr_page;
	cout << " page_offset: " << fsp_info_value->fsp_free.flst_first.addr_offset;
	cout << " last -> page_number: " << fsp_info_value->fsp_free.flst_last.addr_page;
	cout << " page_offset: " << fsp_info_value->fsp_free.flst_last.addr_offset << endl;

	cout << "fsp_free_frag->  len: " << fsp_info_value->fsp_free_frag.flst_len;
	cout << " first -> page_number: " << fsp_info_value->fsp_free_frag.flst_first.addr_page;
	cout << " page_offset: " << fsp_info_value->fsp_free_frag.flst_first.addr_offset;
	cout << " last -> page_number: " << fsp_info_value->fsp_free_frag.flst_last.addr_page;
	cout << " page_offset: " << fsp_info_value->fsp_free_frag.flst_last.addr_offset << endl;

	cout << "fsp_full_frag->  len: " << fsp_info_value->fsp_full_frag.flst_len;
	cout << " first -> page_number: " << fsp_info_value->fsp_full_frag.flst_first.addr_page;
	cout << " page_offset: " << fsp_info_value->fsp_full_frag.flst_first.addr_offset;
	cout << " last -> page_number: " << fsp_info_value->fsp_full_frag.flst_last.addr_page;
	cout << " page_offset: " << fsp_info_value->fsp_full_frag.flst_last.addr_offset << endl;
	*/

	printf("FSP_SEG: \n");
	printf("       fsp_seg_free: len: %d, firset_page_number: %d, firset_page_offset: %d, last_page_number: %d, last_page_offset: %d\n",
		fsp_info_value->fsp_seg_inodes_free.flst_len, fsp_info_value->fsp_seg_inodes_free.flst_first.addr_page, fsp_info_value->fsp_seg_inodes_free.flst_first.addr_offset,
		fsp_info_value->fsp_seg_inodes_free.flst_last.addr_page, fsp_info_value->fsp_seg_inodes_free.flst_last.addr_offset);

	printf("       fsp_seg_full: len: %d, firset_page_number: %d, firset_page_offset: %d, last_page_number: %d, last_page_offset: %d\n",
		fsp_info_value->fsp_seg_inodes_full.flst_len, fsp_info_value->fsp_seg_inodes_full.flst_first.addr_page, fsp_info_value->fsp_seg_inodes_full.flst_first.addr_offset,
		fsp_info_value->fsp_seg_inodes_full.flst_last.addr_page, fsp_info_value->fsp_seg_inodes_full.flst_last.addr_offset);

	/*
	//数据段信息
	cout << "fsp_seg_free-> len: " << fsp_info_value->fsp_seg_inodes_free.flst_len;
	cout << " first -> page_number: " << fsp_info_value->fsp_seg_inodes_free.flst_first.addr_page;
	cout << " page_offset: " << fsp_info_value->fsp_seg_inodes_free.flst_first.addr_offset;
	cout << " last -> page_number: " << fsp_info_value->fsp_seg_inodes_free.flst_last.addr_page;
	cout << " page_offset: " << fsp_info_value->fsp_seg_inodes_free.flst_last.addr_offset << endl;

	cout << "fsp_seg_full-> len: " << fsp_info_value->fsp_seg_inodes_full.flst_len;
	cout << " first -> page_number: " << fsp_info_value->fsp_seg_inodes_full.flst_first.addr_page;
	cout << " page_offset: " << fsp_info_value->fsp_seg_inodes_full.flst_first.addr_offset;
	cout << " last -> page_number: " << fsp_info_value->fsp_seg_inodes_full.flst_last.addr_page;
	cout << " page_offset: " << fsp_info_value->fsp_seg_inodes_full.flst_last.addr_offset << endl;
	*/
}

