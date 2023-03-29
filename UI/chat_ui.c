#include <arpa/inet.h>
#include <ncurses.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dlfcn.h>
#include <netinet/in.h>
#include <stdbool.h>

#define MAX_MESSAGES 100
#define MAX_MESSAGE_LENGTH 256
#define MAX_USERNAME_LENGTH 20
#define MAX_PASSWORD_LENGTH 20
#define MAX_EMAIL_LENGTH 50
#define RESPONSE_BUFFER 100

typedef struct {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    char email[MAX_EMAIL_LENGTH];
} User;

typedef struct {
    char text[MAX_MESSAGE_LENGTH];
    char timestamp[MAX_MESSAGE_LENGTH];
    int sender;
} Message;

typedef struct {
    Message messages[MAX_MESSAGES];
    char input_buffer[MAX_MESSAGE_LENGTH];
    char input[MAX_USERNAME_LENGTH + MAX_PASSWORD_LENGTH + MAX_EMAIL_LENGTH + 9];
    char *channels;
    char *command;
    char *channel_password;
    char *channel_publicity;
    int num_messages;
    int scroll_offset;
    int max_row;
    int max_col;
    int input_row;
    int input_col;
    int input_length;
    int message_bar_row;
    int communicate_to_client;
    int client_to_ui;
} ChatState;

void values_init(ChatState *chat);

void init_ncurses(void);

void print_sections(ChatState *chat);

void show_login_menu(ChatState *chat);

void show_signup_menu(ChatState *chat);

void print_messages(ChatState *chat, User *user);

void get_user_input(ChatState *chat);

void resize_handler(ChatState *chat);

void show_menu(ChatState *chat);

void create_channel(ChatState *chat);

_Noreturn void run(ChatState *chat, User *user);

int validate_login_credentials(User *user);

int validate_new_user_credentials(User *user);

