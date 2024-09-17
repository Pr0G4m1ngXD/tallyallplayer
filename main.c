#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

char printable[] = "\n  You are listening\n  to\n\n  %s\n\n  By %s\n\n  From %s\n";

void runCommand(const char *cmd, char *out) {
    FILE *fp;
    char buffer[BUFFER_SIZE];

    fp = popen(cmd, "r");
    if (fp == NULL) {
        if (errno) {
            printf("Error: %i %s\n", errno, strerror(errno));
        }
        printf("Failed to run command\n");
        exit(1);
    }
    out[0] = '\0';
    while(fgets(buffer, BUFFER_SIZE, fp)) {
        strcat(out, buffer);
    }
    pclose(fp);
}

// Returns 1 if playing, 0 if not, -1 if fault
int playingStatus() {
    char result[BUFFER_SIZE];
    runCommand("playerctl status", result);
    if (!strcmp(result, "Playing")) {
        return 1;
    } else if (!strcmp(result, "Paused")) {
        return 0;
    }
    return -1;
}

// Returns 0 if success, -1 if fault
int splitStringIntoWordsFormated(const char* in, char* out, int maxLineLength, int maxLines) {
    if (!in) {
        return -1;
    }

    int maxLength, linePosition = 0, outLength = 0, wordLength = 0;

    maxLength = strlen(in);

    char word[maxLength];

    for (int i = 0; i <= maxLength; i++) {
        if (in[i] == ' ' || in[i] == '\n' || i == maxLength) {
            if (linePosition > maxLineLength) {
                out[outLength++] = '\n';
                out[outLength++] = ' ';
                out[outLength++] = ' ';
                linePosition = wordLength;
            }

            for (int j = 0; j < wordLength; j++) {
                out[outLength++] = word[j];
                word[j] = '\0';
            }

            if(i == maxLength) {
                out[outLength] = '\0';
            } else if( in[i] == '\n' ) {
                out[outLength] = in[i];
                linePosition=0;
            } else {
                out[outLength] = ' ';
                linePosition++;
            }

            outLength++;
            wordLength=0;
        } else {
            word[wordLength++] = in[i];
            linePosition++;
        }
    }

    return 0;
}

void getKeyFromMetadata(char* data, char* output, const char* key) {
    char* result = (char*)malloc(BUFFER_SIZE);
    strcpy(result, strstr(data, key));

    for (int i = 0; i < strlen(result); i++) {
        if (result[i] == '\n') {
            result[i] = '\0';
            break;
        }
    }

    int offset = strlen(key);
    for (int i = offset; i < strlen(result); i++) {
        if (result[i] == ' ') {
            offset++;
        } else {
            break;
        }
    }

    strcpy(output, result + offset);
    free(result);
}


int updateThumbnail() {
    char metadata[BUFFER_SIZE];
    runCommand("playerctl metadata", metadata);

    char* title = (char*)malloc(BUFFER_SIZE);
    char* artist = (char*)malloc(BUFFER_SIZE);
    char* album = (char*)malloc(BUFFER_SIZE);
    char* artUrl = (char*)malloc(BUFFER_SIZE);
    char* title2 = (char*)malloc(BUFFER_SIZE);
    char* artist2 = (char*)malloc(BUFFER_SIZE);
    char* album2 = (char*)malloc(BUFFER_SIZE);

    char* command = (char*)malloc(BUFFER_SIZE);

    int maxLineLength = 18, maxLines = 2;

    getKeyFromMetadata(metadata, title, "xesam:title");
    getKeyFromMetadata(metadata, artist, "xesam:artist");
    getKeyFromMetadata(metadata, album, "xesam:album");
    getKeyFromMetadata(metadata, artUrl, "mpris:artUrl");

    splitStringIntoWordsFormated(title, title2, maxLineLength, maxLines);
    splitStringIntoWordsFormated(artist, artist2, maxLineLength, maxLines);
    splitStringIntoWordsFormated(album, album2, maxLineLength, maxLines);

    free(title);
    free(artist);
    free(album);

    sprintf(command, "kitten icat --place 40x11@25x1 --align left --scale-up '%s'", artUrl);


    system("clear"); // this feels wrong
    printf(printable, title2, artist2, album2);
    system(command);

    free(command);

    free(title2);
    free(artist2);
    free(album2);
    free(artUrl);

    return 0;
}

int main() {
    int status = 0;
    do {
        status = updateThumbnail();
        sleep(10);
    } while (!status);
    return status;
}
