#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEVICE "/dev/vdev-1"
#define open_mode O_RDWR
#define SIZE_BUFF 1

unsigned char write_buf;
unsigned char read_buf;

int main(void)
{
	int fd;
        char opt;

        fd = open(DEVICE, open_mode);
        if(fd < 0) {
                printf("Can`t open device %s\n", DEVICE);
                return 0;
        }

        unsigned int pos, data;
        unsigned long * addr;
        while (1) {
                printf("Options: \n");
                printf("1 - write\n");
                printf("2 - Read\n");
                printf("3 - lseek\n");
                printf("4 - mmap\n");
                printf("5 - exit\n");
                scanf(" %c", &opt);
                printf("Choose option = %c\n", opt);
                
                switch(opt) {
                        case '1':
                                printf("Enter number to write :");
                                scanf("%u", &data);
                                write_buf = (char)data;
                                //scanf("  %[^\t\n]s", data);
                                write(fd, &write_buf, 1);
                                printf("Done\n");
                                break;
                        case '2':
                        	fflush(stdout);
                                read(fd, &read_buf, SIZE_BUFF);
                                printf("Data = %u\n\n", (unsigned int)read_buf);
                                break;
                        case '3':
                                printf("Position in bytes from start of file: \n");
                                scanf(" %d", &pos);
                                lseek(fd, pos, SEEK_SET);
                                break;
                        case '4':
                                addr = (unsigned long *)mmap(NULL, 4096,
                                                                        PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                                if (addr == MAP_FAILED)
                                {
                                        perror("mmap failure: ");
                                }
                                printf("mmap successful\n");
                                break;
                        case '5':
                                close(fd);
                                exit(1);
                                break;
                        default:
                                printf("Invalid option %d\n",opt);
                                continue;
                }
        }
        close(fd);




	return 0;
}
