#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LENGTH_COMMAND 300
#define MAX_NO_TASKS 100
#define MAX_LENGTH_TITLE 100
#define MAX_LENGTH_DESCRIPTION 200
#define MAX_LENGTH_TIME 33
#define WEEK_CELL_FIRST_COL_WIDTH 10
#define WEEK_CELL_OTHER_COL_WIDTH 20
// Add task data structure
enum Status {IN_PROGRESS, DONE, ARCHIVED};
char * status_name[] = {"In Progress", "Done", "Archived"};
enum CommandType {ADD, EDIT, SHOW, DELETE, QUIT, INVALID};
char * command_name[] = {"ADD", "EDIT", "SHOW", "DELETE", "QUIT", "INVALID"};

struct Task {
    int num;
    char title[MAX_LENGTH_TITLE+1];
    char description[MAX_LENGTH_DESCRIPTION+1];
    char time[MAX_LENGTH_TIME+1];
    enum Status status;
};
void printTask(struct Task * task) {
    printf("--------------------------------------------\n");
    printf("Num: #%d. Title: %s\n", task->num, task->title);
    printf("Description: %s\n", task->description);
    printf("Status: %s\n", status_name[task->status]);
    printf("--------------------------------------------\n");
}

void printUnsupportedTime(struct Task * task) {
    printf("----- Show week view -----\n");
    printf("Error: Unsupported time with non-zero minutes: %s\n", task->time);
    printf("In Task:\n");
    printTask(task);
}

// REQ 1typedef enum CommandType CommandType;
enum CommandType getCommandType(char* command) {
    char first_word[MAX_LENGTH_COMMAND];
    int i = 0;

    // Extract the first word
    while (command[i] != '\0' && command[i] != ' ') {
        first_word[i] = toupper(command[i]);
        i++;
    }
    first_word[i] = '\0';

    for (int j = 0; j < sizeof(command_name) / sizeof(command_name[0]); j++) {
        if (strcasecmp(first_word, command_name[j]) == 0) {
            return j;
        }
    }

    return INVALID;
}


// REQ 2 
void getTitleFromAdd(char* command, char* out_title) {
    char* start = strstr(command, "[");
    char* end = strstr(command, "]");
    if (start && end && end > start) {
        strncpy(out_title, start + 1, end - start - 1);
        out_title[end - start - 1] = '\0';
    }
}

void getDescriptionFromAdd(char* command, char* out_description) {
    char* start = strstr(command, "] [");
    if (start) {
        start += 3; // To skip the "] ["
        char* end = strstr(start, "]");
        if (end && end > start) {
            strncpy(out_description, start, end - start);
            out_description[end - start] = '\0';
        }
    }
}
void getTimeFromAdd(char* command, char* out_time) {
    char* start = strrchr(command, '['); // To find the last occurrence of [
    if (start) {
        start += 1; // To skip the "["
        char* end = strstr(start, "]");
        if (end && end > start) {
            strncpy(out_time, start, end - start);
            out_time[end - start] = '\0';
        }
    }
}
void getTitleFromEdit(char *command, char *out_title) {
    char *title_start = strstr(command, "title:[");
    if (title_start != NULL) {
        title_start += strlen("title:[");
        char *title_end = strchr(title_start, ']');
        if (title_end != NULL) {
            size_t title_length = title_end - title_start;
            strncpy(out_title, title_start, title_length);
            out_title[title_length] = '\0';
        }
    }
}

void getDescriptionFromEdit(char *command, char *out_description) {
    char *description_start = strstr(command, "description:[");
    if (description_start != NULL) {
        description_start += strlen("description:[");
        char *description_end = strchr(description_start, ']');
        if (description_end != NULL) {
            size_t description_length = description_end - description_start;
            strncpy(out_description, description_start, description_length);
            out_description[description_length] = '\0';
        }
    }
}

