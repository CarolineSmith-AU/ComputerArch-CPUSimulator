  .text    
	
main:
	push X
	push X
	mul
	push A
	mul
	push B
	push X
	mul
	add
	push C
	add
	pop D
	END

	.data
X:	.word 3
A:	.word 16
B:	.word 5
C:	.word 4
D:  .word ?
	END