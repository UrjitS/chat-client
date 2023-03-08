//
// Created by chika on 07/03/23.
//
#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <curses.h>
#include <signal.h>
#include <time.h>

// max number of messages in chat history
#define MAX_MESSAGES 100
// max length of each message
#define MAX_MESSAGE_LENGTH 256
// max length for username and password
#define MAX_USERNAME_LENGTH 20
#define MAX_PASSWORD_LENGTH 20

typedef struct {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
} User;

typedef struct {
    char text[MAX_MESSAGE_LENGTH];
    char timestamp[MAX_MESSAGE_LENGTH];
    int sender;
} Message;

void values_init(Message *messages);
void init_ncurses();
void print_sections(int message_bar_row, int input_row, int input_col, char input_buffer[MAX_MESSAGE_LENGTH]);
void show_login_menu();
void show_signup_menu();
int validate_credentials(User user);
void show_menu(Message messages[MAX_MESSAGES], int num_messages, int scroll_offset, int max_row, int input_row, int input_col,
               char input_buffer[MAX_MESSAGE_LENGTH], int input_length);

_Noreturn void run(Message messages[MAX_MESSAGES], int num_messages, int scroll_offset, int max_row, int input_row, int input_col,
                   char input_buffer[MAX_MESSAGE_LENGTH], int input_length);

int main() {
    // array to store chat history
    Message messages[MAX_MESSAGES];
    values_init(messages);
    endwin();
    return 0;
}

