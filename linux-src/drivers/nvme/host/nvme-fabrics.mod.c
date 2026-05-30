#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

MODULE_INFO(intree, "Y");

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

SYMBOL_CRC(nvmf_get_address, 0xab792e65, "_gpl");
SYMBOL_CRC(nvmf_reg_read32, 0x5874e778, "_gpl");
SYMBOL_CRC(nvmf_reg_read64, 0xabe78602, "_gpl");
SYMBOL_CRC(nvmf_reg_write32, 0x15fc90e0, "_gpl");
SYMBOL_CRC(nvmf_connect_admin_queue, 0x12dd41bb, "_gpl");
SYMBOL_CRC(nvmf_connect_io_queue, 0xfaa6bbe8, "_gpl");
SYMBOL_CRC(nvmf_should_reconnect, 0x46cb6a7d, "_gpl");
SYMBOL_CRC(nvmf_register_transport, 0x4b4881a7, "_gpl");
SYMBOL_CRC(nvmf_unregister_transport, 0x68eb2272, "_gpl");
SYMBOL_CRC(nvmf_ip_options_match, 0x98a9cab5, "_gpl");
SYMBOL_CRC(nvmf_free_options, 0x7498de97, "_gpl");

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xab7cf47a, "try_module_get" },
	{ 0x899ac0cf, "misc_deregister" },
	{ 0xa3a691c0, "__class_create" },
	{ 0xc60d0620, "__num_online_cpus" },
	{ 0x656e4a6e, "snprintf" },
	{ 0xd450b025, "class_destroy" },
	{ 0x96848186, "scnprintf" },
	{ 0x37a0cba, "kfree" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x65487097, "__x86_indirect_thunk_rax" },
	{ 0x92997ed8, "_printk" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x296695f, "refcount_warn_saturate" },
	{ 0x2aaecb9c, "put_device" },
	{ 0xa916b694, "strnlen" },
	{ 0x4b01f174, "module_put" },
	{ 0x68f31cbd, "__list_add_valid" },
	{ 0x69e683de, "uuid_gen" },
	{ 0x57bc19d2, "down_write" },
	{ 0x98cfc605, "_dev_err" },
	{ 0xce807a25, "up_write" },
	{ 0x4e3567f7, "match_int" },
	{ 0x22c40c1d, "device_create" },
	{ 0x4dfa8d4b, "mutex_lock" },
	{ 0x5a921311, "strncmp" },
	{ 0x9166fada, "strncpy" },
	{ 0xe1537255, "__list_del_entry_valid" },
	{ 0x84a3b727, "__nvme_submit_sync_cmd" },
	{ 0xdd9be00d, "_dev_warn" },
	{ 0xefb4a73d, "misc_register" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xb67fec0e, "uuid_parse" },
	{ 0x668b19a1, "down_read" },
	{ 0xe2d5255a, "strcmp" },
	{ 0x2d39b0a7, "kstrdup" },
	{ 0x6133eb99, "seq_read" },
	{ 0xdd64e639, "strscpy" },
	{ 0x85df9b6c, "strsep" },
	{ 0x3213f038, "mutex_unlock" },
	{ 0x1d07e365, "memdup_user_nul" },
	{ 0x44e9a829, "match_token" },
	{ 0xa8be6c59, "device_destroy" },
	{ 0x2cf56265, "__dynamic_pr_debug" },
	{ 0x857d9f62, "seq_printf" },
	{ 0xacf4d843, "match_strdup" },
	{ 0x64f09adb, "seq_puts" },
	{ 0xd35841e5, "single_release" },
	{ 0x6c9b8224, "kmalloc_trace" },
	{ 0x754d539c, "strlen" },
	{ 0x45816684, "single_open" },
	{ 0x53b954a2, "up_read" },
	{ 0x5fe5236c, "kmalloc_caches" },
	{ 0xa24f23d8, "__request_module" },
	{ 0x41fdc1e4, "module_layout" },
};

MODULE_INFO(depends, "");

