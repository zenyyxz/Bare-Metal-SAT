section .text
    global is_literal_satisfied
    global is_literal_falsified
    global bitset_set
    global bitset_clear
    global bitset_get

; int is_literal_satisfied(int literal, int assignment)
; rdi = literal, rsi = assignment
; Returns 1 if satisfied, 0 otherwise
; assignment: 0=UNASSIGNED, 1=TRUE, 2=FALSE
is_literal_satisfied:
    ; Check if assignment is UNASSIGNED (0)
    test rsi, rsi
    jz .not_satisfied

    ; If literal > 0, check if assignment is TRUE (1)
    test edi, edi
    jg .check_true

    ; If literal < 0, check if assignment is FALSE (2)
    cmp rsi, 2
    je .satisfied
    jmp .not_satisfied

.check_true:
    cmp rsi, 1
    je .satisfied
    jmp .not_satisfied

.satisfied:
    mov eax, 1
    ret

.not_satisfied:
    xor eax, eax
    ret

; int is_literal_falsified(int literal, int assignment)
; rdi = literal, rsi = assignment
; Returns 1 if falsified, 0 otherwise
; assignment: 0=UNASSIGNED, 1=TRUE, 2=FALSE
is_literal_falsified:
    ; Check if assignment is UNASSIGNED (0)
    test rsi, rsi
    jz .not_falsified

    ; If literal > 0, check if assignment is FALSE (2)
    test edi, edi
    jg .check_false

    ; If literal < 0, check if assignment is TRUE (1)
    cmp rsi, 1
    je .falsified
    jmp .not_falsified

.check_false:
    cmp rsi, 2
    je .falsified
    jmp .not_falsified

.falsified:
    mov eax, 1
    ret

.not_falsified:
    xor eax, eax
    ret

; void bitset_set(Bitset* b, size_t index)
; rdi = b (pointer to Bitset struct), rsi = index
bitset_set:
    mov rax, [rdi]      ; rax = b->bits (pointer to uint64_t array)
    mov rdx, rsi
    shr rdx, 6          ; rdx = index / 64
    and rsi, 63         ; rsi = index % 64
    lock bts [rax + rdx * 8], rsi
    ret

; void bitset_clear(Bitset* b, size_t index)
; rdi = b, rsi = index
bitset_clear:
    mov rax, [rdi]
    mov rdx, rsi
    shr rdx, 6
    and rsi, 63
    lock btr [rax + rdx * 8], rsi
    ret

; int bitset_get(Bitset* b, size_t index)
; rdi = b, rsi = index
bitset_get:
    mov rax, [rdi]
    mov rdx, rsi
    shr rdx, 6
    and rsi, 63
    bt [rax + rdx * 8], rsi
    setc al
    movzx eax, al
    ret
