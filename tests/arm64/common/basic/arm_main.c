/**
 * Copyright (c) 2012 Jean-Christophe Dubois.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * @file arm_main.c
 * @author Jean-Christophe Dubois (jcd@tribudubois.net)
 * @brief Basic firmware main file
 */

#include <arm_cache.h>
#include <arm_mmu.h>
#include <arm_irq.h>
#include <arch_math.h>
#include <arm_board.h>
#include <basic_heap.h>
#include <basic_stdio.h>
#include <basic_string.h>
#include <dhry.h>
#include <libfdt/libfdt.h>
#include <libfdt/fdt_support.h>

static unsigned long memory_size = 0x0;

unsigned long arm_linux_memory_size(void)
{
	return memory_size;
}

/* Works in supervisor mode */
void arm_init(void)
{
	basic_heap_init();

	arm_irq_disable();

	arm_irq_setup();

	basic_stdio_init();

	arm_board_timer_init(10000);

	arm_board_init();

	memory_size = arm_board_ram_size();

	arm_board_timer_enable();

	arm_irq_enable();
}

void arm_cmd_help(int argc, char **argv)
{
	basic_puts("help        - List commands and their usage\n");
	basic_puts("\n");
	basic_puts("hi          - Say hi to ARM test code\n");
	basic_puts("\n");
	basic_puts("hello       - Say hello to ARM test code\n");
	basic_puts("\n");
	basic_puts("wfi_test    - Run wait for irq instruction test for ARM test code\n");
	basic_puts("            Usage: wfi_test [<msecs>]\n");
	basic_puts("            <msecs>  = delay in milliseconds to wait for\n");
	basic_puts("\n");
	basic_puts("mmu_setup   - Setup MMU for ARM test code\n");
	basic_puts("\n");
	basic_puts("mmu_state   - MMU is enabled/disabled for ARM test code\n");
	basic_puts("\n");
	basic_puts("mmu_test    - Run MMU test suite for ARM test code\n");
	basic_puts("\n");
	basic_puts("mmu_cleanup - Cleanup MMU for ARM test code\n");
	basic_puts("\n");
	basic_puts("timer       - Display timer information\n");
	basic_puts("\n");
	basic_puts("dhrystone   - Dhrystone 2.1 benchmark\n");
	basic_puts("            Usage: dhrystone [<iterations>]\n");
	basic_puts("\n");
	basic_puts("hexdump     - Dump memory contents in hex format\n");
	basic_puts("            Usage: hexdump <addr> <count>\n");
	basic_puts("            <addr>  = memory address in hex\n");
	basic_puts("            <count> = byte count in hex\n");
	basic_puts("\n");
	basic_puts("copy        - Copy to target memory from source memory\n");
	basic_puts("            Usage: copy <dest> <src> <count>\n");
	basic_puts("            <dest>  = destination address in hex\n");
	basic_puts("            <src>   = source address in hex\n");
	basic_puts("            <count> = byte count in hex\n");
	basic_puts("\n");
	basic_puts("start_linux - Start linux kernel (device-tree mechanism)\n");
	basic_puts("            Usage: start_linux <kernel_addr> <fdt_addr> [<initrd_addr>] [<initrd_size>]\n");
	basic_puts("            <kernel_addr>  = kernel load address\n");
	basic_puts("            <fdt_addr>     = fdt blob address\n");
	basic_puts("            <initrd_addr>  = initrd load address (optional)\n");
	basic_puts("            <initrd_size>  = initrd size (optional)\n");
	basic_puts("\n");
	basic_puts("fdt_override_u32 - Overrides an integer property in the device tree\n");
	basic_puts("            Usage: fdt_override_u32 <fdt_addr> </path/to/property> <value>\n");
	basic_puts("\n");
	basic_puts("linux_cmdline - Show/Update linux command line\n");
	basic_puts("            Usage: linux_cmdline [<new_linux_cmdline>]\n");
	basic_puts("            <new_linux_cmdline>  = linux command line\n");
	basic_puts("\n");
	basic_puts("linux_memory_size - Show/Update linux memory size\n");
	basic_puts("            Usage: linux_memory_size [<memory_size>]\n");
	basic_puts("            <memory_size>  = memory size in hex\n");
	basic_puts("\n");
	basic_puts("autoexec    - autoexec command list from flash\n");
	basic_puts("            Usage: autoexec\n");
	basic_puts("\n");
	basic_puts("go          - Jump to a given address\n");
	basic_puts("            Usage: go <addr>\n");
	basic_puts("            <addr>  = jump address in hex\n");
	basic_puts("\n");
	basic_puts("reset       - Reset the system\n");
	basic_puts("\n");
}

