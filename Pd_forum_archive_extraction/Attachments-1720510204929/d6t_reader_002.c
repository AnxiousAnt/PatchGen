/* d6t_reader.c is a program that reads from omron d6t IR sensors using the raspberry pi i2c bus */
/* It sends sensor packets in a form readable by a pure-data [netreceive] object. */
// based on elinux.org/interfacing_with_I2C_Devices
// Martin Peach 20130416
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
// Set a Pd [netreceive] to listen on this PORT
#define PORT 33333
// Pd is running on this machine IP
#define IP "192.168.2.15" 
//#define IP "127.0.0.1"
// The SENSOR_PACKET_SIZE is 35 for D6T44L, 19 for D6T8L
#define D6T44L_SENSOR_PACKET_SIZE 35
#define D6T8L_SENSOR_PACKET_SIZE 19
// PD_SELECTOR is the first item in the message sent to netrecceive
#define D6T44L_PD_SELECTOR "d6t44l"
#define D6T8L_PD_SELECTOR "d6t8l"
// SENSOR_SELECT_PIN connects to A of a 4051 multiplexer to select X0 or X1 output for the i2c clock.
// GPIO_17 is rPi GPIO_GEN_0, pin 11 on the P1 header.
#define SENSOR_SELECT_PIN 17
// calc_crc and D6T_checkPEC are from Omron's crc code from the DST-44L/DST-8L Thermal Sensor Whitepaper
unsigned char calc_crc (unsigned char data);
int D6T_checkPEC (unsigned char *buf, int pPEC);

int gpio_init(int gpio_number, int direction);
int gpio_write(int gpio_number, int value);
int gpio_free(int gpio_number);

void  sigint_handler(int sig);

/* non-zero direction is output */
#define GPIO_IN 0
#define GPIO_OUT 1
// Executes when the user presses Ctrl+C. Frees up GPIO pins

void sigint_handler(int sig)
{
    gpio_free(SENSOR_SELECT_PIN);
    exit (sig);
}

int gpio_init(int gpio_number, int direction)
{
    FILE    *fp;
    //create a variable to store whether we are sending a '1' or a '0'
    char    set_value[4];
    char    path[128];
    //Using sysfs we need to write "38" to /sys/class/gpio/export
    //This will create the folder /sys/class/gpio/gpio38
    // use fopen instead of open because we don't want buffering
    if ((fp = fopen("/sys/class/gpio/export", "ab")) == NULL)
    {
        printf("Cannot open export file.\n");
        return(1);
    }
    //Set pointer to beginning of the file (is this necessary?)
    rewind(fp);
    //Write our value of "38" to the file
    sprintf(set_value,"%d", gpio_number);
    fwrite(&set_value, sizeof(char), strlen(set_value), fp);
    fclose(fp);

    printf("...export file accessed, new pin now accessible\n");

    //Open the pin's sysfs file in binary for reading and writing, store file pointer in fp
    sprintf(path, "/sys/class/gpio/gpio%d/direction", gpio_number);
    if ((fp = fopen(path, "rb+")) == NULL)
    {
        printf("Cannot open direction file.\n");
        return(2);
    }
    //Set pointer to beginning of the file
    rewind(fp);
    //Write our value of "out" to the file
    if (direction) strcpy(set_value,"out");
    else strcpy(set_value,"in");
    fwrite(&set_value, sizeof(char), strlen(set_value), fp);
    fclose(fp);
    printf("...direction set to %sput\n", (direction)?"out":"in");
    return 0;
}

int gpio_write(int gpio_number, int value)
{
    FILE    *fp;
    //create a variable to store whether we are sending a '1' or a '0'
    char    set_value[4];
    char    path[128];

    //SET VALUE
    //Open the LED's sysfs file in binary for reading and writing, store file pointer in fp
    sprintf(path, "/sys/class/gpio/gpio%d/value", gpio_number);
    if ((fp = fopen(path, "rb+")) == NULL)
    {
        printf("Cannot open value file.\n");
        return(3);
    }
    //Set pointer to beginning of the file
    rewind(fp);
    //Write our value of "1" to the file
    sprintf(set_value,"%d", value);
    fwrite(&set_value, sizeof(char), strlen(set_value), fp);
    fclose(fp);
    //printf("...value set to %d...\n", value);

    return 0;
}

