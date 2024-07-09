// based on elinux.org/interfacing_with_I2C_Devices
// Martin Peach 20130416
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
//#include <sys/ioctl.h>
#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
// Set a Pd [netreceive] to listen on this PORT
#define PORT 33333
// Pd is running on this machine IP
#define IP "192.168.2.15" 
//#define IP "127.0.0.1"
// The SENSOR_PACKET_SIZE is 35 for D6T44L, 19 for D6T8L
#define SENSOR_PACKET_SIZE 35
// PD_SELECTOR is the first item in the message sent to netrecceive
#define PD_SELECTOR "d6t44l"
// this is Omron's crc code from the DST-44L/DST-8L Thermal Sensor Whitepaper
unsigned char calc_crc (unsigned char data);
int D6T_checkPEC (unsigned char *buf, int pPEC);

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
  int 				file;
  char 				filename[32];
  int                           addr = 0x0A;
  unsigned char                 inbuf[63];
  unsigned char                 outbuf;
  char                          netbuf[256];
  const char                    *buffer;
  int                           i, j, s;
  int                           initialized = 0;
  struct sockaddr_in            sock;

  // set up a socket to send UDP datagrams through
  if (( s = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP )) < 0)
  {
    printf ("Unable to create socket (%d)\n", errno);
    buffer = strerror(errno);
    printf(">> %s <<\n", buffer);
    exit ( EXIT_FAILURE );
  }
  memset (( char *) &sock, 0 , sizeof(sock) );
  sock.sin_family = AF_INET;
  sock.sin_port = htons ( PORT );
  if ( inet_aton( IP, &sock.sin_addr ) == 0 )
  {
    printf ( "Unable to make address from %s\n", IP );
    exit ( EXIT_FAILURE );
  }
  // The Omron sensor streams data after being initialized
  if ( argc > 1 ) initialized = atoi(argv[1]);
  printf("%s: initialized:%d\n", argv[0], initialized);
  // Open the I2C connection
  sprintf(filename, "/dev/i2c-1");
  if ((file = open(filename, O_RDWR)) < 0)
  {
    printf ("Failed to open the bus. (%d)\n", errno);
    exit (EXIT_FAILURE);
  }

  if (ioctl(file, I2C_SLAVE, addr) < 0)
  {
    printf ("Failed to acquire bus access and/or talk to slave. (%d)\n", errno);
    exit (EXIT_FAILURE);
  }
  // start reading packets
  do
  {
    do
    {
      // clear the inbuf first
      for ( i = 0; i < SENSOR_PACKET_SIZE; ++i) inbuf[i] = 0;

      outbuf = 0x4C; // the command
      // we only need to write the command once.
      // After that the sensor will return data
      // any time it is read.
      if (! initialized)
      {
        if (write( file, &outbuf, 1 ) != 1)
        {
          printf("Write failed (%d)\n", errno);
          buffer = strerror(errno);
          printf(">> %s <<\n", buffer);
          exit (EXIT_FAILURE);
        }
        initialized = 1;
      }
      if (read (file, inbuf, SENSOR_PACKET_SIZE) != 35)
      {
        printf("Read failed (%d)\n", errno);
        buffer = strerror(errno);
        printf(">> %s <<\n", buffer);
      }
    } while (!D6T_checkPEC (inbuf, SENSOR_PACKET_SIZE-1));

    for (i = 0; i < SENSOR_PACKET_SIZE-1; i += 2)
    {
      printf("%d ", inbuf[i]+(inbuf[i+1]<<8));
    }
    printf("<0x%02X>\n", inbuf[SENSOR_PACKET_SIZE-1]);

    j = sprintf (netbuf, PD_SELECTOR);
    for (i = 0; i < SENSOR_PACKET_SIZE-1; i += 2)
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
    usleep (400000); // wait 400ms
  } while (1);
  close(file);
  return EXIT_SUCCESS;
}
 