void arm_cmd_hi(int argc, char **argv)
{
	if (argc != 1) {
		basic_puts("hi: no parameters required\n");
		return;
	}

	basic_puts("hello\n");
}

void arm_cmd_hello(int argc, char **argv)
{
	if (argc != 1) {
		basic_puts("hello: no parameters required\n");
		return;
	}

	basic_puts("hi\n");
}

void arm_cmd_wfi_test(int argc, char **argv)
{
	u64 tstamp;
	char time[256];
	int delay = 1000;

	if (argc > 2) {
		basic_puts("wfi_test: could provide only <delay>\n");
		return;
	} else if (argc == 2) {
		delay = basic_str2int(argv[1]);
	}

	basic_puts("Executing WFI instruction\n");
	arm_board_timer_disable();
	arm_board_timer_change_period(delay*1000);
	arm_board_timer_enable();
	tstamp = arm_board_timer_timestamp();
	arm_irq_wfi();
	tstamp = arm_board_timer_timestamp() - tstamp;
	arm_board_timer_disable();
	arm_board_timer_change_period(10000);
	arm_board_timer_enable();
	basic_puts("Resumed from WFI instruction\n");
	basic_puts("Time spent in WFI: ");
	basic_ulonglong2str(time, tstamp);
	basic_puts(time);
	basic_puts(" nsecs\n");
}

#if 0
void arm_cmd_mmu_setup(int argc, char **argv)
{
	if (argc != 1) {
		basic_puts("mmu_setup: no parameters required\n");
		return;
	}

	arm_mmu_setup();
}
#endif

void arm_cmd_mmu_state(int argc, char **argv)
{
	if (argc != 1) {
		basic_puts("mmu_state: no parameters required\n");
		return;
	}

	if (arm_mmu_is_enabled()) {
		basic_puts("MMU Enabled\n");
	} else {
		basic_puts("MMU Disabled\n");
	}
}

#if 0
void arm_cmd_mmu_test(int argc, char **argv)
{
	char str[32];
	u32 total = 0x0, pass = 0x0, fail = 0x0;

	if (argc != 1) {
		basic_puts("mmu_test: no parameters required\n");
		return;
	}

	basic_puts("MMU Section Test Suite ...\n");
	total = 0x0;
	pass = 0x0;
	fail = 0x0;
	arm_mmu_section_test(&total, &pass, &fail);
	basic_puts("  Total: ");
	basic_int2str(str, total);
	basic_puts(str);
	basic_puts("\n");
	basic_puts("  Pass : ");
	basic_int2str(str, pass);
	basic_puts(str);
	basic_puts("\n");
	basic_puts("  Fail : ");
	basic_int2str(str, fail);
	basic_puts(str);
	basic_puts("\n");
	basic_puts("MMU Page Test Suite ...\n");
	total = 0x0;
	pass = 0x0;
	fail = 0x0;
	arm_mmu_page_test(&total, &pass, &fail);
	basic_puts("  Total: ");
	basic_int2str(str, total);
	basic_puts(str);
	basic_puts("\n");
	basic_puts("  Pass : ");
	basic_int2str(str, pass);
	basic_puts(str);
	basic_puts("\n");
	basic_puts("  Fail : ");
	basic_int2str(str, fail);
	basic_puts(str);
	basic_puts("\n");
}
#endif

void arm_cmd_mmu_cleanup(int argc, char **argv)
{
	if (argc != 1) {
		basic_puts("mmu_cleanup: no parameters required\n");
		return;
	}

	arm_mmu_cleanup();
}

void arm_cmd_timer(int argc, char **argv)
{
	char str[32];
	u64 irq_count, irq_delay, tstamp;

	if (argc != 1) {
		basic_puts("timer: no parameters required\n");
		return;
	}

	irq_count = arm_board_timer_irqcount();
	irq_delay = arm_board_timer_irqdelay();
	tstamp = arm_board_timer_timestamp();
	basic_puts("Timer Information ...\n");
	basic_puts("  IRQ Count:  0x");
	basic_ulonglong2hexstr(str, irq_count);
	basic_puts(str);
	basic_puts("\n");
	basic_puts("  IRQ Delay:  ");
	basic_ulonglong2str(str, irq_delay);
	basic_puts(str);
	basic_puts(" nsecs\n");
	basic_puts("  Time Stamp: 0x");
	basic_ulonglong2hexstr(str, tstamp);
	basic_puts(str);
	basic_puts("\n");
}

