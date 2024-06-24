;Tic Tac Toe
ORG 0x100

LXI D, SPLASH
CALL PUT_STRING
CALL PRINT_MAP
JMP MAIN_LOOP

SPLASH:
db " _____ _     _____         _____         ",0x0A, 0x0D
db "|_   _|_|___|_   _|___ ___|_   _|___ ___ ",0x0A, 0x0D
db "  | | | |  _| | | | .'|  _| | | | . | -_|",0x0A, 0x0D
db "  |_| |_|___| |_| |__,|___| |_| |___|___|",0x0A, 0x0D
db "                                         ",0x0A, 0x0D
db "                                         ",0x0A, 0x0D,'$'
NEXT_PLAYER_STRING: db "Player turn: $"
PLAYERO: db "Player O",0x0A, 0x0D,'$'
PLAYERX: db "Player X",0x0A, 0x0D,'$'
END_LINE: db 0x0A,0x0D,'$'
CURRENT: db 'O'
MAP_DATA: db "123456789"
MAP_HORIZONTAL: db "-------", 0x0A,0x0D,'$'



MAIN_LOOP:
    LXI D, NEXT_PLAYER_STRING
    CALL PUT_STRING
    LDA CURRENT
    LXI D, PLAYERO
    CPI 'O'
    JZ IS_O
    LXI D, PLAYERX
IS_O:
    CALL PUT_STRING
    CALL GET_MOVE
    LXI D, END_LINE
    CALL PUT_STRING
    CALL PRINT_MAP
    JMP MAIN_LOOP

GET_MOVE:
    CALL GET_CHAR
    CPI '1'
    JC GET_MOVE
    CPI '9' + 1
    JNC GET_MOVE
    LXI D, MAP_DATA
    XCHG
    SBI '0'
    MVI D, 0
    MOV E, A
    DAD D
    XCHG
    LDAX D
    CPI 'O'
    JZ GET_MOVE
    CPI 'X'
    JZ GET_MOVE
    LDA CURRENT
    STAX D
    CPI 'X'
    JZ CHANGE_CURRENT
    MVI A, 'X'
    JMP END_IF
CHANGE_CURRENT:
    MVI A, 'O'
    JMP END_IF
END_IF:
    STA CURRENT
    RET

DRAW_LINE:
    LXI H, MAP_DATA
    DAD D ; offset

    MVI E, '|'
    CALL PUT_CHAR

    XCHG
    LDAX D
    MOV B, E
    MOV E, A
    CALL PUT_CHAR
    MOV E, B
    XCHG
    
    MVI E, '|'
    CALL PUT_CHAR

    INX H
    XCHG
    LDAX D
    MOV B, E
    MOV E, A
    CALL PUT_CHAR
    MOV E, B
    XCHG

    MVI E, '|'
    CALL PUT_CHAR

    INX H
    XCHG
    LDAX D
    MOV B, E
    MOV E, A
    CALL PUT_CHAR
    MOV E, B
    XCHG

    MVI E, '|'
    CALL PUT_CHAR
    LXI D, END_LINE
    CALL PUT_STRING
    RET

PRINT_MAP:
    LXI D, MAP_HORIZONTAL
    CALL PUT_STRING
    LXI D, 0
    CALL DRAW_LINE
    LXI D, MAP_HORIZONTAL
    CALL PUT_STRING
    LXI D, 3
    CALL DRAW_LINE
    LXI D, MAP_HORIZONTAL
    CALL PUT_STRING
    LXI D, 6
    CALL DRAW_LINE
    LXI D, MAP_HORIZONTAL
    CALL PUT_STRING
    RET

GET_CHAR:
    MVI C, 1 ; C_READ
    CALL 0x0005
    RET

PUT_CHAR:
    MVI C, 2 ; C_WRITE
    CALL 0x0005
    RET

PUT_STRING:
    MVI C, 9 ; C_WRITESTR
    CALL 0x0005
    RET