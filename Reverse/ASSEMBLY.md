     EXPORT start
     start
    
     var_14B0        = -0x14B0
     var_14AC        = -0x14AC
     var_14A8        = -0x14A8
     var_14A4        = -0x14A4
     var_14A0        = -0x14A0
     var_149F        = -0x149F
     var_149E        = -0x149E
     var_18          = -0x18
    
     ; FUNCTION CHUNK AT  SIZE 00000030 BYTES
    
                     SUB             SP, SP, #0x18
                     BL              sub_13DC4
                     LDRB            R12, [SP,#0x18+var_18]
                     CMP             R12, #0xE
                     BNE             loc_13C78
                     PUSH            {R2,R3,R11,LR}
                     CMP             R1, #0x2000
                     MOVLT           R11, #0
                     PUSHLT          {R11}
                     PUSHLT          {R11}
                     BLT             loc_13278
                     MOV             R11, SP
                     SUB             SP, SP, #0x18
                     PUSH            {R0-R6}
                     MOV             R0, #0
                     ADD             R1, R1, #0x400
                     MOV             R2, #3
                     MOV             R3, #0x22 ; '"'
                     MOV             R4, #0xFFFFFFFF
                     MOV             R5, #0
                     MOV             R7, #0xC0
                     SVC             0
                     STR             R0, [R11,#-0x14]
                     POP             {R0-R6}
                     PUSH            {R0-R6}
                     MOV             R4, R0
                     MOV             R5, R1
                     LDR             R6, [R11,#-0x14]
                     MOV             R0, #0x2F ; '/'
                     STRB            R0, [R11,#-0x10]
                     STRB            R0, [R11,#-0xC]
                     MOV             R0, #0x64 ; 'd'
                     STRB            R0, [R11,#-0xF]
                     ADD             R0, R0, #1
                     STRB            R0, [R11,#-0xE]
                     STRB            R0, [R11,#-8]
                     MOV             R0, #0x76 ; 'v'
                     STRB            R0, [R11,#-0xD]
                     MOV             R0, #0x68 ; 'h'
                     STRB            R0, [R11,#-0xB]
                     MOV             R0, #0x77 ; 'w'
                     STRB            R0, [R11,#-0xA]
                     MOV             R0, #0x61 ; 'a'
                     STRB            R0, [R11,#-9]
                     MOV             R0, #0x73 ; 's'
                     STRB            R0, [R11,#-7]
                     MOV             R0, #0
                     STRB            R0, [R11,#-6]
                     STRB            R0, [R11,#-5]
                     SUB             R0, R11, #0x10
                     MOV             R1, #2
                     MOV             R7, #5
                     SVC             0
                     CMP             R0, #0
                     MOVMI           R0, R0
                     MOVMI           R0, R0
                     MOVMI           R0, R0
                     MOVMI           R0, R0
                     STR             R0, [R11,#-4]
                     LDR             R1, =0xC00C6206
                     STR             R4, [R11,#-0x10]
                     STR             R6, [R11,#-0xC]
                     STR             R5, [R11,#-8]
                     SUB             R2, R11, #0x10
                     MOV             R7, #0x36 ; '6'
                     SVC             0
                     LDR             R0, [R11,#-4]
                     MOV             R7, #6
                     SVC             0
                     POP             {R0-R6}
                     LDR             R0, [R11,#-0xC]
                     MOV             SP, R11
                     PUSH            {R0,R1}
    
     loc_13278
                     LDRB            R11, [R0]
                     MOV             R12, #0x600
                     MOV             R11, R11,LSR#3
                     MOV             R12, R12,LSL R11
                     MOV             R11, SP
                     ADD             R12, R12, #0xE80
                     SUB             SP, SP, R12
                     LDR             R12, [R3]
                     STR             R3, [SP,#0x14B0+var_14A8]
                     STR             R12, [SP,#0x14B0+var_14AC]
                     STR             R2, [SP,#0x14B0+var_14B0]
                     ADD             R3, SP, #0x14B0+var_14A4
                     MOV             R12, #0
    
     loc_132AC
                     STR             R12, [R3],#4
                     CMP             R3, R11
                     BNE             loc_132AC
                     ADD             R3, SP, #0x14B0+var_14A4
                     SUB             R2, R1, #2
                     MOV             R1, R0
                     LDRB            R12, [R1],#1
                     AND             R12, R12, #7
                     STRB            R12, [SP,#0x14B0+var_149E]
                     LDRB            R12, [R1],#1
                     MOV             R0, R12,LSR#4
                     STRB            R0, [SP,#0x14B0+var_149F]
                     AND             R12, R12, #0xF
                     STRB            R12, [SP,#0x14B0+var_14A0]
                     ADD             R0, SP, #0x14B0+var_14A0
                     BL              dword_1333C
                     MOV             SP, R11
                     MOV             R3, R0
                     POP             {R0,R1}
                     CMP             R0, #0
                     BEQ             loc_13308
                     MOV             R7, #0x5B ; '['
                     SVC             0
    
     loc_13308
                     MOV             R0, R3
                     MOV             R3, R0
                     POP             {R0,R1}
                     LDR             R1, [R1]
                     ADD             R1, R1, R0
                     MOV             R2, #0
                     MOV             R12, R7
                     MOV             R7, #0xF0002
                     SVC             0
                     MOV             R7, R12
                     MOV             R0, R3
                     POP             {R11,PC}
     ; End of function start