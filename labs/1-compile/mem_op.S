get32:
	ldr r0, [r0]
	bx lr

get16:
	ldrh r0, [r0]
	bx lr

get8:
	ldrb r0, [r0]
	bx lr

put32:
	str r1, [r0]
	bx lr

put16:
	strh r1, [r0]
	bx lr

put8:
	strb r1, [r0]
	bx lr
