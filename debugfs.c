/*
 * Greybus debugfs code
 *
 * Copyright 2014 Google Inc.
 * Copyright 2014 Linaro Ltd.
 *
 * Released under the GPLv2 only.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/debugfs.h>

#include "greybus.h"

static struct dentry *gb_debug_root;
static LIST_HEAD(gb_debug_intf);
static LIST_HEAD(gb_debug_cport);
static DEFINE_SPINLOCK(gb_debug_lock);

struct gb_debug_dentry {
	struct list_head list;
	struct dentry *dentry;
	struct dentry *link;
	int id;
};

static struct gb_debug_dentry *gb_debugfs_lookup(struct list_head *list, int id)
{
	struct gb_debug_dentry *dentry = NULL;
	struct list_head *iter;
	unsigned long flags;

	spin_lock_irqsave(&gb_debug_lock, flags);
	list_for_each(iter, list) {
		dentry = list_entry(iter, struct gb_debug_dentry, list);
		if (dentry->id == id) {
			spin_unlock_irqrestore(&gb_debug_lock, flags);
			return dentry;
		}
	}
	spin_unlock_irqrestore(&gb_debug_lock, flags);

	return NULL;
}

static struct gb_debug_dentry *gb_debugfs_create(const char *name,
						struct dentry *parent,
						struct list_head *list,
						int id)
{
	struct gb_debug_dentry *dentry;
	unsigned long flags;

	dentry = gb_debugfs_lookup(list, id);
	if (!dentry) {
		dentry = kmalloc(sizeof(struct gb_debug_dentry), GFP_KERNEL);
		if (!dentry)
			return NULL;

		dentry->link = NULL;
		dentry->dentry = debugfs_create_dir(name, parent);
		dentry->id = id;
		INIT_LIST_HEAD(&dentry->list);
		spin_lock_irqsave(&gb_debug_lock, flags);
		list_add(&dentry->list, list);
		spin_unlock_irqrestore(&gb_debug_lock, flags);
	}

	return dentry;
}

struct dentry *gb_debugfs_cport_create(struct gb_connection *connection,
					u8 intf_id)
{
	char name[16];
	char path[32];
	struct gb_debug_dentry *intf_dentry;
	struct gb_debug_dentry *cport_dentry;

	snprintf(name, sizeof(name), "intf%u", intf_id);
	intf_dentry = gb_debugfs_create(name, gb_debugfs_get(),
					&gb_debug_intf, intf_id);

	snprintf(name, sizeof(name), "CP%u", connection->hd_cport_id);
	cport_dentry = gb_debugfs_create(name, gb_debugfs_get(),
					&gb_debug_cport,
					connection->hd_cport_id);

	if (intf_dentry && cport_dentry && !cport_dentry->link) {
		snprintf(name, sizeof(name), "CP%u",
			connection->intf_cport_id);
		snprintf(path, sizeof(path), "../CP%u",
			connection->hd_cport_id);
		cport_dentry->link =
			debugfs_create_symlink(name, intf_dentry->dentry, path);
	}

	if (cport_dentry)
		return cport_dentry->dentry;

	return NULL;
}
EXPORT_SYMBOL_GPL(gb_debugfs_cport_create);

void gb_debugfs_cport_destroy(struct gb_connection *connection)
{
	struct gb_debug_dentry *cport_dentry;
	unsigned long flags;

	cport_dentry = gb_debugfs_lookup(&gb_debug_cport,
					connection->hd_cport_id);

	if (cport_dentry) {
		debugfs_remove(cport_dentry->link);
		debugfs_remove_recursive(cport_dentry->dentry);
		spin_lock_irqsave(&gb_debug_lock, flags);
		list_del(&cport_dentry->list);
		spin_unlock_irqrestore(&gb_debug_lock, flags);
		kfree(cport_dentry);
	}
}
EXPORT_SYMBOL_GPL(gb_debugfs_cport_destroy);

void __init gb_debugfs_init(void)
{
	gb_debug_root = debugfs_create_dir("greybus", NULL);
}

void gb_debugfs_cleanup(void)
{
	debugfs_remove_recursive(gb_debug_root);
	gb_debug_root = NULL;
}

struct dentry *gb_debugfs_get(void)
{
	return gb_debug_root;
}
EXPORT_SYMBOL_GPL(gb_debugfs_get);
