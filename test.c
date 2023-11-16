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
struct Task tasks[MAX_NO_TASKS];
int num_tasks = 0;
enum Status {
    IN_PROGRESS,
    DONE,
    ARCHIVED
};
// REQ 1
enum CommandType { ADD, EDIT, SHOW, DELETE, QUIT, INVALID };
const char* command_name[] = { "ADD", "EDIT", "SHOW", "DELETE", "QUIT", "INVALID" };
typedef enum CommandType CommandType;
CommandType getCommandType(char* command) {
    char* space = strchr(command, ' ');
    int length = (space != NULL) ? space - command : strlen(command);
    char first_word[length + 1];
    strncpy(first_word, command, length);
    first_word[length] = '\0';

    for (int i = 0; i < sizeof(command_name) / sizeof(command_name[0]); i++) {
        if (strcmp(first_word, command_name[i]) == 0) {
            return i;
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
int checkTime(char* raw_time) {
    // Extract datetime1 and datetime2 from raw_time
    char datetime1[20];
    char datetime2[20];
    sscanf(raw_time, "%[^-]-%s", datetime1, datetime2);

    // Validate datetime1
    int hh1, mm1, dd1, mo1, yyyy1;
    if (sscanf(datetime1, "%d:%d|%d/%d/%d", &hh1, &mm1, &dd1, &mo1, &yyyy1) != 5) {
        return 11; // Invalid datetime1
    }

    // Validate datetime2
    int hh2, mm2, dd2, mo2, yyyy2;
    if (sscanf(datetime2, "%d:%d|%d/%d/%d", &hh2, &mm2, &dd2, &mo2, &yyyy2) != 5) {
        return 12; // Invalid datetime2
    }

    // Check if datetime2 is earlier than datetime1
    if (yyyy2 < yyyy1 || (yyyy2 == yyyy1 && (mo2 < mo1 || (mo2 == mo1 && (dd2 < dd1 || (dd2 == dd1 && (hh2 < hh1 || (hh2 == hh1 && mm2 < mm1)))))))) {
        return 0; // datetime2 is earlier than datetime1
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
int getFieldFromEdit(char *edit_cmd) {
    // Check if the edit command is NULL
    if (edit_cmd == NULL) {
        printf("Error: Null edit command\n");
        return 0; // Invalid field
    }

    // Find the position of the second whitespace in the edit command
    char *secondSpace = strchr(edit_cmd, ' ');
    if (secondSpace == NULL) {
        return 0; // Invalid field
    }

    // Move to the character after the second whitespace
    secondSpace++;
    // Find the position of the first colon after the second whitespace
    char *colonPosition = strchr(secondSpace, ':');
    if (colonPosition == NULL) {
        return 0; // Invalid field
    }

    // Calculate the length of the potential field string
    size_t fieldLength = colonPosition - secondSpace;

    // Check the validity of the field string
    if (fieldLength == 7) {
        return 1; // Title
    } else if (fieldLength == 13) {
        return 2; // Description
    } else if (fieldLength == 6) {
        return 3; // Time
    } else if (fieldLength == 8) {
        return 4; // Status
    } else {
        return 0; // Invalid field
    }
}

// Test the functions
int main() {
    char sample_input[] = "ADD [Course Intro to Programming] [Room 701-H6] [07:00|03/10/2023-12:00|01/10/2023]";
    char raw_title[50];
    char raw_description[50];
    char raw_time[50];

    getTitleFromAdd(sample_input, raw_title);
    getDescriptionFromAdd(sample_input, raw_description);
    getTimeFromAdd(sample_input, raw_time);

    printf("Title: %s\n", raw_title);
    printf("Description: %s\n", raw_description);
    printf("Time: %s\n", raw_time);

    // Test checkTitle function
    int titleCheck = checkTitle(raw_title);
    if (titleCheck == -1) {
        printf("Title is valid.\n");
    } else if (titleCheck == strlen(raw_title)) {
        printf("Title length exceeds the maximum limit.\n");
    } else {
        printf("Title is invalid at position %d.\n", titleCheck);
    }

    // Test checkDescription function
    int descriptionCheck = checkDescription(raw_description);
    if (descriptionCheck == -1) {
        printf("Description is valid.\n");
    } else if (descriptionCheck == strlen(raw_description)) {
        printf("Description length exceeds the maximum limit.\n");
    } else {
        printf("Description is invalid at position %d.\n", descriptionCheck);
    }

    // Test checkTime function
    int timeCheck = checkTime(raw_time);
    if (timeCheck == -1) {
        printf("Time is valid.\n");
    } else if (timeCheck == 0) {
        printf("Ending time is earlier than the starting time.\n");
    } else {
        printf("Time is invalid for datetime%d.\n", timeCheck);
    }
    // Test getNumFromCommand function
    char sample_command1[] = "ADD [Task] #123";
    char sample_command2[] = "DELETE #invalid";
    char sample_command3[] = "SHOW [Event] [Location]";

    int num1 = getNumFromCommand(sample_command1);
    int num2 = getNumFromCommand(sample_command2);
    int num3 = getNumFromCommand(sample_command3);

    printf("Num from command 1: %d\n", num1); // Should print 123
    printf("Num from command 2: %d\n", num2); // Should print 0 (invalid)
    printf("Num from command 3: %d\n", num3); // Should print -1 (not present)
    return 0;
}