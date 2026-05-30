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

SYMBOL_CRC(nvme_fc_register_localport, 0xfb1526a5, "_gpl");
SYMBOL_CRC(nvme_fc_unregister_localport, 0x3884f8b8, "_gpl");
SYMBOL_CRC(nvme_fc_register_remoteport, 0x0d12e564, "_gpl");
SYMBOL_CRC(nvme_fc_unregister_remoteport, 0xfca9dc99, "_gpl");
SYMBOL_CRC(nvme_fc_rescan_remoteport, 0x3e33ac54, "_gpl");
SYMBOL_CRC(nvme_fc_set_remoteport_devloss, 0x8a9cf5a7, "_gpl");
SYMBOL_CRC(nvme_fc_rcv_ls_req, 0xbb0e18a6, "_gpl");
SYMBOL_CRC(nvme_fc_io_getuuid, 0x58e312f7, "_gpl");

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x218ee168, "nvme_remove_admin_tag_set" },
	{ 0x72b3f838, "nvme_sync_io_queues" },
	{ 0xc31db0ce, "is_vmalloc_addr" },
	{ 0xe7a02573, "ida_alloc_range" },
	{ 0xee7d9f9c, "nvme_uninit_ctrl" },
	{ 0x49cd25ed, "alloc_workqueue" },
	{ 0xa7d5f92e, "ida_destroy" },
	{ 0x5e9168e6, "nvme_complete_rq" },
	{ 0xc60d0620, "__num_online_cpus" },
	{ 0x656e4a6e, "snprintf" },
	{ 0xa6257a2f, "complete" },
	{ 0xc5b6f236, "queue_work_on" },
	{ 0x7498de97, "nvmf_free_options" },
	{ 0xbb94fa8c, "blk_mq_tagset_wait_completed_request" },
	{ 0x8510fc2c, "nvme_start_queues" },
	{ 0x608741b5, "__init_swait_queue_head" },
	{ 0x92540fbf, "finish_wait" },
	{ 0x1a2c15eb, "blk_mq_complete_request_remote" },
	{ 0x65950bec, "dma_unmap_page_attrs" },
	{ 0x2a0105d6, "nvme_setup_cmd" },
	{ 0x98a46505, "nvme_start_ctrl" },
	{ 0xeb9a1d5a, "dma_sync_single_for_device" },
	{ 0x962083cc, "nvme_alloc_admin_tag_set" },
	{ 0xfaa6bbe8, "nvmf_connect_io_queue" },
	{ 0xfba7ddd2, "match_u64" },
	{ 0x37a0cba, "kfree" },
	{ 0x274dd1a3, "sg_free_table_chained" },
	{ 0x8c26d495, "prepare_to_wait_event" },
	{ 0x64b62862, "nvme_wq" },
	{ 0x5874e778, "nvmf_reg_read32" },
	{ 0x3eeb2322, "__wake_up" },
	{ 0x94b7aa6b, "nvme_delete_ctrl" },
	{ 0x8140e725, "get_device" },
	{ 0x34db050b, "_raw_spin_lock_irqsave" },
	{ 0xba8fbd64, "_raw_spin_lock" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xb798db44, "dev_driver_string" },
	{ 0xab792e65, "nvmf_get_address" },
	{ 0x65487097, "__x86_indirect_thunk_rax" },
	{ 0xc7fcece6, "dma_map_page_attrs" },
	{ 0xb5e73116, "flush_delayed_work" },
	{ 0x92997ed8, "_printk" },
	{ 0x8427cc7b, "_raw_spin_lock_irq" },
	{ 0x1000e51, "schedule" },
	{ 0x3b493255, "nvme_remove_io_tag_set" },
	{ 0xf2b6792a, "nvme_cleanup_cmd" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x296695f, "refcount_warn_saturate" },
	{ 0xb2fcb56d, "queue_delayed_work_on" },
	{ 0x68eb2272, "nvmf_unregister_transport" },
	{ 0x2aaecb9c, "put_device" },
	{ 0xa916b694, "strnlen" },
	{ 0xc18fe01, "nvme_change_ctrl_state" },
	{ 0xb2e85585, "_dev_info" },
	{ 0x7249d7f1, "nvme_init_ctrl" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0xb28955d6, "nvme_alloc_io_tag_set" },
	{ 0xfd89cb93, "nvme_fail_nonready_command" },
	{ 0x630c17c9, "kobject_uevent_env" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0x68f31cbd, "__list_add_valid" },
	{ 0x98cfc605, "_dev_err" },
	{ 0xa780a1bf, "__class_register" },
	{ 0x22c40c1d, "device_create" },
	{ 0x8c03d20c, "destroy_workqueue" },
	{ 0x54f10bda, "blk_sync_queue" },
	{ 0x5a921311, "strncmp" },
	{ 0x4b750f53, "_raw_spin_unlock_irq" },
	{ 0x488835d8, "__blk_rq_map_sg" },
	{ 0x9166fada, "strncpy" },
	{ 0xffb7c514, "ida_free" },
	{ 0xabe78602, "nvmf_reg_read64" },
	{ 0x4c9d28b0, "phys_base" },
	{ 0x8a0bae50, "class_unregister" },
	{ 0x49224181, "nvme_reset_wq" },
	{ 0xe1537255, "__list_del_entry_valid" },
	{ 0x12dd41bb, "nvmf_connect_admin_queue" },
	{ 0xd35cce70, "_raw_spin_unlock_irqrestore" },
	{ 0xdc8e2e6a, "nvme_enable_ctrl" },
	{ 0x6163c268, "dma_sync_single_for_cpu" },
	{ 0xdd9be00d, "_dev_warn" },
	{ 0x25974000, "wait_for_completion" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0xe2d5255a, "strcmp" },
	{ 0x15ba50a6, "jiffies" },
	{ 0xe41a47d0, "blk_mq_update_nr_hw_queues" },
	{ 0x97651e6c, "vmemmap_base" },
	{ 0x15fc90e0, "nvmf_reg_write32" },
	{ 0x8e0facd0, "nvme_stop_admin_queue" },
	{ 0x9fa7184a, "cancel_delayed_work_sync" },
	{ 0xc6f46339, "init_timer_key" },
	{ 0x46cb6a7d, "nvmf_should_reconnect" },
	{ 0xe20cd3b4, "nvme_complete_async_event" },
	{ 0x87b8798d, "sg_next" },
	{ 0xedbfd5d, "nvme_stop_queues" },
	{ 0xa8be6c59, "device_destroy" },
	{ 0x1ae52ecb, "nvme_set_queue_count" },
	{ 0x3c12dfe, "cancel_work_sync" },
	{ 0x56470118, "__warn_printk" },
	{ 0xffeedf6a, "delayed_work_timer_fn" },
	{ 0xb2fa093e, "blk_mq_map_queues" },
	{ 0xb78c3aeb, "nvme_start_admin_queue" },
	{ 0x4b4881a7, "nvmf_register_transport" },
	{ 0x1441fad, "blk_mq_start_request" },
	{ 0xc92f2b4b, "blk_mq_tagset_busy_iter" },
	{ 0x4b9ca3c7, "nvme_stop_ctrl" },
	{ 0x67c38866, "dma_unmap_sg_attrs" },
	{ 0x6c9b8224, "kmalloc_trace" },
	{ 0x2c1f51e, "nvme_init_ctrl_finish" },
	{ 0xdbadbf88, "nvme_reset_ctrl" },
	{ 0xb5b54b34, "_raw_spin_unlock" },
	{ 0x1bee4974, "sg_alloc_table_chained" },
	{ 0xf90a1e85, "__x86_indirect_thunk_r8" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0x4eee17c2, "__nvme_check_ready" },
	{ 0x5fe5236c, "kmalloc_caches" },
	{ 0x910ad821, "dma_map_sg_attrs" },
	{ 0x2d3385d3, "system_wq" },
	{ 0x41fdc1e4, "module_layout" },
};

MODULE_INFO(depends, "nvme-fabrics");

