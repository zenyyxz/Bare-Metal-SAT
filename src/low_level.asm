section .text
    global is_literal_satisfied
    global is_literal_falsified
    global simd_find_literal

; int is_literal_satisfied(int literal, int assignment)
is_literal_satisfied:
    test rsi, rsi
    jz .not_sat
    test edi, edi
    jg .pos
    cmp rsi, 2
    je .sat
    jmp .not_sat
.pos:
    cmp rsi, 1
    je .sat
.not_sat:
    xor eax, eax
    ret
.sat:
    mov eax, 1
    ret

; int is_literal_falsified(int literal, int assignment)
is_literal_falsified:
    test rsi, rsi
    jz .not_falsified
    test edi, edi
    jg .pos_f
    cmp rsi, 1
    je .falsified
    jmp .not_falsified
.pos_f:
    cmp rsi, 2
    je .falsified
.not_falsified:
    xor eax, eax
    ret
.falsified:
    mov eax, 1
    ret

; int simd_find_literal(int* lits, int size, int* assigns)
; rdi = lits, rsi = size, rdx = assigns
; This is a placeholder for the AVX2 implementation. 
; A real SIMD implementation would use VPCMPEQD to scan assignments.
; For this environment, we provide a fast assembly loop as the base.
simd_find_literal:
    xor rcx, rcx
.loop:
    cmp rcx, rsi
    jge .not_found
    
    mov eax, [rdi + rcx * 4] ; lit = lits[rcx]
    mov r8, rax
    sar r8, 31               ; r8 = lit < 0 ? -1 : 0
    
    mov r9, rax
    test r9, r9
    jns .pos_lit
    neg r9                   ; r9 = abs(lit)
.pos_lit:
    mov r10d, [rdx + r9 * 4] ; assign = assigns[var]
    
    ; Check if not falsified:
    ; if lit > 0, not falsified if assign != FALSE (2)
    ; if lit < 0, not falsified if assign != TRUE (1)
    test eax, eax
    jg .check_pos
    cmp r10d, 1
    jne .found
    jmp .next
.check_pos:
    cmp r10d, 2
    jne .found

.next:
    inc rcx
    jmp .loop

.found:
    mov eax, ecx
    ret
.not_found:
    mov eax, -1
    ret
