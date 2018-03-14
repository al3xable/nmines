#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

#define CL_DEFAULT 1
#define CL_FIELD_MINE 10
#define CL_FIELD_FLAG 11
#define CL_FIELD_OPEN 12
#define CL_FIELD_CUR 13
#define CL_FIELD_NONE 14

#define ST_GAME 0
#define ST_LOSE 1
#define ST_WIN 2

// Window
uint8_t scr_x, scr_y;

// Mines settings
uint8_t size_x = 8;
uint8_t size_y = 8;
uint8_t mines = 10;

// Game status
uint8_t cur_x = 0;
uint8_t cur_y = 0;
uint8_t st_game = 0;

typedef struct {
    bool mine, flag, open;
} point;

point **field;

void gen_field() {
    srand((unsigned int) time(0));

    field = calloc(size_x, sizeof(point *));

    for (uint8_t i = 0; i < size_x; i++) {
        field[i] = calloc(size_y, sizeof(point));

        for (uint8_t j = 0; j < size_y; j++) {
            field[i][j].mine = false;
            field[i][j].flag = false;
            field[i][j].open = false;
        }
    }

    for (uint8_t i = 0; i < mines; i++) {
        uint8_t x = (uint8_t) (rand() % size_x);
        uint8_t y = (uint8_t) (rand() % size_y);

        if (field[x][y].mine) i--;
        field[x][y].mine = true;
    }
}

// Count of around mines
uint8_t mines_around(uint8_t x, uint8_t y) {
    uint8_t c = 0;

    // LEFT
    if (x > 0 && y > 0 && field[x - 1][y - 1].mine) c++;
    if (y > 0 && field[x][y - 1].mine) c++;
    if (x < size_x - 1 && y > 0 && field[x + 1][y - 1].mine) c++;

    // Center
    if (x > 0 && field[x - 1][y].mine) c++;
    if (x < size_x - 1 && field[x + 1][y].mine) c++;

    // RIGHT
    if (x > 0 && y < size_y - 1 && field[x - 1][y + 1].mine) c++;
    if (y < size_y - 1 && field[x][y + 1].mine) c++;
    if (x < size_x - 1 && y < size_y - 1 && field[x + 1][y + 1].mine) c++;

    return c;
}

// Open all mines
void mine_open_all() {
    for (uint8_t i = 0; i < size_x; i++) {
        for (uint8_t j = 0; j < size_y; j++) {
            if (field[i][j].mine)
                field[i][j].open = true;
        }
    }
}

#define MINE_CHECK(x, y) x >= 0 && x <= size_x-1 && y >= 0 && y <= size_y-1

// Open mine
void mine_open(uint8_t x, uint8_t y) {
    field[x][y].open = true;

    // Check
    uint8_t c = 0;
    for (uint8_t i = 0; i < size_x; i++) {
        for (uint8_t j = 0; j < size_y; j++) {
            if ((field[i][j].mine && !field[i][j].open) || (!field[i][j].mine && field[i][j].open)) c++;
        }
    }

    if (field[x][y].mine) {
        mine_open_all();
        st_game = ST_LOSE;
    } else if (c == size_x * size_y) {
        mine_open_all();
        st_game = ST_WIN;
    } else if (mines_around(x, y) == 0) {
        if (MINE_CHECK(x - 1, y - 1) && !field[x - 1][y - 1].open && !field[x - 1][y - 1].mine) mine_open(x - 1, y - 1);
        if (MINE_CHECK(x, y - 1) && !field[x][y - 1].open && !field[x][y - 1].mine) mine_open(x, y - 1);
        if (MINE_CHECK(x + 1, y - 1) && !field[x + 1][y - 1].open && !field[x + 1][y - 1].mine) mine_open(x + 1, y - 1);

        if (MINE_CHECK(x - 1, y) && !field[x - 1][y].open && !field[x - 1][y].mine) mine_open(x - 1, y);
        if (MINE_CHECK(x + 1, y) && !field[x + 1][y].open && !field[x + 1][y].mine) mine_open(x + 1, y);

        if (MINE_CHECK(x - 1, y + 1) && !field[x - 1][y + 1].open && !field[x - 1][y + 1].mine) mine_open(x - 1, y + 1);
        if (MINE_CHECK(x, y + 1) && !field[x][y + 1].open && !field[x][y + 1].mine) mine_open(x, y + 1);
        if (MINE_CHECK(x + 1, y + 1) && !field[x + 1][y + 1].open && !field[x + 1][y + 1].mine) mine_open(x + 1, y + 1);
    }
}

