	.file	"hrt.c"
	.option nopic
	.attribute arch, "rv32e1p9_m2p0_c2p0_zicsr2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 4
	.text
	.section	.text.hrt_tx,"ax",@progbits
	.align	1
	.type	hrt_tx, @function
hrt_tx:
	lw	a5,4(a0)
	addi	sp,sp,-16
	sw	s0,12(sp)
	sw	s1,8(sp)
	sw	a2,0(sp)
	beq	a5,zero,.L1
	li	a4,32
	div	a4,a4,a1
	lui	t2,%hi(xfer_shift_ctrl)
	addi	a2,t2,%lo(xfer_shift_ctrl)
	lbu	s1,2(a2)
	lbu	a5,1(a2)
	sb	a1,2(a2)
	lbu	a2,3(a2)
	li	t1,3145728
	slli	a5,a5,8
	slli	a2,a2,20
	and	a2,a2,t1
	andi	a5,a5,1792
	or	a5,a5,a2
	li	t1,126976
	slli	a2,a1,12
	and	a2,a2,t1
	or	a5,a5,a2
	addi	a4,a4,-1
	andi	a4,a4,0xff
	sb	a4,%lo(xfer_shift_ctrl)(t2)
	andi	a4,a4,63
	or	a5,a5,a4
 #APP
	csrw 3019, a5
 #NO_APP
	li	a4,2031616
	slli	a5,a1,16
	and	a5,a5,a4
	ori	a5,a5,4
	sw	a5,4(sp)
	li	t1,0
	addi	t0,t2,%lo(xfer_shift_ctrl)
.L3:
	lw	a5,4(a0)
	bltu	t1,a5,.L14
.L1:
	lw	s0,12(sp)
	lw	s1,8(sp)
	addi	sp,sp,16
	jr	ra
.L14:
	lw	a5,4(a0)
	li	a4,1
	sub	a5,a5,t1
	beq	a5,a4,.L4
	li	a4,2
	beq	a5,a4,.L5
.L6:
	lw	a4,0(a0)
	li	a5,0
	beq	a4,zero,.L7
	lw	a5,0(a0)
	slli	a4,t1,2
	add	a5,a5,a4
	lw	a5,0(a5)
	j	.L7
.L4:
	lbu	a2,1(t0)
	lbu	a5,2(t0)
	li	s0,126976
	slli	a2,a2,8
	slli	a5,a5,12
	and	a5,a5,s0
	andi	a2,a2,1792
	or	a2,a2,a5
	lbu	a4,8(a0)
	lbu	a5,3(t0)
	li	s0,3145728
	addi	a4,a4,-1
	slli	a5,a5,20
	andi	a4,a4,0xff
	and	a5,a5,s0
	sb	a4,%lo(xfer_shift_ctrl)(t2)
	or	a5,a2,a5
	andi	a4,a4,63
	or	a5,a5,a4
 #APP
	csrw 3019, a5
 #NO_APP
	lw	a5,12(a0)
.L7:
	beq	a1,s1,.L8
.L9:
 #APP
	csrr a4, 3022
 #NO_APP
	andi	a4,a4,0xff
	bne	a4,zero,.L9
	lw	a4,4(sp)
 #APP
	csrw 3043, a4
 #NO_APP
	mv	s1,a1
.L8:
	lbu	a4,16(a0)
	andi	a2,a4,0xff
	beq	a4,zero,.L10
	li	a4,1
	bne	a2,a4,.L11
 #APP
	csrw 3017, a5
 #NO_APP
.L11:
	bne	t1,zero,.L12
	lw	a5,0(sp)
	lbu	a5,0(a5)
	bne	a5,zero,.L12
	mv	a5,a3
	bne	a3,zero,.L13
	li	a5,1
.L13:
	slli	a3,a5,16
	srli	a3,a3,16
 #APP
	csrw 2005, a3
 #NO_APP
	lw	a5,0(sp)
	li	a4,1
	sb	a4,0(a5)
.L12:
	addi	t1,t1,1
	j	.L3
