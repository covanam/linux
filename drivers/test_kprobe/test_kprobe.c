#include <linux/module.h>
#include <crypto/hash.h>
MODULE_LICENSE("Dual BSD/GPL");

#include "sha256.h"
#include "sha256.c"

const uint8_t firmware[] = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
static const uint8_t reference_hash[32] = {
	0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8, 0xe5, 0xc0, 0x26, 0x93,
	0x0c, 0x3e, 0x60, 0x39, 0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
	0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1
};

static int test_sha256(void) {
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

static int __init module_start(void)
{
	if (test_sha256()) {
		printk("Hashing failed\n");
		return -1;
	}
	return 0;
}

static void hello_exit(void) { return; }

module_init(module_start);
module_exit(hello_exit);