// Render mines field
void render_field(uint8_t px, uint8_t py) {
    for (uint8_t i = 0; i < size_x; i++) {
        for (uint8_t j = 0; j < size_y; j++) {
            bool cur = cur_x == i && cur_y == j;
            if (field[i][j].open && field[i][j].mine) {
                attron(COLOR_PAIR(cur ? CL_FIELD_CUR : CL_FIELD_MINE));
                mvprintw(px + i, py + j * 2, "* ");
            } else if (field[i][j].open) {
                attron(COLOR_PAIR(cur ? CL_FIELD_CUR : CL_FIELD_OPEN));

                uint8_t mines = mines_around(i, j);
                char str[3] = "\0";
                str[0] = (char) (mines == 0 ? ' ' : (char) (mines + '0'));
                str[1] = ' ';
                mvprintw(px + i, py + j * 2, str);
            } else if (field[i][j].flag) {
                attron(COLOR_PAIR(cur ? CL_FIELD_CUR : CL_FIELD_FLAG));
                mvprintw(px + i, py + j * 2, "FF");
            } else {
                attron(COLOR_PAIR(cur ? CL_FIELD_CUR : CL_FIELD_NONE));
                mvprintw(px + i, py + j * 2, "  ");
            }
        }

        attron(COLOR_PAIR(CL_DEFAULT));
    }

    attron(COLOR_PAIR(CL_DEFAULT));
}

// Play game
void start_game() {
    erase();
    bkgd(COLOR_PAIR(CL_DEFAULT));
    st_game = 0;
    gen_field();

    // Game doing
    do {
        render_field(0, 0);
        refresh();

        if (st_game != ST_GAME) break;

        switch (getch()) {
            case 65: // UP
                if (cur_x > 0) cur_x--;
                break;

            case 66: // DOWN
                if (cur_x < size_x - 1) cur_x++;
                break;

            case 67: // RIGHT
                if (cur_y < size_y - 1) cur_y++;
                break;

            case 68: // LEFT
                if (cur_y > 0) cur_y--;
                break;

            case 32: // SPACE
                if (!field[cur_x][cur_y].flag)
                    mine_open(cur_x, cur_y);
                break;

            case 'f': // f
                field[cur_x][cur_y].flag = !field[cur_x][cur_y].flag;
                break;

            case 'x': // X
                return;

            default:
                break;
        }
    } while (1);

    // Status message
    switch (st_game) {
        case ST_LOSE:
            mvprintw(size_x + 2, 0, "You lose.");
            break;

        case ST_WIN:
            mvprintw(size_x + 2, 0, "You win!");
            break;

        default:
            break;
    }

    getch();
    erase();
    bkgd(COLOR_PAIR(CL_DEFAULT));
}

// Menu interface
void open_menu() {
    uint8_t menu_pos = 0, menu_size = 4;

    do {
        // Render menu
        mvprintw(0, 0, "nMines 0.1.0");
        mvprintw(2, 0, "Difficulty:");
        mvprintw(3, 0, "%s Easy (8x8)", (menu_pos == 0 ? "->" : "  "));
        mvprintw(4, 0, "%s Medium (16x16)", (menu_pos == 1 ? "->" : "  "));
        mvprintw(5, 0, "%s Hard (30x16)", (menu_pos == 2 ? "->" : "  "));
        mvprintw(6, 0, "%s EXIT", (menu_pos == 3 ? "->" : "  "));

        // Switches
        switch (getch()) {
            case 65: // UP
                if (menu_pos > 0) menu_pos--;
                break;

            case 66: // DOWN
                if (menu_pos < menu_size - 1) menu_pos++;
                break;

            case 32: // SPACE
                if (menu_pos == 3) return;
                else if (menu_pos == 0) size_x = 8, size_y = 8, mines = 10;
                else if (menu_pos == 1) size_x = 16, size_y = 16, mines = 40;
                else if (menu_pos == 2) size_x = 16, size_y = 30, mines = 99;
                start_game();
                break;

            default:
                break;
        }
    } while (1);
}

int main(uint8_t argc, char *argv[]) {
    initscr();
    noecho();
    curs_set(FALSE);
    getmaxyx(stdscr, scr_x, scr_y);

    if (has_colors() == FALSE) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    start_color();

    // Color pairs
    init_pair(CL_DEFAULT, COLOR_WHITE, COLOR_BLACK);
    init_pair(CL_FIELD_FLAG, COLOR_WHITE, COLOR_YELLOW);
    init_pair(CL_FIELD_MINE, COLOR_WHITE, COLOR_RED);
    init_pair(CL_FIELD_OPEN, COLOR_WHITE, COLOR_CYAN);
    init_pair(CL_FIELD_CUR, COLOR_BLACK, COLOR_WHITE);
    init_pair(CL_FIELD_NONE, COLOR_WHITE, COLOR_BLUE);
    bkgd(COLOR_PAIR(CL_DEFAULT));

    open_menu();

    curs_set(TRUE);
    endwin();
}