.L5:
	lbu	a2,1(t0)
	lbu	a5,2(t0)
	li	s0,126976
	slli	a2,a2,8
	slli	a5,a5,12
	and	a5,a5,s0
	andi	a2,a2,1792
	or	a2,a2,a5
	lbu	a4,9(a0)
	lbu	a5,3(t0)
	li	s0,3145728
	addi	a4,a4,-1
	slli	a5,a5,20
	andi	a4,a4,0xff
	and	a5,a5,s0
	sb	a4,%lo(xfer_shift_ctrl)(t2)
	or	a5,a2,a5
	andi	a4,a4,63
	or	a5,a5,a4
 #APP
	csrw 3019, a5
 #NO_APP
	j	.L6
.L10:
 #APP
	csrw 3016, a5
 #NO_APP
	j	.L11
	.size	hrt_tx, .-hrt_tx
	.section	.text.hrt_write,"ax",@progbits
	.align	1
	.globl	hrt_write
	.type	hrt_write, @function
hrt_write:
	addi	sp,sp,-16
	sw	s0,8(sp)
	sw	ra,12(sp)
	sw	s1,4(sp)
	mv	s0,a0
	sb	zero,3(sp)
	lhu	a5,90(a0)
 #APP
	csrw 3009, a5
 #NO_APP
	li	a5,0
	li	a2,20
	li	a3,4
.L21:
	mul	a4,a5,a2
	add	a4,s0,a4
	lw	a4,4(a4)
	bne	a4,zero,.L20
	addi	a5,a5,1
	bne	a5,a3,.L21
	li	a5,3
.L22:
	li	a4,1
	beq	a5,a4,.L23
	li	a4,3
	beq	a5,a4,.L24
	li	a3,0
	bne	a5,zero,.L25
	lbu	a3,80(s0)
.L43:
	andi	a3,a3,0xff
.L25:
	lui	a4,%hi(xfer_shift_ctrl+2)
	sb	a3,%lo(xfer_shift_ctrl+2)(a4)
 #APP
	csrw 2000, 2
 #NO_APP
	lhu	a4,84(s0)
	slli	a4,a4,16
	srli	a4,a4,16
 #APP
	csrr a2, 2003
 #NO_APP
	li	a1,-65536
	and	a2,a2,a1
	or	a4,a4,a2
 #APP
	csrw 2003, a4
	csrw 3011, 0
 #NO_APP
	li	a2,2031616
	slli	a4,a3,16
	and	a4,a4,a2
	ori	a4,a4,4
 #APP
	csrw 3043, a4
 #NO_APP
	li	a4,20
	mul	a5,a5,a4
	li	a2,1
	add	a5,s0,a5
	lw	a4,4(a5)
	beq	a4,a2,.L26
	li	a2,2
	beq	a4,a2,.L27
	li	a5,32
	div	a5,a5,a3
	j	.L44
.L20:
	andi	a5,a5,0xff
	j	.L22
.L23:
	lbu	a3,81(s0)
	j	.L43
.L24:
	lbu	a3,83(s0)
	j	.L43
.L26:
	lbu	a5,8(a5)
.L44:
 #APP
	csrw 3022, a5
 #NO_APP
	lbu	a5,89(s0)
	lbu	a4,87(s0)
	bne	a5,zero,.L30
	li	a5,1
	sll	a5,a5,a4
	slli	a5,a5,16
	srli	a5,a5,16
 #APP
	csrc 3008, a5
 #NO_APP
.L31:
 #APP
	csrr s1, 3008
 #NO_APP
	lbu	a1,80(s0)
	lhu	a3,84(s0)
	addi	a2,sp,3
	mv	a0,s0
	call	hrt_tx
	lbu	a1,81(s0)
	lhu	a3,84(s0)
	addi	a2,sp,3
	addi	a0,s0,20
	call	hrt_tx
	lbu	a1,82(s0)
	lhu	a3,84(s0)
	addi	a2,sp,3
	addi	a0,s0,40
	call	hrt_tx
	lbu	a1,83(s0)
	lhu	a3,84(s0)
	addi	a2,sp,3
	addi	a0,s0,60
	call	hrt_tx
	lbu	a5,94(s0)
	bne	a5,zero,.L32
	li	a5,4096
	addi	a5,a5,1
 #APP
	csrw 3019, a5
 #NO_APP
	li	a4,131072
	slli	a5,s1,1
	addi	a4,a4,-2
	and	a5,a5,a4
 #APP
	csrw 3012, a5
	csrw 2000, 0
 #NO_APP