int gpio_free(int gpio_number)
{
    FILE    *fp;
    char    set_value[4];
    //Using sysfs we need to write "38" to /sys/class/gpio/unexport
    //This will remove the folder /sys/class/gpio/gpio38
    // use fopen instead of open because we don't want buffering
    if ((fp = fopen("/sys/class/gpio/unexport", "ab")) == NULL)
    {
        printf("Cannot open unexport file.\n");
        return(4);
    }
    //Set pointer to beginning of the file
    rewind(fp);
    //Write our value of "38" to the file
    sprintf(set_value,"%d", gpio_number);
    fwrite(&set_value, sizeof(char), strlen(set_value), fp);
    fclose(fp);

    printf("...unexport file accessed, pin now inaccessible\n");
    return 0;
}

unsigned char calc_crc (unsigned char data)
{
  int           index;
  unsigned char temp;

  for (index = 0; index < 8; index++)
  {
    temp = data;
    data <<= 1;
    if (temp & 0x80) data ^= 0x07;
  }
  return data;
}

int D6T_checkPEC (unsigned char *buf, int pPEC)
{
  unsigned char crc = 0;
  int           i;

  //crc = calc_crc( 0x14 );
  //crc = calc_crc( 0x4C ^ crc );
  crc = calc_crc( 0x15 ^ crc );
  for ( i = 0; i < pPEC; i++)
  {
    crc = calc_crc(buf[i] ^ crc);
    //    if (crc == buf[pPEC]) printf("MATCH at %d\n", i);
  }
  printf("D6T_checkPEC says 0x%02X\n", crc);
  return (crc == buf[pPEC]);
}