int main(int argc, char *argv[]) {
    bool has_port = false;

    int communicate_to_client[2];
    int client_to_ui[2];
    ChatState *chatState = malloc(sizeof(ChatState));

    // check if server ip is provided
    if (argc < 2 || inet_addr(argv[1]) == (in_addr_t) (-1)) {
        printf("Usage: %s <server_ip> [port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (argc == 3) {
        has_port = true;
    }

    // Create pipe
    if (pipe(communicate_to_client) == -1) {
        fprintf(stderr, "UI Pipe failed");
        return 1;
    }
    if (pipe(client_to_ui) == -1) {
        fprintf(stderr, "Client Pipe failed");
        return 1;
    }

    // Fork process
    int num = fork();

    if (num == 0) {
        close(communicate_to_client[1]);  // close the writing end of the pipe
        close(client_to_ui[0]);  // close the reading end of the pipe

        dup2(communicate_to_client[0], STDIN_FILENO);   // redirect the read end of the pipe to stdin
        dup2(client_to_ui[1], STDOUT_FILENO);  // redirect stdout to the write end of the pipe

        close(communicate_to_client[0]);  // close the read end of the pipe
        close(client_to_ui[1]);  // close the writing end of the pipe

        // Open server in background
        if (has_port) {
            char *args[] = {"./scalable_server", argv[1], argv[2], NULL};
            execvp(args[0], args);
        } else {
            char *args[] = {"./scalable_server", argv[1], NULL};
            execvp(args[0], args);
        }

    } else {
        // parent process
        sleep(1);
        close(communicate_to_client[0]);  // close the read end of the pipe
        close(client_to_ui[1]);  // close the writing end of the pipe

        chatState->communicate_to_client = communicate_to_client[1];
        chatState->client_to_ui = client_to_ui[0];

        char input_str[MAX_MESSAGE_LENGTH];
        read(chatState->client_to_ui, input_str, MAX_MESSAGE_LENGTH);
        input_str[strlen(input_str) - 1] = '\0';

        if (strcmp(input_str, "Server Running") == 0) {
            values_init(chatState);
            show_menu(chatState);
            endwin();

            // Wait for child process to finish
            wait(NULL);
            // Close all pipes and exit
            close(communicate_to_client[1]);  // close the pipe
            close(client_to_ui[0]);  // close the pipe
            free(chatState);
            return EXIT_SUCCESS;
        } else {
            printf("Server could not connect\n");
            return EXIT_FAILURE;
        }
    }
}

/**
 * initializes values of the ChatState struct
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
}

/**
 * initialize ncurses
 * */
void init_ncurses(void) {
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
    mvprintw(chat->message_bar_row - 1, chat->input_col, "Channel name: %s", chat->channels);
    mvprintw(chat->message_bar_row, chat->input_col, "Welcome to the chat! :) ");
    mvprintw(chat->input_row, chat->input_col, "Type a message and press 'ENTER' to send: %s", chat->input_buffer);
}

/**
 * this function displays the chat window, in here you see the messages sent. It also resizes the window
 *
 * @param chat ChatState struct
 * @param user User struct
 * */
_Noreturn void run(ChatState *chat, User *user) {
    chat->message_bar_row = chat->max_row - 3;

    // ignore window resize signals initially
    signal(SIGWINCH, SIG_IGN);
    // TODO if the entered string contains a "/" then it is a command (i.e. join channel, leave channel)
    // If not then it is a message to be sent to the server
    // Send message to server in the format:
    // CREATE M <display-name> <channel-name> <message-content>

    while (1) {
        // handles window resizes
        resize_handler(chat);
        // print messages
        print_messages(chat, user);
        // display user input
        print_sections(chat);
        // get user input
        get_user_input(chat);
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
void show_menu(ChatState *chat) {
    int is_login = 0;
    while (1) {
        clear();
        switch (is_login) {
            case 0:
                show_signup_menu(chat);
                break;
            case 1:
                show_login_menu(chat);
                break;
            default:
                break;
        }
        refresh();
        int ch = getch();
        if (ch == KEY_DOWN || ch == KEY_UP) {
            // switch between login and signup menu
            is_login = !is_login;
        }
    }
}

/**
 * this function displays the contents of the signup menu, prompts the user for a username, password and email
 *
 * @param chat ChatState struct
 * */
void show_signup_menu(ChatState *chat) {
    clear();

    // Define variables for menu positions
    int title_row, title_col, username_row, username_col, password_row, password_col, email_row, email_col;

    // Adjust positions based on terminal size
    getmaxyx(stdscr, title_row, title_col);
    title_row /= 3;
    title_col = (title_col - strlen("*** Sign Up ***")) / 2;
    username_row = title_row + 2;
    username_col = (title_col - strlen("Enter a username: ")) / 2;
    password_row = username_row + 1;
    password_col = (title_col - strlen("Enter a password: ")) / 2;
    email_row = password_row + 1;
    email_col = (title_col - strlen("Enter an email: ")) / 2;

    // Print menu with adjusted positions
    mvprintw(title_row, title_col, "*** Sign Up ***");
    mvprintw(username_row, username_col, "Enter a username: ");
    move(username_row, username_col + strlen("Enter a username: "));

    // Read username input and display it on the screen
    char username[MAX_USERNAME_LENGTH];
    getstr(username);
    mvprintw(username_row, username_col + strlen("Enter a username: "), "%s", username);

    mvprintw(password_row, password_col, "Enter a password: ");
    move(password_row, password_col + strlen("Enter a password: "));

    // Read password input and display asterisks instead of the actual characters
    char password[MAX_PASSWORD_LENGTH];
    getstr(password);
    mvprintw(password_row, password_col + strlen("Enter a password: "), "%s", password);

    mvprintw(email_row, email_col - 1, "Enter an email: ");
    move(email_row, email_col + strlen("Enter an email: "));

    // Read email input and display it
    char email[MAX_EMAIL_LENGTH];
    getstr(email);
    mvprintw(email_row, email_col + strlen("Enter an email: "), "%s", email);

    User *user = malloc(sizeof(User));

    memset(user->username, 0, MAX_USERNAME_LENGTH);
    memset(user->password, 0, MAX_PASSWORD_LENGTH);
    memset(user->email, 0, MAX_EMAIL_LENGTH);

    username[strlen(username)] = '\0';
    password[strlen(password)] = '\0';
    email[strlen(email)] = '\0';

    strcpy(user->username, username);
    strcpy(user->password, password);
    strcpy(user->email, email);
    // validate the inputs before sending
    // CREATE U <login-token> <display name> <password>
    if (strlen(user->username) == 0 || strlen(user->password) == 0 || strlen(user->email) == 0) {
        show_signup_menu(chat);
    }

    sprintf(chat->input, "CREATE U %s %s %s", email, username, password);
    write(chat->communicate_to_client, chat->input, strlen(chat->input));

    // Read from client and check response
    char response[RESPONSE_BUFFER];
    ssize_t read_number = read(chat->client_to_ui, response, sizeof(response));
    response[read_number] = '\0';

    if (strcmp(response, "OK") == 0) {
        show_login_menu(chat);
    } else {
        // prints the servers response on the GUI
        mvprintw(0, title_col, "Server response: %s", response);
        mvprintw(1, title_col, "Hit ENTER key to restart");
    }
    refresh();
}

/**
 * this displays the contents of the login menu, prompts the user for their username and password
 *
 * @param chat ChatState struct
 * */
void show_login_menu(ChatState *chat) {
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
    move(username_row, username_col + strlen("Enter your username: "));

    // Read username input and display it on the screen
    char username[MAX_USERNAME_LENGTH];
    getstr(username);
    mvprintw(username_row, username_col + strlen("Enter your username: "), "%s", username);

    mvprintw(password_row, password_col, "Enter your password: ");
    move(password_row, password_col + strlen("Enter your password: "));

    // Read password input and display asterisks instead of the actual characters
    char password[MAX_PASSWORD_LENGTH];
    getstr(password);
    mvprintw(password_row, password_col + strlen("Enter your password: "), "%s", password);

    User *user = malloc(sizeof(User));

    memset(user->username, 0, MAX_USERNAME_LENGTH);
    memset(user->password, 0, MAX_PASSWORD_LENGTH);

    username[strlen(username)] = '\0';
    password[strlen(password)] = '\0';

    strcpy(user->username, username);
    strcpy(user->password, password);

    // Make sure values are not empty or only spaces before sending to server
    if (strlen(user->username) == 0 || strlen(user->password) == 0) {
        show_login_menu(chat);
    }
    sprintf(chat->input, "CREATE A %.*s %.*s", MAX_USERNAME_LENGTH, username, MAX_PASSWORD_LENGTH, password);
    write(chat->communicate_to_client, chat->input, strlen(chat->input));

    // Read from client and check response
    char response[RESPONSE_BUFFER];
    ssize_t read_number = read(chat->client_to_ui, response, sizeof(response));
    response[read_number] = '\0';

    char *resp_code = strtok(response, " ");

    if (strcmp(resp_code, "OK") == 0) {
        chat->channels = strdup(strtok(NULL, "\0"));
        run(chat, user);
    } else {
        // prints the servers response on the GUI
        mvprintw(0, title_col, "Server response: %s", response);
        mvprintw(1, title_col, "Hit ENTER key to restart");
    }
    refresh();
}

/**
 * this function validates the Users login credentials, it makes use of dummy data for now
 *
 * @param user User to be validated
 * */
int validate_login_credentials(User *user) {
    int valid = 1, invalid = 0;

    const char *valid_username = "test";
    const char *valid_password = "test";

    bool condition = strcmp(user->username, valid_username) == 0 && strcmp(user->password, valid_password) == 0;

    if (condition) {
        return valid;
    } else
        return invalid;
}

/**
 * this function validates a new Users credentials
 *
 * @param user User to me validated
 * */
int validate_new_user_credentials(User *user) {
    int valid = 1, invalid = 0;

    bool condition = strlen(user->username) > 0 && strlen(user->password) > 0 && strlen(user->email) > 0;

    if (condition) {
        return valid;
    } else
        return invalid;
}

/**
 * this function prints the messages on the chat window
 *
 * @param chat ChatState struct
 * @param user User struct
 * */
void print_messages(ChatState *chat, User *user) {
    for (int i = 0; i < chat->max_row - 4 && i + chat->scroll_offset < chat->num_messages; ++i) {
        char *slash = strchr(chat->messages[i + chat->scroll_offset].text, '/');
        char join_msg[MAX_MESSAGE_LENGTH];
        if (slash != NULL) {
            char *command = strtok(slash, " ");
            char *channel_name = strtok(NULL, " ");
            char *channel_publicity = strtok(NULL, " ");
            char *channel_password = strtok(NULL, " ");

            // check if the command is "/join"
            if (strcmp(command, "/join") == 0) {
                // parse the publicity parameter
                if (channel_publicity != NULL) {
                    if (strcmp(channel_publicity, "0") == 0) {
                        // send the join channel message to the server
                        snprintf(join_msg, MAX_MESSAGE_LENGTH, "CREATE C %s %s %s %s", channel_name,
                                 user->username, channel_publicity, channel_password);
                        write(chat->communicate_to_client, join_msg, strlen(join_msg));

                        char response[RESPONSE_BUFFER];
                        ssize_t read_number = read(chat->client_to_ui, response, sizeof(response));
                        response[read_number] = '\0';

                        char *resp_code = strtok(response, " ");

                        if (strcmp(resp_code, "OK") != 0) {
                            // prints the servers response on the GUI
                            mvprintw(0, 100, "Server response: %s", response);
                            mvprintw(1, 100, "Hit ENTER key to restart");
                        }
                    } else {
                        snprintf(join_msg, MAX_MESSAGE_LENGTH, "CREATE C %s %s %s", channel_name,
                                 user->username, channel_publicity);
                        write(chat->communicate_to_client, join_msg, strlen(join_msg));

                        char response[RESPONSE_BUFFER];
                        ssize_t read_number = read(chat->client_to_ui, response, sizeof(response));
                        response[read_number] = '\0';
                        char *resp_code = strtok(response, " ");

                        if (strcmp(resp_code, "OK") != 0) {
                            // prints the servers response on the GUI
                            mvprintw(0, 100, "Server response: %s", response);
                            mvprintw(1, 100, "Hit ENTER key to restart");
                        }
                    }
                }
            } else {
                // print messages with sender's username and timestamp
                if (chat->messages[i + chat->scroll_offset].sender == 0) {
                    mvprintw(i + 1, 0, "[%s] %s: %s",
                             chat->messages[i + chat->scroll_offset].timestamp, user->username,
                             chat->messages[i + chat->scroll_offset].text);
                } else {
                    mvprintw(i + 1, 0, "[%s] Goofy: %s",
                             chat->messages[i + chat->scroll_offset].timestamp,
                             chat->messages[i + chat->scroll_offset].text);
                }
            }
        }
    }
}

/**
 * this function handles window resizes
 *
 * @param chat ChatState struct
 * */
void resize_handler(ChatState *chat) {
    int cur_row, cur_col;
    getmaxyx(stdscr, cur_row, cur_col);
    // handle window resize
    if (cur_row != chat->max_row || cur_col != MAX_MESSAGE_LENGTH + 2) {
        chat->max_row = cur_row;
        clear();
    }
}

/**
 * this function retrieves user input and handles the chat history
 *
 * @param chat ChatState struct
 * */
void get_user_input(ChatState *chat) {
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
                chat->messages[i].text[0] = chat->messages[i + 1].text[0];
                chat->messages[i].sender = chat->messages[i + 1].sender;
                strcpy(chat->messages[i].timestamp, chat->messages[i + 1].timestamp);
            }
            chat->num_messages = MAX_MESSAGES;
        }
        // clear input buffer and reset input length
        memset(chat->input_buffer, 0, MAX_MESSAGE_LENGTH);
        chat->input_length = 0;
    } else if (ch == KEY_BACKSPACE || ch == 127 || ch == KEY_DC) {
        // delete character from input buffer
        if (chat->input_length > 0) {
            chat->input_buffer[chat->input_length - 1] = '\0';
            chat->input_length--;
        }
    } else if (ch == KEY_DOWN) {
        // scroll up chat history
        if (chat->scroll_offset < chat->num_messages - chat->max_row + 4) {
            chat->scroll_offset++;
        }
    } else if (ch >= 32 && ch <= 126 && chat->input_length < MAX_MESSAGE_LENGTH + 1) {
        // character to input buffer
        if (chat->input_length < MAX_MESSAGE_LENGTH - 1) {
            chat->input_buffer[chat->input_length] = ch;
            chat->input_length++;
        }
    }
}