  .text    
	
main:

	load A
    mult X
    mult X
    sto Y
    load B
    mult X
    add Y
    add C
    sto D
	END

	.data
X:  .word 3
A:	.word 16
B:	.word 5
C:	.word 4
Y:  .word ?
D:  .word ?
	END