void getTimeFromEdit(char *command, char *out_time) {
    char *time_start = strstr(command, "time:[");
    if (time_start != NULL) {
        time_start += strlen("time:[");
        char *time_end = strchr(time_start, ']');
        if (time_end != NULL) {
            size_t time_length = time_end - time_start;
            strncpy(out_time, time_start, time_length);
            out_time[time_length] = '\0';
        }
    }
}
// Req 9: Status
enum Status getStatusFromEdit(char *edit_cmd) {
    char *status_start = strstr(edit_cmd, "status:[");
    if (status_start != NULL) {
        status_start += strlen("status:[");
        char *status_end = strchr(status_start, ']');
        if (status_end != NULL) {
            char status[20];
            size_t status_length = status_end - status_start;
            strncpy(status, status_start, status_length);
            status[status_length] = '\0';
            
            if (strcmp(status, "In Progress") == 0) {
                return IN_PROGRESS;
            } else if (strcmp(status, "Done") == 0) {
                return DONE;
            } else if (strcmp(status, "Archived") == 0) {
                return ARCHIVED;
            }
        }
    }
    
    // Default status if not found or invalid
    return IN_PROGRESS;
}
// REQ 3
int checkTitle(char * raw_title) {
    // Check if the title length is within the allowed limit
    if (strlen(raw_title) > MAX_LENGTH_TITLE) {
        return strlen(raw_title); // Return the current length of the title
    }
    // Check if the title starts or ends with a whitespace character
    if (raw_title[0] == ' ') {
        return 0; // Error: Title starts or ends with a whitespace character
    } else if (raw_title[strlen(raw_title) - 1] == ' ') {
        return strlen(raw_title);
    }
    
    // Define allowed characters for the title
    char allowedCharacters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ,.-:|/";

    // Check if the title contains only allowed characters
    for (int i = 0; raw_title[i] != '\0'; i++) {
        int isValidChar = 0;
        for (int j = 0; allowedCharacters[j] != '\0'; j++) {
            if (raw_title[i] == allowedCharacters[j]) {
                isValidChar = 1;
                break;
            }
        }
        if (!isValidChar) {
            return i; // Return the position of the first invalid character
        }
    }

    return -1;
}

// REQ 4
int checkDescription(char* raw_description) {
    int length = strlen(raw_description);

    // Check maximum length condition
    if (length > MAX_LENGTH_DESCRIPTION) {
        return length;
    }

    // Check other conditions
    for (int i = 0; i < length; i++) {
        char ch = raw_description[i];

        // Check if it's a valid character
        if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
              (ch >= '0' && ch <= '9') || ch == ' ' || ch == ',' ||
              ch == '.' || ch == '-' || ch == ':' ||
              ch == '|' || ch == '/')) {
            return i;
        }

        // Check for leading or trailing whitespace
        if (i == 0 && ch == ' ') {
            return i;
        }
        if (i == length - 1 && ch == ' ') {
            return i;
        }
    }

    return -1; // Description is valid
}
// REQ 5
int isLeapYear(int year) {
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}
int isValidDate(int day, int month, int year) {
    if (month < 1 || month > 12) return 1;  // Invalid month
    if (day < 1) return 1;                   // Invalid day

    int maxDays = 31;

    if (month == 4 || month == 6 || month == 9 || month == 11) {
        maxDays = 30;
    } else if (month == 2) {
        maxDays = isLeapYear(year) ? 29 : 28;
    }

    return day > maxDays;
}

int isValidHour(int hour) {
    return hour < 0 || hour > 23;
}
int isValidMinute(int minute) {
    return minute < 0 || minute > 59;
}
int isValidMonth(int month) {
    return month < 1 || month > 12;
}

int isValidYear(int year) {
    return year <= 0;
}

