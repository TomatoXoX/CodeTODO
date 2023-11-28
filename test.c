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
#define WEEK_CELL_OTHER_COL_WIDTH 17
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
enum CommandType getCommandType(char* command) {
    char* first_word = strtok(command, " ");
    if (first_word != NULL) {
        for (int j = 0; j < sizeof(command_name) / sizeof(command_name[0]); j++) {
            if (strcasecmp(first_word, command_name[j]) == 0) {
                return j;
            }
        }
    }
    return INVALID;
}
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
        start += 3;
        char* end = strstr(start, "]");
        if (end && end > start) {
            strncpy(out_description, start, end - start);
            out_description[end - start] = '\0';
        }
    }
}
void getTimeFromAdd(char* command, char* out_time) {
    char* start = strrchr(command, '[');
    if (start) {
        start += 1;
        char* end = strstr(start, "]");
        if (end && end > start) {
            strncpy(out_time, start, end - start);
            out_time[end - start] = '\0';
        }
    }
}
void getTitleFromEdit(char *command, char *out_title) {
    char* start = strstr(command, "title:[");
    if (start) {
        start += 7;
        char* end = strchr(start, ']');
        if (end && end > start) {
            strncpy(out_title, start, end - start);
            out_title[end - start] = '\0';
        }
    }
}
void getDescriptionFromEdit(char *command, char *out_description) {
    char* start = strstr(command, "description:[");
    if (start) {
        start += 13;
        char* end = strchr(start, ']');
        if (end && end > start) {
            strncpy(out_description, start, end - start);
            out_description[end - start] = '\0';
        }
    }
}
void getTimeFromEdit(char *command, char *out_time) {
    char* start = strstr(command, "time:[");
    if (start) {
        start += 6;
        char* end = strchr(start, ']');
        if (end && end > start) {
            strncpy(out_time, start, end - start);
            out_time[end - start] = '\0';
        }
    }
}
enum Status getStatusFromEdit(char *edit_cmd) {
    char status[20];
    char* start = strstr(edit_cmd, "status:[");
    if (start) {
        start += 8;
        char* end = strchr(start, ']');
        if (end && end > start) {
            strncpy(status, start, end - start);
            status[end - start] = '\0';
            if (strcmp(status, "In Progress") == 0) {
                return IN_PROGRESS;
            } else if (strcmp(status, "Done") == 0) {
                return DONE;
            } else if (strcmp(status, "Archived") == 0) {
                return ARCHIVED;
            }
        }
    }
    return IN_PROGRESS;
}
// REQ 3
int checkTitle(char * raw_title) {
    if (strlen(raw_title) > MAX_LENGTH_TITLE) {
        return strlen(raw_title); 
    }
    if (raw_title[0] == ' ') {
        return 0;
    } else if (raw_title[strlen(raw_title) - 1] == ' ') {
        return strlen(raw_title);
    }
    char allowedCharacters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ,.-:|/";
    for (int i = 0; raw_title[i] != '\0'; i++) {
        int isValidChar = 0;
        for (int j = 0; allowedCharacters[j] != '\0'; j++) {
            if (raw_title[i] == allowedCharacters[j]) {
                isValidChar = 1;
                break;
            }
        }
        if (!isValidChar) {
            return i; 
        }
    }

    return -1;
}
int checkDescription(char* raw_description) {
    int length = strlen(raw_description);
    if (length > MAX_LENGTH_DESCRIPTION) {
        return length;
    }
    for (int i = 0; i < length; i++) {
        char ch = raw_description[i];
        if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
              (ch >= '0' && ch <= '9') || ch == ' ' || ch == ',' ||
              ch == '.' || ch == '-' || ch == ':' ||
              ch == '|' || ch == '/')) {
            return i;
        }
        if (i == 0 && ch == ' ') {
            return i;
        }
        if (i == length - 1 && ch == ' ') {
            return i;
        }
    }
    return -1;
}
int isLeapYear(int year) {
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}
int isValidDate(int day, int month, int year) {
    if (month < 1 || month > 12) return 1;
    if (day < 1) return 1;                   
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
    if (sscanf(raw_time, "%2d:%2d|%2d/%2d/%4d-%2d:%2d|%2d/%2d/%4d",
               &hour1, &minute1, &day1, &month1, &year1,
               &hour2, &minute2, &day2, &month2, &year2) != 10) {
       
        return 0;
    }
    if (isValidDate(day1, month1, year1)) {
        return 3100 + day1; 
    }
    if (isValidHour(hour1)) {
        return 1100 + hour1; 
    }
    if (isValidMinute(minute1)) {
        return 2100 + minute1; 
    }
    if (isValidMonth(month1)) {
        return 4100 + month1; 
    }
    if (isValidYear(year1)) {
        return 510000 + year1;
    }
    if (isValidDate(day2, month2, year2)) {
        return 3200 + day2; 
    }
    if (isValidHour(hour2)) {
        return 1200 + hour2; 
    }
    if (isValidMinute(minute2)) {
        return 2200 + minute1; 
    }
    if (isValidMonth(month2)) {
        return 4200 + month2; 
    }
    if (isValidYear(year2)) {
        return 520000 + year2; 
    }
    if ((year2 < year1) || (year2 == year1 && (month2 < month1 || (month2 == month1 && (day2 < day1 || (day2 == day1 && (hour2 < hour1 || (hour2 == hour1 && minute2 < minute1)))))))) {
        return 0; // Condition 3 is violated
    }
    return -1;
}
int getNumFromCommand(char *command) {

    if (command == NULL) {
        printf("Error: Null command\n");
        return -1;
    }
    char *hashPosition = strchr(command, '#');

    if (hashPosition == NULL) {
        return -1;
    }
    hashPosition++;
    int num = 0;
    while (*hashPosition >= '0' && *hashPosition <= '9') {
        num = num * 10 + (*hashPosition - '0');
        hashPosition++;
    }
    if (num == 0) {
        return 0;
    }
    return num; 
}

