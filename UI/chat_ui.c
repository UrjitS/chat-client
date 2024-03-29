#include <arpa/inet.h>
#include <ncurses.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <semaphore.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>

#define SEM_NAME "/user_messages_sem"
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
    char username[MAX_USERNAME_LENGTH];
    int sender;
} Message;

typedef struct {
    Message messages[MAX_MESSAGES];
    char input_buffer[MAX_MESSAGE_LENGTH];
    char input[MAX_USERNAME_LENGTH + MAX_PASSWORD_LENGTH + MAX_EMAIL_LENGTH + 9];
    char *available_channel;
    char *current_channel;
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

void get_user_input(ChatState *chat, User *user);

void resize_handler(ChatState *chat);

_Noreturn void show_menu(ChatState *chat);

void handle_create_channel(ChatState *chat, const User *user, char *slash);

void handle_join_channel(ChatState *chat, const User *user, char *slash);

void handle_leaving_channel(ChatState *chat, const User *user, char *slash);

void handle_logout(ChatState *chat, const User *user, char *slash);

void handle_send_messages(ChatState *chat, const User *user, char *slash);

_Noreturn void run(ChatState *chat, User *user);

void *thread_message(void *arg);

sem_t * messaging_semaphore;
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

    // Create a semaphore with initial value 1 in the parent process
    messaging_semaphore = sem_open(SEM_NAME, O_CREAT | O_RDWR, 0644, 0);
    if (messaging_semaphore == SEM_FAILED) {
        perror("sem_open");
        exit(1);
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
//            endwin();
//
//            // Wait for child process to finish
//            wait(NULL);
//            // Close all pipes and exit
//            close(communicate_to_client[1]);  // close the pipe
//            close(client_to_ui[0]);  // close the pipe
//            free(chatState);
//            sem_close(messaging_semaphore);
//            sem_unlink(SEM_NAME);
//            return EXIT_SUCCESS;
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
    chat->current_channel = "N/A";
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
    mvprintw(chat->message_bar_row - 1, chat->input_col, "Available Channels: %s", chat->available_channel);
    mvprintw(chat->message_bar_row, chat->input_col, "Current Channel: %s", chat->current_channel);
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
    // start thread
    pthread_t thread;
    pthread_create(&thread, NULL, thread_message, chat);
    while (1) {
        // handles window resizes
        resize_handler(chat);
        // print messages
        print_messages(chat, user);
        // display user input
        print_sections(chat);
        // get user input
        get_user_input(chat, user);
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
_Noreturn void show_menu(ChatState *chat) {
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
    int title_row, title_col, username_row, username_col, password_row, email_row;

    // Adjust positions based on terminal size
    getmaxyx(stdscr, title_row, title_col);
    title_row /= 3;
    title_col = (title_col - strlen("*** Sign Up ***")) / 2;
    username_row = title_row + 2;
    username_col = (title_col - strlen("Enter a username: ")) / 2;
    password_row = username_row + 1;
    email_row = password_row + 1;

    // Print menu with adjusted positions
    mvprintw(4 , 0, "If you have an account already, type 'login' as your username and hit enter");
    mvprintw(title_row, title_col, "*** Sign Up ***");
    mvprintw(username_row, username_col, "Enter a username: ");
    move(username_row, username_col + strlen("Enter a username: "));

    // Read username input and display it on the screen
    char username[MAX_USERNAME_LENGTH];
    getstr(username);
    if(strcmp(username, "login") == 0){
        show_login_menu(chat);
    }
    mvprintw(username_row, username_col + strlen("Enter a username: "), "%s", username);

    mvprintw(password_row, username_col, "Enter a password: ");
    move(password_row, username_col + strlen("Enter a password: "));

    // Read password input and display asterisks instead of the actual characters
    char password[MAX_PASSWORD_LENGTH];
    getstr(password);
    mvprintw(password_row, username_col + strlen("Enter a password: "), "%s", password);

    mvprintw(email_row, username_col, "Enter an email: ");
    move(email_row, username_col + strlen("Enter an email: "));

    // Read email input and display it
    char email[MAX_EMAIL_LENGTH];
    getstr(email);
    mvprintw(email_row, username_col + strlen("Enter an email: "), "%s", email);

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

    char *resp_code = strtok(response, "\n");

    if (strcmp(resp_code, "OK") == 0) {
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
    int title_row, title_col, username_row, username_col, password_row, email_row;

    // Adjust positions based on terminal size
    getmaxyx(stdscr, title_row, title_col);
    title_row /= 3;
    title_col = (title_col - strlen("*** Login ***")) / 2;
    username_row = title_row + 2;
    username_col = (title_col - strlen("Enter your display name: ")) / 2;
    password_row = username_row + 1;
    email_row = password_row + 1;

    // Print menu with adjusted positions
    mvprintw(title_row, title_col, "*** Login ***");
    mvprintw(username_row, username_col, "Enter your display name: ");
    move(username_row, username_col + strlen("Enter your display name: "));

    // Read username input and display it on the screen
    char username[MAX_USERNAME_LENGTH];
    getstr(username);
    mvprintw(username_row, username_col + strlen("Enter your display name: "), "%s", username);

    mvprintw(password_row, username_col, "Enter your password: ");
    move(password_row, username_col + strlen("Enter your password: "));

    // Read password input and display asterisks instead of the actual characters
    char password[MAX_PASSWORD_LENGTH];
    getstr(password);
    mvprintw(password_row, username_col + strlen("Enter your password: "), "%s", password);

    mvprintw(email_row, username_col, "Enter your email: ");
    move(email_row, username_col + strlen("Enter your email: "));

    // Read email input and display it
    char email[MAX_EMAIL_LENGTH];
    getstr(email);
    mvprintw(email_row, username_col + strlen("Enter your email: "), "%s", email);

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

    // Make sure values are not empty or only spaces before sending to server
    if (strlen(user->username) == 0 || strlen(user->password) == 0 || strlen(user->email) == 0) {
        show_login_menu(chat);
    }
    sprintf(chat->input, "CREATE A %.*s %.*s", MAX_EMAIL_LENGTH, email, MAX_PASSWORD_LENGTH, password);
    write(chat->communicate_to_client, chat->input, strlen(chat->input));

    // Read from client and check response
    char response[RESPONSE_BUFFER];
    ssize_t read_number = read(chat->client_to_ui, response, sizeof(response));
    response[read_number] = '\0';

    char * resp_code = strtok(response, " ");

    if (strcmp(resp_code, "OK") == 0) {
        char channels[1024] = "";
        char * channel;
        while ((channel = strtok(NULL, "\2")) != NULL) {
            strcat(channels, channel);
            strcat(channels, ", ");
        }
        chat->available_channel = strdup(channels);

        run(chat, user);
    } else {
        // prints the servers response on the GUI
        mvprintw(0, title_col, "Server response: %s", response);
        mvprintw(1, title_col, "Hit ENTER key to restart");
    }
    refresh();
}

/**
 * this function prints the messages on the chat window
 *
 * @param chat ChatState struct
 * @param user User struct
 * */
void print_messages(ChatState *chat, User *user) {
    for (int i = 0; i < chat->max_row - 4 && i + chat->scroll_offset < chat->num_messages; ++i) {
        // print messages with sender's username and timestamp
        mvprintw(i + 1, 0, "[%s] %s: %s",
                 chat->messages[i + chat->scroll_offset].timestamp, chat->messages[i + chat->scroll_offset].username,
                 chat->messages[i + chat->scroll_offset].text);
    }
}

void * thread_message(void *arg){
    ChatState* chatState = (ChatState*) arg;

    // wait from semaphore to become available
    while (1) {
        sem_wait(messaging_semaphore);
        // Read from client and check response
        char response[RESPONSE_BUFFER];
        ssize_t read_number = read(chatState->client_to_ui, response, sizeof(response));
        response[read_number] = '\0';

        char * user_name = strtok(response, "\3");
        char * user_message = strtok(NULL, "\3");
        char * time_stamp = strtok(NULL, "\3");

        if (user_name != NULL && user_message != NULL && strcmp(chatState->current_channel, "N/A") != 0) {

            // add message to chat history
            strncpy(chatState->messages[chatState->num_messages].text, user_message, MAX_MESSAGE_LENGTH);
            strncpy(chatState->messages[chatState->num_messages].username, user_name, MAX_USERNAME_LENGTH);

            time_t timestamp_seconds = strtol(time_stamp, NULL, 16);
            struct tm *tm = localtime(&timestamp_seconds);

            char formatted_time[20];
            strftime(formatted_time, 20, "%Y-%m-%d %H:%M:%S", tm);

            strcpy(chatState->messages[chatState->num_messages].timestamp, formatted_time);

            chatState->num_messages++;

            Message * message = &chatState->messages[chatState->num_messages];
            User *user = malloc(sizeof(User));

            memset(user->username, 0, MAX_USERNAME_LENGTH);
            memset(user->password, 0, MAX_PASSWORD_LENGTH);
            memset(user->email, 0, MAX_EMAIL_LENGTH);

            strcpy(user->username, "username");
            strcpy(user->password, "password");
            strcpy(user->email, "email");

            message->sender = 1;

//            print_messages(chatState, user);
            refresh();
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
void get_user_input(ChatState *chat, User *user) {
    // get user input
    int ch = getch();
    if (ch == KEY_ENTER || ch == '\n') {

        if (strlen(chat->input_buffer) > 0) {
            handle_logout(chat, user, strdup(chat->input_buffer));
            handle_create_channel(chat, user, strdup(chat->input_buffer));
            handle_join_channel(chat, user, strdup(chat->input_buffer));
            handle_leaving_channel(chat, user, strdup(chat->input_buffer));
            handle_send_messages(chat, user, strdup(chat->input_buffer));
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

/**
 * this functions handles a create channel request
 *
 * @param chat ChatState struct
 * @param user User struct
 * @param slash the token we use to identify a command
 * */
void handle_create_channel(ChatState *chat, const User *user, char *slash) {
    char join_msg[MAX_MESSAGE_LENGTH];
    char *command = strtok(slash, " ");
    char *channel_name = strtok(NULL, " ");
    char *channel_publicity = strtok(NULL, " ");


    // check if the command is "/create" to create a channel
    if (strcmp(command, "/create") == 0) {
        // parse the publicity parameter
        if (channel_publicity != NULL) {
            snprintf(join_msg, MAX_MESSAGE_LENGTH, "CREATE C %s %s %s", channel_name,
                     user->username, channel_publicity);
            write(chat->communicate_to_client, join_msg, strlen(join_msg));

            char response[RESPONSE_BUFFER];
            ssize_t read_number = read(chat->client_to_ui, response, sizeof(response));
            response[read_number] = '\0';

            char *resp_code = strtok(strdup(response), "\n");

            if (strcmp(resp_code, "OK") == 0) {
                strncpy(chat->messages[(chat->num_messages)].text, "Created Channel\0", MAX_MESSAGE_LENGTH);
                // Add to available channels
                char channel_list[1024];
                strcpy(channel_list, chat->available_channel);
                strcat(channel_list, ", ");
                strcat(channel_list, channel_name);
                chat->available_channel = strdup(channel_list);
                chat->current_channel = strdup(channel_name);
            } else {
                strncpy(chat->messages[(chat->num_messages)].text, response, MAX_MESSAGE_LENGTH);
            }

        }
    }
}

void handle_send_messages(ChatState *chat, const User *user, char *slash) {
    char join_msg[MAX_MESSAGE_LENGTH];

    // send display name, channel name, and message content to server

    if (strchr(slash, '/') == NULL && strcmp(chat->current_channel, "N/A") != 0) {
        snprintf(join_msg, MAX_MESSAGE_LENGTH, "CREATE M %s %s %s\n", user->username,
                 chat->current_channel, slash);
        write(chat->communicate_to_client, join_msg, strlen(join_msg));

        char response[RESPONSE_BUFFER];
        ssize_t read_number = read(chat->client_to_ui, response, sizeof(response));
        response[read_number] = '\0';

        char *resp_code = strtok(strdup(response), "\n");

        if (strcmp(resp_code, "OK2") != 0) {
            strncpy(chat->messages[(chat->num_messages)].text, response, MAX_MESSAGE_LENGTH);
        }
    }
}

/**
 * this functions handles a join channel request
 *
 * @param chat ChatState struct
 * @param user User struct
 * @param slash the token we use to identify a command
 * */
void handle_join_channel(ChatState *chat, const User *user, char *slash) {
    char join_msg[MAX_MESSAGE_LENGTH];
    char * token, *rset;
    token = strtok_r(slash, " ", &rset);

    // check if the command is "/join <name>" to create a channel
    if (strcmp(token, "/join") == 0 && strcmp(chat->current_channel, rset) != 0) {
        // parse the publicity parameter
        if (rset != NULL) {
            // send the join channel message to the server
            snprintf(join_msg, MAX_MESSAGE_LENGTH, "UPDATE C J %s %s\0", user->username, rset);
            write(chat->communicate_to_client, join_msg, strlen(join_msg));

            char response[RESPONSE_BUFFER];
            ssize_t read_number = read(chat->client_to_ui, response, sizeof(response));
            response[read_number] = '\0';

            char *resp_code = strtok(strdup(response), "\n");

            if (strcmp(resp_code, "OK") == 0) {
                // prints the servers response on the GUI
                strncpy(chat->messages[(chat->num_messages)].text, " \0", MAX_MESSAGE_LENGTH);
                chat->current_channel = strdup(rset);
                // Send a read message to the server
//                snprintf(join_msg, MAX_MESSAGE_LENGTH, "READ M %s %s", channel_name, "5");
//                write(chat->communicate_to_client, join_msg, strlen(join_msg));
                //  read the messages from the server and display them

            } else {
                strncpy(chat->messages[(chat->num_messages)].text, response, MAX_MESSAGE_LENGTH);
            }
        }
    }
}

/**
 * this functions handles a leave channel request
 *
 * @param chat ChatState struct
 * @param user User struct
 * @param slash the token we use to identify a command
 * */
void handle_leaving_channel(ChatState *chat, const User *user, char *slash) {
    char join_msg[MAX_MESSAGE_LENGTH];
    char *command = strtok(slash, " ");
    char *channel_name = strtok(NULL, " ");

    // check if the command is "/leave <name>" to create a channel
    if (strcmp(command, "/leave") == 0) {
        // parse the publicity parameter
        if (channel_name != NULL) {
            // send the leave channel message to the server
            snprintf(join_msg, MAX_MESSAGE_LENGTH, "UPDATE C R %s %s", user->username, channel_name);
            write(chat->communicate_to_client, join_msg, strlen(join_msg));

            char response[RESPONSE_BUFFER];
            ssize_t read_number = read(chat->client_to_ui, response, sizeof(response));
            response[read_number] = '\0';

            char *resp_code = strtok(strdup(response), "\n");

            if (strcmp(resp_code, "OK") == 0) {
                // prints the servers response on the GUI
                strncpy(chat->messages[(chat->num_messages)].text, "Leaved Channel\0", MAX_MESSAGE_LENGTH);
                chat->current_channel = strdup("N/A");
            } else {
                strncpy(chat->messages[(chat->num_messages)].text, response, MAX_MESSAGE_LENGTH);
            }
        }
    }
}

/**
 * this functions handles a logout request
 *
 * @param chat ChatState struct
 * @param user User struct
 * @param slash the token we use to identify a command
 * */
void handle_logout(ChatState *chat, const User *user, char *slash) {
    char join_msg[MAX_MESSAGE_LENGTH];
    char *command = strtok(slash, " ");

    // check if the command is "/logout" to logout
    if (strcmp(command, "/logout") == 0) {
        // send the logout message to the server
        snprintf(join_msg, MAX_MESSAGE_LENGTH, "DESTROY A %s", user->username);
        write(chat->communicate_to_client, join_msg, strlen(join_msg));

        char response[RESPONSE_BUFFER];
        ssize_t read_number = read(chat->client_to_ui, response, sizeof(response));
        response[read_number] = '\0';

        char *resp_code = strtok(strdup(response), "\n");

        if (strcmp(resp_code, "OK") == 0) {
            // if okay this should redirect and show the login menu
            show_login_menu(chat);
        } else {
            strncpy(chat->messages[(chat->num_messages)].text, response, MAX_MESSAGE_LENGTH);
            return;
        }
    }
}