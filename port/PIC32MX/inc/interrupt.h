// 2015 Adam Jesionowski

#ifndef INTERRUPT_H_
#define INTERRUPT_H_

/*
 * This file defines the context switching assembly routines for the PIC32MX.
 * It handles switching between the various task and OS stacks,and storing the contents
 * of the PIC registers on those stacks.
 */

/*
 * The code for this file was developed by reading similar code from FreeRTOS and https://github.com/andersm/TNKernel-PIC32,
 * then trying to recreate it. Due to their two different licenses, and my own uncertainty on how to determine what constitues
 * a derived work, this file is left unlicensed, but I want to acknowledge the potential IP issue.
 * Please let me know if there is an issue and how best to resolve it.
 */

// CPU registers:
// zero: always zero
// at: assembler temporary
// v0-v1: function return values
// a0-a3: function arguments
// t0-t7: temporary
// s0-s7: saved temporary
// t8-t9: temporary
// k0-k1: kernel temporary
// gp: global pointer
// sp: stack pointer
// fp: frame pointer
// ra: return address

// Others:
// PC: program counter
// hi: MAD hi
// lo: MAD lo
//status - register 12 in coprocessor 0, stores interrupt mask and enable bits
//cause - register 13 in coprocessor 0, stores exception type and pending interrupt bits
//epc - register 14 in coprocessor 0, stores address of instruction causing exception

#define STACK_SIZE 128

.extern IntCount
.extern TaskStackPtr
.extern OSStackPtr

.macro SAVE_REGISTERS
    // Make room to save the context
    addiu	sp, sp, -STACK_SIZE

    // Save registers to the stack
    sw		s0, 124(sp)
    sw		s1, 120(sp)
    sw		s2, 116(sp)
    sw		s3, 112(sp)
    sw		s4, 108(sp)
    sw		s5, 104(sp)
    sw		s6, 100(sp)
    sw		s7, 96(sp)
    sw		$1, 92(sp) // at
    sw		v0, 88(sp)
    sw		v1, 84(sp)
    sw		a0, 80(sp)
    sw		a1, 76(sp)
    sw		a2, 72(sp)
    sw		a3, 68(sp)
    sw		t0, 64(sp)
    sw		t1, 60(sp)
    sw		t2, 56(sp)
    sw		t3, 52(sp)
    sw		t4, 48(sp)
    sw		t5, 44(sp)
    sw		t6, 40(sp)
    sw		t7, 36(sp)
    sw		t8, 32(sp)
    sw		t9, 28(sp)
    sw		s8, 24(sp)
    sw		ra, 20(sp)

    // Use kernel temporary registers for temp storing CP0 registers
    mfhi	k0
    mflo	k1
    sw		k0, 16(sp)
    sw		k1, 12(sp)

    mfc0	k0, _CP0_CAUSE
    mfc0	k1, _CP0_STATUS

    sw		k0, 8(sp)
    sw		k1, 4(sp)

    mfc0	k1, _CP0_EPC
    sw		k1, 0(sp)

    // 0 means we're in a task, save to that task's stack then switch to OS stack
    // 1 or above means we're in an interrupt, don't do anything
    lw		k0, IntCount

    // No need to switch if we're above zero
    bne		k0, zero, 1f
    nop

    // Preserve the old sp
    add		k1, zero, sp
    sw		k1, TaskStackPtr

    // Swap from the current stack pointer to the OS stack
    lw		sp, OSStackPtr

1:
    // Increment interrupt count
    addi	k0, k0, 1
    sw		k0, IntCount

    // Alright, with our stack set up we can re-enable interrupts above the current priority
    // Cause Bits 10-12 are Requested Interrupt Priority Level bits
    // Status Bits 10-12 are Interrupt Priority Level bits
    // We want to move the cause bits to status
    mfc0	k0, _CP0_CAUSE
    mfc0	k1, _CP0_STATUS

    // Extract the RIPL
    ext		k0, k0, 10, 3

    // Insert it into status
    ins		k1, k0, 10, 3

    // Also zero out UM, ERL, and EXL (enables ints)
    ins		k1, zero, 1, 4

    // Set the register, enabling interrupts
    mtc0 	k1, _CP0_STATUS

    // Continue on to our interrupt
    .endm


.macro LOAD_REGISTERS
    // Start by disabling interrupts
    di

    // Load the interrupt count to see if we're going to need to switch back stack pointers
    lw		k0, IntCount

    // Decrement nesting count and store the new value
    addiu	k0, k0, -1
    sw		k0, IntCount

    // 0 means we're in a task, and we're going to restore that task's stack
    // 1 and above means we're in an interrupt, and are going to restore the OS stack
    bne		k0, zero, 1f
    nop

    // IntCount is zero, switch the sp to TaskStackPtr
    // This is also where context can switch sp from one task to the next
    lw		sp, TaskStackPtr

1:
    // Then, restore our registers from the stack
    lw		s0, 124(sp)
    lw		s1, 120(sp)
    lw		s2, 116(sp)
    lw		s3, 112(sp)
    lw		s4, 108(sp)
    lw		s5, 104(sp)
    lw		s6, 100(sp)
    lw		s7, 96(sp)
    lw		$1, 92(sp) // at
    lw		v0, 88(sp)
    lw		v1, 84(sp)
    lw		a0, 80(sp)
    lw		a1, 76(sp)
    lw		a2, 72(sp)
    lw		a3, 68(sp)
    lw		t0, 64(sp)
    lw		t1, 60(sp)
    lw		t2, 56(sp)
    lw		t3, 52(sp)
    lw		t4, 48(sp)
    lw		t5, 44(sp)
    lw		t6, 40(sp)
    lw		t7, 36(sp)
    lw		t8, 32(sp)
    lw		t9, 28(sp)
    lw		s8, 24(sp)
    lw		ra, 20(sp)

    lw		k0, 16(sp)
    lw		k1, 12(sp)
    mthi	k0
    mtlo	k1

    lw		k0, 8(sp)
    mtc0	k0, _CP0_CAUSE

    lw		k0, 0(sp)
    mtc0	k0, _CP0_EPC

    // Status should be done last
    lw		k1, 4(sp)

    // We're done with the stack space
    addiu	sp, sp, STACK_SIZE

    // Restore interrupts
    mtc0	k1, _CP0_STATUS

    eret
    nop

    .endm

/*
 * Using interrupts should look like this:
 * In a .c file (see port.c), declare the ISR function and the function that will be called
 * void __ISR(_TIMER_1_VECTOR, ipl1) TickTimerInterruptWrapper(void);
 * void TickTimerInterrupt(){}
 *
 * In a .S file, use this macro:
 * isr_macro TickTimerInterrupt TickTimerInterruptWrapper
 */
.macro isr_macro isr_name wrapper_name
    .set nomips16
    .set noreorder

    .extern \isr_name

    .global \wrapper_name
    .ent \wrapper_name
    \wrapper_name:
        SAVE_REGISTERS

        jal \isr_name
        nop

        LOAD_REGISTERS
        .end \wrapper_name
    .endm

#endif /* INTERRUPT_H_ */