int checkTime(char *raw_time) {
    int hour1, minute1, day1, month1, year1;
    int hour2, minute2, day2, month2, year2;

    // Parse raw_time into individual components
    if (sscanf(raw_time, "%2d:%2d|%2d/%2d/%4d-%2d:%2d|%2d/%2d/%4d",
               &hour1, &minute1, &day1, &month1, &year1,
               &hour2, &minute2, &day2, &month2, &year2) != 10) {
        // Invalid format
        return 0;
    }

    // Check if datetime1 is valid
    if (isValidDate(day1, month1, year1)) {
        return 3100 + day1; // Return corresponding value for day1
    }
    if (isValidHour(hour1)) {
        return 1100 + hour1; // Return corresponding value for hour1
    }
    if (isValidMinute(minute1)) {
        return 2100 + minute1; // Return corresponding value for minute1
    }

    if (isValidMonth(month1)) {
        return 4100 + month1; // Return corresponding value for month1
    }

    if (isValidYear(year1)) {
        return 510000 + year1; // Return corresponding value for year1
    }

    // Check if datetime2 is valid
    if (isValidDate(day2, month2, year2)) {
        return 3200 + day2; // Return corresponding value for day2
    }

    if (isValidHour(hour2)) {
        return 1200 + hour2; // Return corresponding value for hour1
    }
    if (isValidMinute(minute2)) {
        return 2200 + minute1; // Return corresponding value for minute1
    }

    if (isValidMonth(month2)) {
        return 4200 + month2; // Return corresponding value for month2
    }

    if (isValidYear(year2)) {
        return 520000 + year2; // Return corresponding value for year2
    }

    // Check if datetime2 is earlier than datetime1
    if ((year2 < year1) || (year2 == year1 && (month2 < month1 || (month2 == month1 && (day2 < day1 || (day2 == day1 && (hour2 < hour1 || (hour2 == hour1 && minute2 < minute1)))))))) {
        return 0; // Condition 3 is violated
    }

    return -1; // Time is valid
}
// REQ 7: Number in Commands
int getNumFromCommand(char *command) {
    // Check if the command is NULL
    if (command == NULL) {
        printf("Error: Null command\n");
        return -1;
    }

    // Find the position of '#' in the command
    char *hashPosition = strchr(command, '#');

    // If '#' is not present, return -1
    if (hashPosition == NULL) {
        return -1;
    }

    // Move to the character after '#' to get the potential number
    hashPosition++;

    // Extract and convert the number
    int num = 0;
    while (*hashPosition >= '0' && *hashPosition <= '9') {
        num = num * 10 + (*hashPosition - '0');
        hashPosition++;
    }

    // Check if at least one digit was found
    if (num == 0) {
        return 0; // Invalid <num>
    }

    return num; // Valid <num>
}
// REQ 8: Information to be changed in the Edit command
int getFieldFromEdit(char* edit_cmd) {
    char* title_start = strstr(edit_cmd, "title:[");
    if (title_start != NULL) {
        return 1; // Title
    }

    char* description_start = strstr(edit_cmd, "description:[");
    if (description_start != NULL) {
        return 2; // Description
    }

    char* time_start = strstr(edit_cmd, "time:[");
    if (time_start != NULL) {
        return 3; // Time
    }

    char* status_start = strstr(edit_cmd, "status:[");
    if (status_start != NULL) {
        return 4; // Status
    }

    return 0; // Invalid field
}

// Req 10
void printAllTasks(struct Task *array_tasks, int no_tasks) {
    for (int i = 0; i < no_tasks; i++) {
        printTask(&array_tasks[i]);
    }
}
// Req 11
void printTaskByNum(struct Task *array_tasks, int no_tasks, int num) {
    for (int i = 0; i < no_tasks; i++) {
        if (array_tasks[i].num == num) {
            printTask(&array_tasks[i]);
            return;
        }
    }
    
    printf("Task with Num #%d not found.\n", num);
}
// Req 12
void printHeadTasks(struct Task *array_tasks, int no_tasks, int quan) {
    int print_quantity = (quan < no_tasks) ? quan : no_tasks;

    for (int i = 0; i < print_quantity; i++) {
        printTask(&array_tasks[i]);
    }
}
// Req 13
void printTailTasks(struct Task *array_tasks, int no_tasks, int quan) {
    int print_quantity = (quan < no_tasks) ? quan : no_tasks;
    
    for (int i = no_tasks - print_quantity; i < no_tasks; i++) {
        printTask(&array_tasks[i]);
    }
}
// Req 14
void printFilteredTasksByTitle(struct Task *array_tasks, int no_tasks, char *filter_title) {
    for (int i = 0; i < no_tasks; i++) {
        if (strstr(array_tasks[i].title, filter_title) != NULL) {
            printTask(&array_tasks[i]);
        }
    }
}
// Req 15
void printFilteredTasksByDescription(struct Task *array_tasks, int no_tasks, char *filter_description) {
    for (int i = 0; i < no_tasks; i++) {
        if (strstr(array_tasks[i].description, filter_description) != NULL) {
            printTask(&array_tasks[i]);
        }
    }
}
// Req 16
void printFilteredTasksByStatus(struct Task *array_tasks, int no_tasks, enum Status filter_status) {
    for (int i = 0; i < no_tasks; i++) {
        if (array_tasks[i].status == filter_status) {
            printTask(&array_tasks[i]);
        }
    }
}

