This is a chat client application written in C that interacts with a scalable chat server. It uses ncurses for user
interface and communicates with the server using pipes. The main function reads a server IP address as a command-line
argument and creates two pipes for communication. Then, it forks a process and runs the scalable server in the
child process. The parent process then waits for the child process to finish, closes all pipes, and exits.

The ChatState struct stores the state of the chat client, including the chat history, user input, and scroll offset.
The values_init() function initializes the values of the ChatState struct and calls the run() function. The run()
function displays the chat window, prints messages, handles window resizes, and displays user input.

The print_sections() function prints out the specified text in the chat window, and the print_messages() function prints
the chat history. The get_user_input() function gets user input from the keyboard and handles user input events.

The show_login_menu() and show_signup_menu() functions display the login and sign-up menus, respectively.
The validate_login_credentials() function validates user credentials by comparing them with stored user credentials.

Overall, this chat client application provides a basic chat interface and communicates with the scalable chat server
using pipes. It can be improved by adding more features such as private messaging, file sharing, and user profiles.
