//
// Created by chika on 07/03/23.
//
#include <ncurses.h>
#include <string.h>
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

typedef struct {
    Message messages[MAX_MESSAGES];
    char input_buffer[MAX_MESSAGE_LENGTH];
    int num_messages;
    int scroll_offset;
    int max_row;
    int max_col;
    int input_row;
    int input_col;
    int input_length;
    int message_bar_row;
}ChatState;

void values_init(ChatState *chat);
void init_ncurses();
void print_sections(ChatState *chat);
void show_login_menu();
void show_signup_menu();
int validate_credentials(User user);
void show_menu(ChatState *chat);
_Noreturn void run(ChatState *chat);

int main() {
    ChatState chatState;
    values_init(&chatState);
    endwin();
    return 0;
}

/**
 * initializes values of the ChatState struct and calls the show_menu() or run_menu() function
 *
 * @param chat the ChatState struct
 * */
void values_init(ChatState *chat) {
    // number of messages in chat history
    chat->num_messages = 0;
    // number of messages to scroll up or down by
    chat->scroll_offset = 0;
    // length of user input
    chat->input_length = 0;

    init_ncurses();

    // get maximum row and column dimensions of terminal window
    getmaxyx(stdscr, chat->max_row, chat->max_col);

    chat->input_row = chat->max_row - 2;
    chat->input_col = 2;

    show_menu(chat);
//    run(chat);
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
 *  prints out the specified text in the chat window
 *
 * @param chat ChatState struct
 * */
void print_sections(ChatState *chat) {
    mvprintw(chat->message_bar_row, chat->input_col, "Welcome to the chat! :) ");
    mvprintw(chat->input_row, chat->input_col, "Type a message and press 'ENTER' to send: %s", chat->input_buffer);
}

/**
 * this function displays the chat window, in here you see the messages sent. It also resizes the window
 *
 * @param chat ChatState struct
 * */
_Noreturn void run(ChatState *chat) {

    chat->message_bar_row = chat->max_row - 3;

    // ignore window resize signals initially
    signal(SIGWINCH, SIG_IGN);

    while (1) {
        // get current window size
        int cur_row, cur_col;
        getmaxyx(stdscr, cur_row, cur_col);

        // handle window resize
        if (cur_row != chat->max_row || cur_col != MAX_MESSAGE_LENGTH + 2){
            chat->max_row = cur_row;
            clear();
        }

        // print messages
        for (int i = 0; i < chat->max_row - 4 && i + chat->scroll_offset < chat->num_messages; ++i) {
            // print messages with sender indicated
            if (chat->messages[i + chat->scroll_offset].sender == 0) {
                mvprintw(i + 1, 0, "[%s] You: %s",
                         chat->messages[i + chat->scroll_offset].timestamp, chat->messages[i + chat->scroll_offset].text);
            } else {
                mvprintw(i + 1, 0, "[%s] Stranger: %s",
                         chat->messages[i + chat->scroll_offset].timestamp, chat->messages[i + chat->scroll_offset].text);
            }
        }

        // display user input
        print_sections(chat);
        // get user input
        int ch = getch();
        if (ch == KEY_ENTER || ch == '\n') {
            // add user input to chat history with sender set to 0
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            strftime(chat->messages[chat->num_messages].timestamp,
                     sizeof(chat->messages[chat->num_messages].timestamp), "%Y-%m-%d %H:%M:%S", tm);
            strncpy(chat->messages[chat->num_messages].text, chat->input_buffer, MAX_MESSAGE_LENGTH);
            chat->messages[chat->num_messages].sender = 0;
            chat->num_messages++;
            if (chat->num_messages > MAX_MESSAGES) {
                // if chat history is full, remove the oldest message
                for (int i = 0; i < MAX_MESSAGES - 1; ++i) {
                    chat->messages[i].text[0] = chat->messages[i+1].text[0];
                    chat->messages[i].sender = chat->messages[i+1].sender;
                    strcpy(chat->messages[i].timestamp, chat->messages[i+1].timestamp);
                }
                chat->num_messages = MAX_MESSAGES;
            }
            // clear input buffer and reset input length
            memset(chat->input_buffer, 0, MAX_MESSAGE_LENGTH);
            chat->input_length = 0;
        }
        else if (ch == KEY_BACKSPACE || ch == 127 || ch == KEY_DC)
        {
            // delete character from input buffer
            if (chat->input_length > 0)
            {
                chat->input_buffer[chat->input_length - 1] = '\0';
                chat->input_length--;
            }
        }
        else if (ch == KEY_UP)
        {
            // scroll up chat history
            if (chat->scroll_offset < chat->num_messages - chat->max_row + 4)
            {
                chat->scroll_offset--;
            }
        }
        else if (ch >= 32 && ch <= 126 && chat->input_length < MAX_MESSAGE_LENGTH + 1)
        {
            // character to input buffer
            if (chat->input_length < MAX_MESSAGE_LENGTH - 1)
            {
                chat->input_buffer[chat->input_length] = ch;
                chat->input_length++;
            }
        }
        // clear screen
        clear();
        // refresh screen
        refresh();
    }
}

/**
 * this function shows the startup menu, it first shows the sign up window and if you press the down arrow key it shows
 * the login window
 *
 * @param chat ChatState struct
 * */
void show_menu(ChatState *chat){
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
            mvprintw(chat->input_row + 2, 0, "Enter username: ");
            getnstr(user.username, MAX_USERNAME_LENGTH);
            mvprintw(chat->input_row + 3, 0, "Enter password: ");
            getnstr(user.password, MAX_PASSWORD_LENGTH);
            noecho();
            int valid = validate_credentials(user);
            if (valid){
                // start the chat
                run(chat);
            } else {
                // invalid credentials
                mvprintw(chat->input_row + 5, 0, "Invalid username or password. Press any key to continue.");
                getch();
            }
        } else if (ch == KEY_DOWN || ch == KEY_UP) {
            // switch between login and signup menu
            is_login = !is_login;
        }
    }
}

/**
 * this displays the contents of the login menu
 *
 * */
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

/**
 * this function displays the contents of the signup menu
 *
 * */
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
    move(username_row, username_col + strlen("Enter your username: "));

    refresh();
}

/**
 * this function validates the Users credentials, it makes use of dummy data for now
 *
 * @param user User to me validated
 * */
int validate_credentials(User user){
    int valid = 1, invalid = 0;

    const char* valid_username = "test";
    const char* valid_password = "test";

    if (strcmp(user.username, valid_username) == 0 && strcmp(user.password, valid_password) == 0){
        return valid;
    } else
        return invalid;
}