void arm_cmd_dhrystone(int argc, char **argv)
{
	char str[32];
	int iters = 1000000;
	if (argc > 2) {
		basic_puts("dhrystone: could provide only <iter_number>\n");
		return;
	} else if (argc == 2) {
		iters = basic_str2int(argv[1]);
	} else {
		basic_puts("dhrystone: number of iterations not provided\n");
		basic_puts("dhrystone: using default ");
		basic_int2str (str, iters);
		basic_puts(str);
		basic_puts(" iterations\n");
	}
	arm_board_timer_disable();
	dhry_main(iters);
	arm_board_timer_enable();
}

void arm_cmd_hexdump(int argc, char **argv)
{
	char str[32];
	u32 *addr;
	u32 i, count, len;
	if (argc != 3) {
		basic_puts("hexdump: must provide <addr> and <count>\n");
		return;
	}
	addr = (u32 *)basic_hexstr2ulonglong(argv[1]);
	count = basic_hexstr2uint(argv[2]);
	for (i = 0; i < (count / 4); i++) {
		if (i % 4 == 0) {
			basic_ulonglong2hexstr(str, (u64)&addr[i]);
			len = basic_strlen(str);
			while (len < 8) {
				basic_puts("0");
				len++;
			}
			basic_puts(str);
			basic_puts(": ");
		}
		basic_uint2hexstr(str, addr[i]);
		len = basic_strlen(str);
		while (len < 8) {
			basic_puts("0");
			len++;
		}
		basic_puts(str);
		if (i % 4 == 3) {
			basic_puts("\n");
		} else {
			basic_puts(" ");
		}
	}
	basic_puts("\n");
}

void arm_cmd_copy(int argc, char **argv)
{
	u64 tstamp;
	char time[256];
	void *dst, *src;
	virtual_addr_t dest_va;
	u32 i, count;

	/* Determine copy args */
	if (argc != 4) {
		basic_puts("copy: must provide <dest>, <src>, and <count>\n");
		return;
	}
	dst = (void *)basic_hexstr2ulonglong(argv[1]);
	dest_va = (virtual_addr_t)dst;
	src = (void *)basic_hexstr2ulonglong(argv[2]);
	count = basic_hexstr2uint(argv[3]);

	/* Disable timer and get start timestamp */
	arm_board_timer_disable();
	tstamp = arm_board_timer_timestamp();

	/* It might happen that we are running Basic firmware
	 * after a reboot from Guest Linux in which case both
	 * I-Cache and D-Cache will have stale contents. We need
	 * to cleanup these stale contents while copying so that
	 * we see correct contents of destination even after
	 * MMU ON.
	 */
	arm_clean_invalidate_dcache_mva_range(dest_va, dest_va + count);

	/* Copy contents */
	if (!((virtual_addr_t)dst & 0x7) &&
	    !((virtual_addr_t)src & 0x7) &&
	    !(count & 0x7)) {
		for (i = 0; i < (count/sizeof(u64)); i++) {
			((u64 *)dst)[i] = ((u64 *)src)[i];
		}
	} else if (!((virtual_addr_t)dst & 0x3) &&
		   !((virtual_addr_t)src & 0x3) &&
		   !(count & 0x3)) {
		for (i = 0; i < (count/sizeof(u32)); i++) {
			((u32 *)dst)[i] = ((u32 *)src)[i];
		}
	} else if (!((virtual_addr_t)dst & 0x1) &&
		   !((virtual_addr_t)src & 0x1) &&
		   !(count & 0x1)) {
		for (i = 0; i < (count/sizeof(u16)); i++) {
			((u16 *)dst)[i] = ((u16 *)src)[i];
		}
	} else {
		for (i = 0; i < (count/sizeof(u8)); i++) {
			((u8 *)dst)[i] = ((u8 *)src)[i];
		}
	}

	/* Enable timer and get end timestamp */
	tstamp = arm_board_timer_timestamp() - tstamp;
	tstamp = arch_udiv64(tstamp, 1000);
	arm_board_timer_enable();

	/* Print time taken */
	basic_ulonglong2str(time, tstamp);
	basic_puts("copy took ");
	basic_puts(time);
	basic_puts(" usecs for ");
	basic_puts(argv[3]);
	basic_puts(" bytes\n");
}

char linux_cmdline[1024];

