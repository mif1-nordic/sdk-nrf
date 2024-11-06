	.file	"hrt.c"
	.option nopic
	.attribute arch, "rv32e1p9_m2p0_c2p0_zicsr2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 4
	.text
	.section	.text.write_single_by_word,"ax",@progbits
	.align	1
	.globl	write_single_by_word
	.type	write_single_by_word, @function
write_single_by_word:
	addi	sp,sp,-16
	sw	s0,12(sp)
 #APP
	csrr t1, 3009
 #NO_APP
	slli	t1,t1,16
	srli	t1,t1,16
	ori	t1,t1,35
 #APP
	csrw 3009, t1
	csrr t1, 3008
 #NO_APP
	slli	t1,t1,16
	srli	t1,t1,16
	ori	t1,t1,32
 #APP
	csrw 3008, t1
 #NO_APP
	li	t1,4
	sw	t1,4(sp)
	li	t1,1
	sh	t1,8(sp)
	li	t1,65536
	addi	t1,t1,4
 #APP
	csrw 3043, t1
	csrw 3045, 0
	csrr t1, 1996
 #NO_APP
	andi	t2,t1,1
	sb	t2,0(sp)
	srli	t0,t1,4
	andi	t0,t0,1
	sb	t0,1(sp)
	srli	t1,t1,8
	andi	t1,t1,1
	sb	t1,2(sp)
	sb	zero,2(sp)
	slli	t0,t0,4
	or	t2,t2,t0
 #APP
	csrw 1996, t2
 #NO_APP
	li	t1,31
	bleu	a3,t1,.L10
.L3:
 #APP
	csrw 2000, 2
 #NO_APP
	slli	t1,a2,16
	srli	t1,t1,16
 #APP
	csrr t0, 2003
 #NO_APP
	li	t2,-65536
	and	t0,t0,t2
	or	a2,t1,t0
 #APP
	csrw 2003, a2
	csrw 3022, a3
 #NO_APP
	addi	a3,a3,-1
	andi	a3,a3,0xff
 #APP
	csrw 3023, a3
	csrr a3, 3008
 #NO_APP
	slli	a3,a3,16
	srli	a3,a3,16
	andi	a3,a3,-33
	slli	a3,a3,16
	srli	a3,a3,16
	beq	a4,zero,.L11
	li	a2,32
.L5:
	or	a3,a2,a3
 #APP
	csrw 3008, a3
 #NO_APP
	slli	a3,t1,1
	add	t1,t1,a3
	slli	t1,t1,16
	srli	t1,t1,16
 #APP
	csrw 2005, t1
 #NO_APP
	li	a3,0
	j	.L6
.L10:
	li	t1,0
.L2:
	bgeu	t1,a1,.L3
	slli	t0,t1,2
	add	t0,a0,t0
	lw	t2,0(t0)
	li	s0,32
	sub	s0,s0,a3
	sll	t2,t2,s0
	sw	t2,0(t0)
	addi	t1,t1,1
	andi	t1,t1,0xff
	j	.L2
.L11:
	li	a2,0
	j	.L5
.L7:
	slli	a2,a3,2
	add	a2,a0,a2
	lw	a2,0(a2)
 #APP
	csrw 3016, a2
 #NO_APP
	addi	a3,a3,1
	andi	a3,a3,0xff
.L6:
	bltu	a3,a1,.L7
 #APP
	csrw 3012, 0
 #NO_APP
	sw	zero,4(sp)
	lhu	a3,8(sp)
	slli	a3,a3,16
	li	a2,2031616
	and	a3,a3,a2
 #APP
	csrw 3043, a3
	csrw 3045, 0
 #NO_APP
	bne	a5,zero,.L8
 #APP
	csrr a5, 3008
 #NO_APP
	slli	a5,a5,16
	srli	a5,a5,16
	andi	a5,a5,-34
	slli	a5,a5,16
	srli	a5,a5,16
	beq	a4,zero,.L12
	li	a4,0
.L9:
	or	a5,a4,a5
 #APP
	csrw 3008, a5
 #NO_APP
.L8:
 #APP
	csrw 2000, 0
 #NO_APP
	lw	s0,12(sp)
	addi	sp,sp,16
	jr	ra