// Req 18

bool deleteTask(struct Task *array_tasks, int no_tasks, int num) {
    int index = -1;

    // Find the index of the task with the given num
    for (int i = 0; i < no_tasks; i++) {
        if (array_tasks[i].num == num) {
            index = i;
            break;
        }
    }

    // If the task was not found, return false
    if (index == -1) {
        return false;
    }

    // Shift the tasks to the left to remove the task at the given index
    for (int i = index; i < no_tasks - 1; i++) {
        array_tasks[i] = array_tasks[i + 1];
    }

    // Update the num member variable of the remaining tasks
    for (int i = 0; i < no_tasks - 1; i++) {
        array_tasks[i].num = i + 1;
    }

    return true;
}
// REQ 17: Add tasks
bool addTask(struct Task *array_tasks, int no_tasks, char *new_title, char *new_description, char *new_time) {
    if (no_tasks >= MAX_NO_TASKS) {
        return false;  // Maximum number of tasks reached
    }
    
    struct Task new_task;
    new_task.num = no_tasks + 1;  // Assign task number
    snprintf(new_task.title, sizeof(new_task.title), "%s", new_title);
    snprintf(new_task.description, sizeof(new_task.description), "%s", new_description);
    snprintf(new_task.time, sizeof(new_task.time), "%s", new_time);
    new_task.status = IN_PROGRESS;  // Set default status
    
    array_tasks[no_tasks] = new_task;  // Add task to the array
    return true;
}
// Function to parse a date string in the format "DD/MM/YYYY"
void parseDate(const char * str, int *day, int *month, int *year){
    sscanf(str,"%d/%d/%d", day, month, year);
}

// Function to check if two dates are in the same week
bool isSameWeek(int day1, int month1, int year1, int day2, int month2, int year2) {
    // This is a basic check assuming all months have 30 days
    // You might want to replace this with a more accurate check
    int ordinal1 = day1 + month1 * 30 + year1 * 365;
    int ordinal2 = day2 + month2 * 30 + year2 * 365;
    return abs(ordinal1 - ordinal2) < 7;
}

// Implementation of printWeekTime function
int printWeekTime(struct Task *array_tasks, int no_tasks, char *date) {
    int given_day, given_month, given_year;
    sscanf(date, "%*[^/]/%d/%d/%d", &given_day, &given_month, &given_year);

    // Generate weekly task display
    printf("%-*s", WEEK_CELL_FIRST_COL_WIDTH, "YEAR");
    for (int i = 0; i < 7; i++) {
        printf("%-*s", WEEK_CELL_OTHER_COL_WIDTH, "DAY/MO");
    }
    printf("\n");

    for (int h = 0; h < 24; h++) {
        printf("%02d:00", h);
        for (int i = 0; i < no_tasks; i++) {
            struct Task task = array_tasks[i];
            int start_hour, start_minute, end_hour, end_minute;
            int task_day, task_month, task_year;
            sscanf(task.time, "%d:%d|%d/%d/%d-%d:%d", &start_hour, &start_minute, &task_day, &task_month, &task_year, &end_hour, &end_minute);
            if (start_minute != 0 || end_minute != 0) {
                printUnsupportedTime(&task);
                return i;
            }
            if (abs((given_year*365 + given_month*30 + given_day) - (task_year*365 + task_month*30 + task_day)) < 7 && start_hour <= h && h < end_hour) {
                char task_title[MAX_LENGTH_TITLE+4];
                strncpy(task_title, task.title, WEEK_CELL_OTHER_COL_WIDTH-4);
                if (strlen(task.title) > WEEK_CELL_OTHER_COL_WIDTH-4) {
                    strcat(task_title, "...");
                }
                printf("%-*s", WEEK_CELL_OTHER_COL_WIDTH, task_title);
            } else {
                printf("%-*s", WEEK_CELL_OTHER_COL_WIDTH, "");
            }
        }
        printf("\n");
    }

    // If all tasks can be displayed for the week, return -1
    return -1;
}
// Test the functions
int main() {
    char raw_time[] = "12:30|24/11/2023-14:45|66/11/2023";
    int result = checkTime(raw_time);

    if (result == -1) {
        printf("Time is valid.\n");
    } else {
        printf("Time is invalid. Error code: %d\n", result);
    }

    return 0;
}
