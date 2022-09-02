#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#define DEVICE "/dev/vdev-1"
#define open_mode O_RDWR
#define SIZE_BUFF 1
#define FILE_SIZE 4096

unsigned char write_buf;
unsigned char read_buf;

int main(void) {
    int fd;
    char opt;

    fd = open("/dev/vdev-1", O_RDWR);
    if (fd < 0) {
        printf("Can`t open device %s\n", DEVICE);
        return 0;
    }

    unsigned int pos, data;
    char *addr;
    while (1) {
        printf("Options: \n");
        printf("1 - write\n");
        printf("2 - read\n");
        printf("3 - lseek\n");
        printf("4 - mmap\n");
        printf("5 - exit\n");
        scanf(" %c", &opt);
        printf("Choose option = %c\n", opt);

        switch (opt) {
            case '1':
                printf("Enter number (0 - 255) to write: ");
                scanf("%u", &data);
                write_buf = (char) data;
                write(fd, &write_buf, 1);
                printf("Done\n");
                break;
            case '2':
                fflush(stdout);
                read(fd, &read_buf, SIZE_BUFF);
                printf("Data = (unsigned int) %u | (char) %c\n\n", (unsigned int) read_buf, read_buf);
                break;
            case '3':
                printf("Position in bytes from start of file: \n");
                scanf(" %d", &pos);
                lseek(fd, pos, SEEK_SET);
                break;
            case '4':
                addr = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                if (addr == MAP_FAILED) {
                    perror("Failed to mmap: ");
                    close(fd);
                    return -1;
                }
                char data[FILE_SIZE];
                memcpy(data, addr, FILE_SIZE);
                printf("Current file data:\n%s\n", data);

                char new_data[] = "Test-test-0987654321!@#$%%&*///";
                memcpy(addr, new_data, sizeof(new_data));
                printf("Wrote %u bytes of new data:\n%s\n", sizeof(new_data), new_data);
                break;
            case '5':
                close(fd);
                exit(1);
                break;
            default:
                printf("Invalid option %d\n", opt);
                continue;
        }
    }
    close(fd);
}
