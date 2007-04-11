#include <stdio.h>
#include <stdlib.h>
#include "kerncompat.h"
#include "radix-tree.h"
#include "ctree.h"
#include "disk-io.h"

void btrfs_print_leaf(struct btrfs_root *root, struct btrfs_leaf *l)
{
	int i;
	u32 nr = btrfs_header_nritems(&l->header);
	struct btrfs_item *item;
	struct btrfs_extent_item *ei;
	struct btrfs_root_item *ri;
	struct btrfs_dir_item *di;
	struct btrfs_device_item *devi;
	struct btrfs_inode_item *ii;
	struct btrfs_file_extent_item *fi;
	char *p;
	u32 type;

	printf("leaf %Lu ptrs %d free space %d generation %Lu\n",
		btrfs_header_blocknr(&l->header), nr,
		btrfs_leaf_free_space(root, l),
		btrfs_header_generation(&l->header));
	fflush(stdout);
	for (i = 0 ; i < nr ; i++) {
		item = l->items + i;
		type = btrfs_disk_key_type(&item->key);
		printf("\titem %d key (%Lu %Lu %u) itemoff %d itemsize %d\n",
			i,
			btrfs_disk_key_objectid(&item->key),
			btrfs_disk_key_offset(&item->key),
			btrfs_disk_key_flags(&item->key),
			btrfs_item_offset(item),
			btrfs_item_size(item));
		switch (type) {
		case BTRFS_INODE_ITEM_KEY:
			ii = btrfs_item_ptr(l, i, struct btrfs_inode_item);
			printf("\t\tinode generation %Lu size %Lu mode %o\n",
			       btrfs_inode_generation(ii),
			       btrfs_inode_size(ii),
			       btrfs_inode_mode(ii));
			break;
		case BTRFS_INLINE_DATA_KEY:
			p = btrfs_item_ptr(l, i, char);
			printf("\t\tinline data %.*s\n", 10, p);
			break;
		case BTRFS_DIR_ITEM_KEY:
			di = btrfs_item_ptr(l, i, struct btrfs_dir_item);
			printf("\t\tdir oid %Lu flags %u type %u\n",
				btrfs_disk_key_objectid(&di->location),
				btrfs_dir_flags(di),
				btrfs_dir_type(di));
			printf("\t\tname %.*s\n",
			       btrfs_dir_name_len(di),(char *)(di + 1));
			break;
		case BTRFS_DIR_INDEX_KEY:
			di = btrfs_item_ptr(l, i, struct btrfs_dir_item);
			printf("\t\tdir index %Lu flags %u type %u\n",
				btrfs_disk_key_objectid(&di->location),
				btrfs_dir_flags(di),
				btrfs_dir_type(di));
			printf("\t\tname %.*s\n",
			       btrfs_dir_name_len(di),(char *)(di + 1));
			break;
		case BTRFS_ROOT_ITEM_KEY:
			ri = btrfs_item_ptr(l, i, struct btrfs_root_item);
			printf("\t\troot data blocknr %Lu dirid %Lu refs %u\n",
				btrfs_root_blocknr(ri),
				btrfs_root_dirid(ri),
				btrfs_root_refs(ri));
			break;
		case BTRFS_EXTENT_ITEM_KEY:
			ei = btrfs_item_ptr(l, i, struct btrfs_extent_item);
			printf("\t\textent data refs %u\n",
				btrfs_extent_refs(ei));
			break;
		case BTRFS_EXTENT_DATA_KEY:
			fi = btrfs_item_ptr(l, i,
					    struct btrfs_file_extent_item);
			printf("\t\textent data disk block %Lu nr %Lu\n",
			       btrfs_file_extent_disk_blocknr(fi),
			       btrfs_file_extent_disk_num_blocks(fi));
			printf("\t\textent data offset %Lu nr %Lu\n",
			       btrfs_file_extent_offset(fi),
			       btrfs_file_extent_num_blocks(fi));
			break;
		case BTRFS_DEV_ITEM_KEY:
			devi = btrfs_item_ptr(l, i, struct btrfs_device_item);
			printf("\t\tdev namelen %u name %.*s\n",
				btrfs_device_pathlen(devi),
				btrfs_device_pathlen(devi),
				(char *)(devi + 1));
			break;
		case BTRFS_STRING_ITEM_KEY:
			printf("\t\titem data %.*s\n", btrfs_item_size(item),
				btrfs_leaf_data(l) + btrfs_item_offset(item));
			break;
		};
		fflush(stdout);
	}
}
void btrfs_print_tree(struct btrfs_root *root, struct btrfs_buffer *t)
{
	int i;
	u32 nr;
	struct btrfs_node *c;

	if (!t)
		return;
	c = &t->node;
	nr = btrfs_header_nritems(&c->header);
	if (btrfs_is_leaf(c)) {
		btrfs_print_leaf(root, (struct btrfs_leaf *)c);
		return;
	}
	printf("node %Lu level %d ptrs %d free %u generation %Lu\n",
	       t->blocknr,
	        btrfs_header_level(&c->header), nr,
		(u32)BTRFS_NODEPTRS_PER_BLOCK(root) - nr,
		btrfs_header_generation(&c->header));
	fflush(stdout);
	for (i = 0; i < nr; i++) {
		printf("\tkey %d (%Lu %u %Lu) block %Lu\n",
		       i,
		       c->ptrs[i].key.objectid,
		       c->ptrs[i].key.flags,
		       c->ptrs[i].key.offset,
		       btrfs_node_blockptr(c, i));
		fflush(stdout);
	}
	for (i = 0; i < nr; i++) {
		struct btrfs_buffer *next_buf = read_tree_block(root,
						btrfs_node_blockptr(c, i));
		struct btrfs_node *next = &next_buf->node;
		if (btrfs_is_leaf(next) &&
		    btrfs_header_level(&c->header) != 1)
			BUG();
		if (btrfs_header_level(&next->header) !=
			btrfs_header_level(&c->header) - 1)
			BUG();
		btrfs_print_tree(root, next_buf);
		btrfs_block_release(root, next_buf);
	}
}

