//
// Created by chika on 07/03/23.
//
#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <curses.h>

// max number of messages in chat history
#define MAX_MESSAGES 100

// max length of each message
#define MAX_MESSAGE_LENGTH 256

void values_init();

void init_ncurses();

void setup_bar_and_user_input_sections(int message_bar_row, int input_row);

typedef struct {
    char text[MAX_MESSAGE_LENGTH];
    int sender;
} Message;

_Noreturn void run(Message messages[MAX_MESSAGES], int num_messages, int scroll_offset, int max_row, int input_row, int input_col,
                   char input_buffer[MAX_MESSAGE_LENGTH], int input_length);

int main() {
    //printf("Hello, World!\n");
    values_init();
    endwin();
    return 0;
}

void values_init() {
    // array to store chat history
    Message messages[MAX_MESSAGES][MAX_MESSAGE_LENGTH];
    // number of messages in chat history
    int num_messages = 0;
    // number of messages to scroll up or down by
    int scroll_offset = 0;
    // maximum row and column dimensions of terminal window
    int max_row, max_col;
    // row where message bar starts
    int message_bar_row;
    // row and column where user input starts
    int input_row, input_col;
    // buffer to store user input
    char input_buffer[MAX_MESSAGE_LENGTH];
    // length of user input
    int input_length = 0;

    init_ncurses();

    // get maximum row and column dimensions of terminal window
    getmaxyx(stdscr, max_row, max_col);

    message_bar_row = max_row - 3;
    input_row = max_row - 2;
    input_col = 2;
    setup_bar_and_user_input_sections(message_bar_row, input_row);

//    run(messages, num_messages, scroll_offset, max_row, max_col, input_row, input_col, input_buffer,
//        input_length);

    run((Message *) messages, num_messages, scroll_offset, max_row, input_row, input_col, input_buffer, input_length);
}

/**
 * initialize ncurses
 * */
void init_ncurses() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
}

/**
 * sets up message bar and user input sections
 *
 * @param max_row max row to be set
 * */
void setup_bar_and_user_input_sections(int message_bar_row, int input_row) {
    mvprintw(message_bar_row, 0, "Chat history: ");
    mvprintw(input_row, 0, "Type a message and press 'ENTER' to send: ");
}

_Noreturn void run(Message messages[MAX_MESSAGES], int num_messages, int scroll_offset, int max_row, int input_row, int input_col,
                   char input_buffer[MAX_MESSAGE_LENGTH], int input_length) {
    while (1) {
        for (int i = 0; i < max_row - 4 && i + scroll_offset < num_messages; ++i) {
            // print messages with sender indicated
            if (messages[i + scroll_offset].sender == 0) {
                mvprintw(i + 1, 0, "You: %s", messages[i + scroll_offset].text);
            } else {
                mvprintw(i + 1, 0, "Stranger: %s", messages[i + scroll_offset].text);
            }
        }

        // display user input
        mvprintw(input_row, input_col, "%s", input_buffer);

        // get user input
        int ch = getch();
        if (ch == KEY_ENTER || ch == '\n') {
            // add user input to chat history with sender set to 0
            strncpy(messages[num_messages].text, input_buffer, MAX_MESSAGE_LENGTH);
            messages[num_messages].sender = 0;
            num_messages++;
            if (num_messages > MAX_MESSAGES) {
                // if chat history is full, remove the oldest message
                for (int i = 0; i < MAX_MESSAGES - 1; ++i) {
                    messages[i].text[0] = messages[i+1].text[0];
                    messages[i].sender = messages[i+1].sender;
                }
                num_messages = MAX_MESSAGES;
            }

            // clear input buffer and reset input length
            memset(input_buffer, 0, MAX_MESSAGE_LENGTH);
            input_length = 0;
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == KEY_DC) {
            // delete character from input buffer
            if (input_length > 0) {
                input_buffer[input_length - 1] = '\0';
                input_length--;
            }
        } else if (ch == KEY_UP) {
            // scroll up chat history
            if (scroll_offset < num_messages - max_row + 4) {
                scroll_offset--;
            }
        } else if (ch >= 32 && ch <= 126 && input_length < MAX_MESSAGE_LENGTH + 1) {
            // character to input buffer
            if (input_length < MAX_MESSAGE_LENGTH - 1) {
                input_buffer[input_length] = ch;
                input_length++;
            }
        }
        // clear screen
        clear();

        // refresh screen
        refresh();
    }
}
