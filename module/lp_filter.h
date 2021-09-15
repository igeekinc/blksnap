/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include <linux/types.h>
#include <linux/list.h>
#include <linux/blk_types.h>

/*
 * Each filter can skip the bio request or complete it,
 * or even redirect it to another block device.
 */
enum {
	FLT_ST_PASS,
	FLT_ST_COMPLETE
};

struct filter_operations {
	int (*submit_bio_cb)(struct bio *bio, void *ctx);
	void (*detach_cb)(void *ctx);
};

struct blk_filter {
	struct list_head link;
#if defined(HAVE_BI_BDEV)
	dev_t	dev_id;
#elif defined(HAVE_BI_BDISK)
	int	major;		/* major number of disks driver */
	u8	partno;
#else
#error "Invalid kernel configuration"
#endif
	const struct filter_operations *fops;
	void *ctx;
};

int filter_add(struct block_device *bdev, const struct filter_operations *fops, void *ctx);
int filter_del(struct block_device *bdev);

int filter_enable(void );
