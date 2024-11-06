// This file only contains macros

#ifndef PRIVATE_H
#define PRIVATE_H

#define SCREEN_W 512
#define SCREEN_H 512

#define CELL_SZ 64

#define NUM_OF_CHESS_PIECES 32
#define HALF_PIECE_SZ 32

#define TEAM_SIZE 16
#define CELLS_PER_ROW (SCREEN_W / CELL_SZ)

#define BOARD_SZ 64

#define ROOK    1
#define KNIGHT  2
#define BISHOP  3
#define QUEEN   4
#define KING    5
#define PAWN    6

#define MAX_PLAYERS 2

#define TRUE 1
#define FALSE 0

#define TEXTURE_POOL_SIZE 32
#define PIECE_POOL_SIZE 32
#define PROMOTION_PIECES_POOL_SIZE 4

#define CHECK_IDX_RANGE(x) (x < 0 || x >= BOARD_SZ)

#define ever ;;

#define strcat_macro(str1, str2) str1 " " #str2

#define PROMOTION_PIECES_COUNT 4

#define INVALID_INDEX -1

#define MAX_GAME_STATES 4

#define UPPER_LEFT_ROOK_INDEX 0
#define UPPER_RIGHT_ROOK_INDEX 7
#define LOWER_LEFT_ROOK_INDEX 56
#define LOWER_RIGHT_ROOK_INDEX 63

#define MAX_QUEUE_SIZE 50
#define MAX_BUFFER_SIZE 64

#define LMB_INDEX 1

#define CHECK(f, ret, msg) if(!f){\
    fprintf(stderr, strcat_macro(msg, "\n"));\
    return ret;\
}\

#endif