int getFieldFromEdit(char* edit_cmd) {
    char* title_start = strstr(edit_cmd, "title:[");
    if (title_start != NULL) {
        return 1; 
    }

    char* description_start = strstr(edit_cmd, "description:[");
    if (description_start != NULL) {
        return 2; 
    }

    char* time_start = strstr(edit_cmd, "time:[");
    if (time_start != NULL) {
        return 3; 
    }

    char* status_start = strstr(edit_cmd, "status:[");
    if (status_start != NULL) {
        return 4; // Status
    }

    return 0;
}

void printAllTasks(struct Task *array_tasks, int no_tasks) {
    for (int i = 0; i < no_tasks; i++) {
        printTask(&array_tasks[i]);
    }
}

void printTaskByNum(struct Task *array_tasks, int no_tasks, int num) {
    for (int i = 0; i < no_tasks; i++) {
        if (array_tasks[i].num == num) {
            printTask(&array_tasks[i]);
            return;
        }
    }
    
    printf("Task with Num #%d not found.\n", num);
}

void printHeadTasks(struct Task *array_tasks, int no_tasks, int quan) {
    int print_quantity = (quan < no_tasks) ? quan : no_tasks;

    for (int i = 0; i < print_quantity; i++) {
        printTask(&array_tasks[i]);
    }
}

void printTailTasks(struct Task *array_tasks, int no_tasks, int quan) {
    int print_quantity = (quan < no_tasks) ? quan : no_tasks;
    
    for (int i = no_tasks - print_quantity; i < no_tasks; i++) {
        printTask(&array_tasks[i]);
    }
}

void printFilteredTasksByTitle(struct Task *array_tasks, int no_tasks, char *filter_title) {
    for (int i = 0; i < no_tasks; i++) {
        if (strstr(array_tasks[i].title, filter_title) != NULL) {
            printTask(&array_tasks[i]);
        }
    }
}

void printFilteredTasksByDescription(struct Task *array_tasks, int no_tasks, char *filter_description) {
    for (int i = 0; i < no_tasks; i++) {
        if (strstr(array_tasks[i].description, filter_description) != NULL) {
            printTask(&array_tasks[i]);
        }
    }
}

void printFilteredTasksByStatus(struct Task *array_tasks, int no_tasks, enum Status filter_status) {
    for (int i = 0; i < no_tasks; i++) {
        if (array_tasks[i].status == filter_status) {
            printTask(&array_tasks[i]);
        }
    }
}



