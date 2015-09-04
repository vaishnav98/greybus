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

struct dentry *gb_debugfs_cport_create(struct gb_connection *connection,
					u8 intf_id)
{
	const char *name;

	if (connection->dentry == NULL) {
		name = dev_name(&connection->dev);
		connection->dentry = debugfs_create_dir(name, gb_debug_root);	
	}

	return connection->dentry;
}
EXPORT_SYMBOL_GPL(gb_debugfs_cport_create);

void gb_debugfs_cport_destroy(struct gb_connection *connection)
{
	if (connection->dentry)
		debugfs_remove_recursive(connection->dentry);
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
