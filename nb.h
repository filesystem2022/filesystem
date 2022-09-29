
#define DATA_BLOCK_SIZE 4096

struct disk_super_block {
    int32 s_first_data_block;               /* First Data Block */
    int32 s_blocks_per_group;               /* # Blocks per group 每个组块的块数量*/
    int32 s_mtime;                          /* Mount time */
    int32 s_wtime;                          /* Write time */
    int16 s_magic;                          /* Magic signature 如果不是=0xEFBB 就是有问题 */

    int16 s_state;                          /* File system state */
    int32 s_lastcheck;                      /* time of last check */
    int8 s_uuid[16];                         /* 128-bit uuid for volume */
    char s_volume_name[16];                 /* volume name */

    //todo

    int32 s_reserved[4096];                 /* Padding to the end of the block */
    int32 s_checksum;                       /* crc32c(superblock) */
};

struct bad_block_desc {
    int32 s_start_block;
    int32 s_len;
};

struct physical_disk_desc {
    disk_super_block s_sb;
    ndoe_desc s_node_desc[];
    bad_block_desc s_bad_block[];
    //todo
    //log
};

struct node_super_block {
    int32 s_first_data_block;

    //todo

    int32 s_reserved[DATA_BLOCK_SIZE-NB_SUPER_SIZE];  /* Padding to the end of the block */
    int32 s_checksum;                                 /* crc32c(superblock) */
};

struct zone_desc {
    int32 s_first_data_block;

};

struct ndoe_desc {
    node_super_block s_sb;
    zone_desc s_zone;
    char s_block_bitmap[64*DATA_BLOCK_SIZE];
    object_header s_obj_meta;
    extent_header s_ext_meta;
};

struct object_header {
    int16 obj_magic;  
};

//256 bytes
/*
 * Structure of an inode on the disk
 */
struct object_inode
{
    int16	i_mode;		/* File mode */
	int16	i_uid;		/* Low 16 bits of Owner Uid */
	int32   i_size_lo;	/* Size in bytes */
	int32	i_atime;	/* Access time */
	int32	i_ctime;	/* Inode Change time */
	int32	i_mtime;	/* Modification time */
	int32	i_dtime;	/* Deletion Time */
	int32	i_gid;		/* Low 16 bits of Group Id */
	int16	i_links_count;	/* Links count */
	int32	i_blocks_lo;	/* Blocks count */
	int32	i_flags;	/* File flags */
	union {
		struct {
			int32  l_i_version;
		} linux1;
		struct {
			int32  h_i_translator;
		} hurd1;
		struct {
			int32  m_i_reserved1;
		} masix1;
	} osd1;				/* OS dependent 1 */
	int32	i_block[EXT4_N_BLOCKS];/* Pointers to blocks */
	int32	i_generation;	/* File version (for NFS) */
	int32	i_file_acl_lo;	/* File ACL */
	int32	i_size_high;
	int32	i_obso_faddr;	/* Obsoleted fragment address */
	union {
		struct {
			int16	l_i_blocks_high; /* were l_i_reserved1 */
			int16	l_i_file_acl_high;
			int16	l_i_uid_high;	/* these 2 fields */
			int16	l_i_gid_high;	/* were reserved2[0] */
			int16	l_i_checksum_lo;/* crc32c(uuid+inum+inode) LE */
			int16	l_i_reserved;
		} linux2;
		struct {
			uint16	h_i_reserved1;	/* Obsoleted fragment number/size which are removed in ext4 */
			uint16	h_i_mode_high;
			uint16	h_i_uid_high;
			uint16	h_i_gid_high;
			uint32	h_i_author;
		} hurd2;
		struct {
			int16	h_i_reserved1;	/* Obsoleted fragment number/size which are removed in ext4 */
			int16	m_i_file_acl_high;
			uint32	m_i_reserved2[2];
		} masix2;
	} osd2;				/* OS dependent 2 */
	int16	i_extra_isize;
	int16	i_checksum_hi;	/* crc32c(uuid+inum+inode) BE */
	int32   i_ctime_extra;  /* extra Change time      (nsec << 2 | epoch) */
	int32   i_mtime_extra;  /* extra Modification time(nsec << 2 | epoch) */
	int32   i_atime_extra;  /* extra Access time      (nsec << 2 | epoch) */
	int32   i_crtime;       /* File Creation time */
	int32   i_crtime_extra; /* extra FileCreationtime (nsec << 2 | epoch) */
	int32   i_version_hi;	/* high 32 bits for 64-bit version */
	int32	i_projid;	/* Project ID */
}
/*In ext4, the file to logical block map has been replaced with an extent tree. Under the old scheme, allocating a contiguous run of 1,000 blocks requires an indirect block to map all 1,000 entries; 
with extents, the mapping is reduced to a single struct ext4_extent with ee_len = 1000. 
If flex_bg is enabled, it is possible to allocate very large files with a single extent, at a considerable reduction in metadata block use, 
and some improvement in disk efficiency. The inode must have the extents flag (0x80000) flag set for this feature to be in use.

Extents are arranged as a tree. Each node of the tree begins with a struct ext4_extent_header. If the node is an interior node (eh.eh_depth > 0), 
the header is followed by eh.eh_entries instances of struct ext4_extent_idx; each of these index entries points to a block containing more nodes in the extent tree. 
If the node is a leaf node (eh.eh_depth == 0), then the header is followed by eh.eh_entries instances of struct ext4_extent; 
these instances point to the file's data blocks. The root node of the extent tree is stored in inode.i_block, 
which allows for the first four extents to be recorded without the use of extra metadata block
*/

//12byte
struct extent_header{
	int16 eh_magic;  //Magic number, 0xF30A.
	int16 eh_entires; //Number of valid entries following the header.
	int16 eh_max; //Maximum number of entries that could follow the header.
	int16 eh_depth; //Depth of this extent node in the extent tree. 
	                //0 = this extent node points to data blocks; otherwise, this extent node points to other extent nodes. 
					//The extent tree can be at most 5 levels deep: a logical block number can be at most 2^32, and the smallest n that satisfies 4*(((blocksize - 12)/12)^n) >= 2^32 is 5.
	int32 eh_generation; // Generation of the tree. (Used by Lustre, but not standard ext4).
};

//12 bytes
struct extent_idx{
	int32 ei_block;	//This index node covers file blocks from 'block' onward.
	int32 ei_leaf_lo; //Lower 32-bits of the block number of the extent node that is the next level lower in the tree. 
						//The tree node pointed to can be either another internal node or a leaf node, described below.
	int16 ei_leaf_hi; //Upper 16-bits of the previous field.
	uint16 ei_unused;
};

//12 bytes
struct extent{
	int32 ee_block;
	int16 ee_len; //Number of blocks covered by extent. If the value of this field is <= 32768, 
				// the extent is initialized. If the value of the field is > 32768, the extent is uninitialized and the actual extent length is ee_len - 32768. 
				// Therefore, the maximum length of a initialized extent is 32768 blocks, and the maximum length of an uninitialized extent is 32767.
	int16 ee_start_hi; //Upper 16-bits of the block number to which this extent points.
	int32 ee_start_lo; //Lower 32-bits of the block number to which this extent points.
};

//4bytes
struct extent_tail{
	int32 eb_checksum; //hecksum of the extent block, crc32c(uuid+inum+igeneration+extentblock)

};