bool deleteTask(struct Task *array_tasks, int no_tasks, int num) {
    int index = -1;

    
    for (int i = 0; i < no_tasks; i++) {
        if (array_tasks[i].num == num) {
            index = i;
            break;
        }
    }


    if (index == -1) {
        return false;
    }

   
    for (int i = index; i < no_tasks - 1; i++) {
        array_tasks[i] = array_tasks[i + 1];
    }

    
    for (int i = 0; i < no_tasks - 1; i++) {
        array_tasks[i].num = i + 1;
    }

    return true;
}

bool addTask(struct Task *array_tasks, int no_tasks, char *new_title, char *new_description, char *new_time) {
    if (no_tasks >= MAX_NO_TASKS) {
        return false;  
    }
    
    struct Task new_task;
    new_task.num = no_tasks + 1;
    snprintf(new_task.title, sizeof(new_task.title), "%s", new_title);
    snprintf(new_task.description, sizeof(new_task.description), "%s", new_description);
    snprintf(new_task.time, sizeof(new_task.time), "%s", new_time);
    new_task.status = IN_PROGRESS; 
    
    array_tasks[no_tasks] = new_task;  
    return true;
}
int dayOfWeek(int day, int month, int year) {
    if (month < 3) {
        month += 12;
        year--;
    }
    int h = (day + 2*month + 3*(month + 1)/5 + year + year/4 - year/100 + year/400) % 7;
    return h;
}

int dateToInt(int d, int m, int y) {
    return y * 10000 + m * 100 + d;
}

int printWeekTime(struct Task * array_tasks, int no_tasks, char * date) {
    
    char weekDay[4];
    int givenDay, givenMonth, givenYear;
    sscanf(date, "%3s/%d/%d/%d", weekDay, &givenDay, &givenMonth, &givenYear); 

    
    int startWeekTime = dateToInt(givenDay, givenMonth, givenYear);
    int endWeekTime = dateToInt(givenDay + 6, givenMonth, givenYear);

    
    for (int i = 0; i < no_tasks; i++) {
        
        int taskStartHour, taskStartMinute, taskStartDay, taskStartMonth, taskStartYear;
        int taskEndHour, taskEndMinute, taskEndDay, taskEndMonth, taskEndYear;
        sscanf(array_tasks[i].time, "%2d:%2d|%2d/%2d/%4d-%2d:%2d|%2d/%2d/%4d", 
               &taskStartHour, &taskStartMinute, &taskStartDay, &taskStartMonth, &taskStartYear,
               &taskEndHour, &taskEndMinute, &taskEndDay, &taskEndMonth, &taskEndYear);

       
        int taskStartTime_t = dateToInt(taskStartDay, taskStartMonth, taskStartYear);
        int taskEndTime_t = dateToInt(taskEndDay, taskEndMonth, taskEndYear);

        
        if (taskStartTime_t >= startWeekTime && taskEndTime_t <= endWeekTime) {
            
            printTask(&array_tasks[i]);
        } else {
            
            printUnsupportedTime(&array_tasks[i]);
            return i;
        }
    }
    
    return -1; 
}
// Test the functions
int main() {
    // Create an array of tasks
    struct Task array_tasks[5] = {
        {1, "Course Intro to Programming", "Room 701-H6 - orange", "07:00|16/10/2023-09:00|16/10/2023", IN_PROGRESS},
        {2, "Meeting Project Apple", "Room 701-H6 - apple", "08:00|17/10/2023-09:00|17/10/2023", IN_PROGRESS},
        {3, "Meeting Project Banana", "Room 701-H6 - orange", "07:00|19/10/2023-08:00|19/10/2023", IN_PROGRESS},
        {4, "Do an Tong hop - TTNT", "Room 701-H6 - orange", "09:00|21/10/2023-10:00|21/10/2023", IN_PROGRESS},
        {5, "Course Intro to Programming - apple", "Room 701-H6 - banana", "07:00|01/10/2023-12:00|01/10/2023", DONE},
    };

  
    int no_tasks = 5;

    // Set the date
    char date[] = "wed/18/10/2023";

    // Call the printWeekTime function and capture its return value
    int result = printWeekTime(array_tasks, no_tasks, date);

    // Check the return value
    if (result == -1) {
        printf("All tasks for the week were successfully displayed.\n");
    } else {
        printf("The task at index %d could not be displayed.\n", result);
    }
return 0; 
}