.L12:
	li	a4,32
	j	.L9
	.size	write_single_by_word, .-write_single_by_word
	.section	.text.write_quad_by_word,"ax",@progbits
	.align	1
	.globl	write_quad_by_word
	.type	write_quad_by_word, @function
write_quad_by_word:
	addi	sp,sp,-16
	sw	s0,12(sp)
 #APP
	csrr t1, 3009
 #NO_APP
	slli	t1,t1,16
	srli	t1,t1,16
	ori	t1,t1,63
 #APP
	csrw 3009, t1
	csrr t1, 3008
 #NO_APP
	slli	t1,t1,16
	srli	t1,t1,16
	ori	t1,t1,32
 #APP
	csrw 3008, t1
 #NO_APP
	li	t1,4
	sw	t1,4(sp)
	sh	t1,8(sp)
	li	t1,262144
	addi	t1,t1,4
 #APP
	csrw 3043, t1
	csrw 3045, 0
	csrr t1, 1996
 #NO_APP
	andi	t2,t1,1
	sb	t2,0(sp)
	srli	t0,t1,4
	andi	t0,t0,1
	sb	t0,1(sp)
	srli	t1,t1,8
	andi	t1,t1,1
	sb	t1,2(sp)
	sb	zero,2(sp)
	slli	t0,t0,4
	or	t2,t2,t0
 #APP
	csrw 1996, t2
 #NO_APP
	li	t1,31
	bleu	a3,t1,.L23
.L16:
 #APP
	csrw 2000, 2
 #NO_APP
	slli	t1,a2,16
	srli	t1,t1,16
 #APP
	csrr t0, 2003
 #NO_APP
	li	t2,-65536
	and	t0,t0,t2
	or	a2,t1,t0
 #APP
	csrw 2003, a2
 #NO_APP
	srli	a3,a3,2
 #APP
	csrw 3022, a3
 #NO_APP
	addi	a3,a3,-1
	andi	a3,a3,0xff
 #APP
	csrw 3023, a3
	csrr a3, 3008
 #NO_APP
	slli	a3,a3,16
	srli	a3,a3,16
	andi	a3,a3,-33
	slli	a3,a3,16
	srli	a3,a3,16
	beq	a4,zero,.L24
	li	a2,32
.L18:
	or	a3,a2,a3
 #APP
	csrw 3008, a3
 #NO_APP
	slli	a3,t1,1
	add	t1,t1,a3
	slli	t1,t1,16
	srli	t1,t1,16
 #APP
	csrw 2005, t1
 #NO_APP
	li	a3,0
	j	.L19
.L23:
	li	t1,0
.L15:
	bgeu	t1,a1,.L16
	slli	t0,t1,2
	add	t0,a0,t0
	lw	t2,0(t0)
	li	s0,32
	sub	s0,s0,a3
	sll	t2,t2,s0
	sw	t2,0(t0)
	addi	t1,t1,1
	andi	t1,t1,0xff
	j	.L15
.L24:
	li	a2,0
	j	.L18
.L20:
	slli	a2,a3,2
	add	a2,a0,a2
	lw	a2,0(a2)
 #APP
	csrw 3016, a2
 #NO_APP
	addi	a3,a3,1
	andi	a3,a3,0xff
.L19:
	bltu	a3,a1,.L20
 #APP
	csrw 3012, 0
 #NO_APP
	sw	zero,4(sp)
	lhu	a3,8(sp)
	slli	a3,a3,16
	li	a2,2031616
	and	a3,a3,a2
 #APP
	csrw 3043, a3
	csrw 3045, 0
 #NO_APP
	bne	a5,zero,.L21
 #APP
	csrr a5, 3008
 #NO_APP
	slli	a5,a5,16
	srli	a5,a5,16
	andi	a5,a5,-34
	slli	a5,a5,16
	srli	a5,a5,16
	beq	a4,zero,.L25
	li	a4,0
.L22:
	or	a5,a4,a5
 #APP
	csrw 3008, a5
 #NO_APP
.L21:
 #APP
	csrw 2000, 0
 #NO_APP
	lw	s0,12(sp)
	addi	sp,sp,16
	jr	ra
.L25:
	li	a4,32
	j	.L22
	.size	write_quad_by_word, .-write_quad_by_word