.L33:
 #APP
	csrw 2005, 0
 #NO_APP
	lbu	a5,88(s0)
	bne	a5,zero,.L19
	lbu	a5,89(s0)
	lbu	a4,87(s0)
	bne	a5,zero,.L36
	li	a5,1
	sll	a5,a5,a4
	slli	a5,a5,16
	srli	a5,a5,16
 #APP
	csrs 3008, a5
 #NO_APP
.L19:
	lw	ra,12(sp)
	lw	s0,8(sp)
	lw	s1,4(sp)
	addi	sp,sp,16
	jr	ra
.L27:
	lbu	a5,9(a5)
	j	.L44
.L30:
	li	a5,1
	sll	a5,a5,a4
	slli	a5,a5,16
	srli	a5,a5,16
 #APP
	csrs 3008, a5
 #NO_APP
	j	.L31
.L32:
 #APP
	csrr a5, 3022
 #NO_APP
	andi	a5,a5,0xff
	bne	a5,zero,.L32
 #APP
	csrw 2000, 0
 #NO_APP
	li	a5,4096
	addi	a5,a5,1
 #APP
	csrw 3019, a5
 #NO_APP
	lbu	a3,94(s0)
	li	a5,1
	andi	a4,a3,0xff
	bne	a3,a5,.L34
	lbu	a5,86(s0)
	sll	a5,a4,a5
	slli	a5,a5,16
	srli	a5,a5,16
 #APP
	csrc 3008, a5
 #NO_APP
	j	.L33
.L34:
	lbu	a3,94(s0)
	li	a4,3
	bne	a3,a4,.L33
	lbu	a4,86(s0)
	sll	a5,a5,a4
	slli	a5,a5,16
	srli	a5,a5,16
 #APP
	csrs 3008, a5
 #NO_APP
	j	.L33
.L36:
	li	a5,1
	sll	a5,a5,a4
	slli	a5,a5,16
	srli	a5,a5,16
 #APP
	csrc 3008, a5
 #NO_APP
	j	.L19
	.size	hrt_write, .-hrt_write
	.section	.text.hrt_read,"ax",@progbits
	.align	1
	.globl	hrt_read
	.type	hrt_read, @function
hrt_read:
	addi	sp,sp,-48
	sw	s0,40(sp)
	sw	ra,44(sp)
	sw	s1,36(sp)
	lbu	s1,83(a0)
	mv	s0,a0
	li	a0,32
	div	s1,a0,s1
	lbu	a4,83(s0)
	lbu	a3,83(s0)
	lw	a5,64(s0)
	lbu	a1,88(s0)
	lw	a2,60(s0)
	lbu	t0,69(s0)
	lbu	t1,83(s0)
	andi	a4,a4,0xff
	andi	a3,a3,0xff
	andi	a1,a1,0xff
	mul	t1,t0,t1
	lbu	t0,68(s0)
	addi	s1,s1,-1
	andi	s1,s1,0xff
	sw	t1,0(sp)
	lbu	t1,83(s0)
	mul	t1,t0,t1
	sw	t1,4(sp)
	lw	t1,0(sp)
	sub	a0,a0,t1
	sw	a0,8(sp)
	lbu	a0,89(s0)
	lbu	t1,87(s0)
	bne	a0,zero,.L46
	li	a0,1
	sll	a0,a0,t1
	slli	a0,a0,16
	srli	a0,a0,16
 #APP
	csrc 3008, a0
 #NO_APP
.L47:
	sw	a2,32(sp)
	sw	a1,28(sp)
	sw	a5,24(sp)
	sw	a3,20(sp)
	sw	a4,16(sp)
 #APP
	csrr a5, 3008
 #NO_APP
	li	t1,1
	sw	zero,64(s0)
	sb	t1,88(s0)
	mv	a0,s0
	sw	a5,12(sp)
	call	hrt_write
	lw	a5,24(sp)
	lw	a1,28(sp)
	li	t1,1
	sw	a5,64(s0)
	sb	a1,88(s0)
	lbu	a1,83(s0)
	lw	a4,16(sp)
	lw	a3,20(sp)
	lw	a2,32(sp)
	bne	a1,t1,.L48
	lhu	a1,90(s0)
	slli	a1,a1,16
	srli	a1,a1,16
	andi	a1,a1,-5
	slli	a1,a1,16
	srli	a1,a1,16
	sh	a1,90(s0)
	lhu	a1,90(s0)
