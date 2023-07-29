#include <linux/init.h>
#include <linux/module.h>
#include <linux/kprobes.h>
//#include <linux/string.h>
MODULE_LICENSE("Dual BSD/GPL");

__attribute__ ((naked))
int sample_c_j(void) {
	__asm__(
		"c.j label_cj\n"
		"li a0, 0\n ret\n"
		"li a0, 1\n ret\n"
		"li a0, 2\n ret\n"
		"li a0, 3\n ret\n"
		"label_cj:\n"
		"li a0, 4\n ret\n"
		"li a0, 5\n ret\n"
		"li a0, 6\n ret\n"
	);
}

__attribute__ ((naked))
int sample_c_jr(void) {
	__asm__(
		"la a0, label_c_jr\n"
		"c_jr_location:\n"
		"c.jr a0\n"
		"li a0, 0\n ret\n"
		"li a0, 1\n ret\n"
		"li a0, 2\n ret\n"
		"li a0, 3\n ret\n"
		"label_c_jr:\n"
		"li a0, 4\n ret\n"
		"li a0, 5\n ret\n"
		"li a0, 6\n ret\n"
	);
}

__attribute__ ((naked))
int sample_c_beqz(int a0) {
	__asm__(
		"c.beqz a0, beqz_label\n"
		"li a0, 10\n ret\n"
		"beqz_label:\n"
		"li a0, 4\n ret\n"
	);
}

__attribute__ ((naked))
int sample_c_bnez(int a0) {
	__asm__(
		"c.bnez a0, bnez_label\n"
		"li a0, 10\n ret\n"
		"bnez_label:\n"
		"li a0, 4\n ret\n"
	);
}

static int pre_handler(struct kprobe *p, struct pt_regs *regs) {
	printk("pre_handler() called\n");

	return 0;
}

static int test_c_j(void) {
	static struct kprobe kp;

	int ret;

	/* Test C.J */
	kp.symbol_name = "sample_c_j";
	kp.pre_handler = pre_handler;

	ret = register_kprobe(&kp);
	if (ret) {
		printk("Couldn't register kprobe, err=%d\n", ret);
		return -1;
	}

	ret = sample_c_j();
	if (ret != 4) {
		printk("ERROR: expect value 4, got %d\n", ret);
		return -1;
	}
	else {
		printk("Got value 4, all good!\n");
		return 0;
	}
}

static int test_c_jr(void) {
	static struct kprobe kp;
	int ret;

	/* Test C.JR */
	kp.symbol_name = "c_jr_location";
	kp.pre_handler = pre_handler;

	ret = register_kprobe(&kp);
	if (ret) {
		printk("Couldn't register kprobe, err=%d\n", ret);
		return -1;
	}

	ret = sample_c_jr();
	if (ret != 4) {
		printk("Expect value 4, got %d\n", ret);
		return -1;
	}
	else {
		printk("Got value 4, all good!\n");
		return 0;
	}
}

static int test_c_bnez(void) {
	static struct kprobe kp;

	int ret;

	/* Test C.JR */
	kp.symbol_name = "sample_c_bnez";
	kp.pre_handler = pre_handler;

	ret = register_kprobe(&kp);
	if (ret) {
		printk("Couldn't register kprobe, err=%d\n", ret);
		return -1;
	}

	ret = sample_c_bnez(1);
	if (ret != 4) {
		printk("Expect value 4, got %d\n", ret);
		return -1;
	} else {
		printk("Got value 4, all good!\n");
	}

	ret = sample_c_bnez(0);
	if (ret != 10) {
		printk("Expect value 10, got %d\n", ret);
		return -1;
	} else {
		printk("Got value 4, all good!\n");
	}

	return 0;
}

static int test_c_beqz(void) {
	static struct kprobe kp;

	int ret;

	/* Test C.JR */
	kp.symbol_name = "sample_c_beqz";
	kp.pre_handler = pre_handler;

	ret = register_kprobe(&kp);
	if (ret) {
		printk("Couldn't register kprobe, err=%d\n", ret);
		return -1;
	}

	ret = sample_c_beqz(0);
	if (ret != 4) {
		printk("Expect value 4, got %d\n", ret);
		return -1;
	}
	else {
		printk("Got value 4, all good!\n");
		return 0;
	}

	ret = sample_c_beqz(1);
	if (ret != 10) {
		printk("Expect value 10, got %d\n", ret);
		return -1;
	}
	else {
		printk("Got value 4, all good!\n");
		return 0;
	}
}

static int hello_init(void)
{
	printk("Hello\n");

	printk("Testing C.J...\n");
	if (test_c_j())
		return -1;

	printk("Testing C.JR...\n");
	if (test_c_jr())
		return -1;

	printk("Testing C.BNEZ...\n");
	if (test_c_bnez())
		return -1;

	printk("Testing C.BEQZ...\n");
	if (test_c_beqz())
		return -1;

	return 0;
}

static void hello_exit(void)
{
	printk("Goodbye\n");
}

module_init(hello_init);
module_exit(hello_exit);
