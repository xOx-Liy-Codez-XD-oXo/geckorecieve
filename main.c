#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

int main() {
	int fd, res;
	struct termios oldtio, newtio;
	char buf[256];
	char searchstringa[] = "LIYPHOTOJPEG", searchstringb[] = "LIYPHOTOEND";
	int filereadactive = 0;
	int searchidx = 0;
	int numphotosprinted = 0;
	char currentfilename[256];
	FILE * currentfile = NULL;

	fd = open("/dev/tty.usbserial-GECKUSB0", O_RDONLY | O_NONBLOCK | O_NOCTTY);
	if(fd < 0) return -1;
	
	tcgetattr(fd, &oldtio);
	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag = B115200 | CRTSCTS | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = 0;
	newtio.c_oflag = 0;
	newtio.c_lflag = ICANON;
	
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);
	
	while(1) {
		res = read(fd, buf, 256);
		buf[res] = 0;
		if(filereadactive) {
			for(int i = 0; i < res; i++) {
				//printf("Reading image\n");
				fwrite(&(buf[i]), 1, 1, currentfile);
				if(buf[i] == searchstringb[searchidx]) {
					if(searchidx == 9) {
						filereadactive = 0;
						searchidx = 0;
						printf("\nFile read done\n");
						fclose(currentfile);
					} else searchidx++;
				} else searchidx = 0;
			}
		} else {
			for(int i = 0; i < res; i++) {
				if(filereadactive) {
					fwrite(&(buf[i]), 1, 1, currentfile);
					//printf("File read boobstrap\n");
				} else {
					if(buf[i] == searchstringa[searchidx]) {
						if(searchidx == 11) {
							filereadactive = 1;
							searchidx = 0;
							printf("\nFile read begin\n");
							snprintf(currentfilename, 256, "%d.jpg", numphotosprinted);
							numphotosprinted++;
							currentfile = fopen(currentfilename, "wb");
						} else searchidx++;
					} else searchidx = 0;
				}
			}
			printf("%s", buf);
		}
		memset(buf, 0, 256);
	}

	tcsetattr(fd, TCSANOW, &oldtio);
	return 0;
}
