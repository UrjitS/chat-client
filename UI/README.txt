
This code creates a basic chat UI with a message bar and user input section.
The chat history is stored in a two-dimensional array of characters, and up to `MAX_MESSAGES` messages can be stored.
The `scroll_offset` variable keeps track of how many messages to scroll up or down by.

In the main loop, the chat history is displayed by iterating through the array and printing each message to the screen.

The user input is displayed at the bottom of the screen, and user input is read from the keyboard using the `getch()` function.

If the user presses Enter, the input is added to the chat history array, and if the array is full, the oldest message is removed.

If the user presses Backspace, the last character is removed from the input buffer. If the user presses the up or down arrow keys,
the chat history is scrolled up or down, respectively. If the user types a printable character, it is added to the input buffer.

Finally, the screen is cleared and refreshed to update the display. The program runs in an infinite loop until it is terminated by the user.

Note that this code is only a starting point and may need to be modified depending on your specific requirements.
For example, you may want to add support for displaying usernames or timestamps, or you may want to add a graphical
scroll bar instead of simply scrolling the chat history up and down.


//        int ch = getch();
//        if (ch == KEY_ENTER || ch == '\n'){
//            // validate user credentials
//            User user;
//            echo();
//            mvprintw(state->input_row + 2, 0, "Enter username: ");
//            getnstr(user.username, MAX_USERNAME_LENGTH);
//            mvprintw(state->input_row + 3, 0, "Enter password: ");
//            getnstr(user.password, MAX_PASSWORD_LENGTH);
//            noecho();
//            int valid = validate_credentials(user);
//            if (valid){
//                // start the chat
//                run(state);
//            } else {
//                // invalid credentials
//                mvprintw(state->input_row + 5, 0, "Invalid username or password. Press any key to continue.");
//                getch();
//            }
//        } else if (ch == KEY_DOWN || ch == KEY_UP) {
//            // switch between login and signup menu
//            is_login = !is_login;
//        }