typedef void (* linux_entry_t) (u64 fdt_addr, u64 arg0, u64 arg1, u64 arg2);

void dump_fdt(void *);

void arm_cmd_start_linux(int argc, char **argv)
{
	u64 kernel_addr, fdt_addr;
	u64 initrd_addr, initrd_size;
	virtual_addr_t nuke_va;
	int err;
	char cfg_str[10];
	u64 meminfo[2];

	if (argc < 3) {
		basic_puts("start_linux: must provide <kernel_addr> and "
			 "<fdt_addr>\n");
		basic_puts("start_linux: <initrd_addr> and <initrd_size> "
			 "are optional\n");
		return;
	}

	/* Parse the arguments from command line */
	kernel_addr = basic_hexstr2ulonglong(argv[1]);
	fdt_addr = basic_hexstr2ulonglong(argv[2]);
	if (argc > 3) {
		initrd_addr = basic_hexstr2ulonglong(argv[3]);
	} else {
		initrd_addr = 0;
	}
	if (argc > 4) {
		initrd_size = basic_hexstr2ulonglong(argv[4]);
	} else {
		initrd_size = 0;
	}

	/* Linux ARM64 kernel expects us to boot from 0x80000
	 * aligned address, perferrably RAM start + 0x80000 address.
	 * The 0x80000 bytes above kernel start address is used by
	 * Linux ARM64 kernel to setup boot page tables.
	 *
	 * It might happen that we are running Basic firmware
	 * after a reboot from Guest Linux in which case both
	 * I-Cache and D-Cache will have stale contents. If we
	 * don't cleanup these stale contents then Linux kernel
	 * will not see correct contents boot page tables after
	 * MMU ON.
	 *
	 * To take care of above described issue, we nuke the
	 * 2MB area containing kernel start and boot page tables.
	 */
	nuke_va = kernel_addr & ~(0x200000 - 1);
	arm_clean_invalidate_dcache_mva_range(nuke_va, nuke_va + 0x200000);

	/* Disable interrupts, disable timer and cleanup MMU */
	arm_board_timer_disable();
	arm_irq_disable();
	arm_mmu_cleanup();

	/* Increase fdt blob size by 8KB */
	fdt_increase_size((void *)fdt_addr, 0x2000);

	meminfo[0] = arm_board_ram_start();
	meminfo[1] = arm_board_ram_size();
	/* Fillup/fixup the fdt blob with following:
	 * 		- initrd start, end
	 * 		- kernel cmd line
	 * 		- number of cpus   */
	if ((err = fdt_fixup_memory_banks((void *)fdt_addr, (&meminfo[0]), 
							(&meminfo[1]), 1))) {
		basic_printf("%s: fdt_fixup_memory_banks() failed: %s\n",
			   __func__, fdt_strerror(err));
		return;
	}
	sprintf(cfg_str, " mem=%dM", (int)(meminfo[1] >> 20));
	basic_strcat(linux_cmdline, cfg_str);
	if ((err = fdt_chosen((void *)fdt_addr, 1, linux_cmdline))) {
		basic_printf("%s: fdt_chosen() failed: %s\n", __func__, 
				fdt_strerror(err));
		return;
	}
	if (initrd_size) {
		if ((err = fdt_initrd((void *)fdt_addr, initrd_addr, 
					initrd_addr + initrd_size, 1))) {
			basic_printf("%s: fdt_initrd() failed: %s\n",
				   __func__, fdt_strerror(err));
			return;
		}
	}

	/* Do board specific fdt fixup */
	arm_board_fdt_fixup((void *)fdt_addr);

	/* Jump to Linux Kernel
	 * r0 -> dtb address
	 */
	basic_puts("Jumping into linux ...\n");
	((linux_entry_t)kernel_addr)(fdt_addr, 0x0, 0x0, 0x0);

	/* We should never reach here */
	while (1);

	return;
}

