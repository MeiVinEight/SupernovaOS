CODE SEGMENT


	START PROC
		ORG 9000H

		DB 0EAH
		DW OFFSET AP_START, 00H
		AP_START:
		XOR AX, AX
		MOV DS, AX
		MOV ES, AX
		MOV SS, AX
		MOV AX, 0A000H
		MOV SP, AX
		; 1. GDT AT 0070H
		; 2. PAGE AT 1000H
		; 3. CLI
		CLI

		;- - - - - - - - - - - - - - - - - - - - - - - - - - -

		;4. OPEN A20
		IN AL, 92H
		OR AL, 02H
		OUT 92H, AL

		;- - - - - - - - - - - - - - - - - - - - - - - - - - -

		;5. Enable PAE and PGE
		;MOV EAX, CR4
		DB 66H, 0FH, 20H, 20H
		DB 66H
		OR AX, 20H; CR4.PAE
		DB 66H
		OR AX, 0080H; CR4.PGE
		DW 00H
		;MOV CR4, EAX
		DB 66H, 0FH, 22H, 20H

		;- - - - - - - - - - - - - - - - - - - - - - - - - - -

		;6. Set Page Table address CR3 bit 12:63
		DB 66H
		MOV AX, 1000H; PHY ADDR 0x00001000
		DW 0000H
		;MOV CR3, EAX
		DB 66H, 0FH, 22H, 18H

		;- - - - - - - - - - - - - - - - - - - - - - - - - - -

		; 7. Load GDT
		XOR AX, AX
		MOV CX, 70H
		MOV SI, 18H
		PUSH AX
		PUSH AX
		PUSH AX
		PUSH CX
		PUSH SI
		MOV SI, SP
		; LGDT [SI]
		DB 0FH, 01H, 14H
		ADD SP, 0AH

		;- - - - - - - - - - - - - - - - - - - - - - - - - - -

		;8. Set EFER.LME to 1 (Long Mode Enable)
		DB 66H
		MOV CX, 0080H
		DW 0C000H
		;RDMSR     ;EFER
		DB 0FH, 32H
		DB 66H
		OR AX, 100H ;EFER.LME
		DW 00H
		;WRMSR
		DB 0FH, 30H

		;- - - - - - - - - - - - - - - - - - - - - - - - - - -

		;9. Enable CR0.PE and CR0.PG
		;MOV EAX, CR0
		DB 66H, 0FH, 20H, 00H
		;BIT  0 is CR0.PE
		;BIT 31 is CR0.PG
		;MOV ECX, 80000001H
		DB 66H
		MOV CX, 0001H
		DW 8000H
		DB 66H
		OR AX, CX
		;MOV CR0, EAX
		DB 66H, 0FH, 22H, 00H

		;- - - - - - - - - - - - - - - - - - - - - - - - - - -

		;X. JMP FAR 0008:LONGMODE
		DB 66H, 0EAH
		JMPENTRY:
		DW OFFSET LONGMODE
		DW 0000H
		DW 0008H

		LONGMODE:
		DB 031H, 0C9H      ; XOR ECX, ECX
		DB 066H, 08EH, 0E1H; MOV FS, CX
		DB 066H, 08EH, 0E9H; MOV GS, CX
		DB 083H, 0C1H, 010H; ADD ECX, 10H
		DB 066H, 08EH, 0C1H; MOV ES, CX
		DB 066H, 08EH, 0D1H; MOV SS, CX
		DB 066H, 08EH, 0D9H; MOV DS, CX
		DB 048H, 0BCH
		DD 0000A000H
		DD 0FFFF8000H      ; MOV RSP, FFFF80000000A000H
		DB 06AH, 008H      ; PUSH 08H
		DB 048H, 0B8H
		DQ 0FFFF800000000000H; MOV RAX, 0FFFF800000000000H
		DB 050H            ; PUSH 50H
		DB 048H, 0CBH      ; RETFQ

	START ENDP

CODE ENDS

END START