.L82:
 #APP
	csrw 3009, a1
	csrw 3011, 2
 #NO_APP
	li	a1,2031616
	slli	a3,a3,16
	and	a3,a3,a1
	ori	a1,a3,4
 #APP
	csrw 3043, a1
	csrw 3022, 1
	csrw 2000, 2
	csrw 2001, 2
 #NO_APP
	lhu	a1,84(s0)
	slli	a1,a1,16
	srli	a1,a1,16
 #APP
	csrr a0, 2003
 #NO_APP
	li	t1,-65536
	and	a0,a0,t1
	or	a1,a1,a0
 #APP
	csrw 2003, a1
 #NO_APP
	lhu	a1,84(s0)
	slli	a1,a1,16
	srli	a1,a1,16
 #APP
	csrr a0, 2003
 #NO_APP
	slli	a1,a1,1
	slli	a0,a0,16
	addi	a1,a1,1
	srli	a0,a0,16
	slli	a1,a1,16
	or	a1,a1,a0
 #APP
	csrw 2003, a1
 #NO_APP
	li	a1,126976
	slli	a4,a4,12
	and	a4,a4,a1
	andi	a0,s1,63
	li	a1,2097152
	or	a0,a0,a4
	addi	a1,a1,1024
	or	a0,a0,a1
 #APP
	csrw 3019, a0
	csrw 3017, 0
 #NO_APP
	li	a0,1
	bne	a5,a0,.L50
	lbu	t1,68(s0)
	li	a0,2
	bne	t1,a0,.L51
	lhu	a5,84(s0)
	li	a2,65536
	add	a5,a5,a2
 #APP
	csrw 2002, a5
 #NO_APP
.L52:
 #APP
	csrr a2, 2005
	csrr a5, 2005
 #NO_APP
	slli	a2,a2,16
	slli	a5,a5,16
	srli	a2,a2,16
	srli	a5,a5,16
	bne	a2,a5,.L52
	lw	a5,12(sp)
	slli	a5,a5,24
 #APP
	csrw 3017, a5
	csrw 3043, a3
	csrr a5, 3018
 #NO_APP
	lw	a2,4(sp)
	lw	a3,60(s0)
	addi	a2,a2,16
	srl	a5,a5,a2
	andi	a5,a5,0xff
	sb	a5,0(a3)
.L53:
 #APP
	csrw 2000, 0
	csrw 2001, 0
 #NO_APP
	andi	s1,s1,63
	or	s1,s1,a4
 #APP
	csrw 3019, s1
 #NO_APP
	lbu	a5,88(s0)
	bne	a5,zero,.L72
	lbu	a5,89(s0)
	lbu	a4,87(s0)
	bne	a5,zero,.L73
	li	a5,1
	sll	a5,a5,a4
	slli	a5,a5,16
	srli	a5,a5,16
 #APP
	csrs 3008, a5
 #NO_APP
.L72:
	lhu	a5,90(s0)
	ori	a5,a5,4
	sh	a5,90(s0)
	lhu	a5,90(s0)
 #APP
	csrw 3009, a5
 #NO_APP
	lw	ra,44(sp)
	lw	s0,40(sp)
	lw	s1,36(sp)
	addi	sp,sp,48
	jr	ra
.L46:
	li	a0,1
	sll	a0,a0,t1
	slli	a0,a0,16
	srli	a0,a0,16
 #APP
	csrs 3008, a0
 #NO_APP
	j	.L47
.L48:
	lhu	a1,92(s0)
	j	.L82
.L50:
	li	a0,2
	bne	a5,a0,.L79
	lbu	a0,69(s0)
	addi	a0,a0,-3
	andi	a0,a0,63
	or	a0,a0,a4
	or	a1,a0,a1
 #APP
	csrw 3019, a1
 #NO_APP
	lbu	s1,68(s0)
.L83:
	lhu	a1,84(s0)
	li	a0,65536
	addi	s1,s1,-1
	andi	s1,s1,0xff
	add	a1,a1,a0
 #APP
	csrw 2002, a1
	csrr a1, 3018
 #NO_APP
	li	t1,2097152
	andi	a1,s1,63
	or	a1,a1,a4
	addi	t1,t1,1024
	or	a1,a1,t1
 #APP
	csrw 3019, a1
 #NO_APP
	li	t2,32
	mv	t0,a2
	addi	a1,a5,-2
