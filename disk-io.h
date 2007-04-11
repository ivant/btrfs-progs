#ifndef __DISKIO__
#define __DISKIO__
#include "list.h"

struct btrfs_buffer {
	u64 blocknr;
	int count;
	struct list_head dirty;
	struct list_head cache;
	union {
		struct btrfs_node node;
		struct btrfs_leaf leaf;
	};
};

struct btrfs_buffer *read_tree_block(struct btrfs_root *root, u64 blocknr);
struct btrfs_buffer *find_tree_block(struct btrfs_root *root, u64 blocknr);
int write_tree_block(struct btrfs_trans_handle *trans, struct btrfs_root *root,
		     struct btrfs_buffer *buf);
int dirty_tree_block(struct btrfs_trans_handle *trans, struct btrfs_root *root,
		     struct btrfs_buffer *buf);
int clean_tree_block(struct btrfs_trans_handle *trans,
		     struct btrfs_root *root, struct btrfs_buffer *buf);
int btrfs_commit_transaction(struct btrfs_trans_handle *trans, struct btrfs_root
			     *root, struct btrfs_super_block *s);
struct btrfs_root *open_ctree(char *filename, struct btrfs_super_block *s);
struct btrfs_root *open_ctree_fd(int fp, struct btrfs_super_block *super);
int close_ctree(struct btrfs_root *root, struct btrfs_super_block *s);
void btrfs_block_release(struct btrfs_root *root, struct btrfs_buffer *buf);
int write_ctree_super(struct btrfs_trans_handle *trans, struct btrfs_root *root,
		      struct btrfs_super_block *s);
#define BTRFS_SUPER_INFO_OFFSET (16 * 1024)

#endif