void values_init(Message *messages) {
    // number of messages in chat history
    int num_messages = 0;
    // number of messages to scroll up or down by
    int scroll_offset = 0;
    // maximum row and column dimensions of terminal window
    int max_row, max_col;
    // row and column where user input starts
    int input_row, input_col;
    // buffer to store user input
    char input_buffer[MAX_MESSAGE_LENGTH];
    // length of user input
    int input_length = 0;

    init_ncurses();

    // get maximum row and column dimensions of terminal window
    getmaxyx(stdscr, max_row, max_col);

    input_row = max_row - 2;
    input_col = 2;
    //print_sections(message_bar_row, input_row);

//    run(messages, num_messages, scroll_offset, max_row, max_col, input_row, input_col, input_buffer,
//        input_length);

    show_menu(messages, num_messages, scroll_offset, max_row, input_row, input_col, input_buffer, input_length);

//    run(messages, num_messages, scroll_offset, max_row, input_row, input_col, input_buffer, input_length);
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
void print_sections(int message_bar_row, int input_row, int input_col, char input_buffer[MAX_MESSAGE_LENGTH]) {
    mvprintw(message_bar_row, input_col, "Welcome to the chat! :) ");
    mvprintw(input_row, input_col, "Type a message and press 'ENTER' to send: %s", input_buffer);
}

_Noreturn void run(Message messages[MAX_MESSAGES], int num_messages, int scroll_offset, int max_row, int input_row, int input_col,
                   char input_buffer[MAX_MESSAGE_LENGTH], int input_length) {

    int message_bar_row;
    message_bar_row = max_row - 3;

    // ignore window resize signals initially
    signal(SIGWINCH, SIG_IGN);

    while (1) {
        // get current window size
        int cur_row, cur_col;
        getmaxyx(stdscr, cur_row, cur_col);

        // handle window resize
        if (cur_row != max_row || cur_col != MAX_MESSAGE_LENGTH + 2){
            max_row = cur_row;
            clear();
        }

        // print messages
        for (int i = 0; i < max_row - 4 && i + scroll_offset < num_messages; ++i) {
            // print messages with sender indicated
            if (messages[i + scroll_offset].sender == 0) {
                mvprintw(i + 1, 0, "[%s] You: %s",
                         messages[i + scroll_offset].timestamp, messages[i + scroll_offset].text);
            } else {
                mvprintw(i + 1, 0, "[%s] Stranger: %s",
                         messages[i + scroll_offset].timestamp, messages[i + scroll_offset].text);
            }
        }

        // display user input
        print_sections(message_bar_row, input_row, input_col, input_buffer);

        // get user input
        int ch = getch();
        if (ch == KEY_ENTER || ch == '\n') {
            // add user input to chat history with sender set to 0
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            strftime(messages[num_messages].timestamp,
                     sizeof(messages[num_messages].timestamp), "%Y-%m-%d %H:%M:%S", tm);
            strncpy(messages[num_messages].text, input_buffer, MAX_MESSAGE_LENGTH);
            messages[num_messages].sender = 0;
            num_messages++;
            if (num_messages > MAX_MESSAGES) {
                // if chat history is full, remove the oldest message
                for (int i = 0; i < MAX_MESSAGES - 1; ++i) {
                    messages[i].text[0] = messages[i+1].text[0];
                    messages[i].sender = messages[i+1].sender;
                    strcpy(messages[i].timestamp, messages[i+1].timestamp);
                }
                num_messages = MAX_MESSAGES;
            }
            // clear input buffer and reset input length
            memset(input_buffer, 0, MAX_MESSAGE_LENGTH);
            input_length = 0;
        }
        else if (ch == KEY_BACKSPACE || ch == 127 || ch == KEY_DC)
        {
            // delete character from input buffer
            if (input_length > 0)
            {
                input_buffer[input_length - 1] = '\0';
                input_length--;
            }
        }
        else if (ch == KEY_UP)
        {
            // scroll up chat history
            if (scroll_offset < num_messages - max_row + 4)
            {
                scroll_offset--;
            }
        }
        else if (ch >= 32 && ch <= 126 && input_length < MAX_MESSAGE_LENGTH + 1)
        {
            // character to input buffer
            if (input_length < MAX_MESSAGE_LENGTH - 1)
            {
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

void show_menu(Message messages[MAX_MESSAGES], int num_messages, int scroll_offset, int max_row, int input_row, int input_col,
               char input_buffer[MAX_MESSAGE_LENGTH], int input_length){
    int is_login = 0;
    while (1){
        clear();
        if (is_login){
            show_login_menu();
        } else {
            show_signup_menu();
        }
        refresh();

        int ch = getch();
        if (ch == KEY_ENTER || ch == '\n'){
            // validate user credentials
            User user;
            echo();
            mvprintw(input_row + 2, 0, "Enter username: ");
            getnstr(user.username, MAX_USERNAME_LENGTH);
            mvprintw(input_row + 3, 0, "Enter password: ");
            getnstr(user.password, MAX_PASSWORD_LENGTH);
            noecho();
            int valid = validate_credentials(user);
            if (valid){
                // start the chat
                run((Message *) messages, num_messages, scroll_offset, max_row,
                    input_row, input_col, input_buffer, input_length);
            } else {
                // invalid credentials
                mvprintw(input_row + 5, 0, "Invalid username or password. Press any key to continue.");
                getch();
            }
        } else if (ch == KEY_DOWN || ch == KEY_UP) {
            // switch between login and signup menu
            is_login = !is_login;
        }
    }
}

void show_login_menu(){
    clear();

    // Define variables for menu positions
    int title_row, title_col, username_row, username_col, password_row, password_col;

    // Adjust positions based on terminal size
    getmaxyx(stdscr, title_row, title_col);
    title_row /= 3;
    title_col = (title_col - strlen("*** Login ***")) / 2;
    username_row = title_row + 2;
    username_col = (title_col - strlen("Enter your username: ")) / 2;
    password_row = username_row + 1;
    password_col = (title_col - strlen("Enter your password: ")) / 2;

    // Print menu with adjusted positions
    mvprintw(title_row, title_col, "*** Login ***");
    mvprintw(username_row, username_col, "Enter your username: ");
    mvprintw(password_row, password_col, "Enter your password: ");
    move(username_row, username_col + strlen("Enter your username: "));
    refresh();
}

void show_signup_menu(){
    clear();

    // Define variables for menu positions
    int title_row, title_col, username_row, username_col, password_row, password_col;

    // Adjust positions based on terminal size
    getmaxyx(stdscr, title_row, title_col);
    title_row /= 3;
    title_col = (title_col - strlen("*** Sign Up ***")) / 2;
    username_row = title_row + 2;
    username_col = (title_col - strlen("Enter a username: ")) / 2;
    password_row = username_row + 1;
    password_col = (title_col - strlen("Enter a password: ")) / 2;

    // Print menu with adjusted positions
    mvprintw(title_row, title_col, "*** Sign Up ***");
    mvprintw(username_row, username_col, "Enter a username: ");
    mvprintw(password_row, password_col, "Enter a password: ");

    refresh();
}

int validate_credentials(User user){
    const char* valid_username = "test";
    const char* valid_password = "test";

    if (strcmp(user.username, valid_username) == 0 && strcmp(user.password, valid_password) == 0){
        return 1;
    } else
        return 0;
}