.L59:
	li	a0,-1
	bne	a1,a0,.L64
.L65:
 #APP
	csrr a0, 2005
	csrr a1, 2005
 #NO_APP
	slli	a0,a0,16
	slli	a1,a1,16
	srli	a0,a0,16
	srli	a1,a1,16
	bne	a0,a1,.L65
	lw	a1,12(sp)
	slli	a1,a1,24
 #APP
	csrw 3017, a1
	csrw 3043, a3
	csrr a3, 3018
 #NO_APP
	lw	a0,4(sp)
	li	a1,32
	sub	a1,a1,a0
	srl	a3,a3,a1
	lw	a1,8(sp)
	beq	a1,zero,.L66
	li	a1,1
	beq	a5,a1,.L66
	slli	a1,a5,2
	addi	a1,a1,-8
	add	a1,a2,a1
	lw	t1,0(a1)
	lw	a0,8(sp)
	srl	t1,t1,a0
	lw	a0,0(sp)
	sll	a0,a3,a0
	or	a0,a0,t1
	lw	t1,8(sp)
	sw	a0,0(a1)
	srl	a3,a3,t1
.L66:
	lw	a0,4(sp)
	lw	a1,0(sp)
	add	a1,a1,a0
	addi	a1,a1,-33
	srli	a1,a1,3
	addi	a1,a1,1
	li	a0,3
	beq	a1,a0,.L67
	bgtu	a1,a0,.L68
	li	a0,1
	bne	a1,a0,.L70
	slli	a5,a5,2
	add	a2,a2,a5
	sb	a3,-4(a2)
	j	.L53
.L51:
	lbu	a0,68(s0)
	addi	a0,a0,-3
	andi	a0,a0,63
	or	a0,a0,a4
	or	a1,a0,a1
 #APP
	csrw 3019, a1
 #NO_APP
	li	a1,0
.L56:
	li	a0,2
	bne	a1,a0,.L58
	lbu	s1,69(s0)
	j	.L83
.L79:
	lbu	t1,83(s0)
	li	a0,32
	div	a0,a0,t1
	addi	a0,a0,-3
	andi	a0,a0,63
	or	a0,a0,a4
	or	a1,a0,a1
 #APP
	csrw 3019, a1
 #NO_APP
	addi	a1,a5,-1
	j	.L56
.L58:
	lbu	a1,83(s0)
	li	s1,32
	div	s1,s1,a1
	j	.L83
.L64:
	li	a0,1
	beq	a1,a0,.L60
	li	a0,2
	beq	a1,a0,.L61
	lbu	s1,83(s0)
	div	s1,t2,s1
	j	.L84
.L60:
	lbu	s1,68(s0)
.L84:
	addi	s1,s1,-1
	andi	s1,s1,0xff
 #APP
	csrr a0, 3018
 #NO_APP
	sw	a0,0(t0)
	andi	a0,s1,63
	or	a0,a0,a4
	or	a0,a0,t1
 #APP
	csrw 3019, a0
 #NO_APP
	addi	t0,t0,4
	addi	a1,a1,-1
	j	.L59
.L61:
	lbu	s1,69(s0)
	j	.L84
.L68:
	li	a0,4
	bne	a1,a0,.L53
	slli	a5,a5,2
	add	a5,a2,a5
	sw	a3,-4(a5)
	j	.L53
.L67:
	slli	a1,a5,2
	add	a1,a2,a1
	srli	a0,a3,16
	sb	a0,-2(a1)
.L70:
	slli	a5,a5,2
	add	a5,a2,a5
	sh	a3,-4(a5)
	j	.L53
.L73:
	li	a5,1
	sll	a5,a5,a4
	slli	a5,a5,16
	srli	a5,a5,16
 #APP
	csrc 3008, a5
 #NO_APP
	j	.L72
	.size	hrt_read, .-hrt_read
	.section	.sdata.xfer_shift_ctrl,"aw"
	.align	2
	.type	xfer_shift_ctrl, @object
	.size	xfer_shift_ctrl, 4
xfer_shift_ctrl:
	.byte	31
	.byte	4
	.byte	1
	.byte	0
