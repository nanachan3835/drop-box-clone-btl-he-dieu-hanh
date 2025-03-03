#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <limits.h>

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

void handle_events(int fd) {
    char buf[BUF_LEN];
    ssize_t numRead;
    char *ptr;
    struct inotify_event *event;

    while (1) {
        numRead = read(fd, buf, BUF_LEN);
        if (numRead <= 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        for (ptr = buf; ptr < buf + numRead; ) {
            event = (struct inotify_event *) ptr;
            printf("Event: ");

            if (event->mask & IN_CREATE) {
                printf("File created: ");
            } else if (event->mask & IN_DELETE) {
                printf("File deleted: ");
            } else if (event->mask & IN_MODIFY) {
                printf("File modified: ");
            } else if (event->mask & IN_MOVED_FROM) {
                printf("File moved out: ");
            } else if (event->mask & IN_MOVED_TO) {
                printf("File moved in: ");
            }

            if (event->len > 0) {
                printf("%s\n", event->name);
            } else {
                printf("Unknown file\n");
            }

            ptr += sizeof(struct inotify_event) + event->len;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <directory_to_watch>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int inotifyFd = inotify_init();
    if (inotifyFd == -1) {
        perror("inotify_init");
        exit(EXIT_FAILURE);
    }

    int wd = inotify_add_watch(inotifyFd, argv[1], IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);
    if (wd == -1) {
        perror("inotify_add_watch");
        exit(EXIT_FAILURE);
    }

    printf("Watching directory: %s\n", argv[1]);
    handle_events(inotifyFd);

    inotify_rm_watch(inotifyFd, wd);
    close(inotifyFd);
    return 0;
}
