#include <linux/module.h>
#include <crypto/hash.h>
#include <linux/slab.h>
#include <linux/kprobes.h>
MODULE_LICENSE("Dual BSD/GPL");

#include "sha256.h"

const uint8_t firmware[] = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
static const uint8_t reference_hash[32] = {
	0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8, 0xe5, 0xc0, 0x26, 0x93,
	0x0c, 0x3e, 0x60, 0x39, 0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
	0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1
};

int test_sha256(void) {
	struct tc_sha256_state_struct state;
	uint8_t computed_hash[32];

	tc_sha256_init(&state);
	tc_sha256_update(&state, firmware, sizeof(firmware) - 1);
	tc_sha256_final(computed_hash, &state);

	if (memcmp(computed_hash, reference_hash, 32)) {
		return 1;
	}
	else {
		return 0;
	}
}

static int callback_called = 0;
static int kprobe_handler(struct kprobe *p, struct pt_regs *regs) {
	callback_called++;
	return 0;
}

static int install_probes(void) {
	struct kprobe *kp = kmalloc(100 * sizeof(struct kprobe), GFP_KERNEL);

	int ret;
	unsigned offset = 0;

	for (int i = 0; i < 100; ++i) {
		kp[i].symbol_name = "tc_sha256_final";
		kp[i].pre_handler = kprobe_handler;
		kp[i].offset = offset;

		ret = register_kprobe(&kp[i]);
		if (ret) {
			printk("Couldn't register kprobe, err=%d\n", ret);
			return -1;
		} else {
			printk("Successfully install  kprobe\n");
		}

		offset += GET_INSN_LENGTH(*kp[i].addr);
	}

	return 0;
}

static int install_illegal_probes(void) {
	struct kprobe *kp = kmalloc(7 * sizeof(struct kprobe), GFP_KERNEL);

	int ret;
	unsigned offset = 0;

	for (int i = 0; i < 7; ++i) {
		kp[i].symbol_name = "ret_from_exception";
		kp[i].pre_handler = kprobe_handler;
		kp[i].offset = offset;

		ret = register_kprobe(&kp[i]);
		if (ret) {
			printk("Couldn't register illegal kprobe, err=%d\n", ret);
		} else {
			printk("Successfully install illegal kprobe\n");
			return -1;
		}

		offset += GET_INSN_LENGTH(*kp[i].addr);
	}

	return 0;
}

static int __init module_start(void)
{
	printk("Hello world\n");

	if (install_probes()) {
		return -1;
	}

	if (test_sha256()) {
		printk("Hashing failed\n");
		return -1;
	} else {
		printk("Hash is still correct\n");
	}

	if (install_illegal_probes()) {
		return -1;
	}

	if (callback_called != 5)
		printk("Expected number of callback to be 5, got %d\n", callback_called);

	return 0;
}

static void hello_exit(void) { return; }

module_init(module_start);
module_exit(hello_exit);