void arm_cmd_fdt_override_u32(int argc, char **argv)
{
	const char *path;
	u32 val;
	int err;
	int nodeoffset;
	char *prop;
	void *fdt;
	const struct fdt_property *p;

	if (argc < 4) {
		basic_puts("fdt_override_u32: must provide <fdt_addr> </path/to/property> and <value>\n");
		return;
	}

	fdt = (void *)basic_hexstr2ulonglong(argv[1]);
	path = argv[2];
	val = cpu_to_be32(basic_str2int(argv[3]));
	prop = basic_strrchr(path, '/');
	if (!prop) {
		basic_puts("*** Failed to parse node\n");
		return;
	}
	*(prop++) = '\0';

	nodeoffset = fdt_path_offset(fdt, path);
	if (nodeoffset < 0) {
		printf("*** Path \"%s\" does not exist\n", path);
		return;
	}

	p = fdt_get_property(fdt, nodeoffset, prop, NULL);
	if (!p) {
		printf("*** Failed to find property \"%s\" of node \"%s\"\n",
		       prop, path);
		return;
	}

	err = fdt_setprop(fdt, nodeoffset, prop, &val, sizeof(u32));
	if (err) {
		printf("*** Failed to set property \"%s\" of node \"%s\". "
		       "Error: %i\n", prop, path, err);
		return;
	}
}

void arm_cmd_linux_cmdline(int argc, char **argv)
{
	if (argc >= 2) {
		int cnt = 1;
		linux_cmdline[0] = 0;

		while (cnt < argc) {
			basic_strcat(linux_cmdline, argv[cnt]);
			basic_strcat(linux_cmdline, " ");
			cnt++;
		}
	}

	basic_puts("linux_cmdline = \"");
	basic_puts(linux_cmdline);
	basic_puts("\"\n");

	return;
}

void arm_cmd_linux_memory_size(int argc, char **argv)
{
	char str[32];

	if (argc == 2) {
		memory_size = (u32)basic_hexstr2uint(argv[1]);
	}

	basic_puts("linux_memory_size = 0x");
	basic_uint2hexstr(str, memory_size);
	basic_puts(str);
	basic_puts(" Bytes\n");

	return;
}

void arm_exec(char *line);

void arm_cmd_autoexec(int argc, char **argv)
{
#define ARM_CMD_AUTOEXEC_BUF_SIZE	4096
	static int lock = 0;
	int len, pos = 0;
	char *ptr = (char *)(arm_board_autoexec_addr());
	char buffer[ARM_CMD_AUTOEXEC_BUF_SIZE];

	if (argc != 1) {
		basic_puts("autoexec: no parameters required\n");
		return;
	}

	/* autoexec is not recursive */
	if (lock) {
		basic_puts("ignoring autoexec calling autoexec\n");
		return;
	}

	lock = 1;

	/* determine length of command list */
	len = 0;
	while ((len < ARM_CMD_AUTOEXEC_BUF_SIZE) &&
	       basic_isprintable(ptr[len])) {
		len++;
	}

	/* sanity check on command list */
	if (!len) {
		basic_puts("command list not found !!!\n");
		goto unlock;
	}
	if (len >= ARM_CMD_AUTOEXEC_BUF_SIZE) {
		basic_printf("command list len=%d too big !!!\n", len);
		goto unlock;
	}

	/* copy commands from NOR flash */
	basic_memcpy(buffer, ptr, len);
	buffer[len] = '\0';

	/* now we process them */
	while (pos < len) {
		ptr = &buffer[pos];

		/* We need to separate the commands */
		while ((buffer[pos] != '\r') &&
			(buffer[pos] != '\n') &&
			(buffer[pos] != 0)) {
			pos++;
		}
		buffer[pos] = '\0';
		pos++;

		/* print the command */
		basic_puts("autoexec(");
		basic_puts(ptr);
		basic_puts(")\n");
		/* execute it */
		arm_exec(ptr);
	}

unlock:
	lock = 0;

	return;
}

void arm_cmd_go(int argc, char **argv)
{
	char str[32];
	void (* jump)(void);

	if (argc != 2) {
		basic_puts("go: must provide destination address\n");
		return;
	}

	arm_board_timer_disable();

	jump = (void (*)(void))basic_hexstr2ulonglong(argv[1]);
	basic_ulonglong2hexstr(str, (u64)jump);
	basic_puts("Jumping to location 0x");
	basic_puts(str);
	basic_puts(" ...\n");
	jump ();

	arm_board_timer_enable();
}

void arm_cmd_reset(int argc, char **argv)
{
	if (argc != 1) {
		basic_puts("reset: no parameters required\n");
		return;
	}

	basic_puts("System reset ...\n\n");

	arm_board_reset();

	while (1);
}

#define ARM_MAX_ARG_SIZE	32

