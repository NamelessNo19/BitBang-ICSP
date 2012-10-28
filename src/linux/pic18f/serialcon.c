#include "serialcon.h"

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>


#define TIMEOUT_SEC 5

int portOpen = FALSE;
int portFd = -1;

fd_set set;
struct timeval timeout;


int serOpen(char *port)
{
	if (portOpen)
	{
		printf("There already is a connection.\n");
		return TRUE;
	}

	struct termios serOpts;

	
	portFd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
	if (portFd == -1 ) 
	{
		printf("Unable to open Serial Port '%s'.\n" , port);
		return FALSE;
	}
	
	portOpen = TRUE;
	fcntl(portFd, F_SETFL,0);
	
	FD_ZERO(&set); /* clear the set */
    FD_SET(portFd, &set); /* add our file descriptor to the set */

	timeout.tv_sec = TIMEOUT_SEC;
	timeout.tv_usec = 0;

	tcgetattr(portFd, &serOpts);
	cfsetispeed(&serOpts, B9600);
	cfsetospeed(&serOpts, B9600);
	serOpts.c_iflag &= ~(BRKINT | ICRNL | ISTRIP | IXON);
	serOpts.c_cflag &= ~(CSIZE | PARENB); 
	serOpts.c_cflag |= CS8;
	serOpts.c_oflag &= ~(OPOST);
	serOpts.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);
	serOpts.c_cc[VMIN] = 0;
	serOpts.c_cc[VTIME] = 0;
	if (tcsetattr(portFd, TCSANOW, &serOpts) != 0)
	  {
	    printf("WARNING: Setting serial attributes failed. Serial connection might not work properly.\n");
	  }
	
	printf("Port opened.\n");
	return TRUE;
}


void serWrite(char *buf, size_t len)
{
	write(portFd, buf, len);
}

int serRead(char *buf, size_t len, int fillBuffer)
{
    if (!portOpen) 
		return FALSE;
		
	memset(buf, '\0', len);
	int rv;
	size_t readCount = 0;
	len--; // Assure string termination 

	timeout.tv_sec = TIMEOUT_SEC;
	timeout.tv_usec = 0;
   
   do
   {
    int rv = select(portFd + 1, &set, NULL, NULL, &timeout);
	if(rv == -1) 
	{
		perror("select"); /* an error accured */
		return FALSE;
	}
	else if(rv == 0) 
	{
		printf("No response.\n");
		return FALSE; // Timeout
	}
	else
		readCount += read(portFd, buf + readCount, len - readCount);
	} while (fillBuffer && readCount < len);
	
	return TRUE;
}

void serClose()
{
	if (!portOpen)
	{
		printf("Nothing to close.\n");
		return;
	}
	
	close(portFd);
	portOpen = FALSE;
	printf("Port closed.\n");
	return;
}

void flushInBuf() 
{
	char tmp[16];
	int rd, cnt;
	cnt = 0;
	do{ rd = read(portFd, &tmp, 16); cnt += rd; } while(rd > 0);
	
	printf("Flushed %d bytes.\n", cnt);
}


uint16_t crc16_update(uint16_t crc, uint8_t a)
    {
        int i;

        crc ^= a;
        for (i = 0; i < 8; ++i)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc = (crc >> 1);
        }

        return crc;
    }