int main (int argc, char **argv)
{
  int 							        i2c_file;
  char 							        filename[32];
  int                       addr = 0x0A;
  unsigned char             inbuf[63];
  unsigned char             outbuf;
  char                      netbuf[256];
  const char                *buffer;
  int                       i, j, s;
  int                       initialized = 0;
  int							          sensor = 0; // which one
  int                       bufsize = D6T44L_SENSOR_PACKET_SIZE; // read packet size for the current sensor
  struct sockaddr_in        sock;

  // initialize the GPIO pin(s) to select the sensor via a multiplexer on its clock line
  // GPIO_17 == GPIO_GEN0 on the pi == pin 11 on connector P1
  i = gpio_init(SENSOR_SELECT_PIN, GPIO_OUT); // make pin an output
  if (0 != i)
  {
	    printf ("Unable to init GPIO %d (%d)\n", SENSOR_SELECT_PIN, i);
	    sigint_handler ( EXIT_FAILURE );
  }
  // select sensor in channel 0
  i = gpio_write(SENSOR_SELECT_PIN, sensor&0x01); // output low
  if (0 != i)
  {
	    printf ("Unable to write GPIO %d (%d)\n", SENSOR_SELECT_PIN, i);
	    sigint_handler ( EXIT_FAILURE );
  }
  // Assign a handler to free the GPIO pin on Ctrl+C.
  signal (SIGINT, (__sighandler_t)sigint_handler);

  // set up a socket to send UDP datagrams through
  if (( s = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP )) < 0)
  {
    printf ("Unable to create socket (%d)\n", errno);
    buffer = strerror(errno);
    printf(">> %s <<\n", buffer);
    sigint_handler ( EXIT_FAILURE );
  }
  memset (( char *) &sock, 0 , sizeof(sock) ); // clear the sock struct
  sock.sin_family = AF_INET;
  sock.sin_port = htons ( PORT );
  if ( inet_aton( IP, &sock.sin_addr ) == 0 )
  {
    printf ( "Unable to make address from %s\n", IP );
    sigint_handler ( EXIT_FAILURE );
  }
  // The Omron sensor streams data after being initialized so we only need to write it once
  if ( argc > 1 ) initialized = atoi(argv[1]);
  printf("%s: initialized:%d\n", argv[0], initialized);
  // Open the I2C connection
  sprintf(filename, "/dev/i2c-1");
  if ((i2c_file = open(filename, O_RDWR)) < 0)
  {
    printf ("Failed to open the bus. (%d)\n", errno);
    sigint_handler (EXIT_FAILURE);
  }
  // set the slave address
  if (ioctl(i2c_file, I2C_SLAVE, addr) < 0)
  {
    printf ("Failed to acquire bus access and/or talk to slave. (%d)\n", errno);
    sigint_handler (EXIT_FAILURE);
  }
  // start reading packets
  do
  {
    // read the first sensor, a D6T44L
    sensor = 0;
    bufsize = D6T44L_SENSOR_PACKET_SIZE; // packet size for sensor 0
    i = gpio_write(SENSOR_SELECT_PIN, sensor&0x01); // select sensor in channel 0
    if (0 != i)
    {
      printf ("Unable to write GPIO %d (%d)\n", SENSOR_SELECT_PIN, i);
      sigint_handler ( EXIT_FAILURE );
    }
    do // read packets until we get one with no errors
    {
    	// clear the inbuf first
    	for ( i = 0; i < bufsize; ++i) inbuf[i] = 0;

    	// we only need to write the command once.
    	// After that the sensor will return data
    	// any time it is read.
    	if ( !initialized )
    	{
        outbuf = 0x4C; // the command
    		if (write( i2c_file, &outbuf, 1 ) != 1)
    		{
    			printf("Write failed (%d)\n", errno);
    			buffer = strerror(errno);
    			printf(">> %s <<\n", buffer);
    			sigint_handler (EXIT_FAILURE);
    		}
    		//initialized = 1; // only set after all sensors are intialized
    	}
    	// read a packet
    	if (read (i2c_file, inbuf, bufsize) != bufsize)
    	{
    		printf("Read failed (%d)\n", errno);
    		buffer = strerror(errno);
    		printf(">> %s <<\n", buffer);
    	}
    } while (!D6T_checkPEC (inbuf, bufsize-1));
    // print the result
    for (i = 0; i < bufsize-1; i += 2)
    {
      printf("%d ", inbuf[i]+(inbuf[i+1]<<8));
    }
    printf("<0x%02X>\n", inbuf[bufsize-1]);
    // send the result to Pd
    j = sprintf (netbuf, D6T44L_PD_SELECTOR);
    for (i = 0; i < bufsize-1; i += 2)
    {
      j += sprintf(&netbuf[j], " %d", inbuf[i] + (inbuf[i+1]<<8));
    }
    j += sprintf(&netbuf[j], "\n"); // CR NOT semicolon for Pd's [netreceive]
    if (sendto ( s, netbuf, j, 0, (const struct sockaddr *)&sock, sizeof(sock)) < 0 )
    {
      printf ("sendto error (%d)\n", errno);
      buffer = strerror (errno);
      printf (">> %s <<\n", buffer);
    }
    printf("%s\n", netbuf);

    usleep (200000); // wait 200ms

    // read second sensor, a D6T8L
    sensor = 1;
    bufsize = D6T8L_SENSOR_PACKET_SIZE; // sensor 1 is D6T8L
    i = gpio_write(SENSOR_SELECT_PIN, sensor&0x01); // select sensor in channel 1
    if (0 != i)
    {
      printf ("Unable to write GPIO %d (%d)\n", SENSOR_SELECT_PIN, i);
      sigint_handler ( EXIT_FAILURE );
    }
    do
    {

      // clear the inbuf first
      for ( i = 0; i < D6T8L_SENSOR_PACKET_SIZE; ++i) inbuf[i] = 0;

      outbuf = 0x4C; // the command
      // we only need to write the command once.
      // After that the sensor will return data
      // any time it is read.
      if (! initialized)
      {
        if (write( i2c_file, &outbuf, 1 ) != 1)
        {
          printf("Write failed (%d)\n", errno);
          buffer = strerror(errno);
          printf(">> %s <<\n", buffer);
          sigint_handler (EXIT_FAILURE);
        }
        initialized = 1;
      }
      if (read (i2c_file, inbuf, D6T8L_SENSOR_PACKET_SIZE) != D6T8L_SENSOR_PACKET_SIZE)
      {
        printf("Read failed (%d)\n", errno);
        buffer = strerror(errno);
        printf(">> %s <<\n", buffer);
      }
    } while (!D6T_checkPEC (inbuf, D6T8L_SENSOR_PACKET_SIZE-1));
    // print to console
    for (i = 0; i < D6T8L_SENSOR_PACKET_SIZE-1; i += 2)
    {
      printf("%d ", inbuf[i]+(inbuf[i+1]<<8));
    }
    printf("<0x%02X>\n", inbuf[bufsize-1]);
    // send to Pd
    j = sprintf (netbuf, D6T8L_PD_SELECTOR);
    for (i = 0; i < D6T8L_SENSOR_PACKET_SIZE-1; i += 2)
    {
      j += sprintf(&netbuf[j], " %d", inbuf[i] + (inbuf[i+1]<<8));
    }
    j += sprintf(&netbuf[j], "\n"); // CR NOT semicolon for Pd's [netreceive]
    if (sendto ( s, netbuf, j, 0, (const struct sockaddr *)&sock, sizeof(sock)) < 0 )
    {
      printf ("sendto error (%d)\n", errno);
      buffer = strerror (errno);
      printf (">> %s <<\n", buffer);
    }
    printf("%s\n", netbuf);
    // sensors are supposed to update about every 400ms
    usleep (200000); // wait 200ms
  } while (1);
  close(i2c_file);
  return EXIT_SUCCESS;
}
 
