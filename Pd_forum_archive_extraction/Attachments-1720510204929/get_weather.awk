#!/bin/awk -f
# based on  http://www.gnu.org/software/gawk/manual/gawkinet/gawkinet.html#GETURL    
# Gets an html text weather report for Montreal, extracts the current temperature in degrees Celsius
# and pressure in kiloPascals.
# Waits for some message from pd on tcp port 8888.
# When it gets a message, it GETs the URL and parses it for the current temperature and pressure.
# Then it sends the numbers to pd on tcp port 8887
# See pd_server_awk_tester.pd

BEGIN {
  Host = "text.weatheroffice.ec.gc.ca"
  URL = "http://text.weatheroffice.ec.gc.ca/forecast/city_e.html?qc-147&unit=m"
  Port = 80
  Method = "GET"
  HttpService = "/inet/tcp/0/" Host "/" Port
  do {
    RS = "\n"
    "/inet/tcp/8888/0/0" |& getline
    ORS = RS = "\r\n\r\n"
    print Method " " URL " HTTP/1.0" |& HttpService
    HttpService |& getline Header
#  print Header > "/dev/stderr"
    RS = "<dt>"
    while ((HttpService |& getline) > 0) {
      if ($0 ~ "Temperature" ) {
	temp = "Temperature" $2 $3
      }
      if ($0 ~ "Pressure" ) {
	press = "Pressure" $2 $3
      }
    }
    close(HttpService)
    split(temp, a, "<dd>")
    split (a[2], b, "&")
    split(press, c, "<dd>")
    split (c[2], d, "&")
    output = b[1]" "d[1] ";"
    print output
# Use [netreceive 8899] in PD. Note that the semicolon must be present
# or the message will not come out of netreceive.
    print output |& "/inet/tcp/0/localhost/8887/"
  } while (1)
# well we never get here but:
  close("/inet/tcp/0/localhost/8887")
  close("/inet/tcp/8888/0/0")
}
