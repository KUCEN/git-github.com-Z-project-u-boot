/*
 * (C) Copyright 2016
 * Alexander Graf <agraf@suse.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SETJMP_H_
#define _SETJMP_H_	1

struct jmp_buf_data {
	ulong target;
	ulong regs[5];
};

typedef struct jmp_buf_data jmp_buf[1];

static inline int setjmp(jmp_buf jmp)
{
	long r = 0;

#ifdef CONFIG_ARM64
	asm volatile(
		"adr x1, jmp_target\n"
		"str x1, %1\n"
		"stp x26, x27, %2\n"
		"stp x28, x29, %3\n"
		"mov x1, sp\n"
		"str x1, %4\n"
		"b 2f\n"
		"jmp_target: "
		"mov %0, #1\n"
		"2:\n"
		: "+r" (r), "=m" (jmp->target),
		  "=m" (jmp->regs[0]), "=m" (jmp->regs[2]),
		  "=m" (jmp->regs[4])
		:
		: "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
		  "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
		  "x16", "x17", "x18", "x19", "x20", "x21", "x22",
		  "x23", "x24", "x25", /* x26, x27, x28, x29, sp */
		  "x30", "cc", "memory");
#else
	asm volatile(
#ifdef CONFIG_SYS_THUMB_BUILD
		"adr r0, jmp_target + 1\n"
#else
		"adr r0, jmp_target\n"
#endif
		"mov r1, %1\n"
		"mov r2, sp\n"
		"stm r1, {r0, r2, r4, r5, r6, r7}\n"
		"b 2f\n"
		"jmp_target: "
		"mov %0, #1\n"
		"2:\n"
		: "+l" (r)
		: "l" (&jmp->target)
		: "r0", "r1", "r2", "r3", /* "r4", "r5", "r6", "r7", */
		  "r8", "r9", "r10", "r11", /* sp, */ "ip", "lr",
		  "cc", "memory");
#endif

printf("%s:%d target=%#lx\n", __func__, __LINE__, jmp->target);

	return r;
}

static inline __noreturn void longjmp(jmp_buf jmp)
{
#ifdef CONFIG_ARM64
	asm volatile(
		"ldr x0, %0\n"
		"ldr x1, %3\n"
		"mov sp, x1\n"
		"ldp x26, x27, %1\n"
		"ldp x28, x25, %2\n"
		"mov x29, x25\n"
		"br x0\n"
		:
		: "m" (jmp->target), "m" (jmp->regs[0]), "m" (jmp->regs[2]),
		  "m" (jmp->regs[4])
		: "x0", "x1", "x25", "x26", "x27", "x28");
#else
	asm volatile(
		"mov r1, %0\n"
		"ldm r1, {r0, r2, r4, r5, r6, r7}\n"
		"mov sp, r2\n"
		"bx r0\n"
		:
		: "l" (&jmp->target)
		: "r1");
#endif

	while (1) { }
}


#endif /* _SETJMP_H_ */