void arm_exec(char *line)
{
	int argc = 0, pos = 0, cnt = 0;
	char *argv[ARM_MAX_ARG_SIZE];

	while (line[pos] && (argc < ARM_MAX_ARG_SIZE)) {
		if ((line[pos] == '\r') ||
		    (line[pos] == '\n')) {
			line[pos] = '\0';
			break;
		}
		if (line[pos] == ' ') {
			if (cnt > 0) {
				line[pos] = '\0';
				cnt = 0;
			}
		} else {
			if (cnt == 0) {
				argv[argc] = &line[pos];
				argc++;
			}
			cnt++;
		}
		pos++;
	}

	if (argc) {
		if (basic_strcmp(argv[0], "help") == 0) {
			arm_cmd_help(argc, argv);
		} else if (basic_strcmp(argv[0], "hi") == 0) {
			arm_cmd_hi(argc, argv);
		} else if (basic_strcmp(argv[0], "hello") == 0) {
			arm_cmd_hello(argc, argv);
		} else if (basic_strcmp(argv[0], "wfi_test") == 0) {
			arm_cmd_wfi_test(argc, argv);
#if 0
		} else if (basic_strcmp(argv[0], "mmu_setup") == 0) {
			arm_cmd_mmu_setup(argc, argv);
#endif
		} else if (basic_strcmp(argv[0], "mmu_state") == 0) {
			arm_cmd_mmu_state(argc, argv);
#if 0
		} else if (basic_strcmp(argv[0], "mmu_test") == 0) {
			arm_cmd_mmu_test(argc, argv);
#endif
		} else if (basic_strcmp(argv[0], "mmu_cleanup") == 0) {
			arm_cmd_mmu_cleanup(argc, argv);
		} else if (basic_strcmp(argv[0], "timer") == 0) {
			arm_cmd_timer(argc, argv);
		} else if (basic_strcmp(argv[0], "dhrystone") == 0) {
			arm_cmd_dhrystone(argc, argv);
		} else if (basic_strcmp(argv[0], "hexdump") == 0) {
			arm_cmd_hexdump(argc, argv);
		} else if (basic_strcmp(argv[0], "copy") == 0) {
			arm_cmd_copy(argc, argv);
		} else if (basic_strcmp(argv[0], "start_linux") == 0) {
			arm_cmd_start_linux(argc, argv);
                } else if (basic_strcmp(argv[0], "fdt_override_u32") == 0) {
			arm_cmd_fdt_override_u32(argc, argv);
		} else if (basic_strcmp(argv[0], "linux_cmdline") == 0) {
			arm_cmd_linux_cmdline(argc, argv);
		} else if (basic_strcmp(argv[0], "linux_memory_size") == 0) {
			arm_cmd_linux_memory_size(argc, argv);
		} else if (basic_strcmp(argv[0], "autoexec") == 0) {
			arm_cmd_autoexec(argc, argv);
		} else if (basic_strcmp(argv[0], "go") == 0) {
			arm_cmd_go(argc, argv);
		} else if (basic_strcmp(argv[0], "reset") == 0) {
			arm_cmd_reset(argc, argv);
		} else {
			basic_puts("Unknown command\n");
		}
	}
}

#define ARM_MAX_CMD_STR_SIZE	256

/* Works in user mode */
void arm_main(void)
{
	u64 tstamp;
	u32 i, key_pressed, boot_delay;
	char line[ARM_MAX_CMD_STR_SIZE];

	/* Setup board specific linux default cmdline */
	arm_board_linux_default_cmdline(linux_cmdline, 
					sizeof(linux_cmdline));

	basic_puts(arm_board_name());
	basic_puts(" Basic Firmware\n\n");

	boot_delay = arm_board_boot_delay();
	if (boot_delay == 0xffffffff) {
		basic_puts("autoboot: disabled\n\n");
	} else {
		basic_puts("autoboot: enabled\n");
		while (boot_delay) {
			basic_int2str(line, (int)boot_delay);
			basic_puts("autoboot: waiting for ");
			basic_puts(line);
			basic_puts(" secs (press any key)\n");
			key_pressed = 0;
			tstamp = arm_board_timer_timestamp();
			while ((arm_board_timer_timestamp() - tstamp)
							< 1000000000) {
				for (i = 0; i < 10000; i++) ;
				if (basic_can_getc()) {
					basic_getc();
					key_pressed = 1;
					break;
				}
			}
			if (key_pressed)
				break;
			boot_delay--;
		}
		basic_puts("\n");
		if (!boot_delay) {
			basic_strcpy(line, "autoexec\n");
			arm_exec(line);
		}
	}

	while(1) {
		basic_puts("basic# ");

		basic_gets(line, ARM_MAX_CMD_STR_SIZE, '\n');

		arm_exec(line);
	}
}
