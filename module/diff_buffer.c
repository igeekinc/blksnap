// SPDX-License-Identifier: GPL-2.0
#define pr_fmt(fmt) KBUILD_MODNAME "-diff-buffer: " fmt
#include "memory_checker.h"
#include "params.h"
#include "diff_buffer.h"
#include "diff_area.h"
#ifdef STANDALONE_BDEVFILTER
#include "log.h"
#endif

#ifdef BLK_SNAP_DEBUG_DIFF_BUFFER
static atomic_t diff_buffer_allocated_counter;

static int diff_buffer_allocated_counter_get(void)
{
	return atomic_read(&diff_buffer_allocated_counter);
}

static atomic_t diff_buffer_take_cnt;
static int diff_buffer_take_cnt_get(void)
{
	return atomic_read(&diff_buffer_take_cnt);
}

#endif

static void diff_buffer_free(struct diff_buffer *diff_buffer)
{
	size_t inx = 0;
	struct page *page;

	if (unlikely(!diff_buffer))
		return;

	for (inx = 0; inx < diff_buffer->page_count; inx++) {
		page = diff_buffer->pages[inx];
		if (page) {
			__free_page(page);
			memory_object_dec(memory_object_page);
		}
	}

	kfree(diff_buffer);
	memory_object_dec(memory_object_diff_buffer);
#ifdef BLK_SNAP_DEBUG_DIFF_BUFFER
	atomic_dec(&diff_buffer_allocated_counter);
#endif
}

static struct diff_buffer *
diff_buffer_new(size_t page_count, size_t buffer_size, gfp_t gfp_mask)
{
	struct diff_buffer *diff_buffer;
	size_t inx = 0;
	struct page *page;

	if (unlikely(page_count <= 0))
		return NULL;

	/*
	 * In case of overflow, it is better to get a null pointer
	 * than a pointer to some memory area. Therefore + 1.
	 */
	diff_buffer = kzalloc(sizeof(struct diff_buffer) +
				      (page_count + 1) * sizeof(struct page *),
			      gfp_mask);
	if (!diff_buffer)
		return NULL;
	memory_object_inc(memory_object_diff_buffer);

#ifdef BLK_SNAP_DEBUG_DIFF_BUFFER
	diff_buffer->number = atomic_inc_return(&diff_buffer_allocated_counter);
#endif
	INIT_LIST_HEAD(&diff_buffer->link);
	diff_buffer->size = buffer_size;
	diff_buffer->page_count = page_count;

	for (inx = 0; inx < page_count; inx++) {
		page = alloc_page(gfp_mask);
		if (!page)
			goto fail;
		memory_object_inc(memory_object_page);

		diff_buffer->pages[inx] = page;
	}
	return diff_buffer;
fail:
	diff_buffer_free(diff_buffer);
	return NULL;
}

struct diff_buffer *diff_buffer_take(struct diff_area *diff_area,
				     const bool is_nowait)
{
	struct diff_buffer *diff_buffer = NULL;
	sector_t chunk_sectors;
	size_t page_count;
	size_t buffer_size;

	spin_lock(&diff_area->free_diff_buffers_lock);
	diff_buffer = list_first_entry_or_null(&diff_area->free_diff_buffers,
					       struct diff_buffer, link);
	if (diff_buffer) {
		list_del(&diff_buffer->link);
		atomic_dec(&diff_area->free_diff_buffers_count);
	}
	spin_unlock(&diff_area->free_diff_buffers_lock);

	/* Return free buffer if it was found in a pool */
	if (diff_buffer) {
#ifdef BLK_SNAP_DEBUG_DIFF_BUFFER
		atomic_inc(&diff_buffer_take_cnt);
#endif
		return diff_buffer;
	}

	/* Allocate new buffer */
	chunk_sectors = diff_area_chunk_sectors(diff_area);
	page_count = round_up(chunk_sectors, PAGE_SECTORS) / PAGE_SECTORS;
	buffer_size = chunk_sectors << SECTOR_SHIFT;

	diff_buffer =
		diff_buffer_new(page_count, buffer_size,
				is_nowait ? (GFP_NOIO | GFP_NOWAIT) : GFP_NOIO);
	if (unlikely(!diff_buffer)) {
		if (is_nowait)
			return ERR_PTR(-EAGAIN);
		else
			return ERR_PTR(-ENOMEM);
	}

#ifdef BLK_SNAP_DEBUG_DIFF_BUFFER
	atomic_inc(&diff_buffer_take_cnt);
#endif
	return diff_buffer;
}

void diff_buffer_release(struct diff_area *diff_area,
			 struct diff_buffer *diff_buffer)
{
#ifdef BLK_SNAP_DEBUG_DIFF_BUFFER
	atomic_dec(&diff_buffer_take_cnt);
#endif
	if (atomic_read(&diff_area->free_diff_buffers_count) >
	    free_diff_buffer_pool_size) {
		diff_buffer_free(diff_buffer);
		return;
	}
	spin_lock(&diff_area->free_diff_buffers_lock);
	list_add_tail(&diff_buffer->link, &diff_area->free_diff_buffers);
	atomic_inc(&diff_area->free_diff_buffers_count);
	spin_unlock(&diff_area->free_diff_buffers_lock);
}

void diff_buffer_cleanup(struct diff_area *diff_area)
{
	struct diff_buffer *diff_buffer = NULL;

#ifdef BLK_SNAP_DEBUG_DIFF_BUFFER
	pr_debug("Cleanup %d buffers\n", diff_buffer_allocated_counter_get());
#endif
	do {
		spin_lock(&diff_area->free_diff_buffers_lock);
		diff_buffer =
			list_first_entry_or_null(&diff_area->free_diff_buffers,
						 struct diff_buffer, link);
		if (diff_buffer) {
			list_del(&diff_buffer->link);
			atomic_dec(&diff_area->free_diff_buffers_count);
		}
		spin_unlock(&diff_area->free_diff_buffers_lock);

		if (diff_buffer)
			diff_buffer_free(diff_buffer);
	} while (diff_buffer);
#ifdef BLK_SNAP_DEBUG_DIFF_BUFFER
	if (diff_buffer_allocated_counter_get())
		pr_debug("Some buffers %d still available\n",
			 diff_buffer_allocated_counter_get());
	pr_debug("%d diff buffers is not released\n",
		 diff_buffer_take_cnt_get());
#endif
}
