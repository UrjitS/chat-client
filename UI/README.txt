
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


//_Noreturn void
//run(char messages[100][256], int num_messages, int scroll_offset, int max_row, int max_col, int input_row, int input_col,
//    char input_buffer[256], int input_length);


//void run(char messages[100][256], int num_messages, int scroll_offset, int max_row, int max_col, int input_row, int input_col,
//         char input_buffer[256], int input_length) {
//    while (1) {
//        // calculate position and height of scroll bar
//        int bar_pos = (int)(float)scroll_offset / (float )num_messages * (float )(max_row - 4);
//        int bar_height = (int)((float)(max_row - 4) / (float )num_messages * (float )(max_row - 4));
//
//        // print scroll bar
//        for (int i = 0; i < max_row - 4; ++i) {
//            mvprintw(i + 1, max_col - 2, "|");
//        }
//
//        for (int i = bar_pos; i < bar_pos + bar_height && i < max_row - 4; ++i) {
//            mvprintw(i + 1, max_col - 2, "#");
//        }
//
//        for (int i = 0; i < max_row - 4 && i + scroll_offset < num_messages; ++i) {
////             mvprintw(i + 1, 0, messages[i + scroll_offset]);
//            mvprintw(i + 1, 0, "%s", messages[i + scroll_offset]);
//        }
//
//        // display user input
////        mvprintw(input_row, input_col, input_buffer);
//        mvprintw(input_row, input_col, "%s", input_buffer);
//
//        // get user input
//        int ch = getch();
//        if (ch == KEY_ENTER || ch == '\n') {
//            // add user input to chat history
//            // prompt user for name or identifier
//            char name[20];
//            mvprintw(input_row, 0, "Enter your name or identifier: ");
//            getstr(name);
//            snprintf(messages[num_messages], MAX_MESSAGE_LENGTH + 20, "%s: %s", name, input_buffer);
//            //strncpy(messages[num_messages], input_buffer, MAX_MESSAGE_LENGTH);
//            num_messages++;
//            if (num_messages > MAX_MESSAGES) {
//                // if chat history is full, remove the oldest message
//                for (int i = 0; i < MAX_MESSAGES - 1; ++i) {
//                    //messages[i][0] = messages[i+1][0];
//                    strncpy(messages[i], messages[i + 1], MAX_MESSAGE_LENGTH + 20);
//                }
//                num_messages = MAX_MESSAGES;
//            }
//
//            // clear input buffer and reset input length
//            memset(input_buffer, 0, MAX_MESSAGE_LENGTH);
//            input_length = 0;
//        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == KEY_DC) {
//            // delete character from input buffer
//            if (input_length > 0) {
//                input_buffer[input_length - 1] = '\0';
//                input_length--;
//            }
//        } else if (ch == KEY_UP) {
//            // scroll up chat history
//            if (scroll_offset < num_messages - max_row + 4) {
//                scroll_offset--;
//            }
//        } else if (ch >= 32 && ch <= 126 && input_length < MAX_MESSAGE_LENGTH + 1) {
//            // character to input buffer
//            if (input_length < MAX_MESSAGE_LENGTH - 1) {
//                input_buffer[input_length] = ch;
//                input_length++;
//            }
//        }
//        // clear screen
//        clear();
//
//        // refresh screen
//        refresh();
//    }
//}
