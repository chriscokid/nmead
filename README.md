# nmead
NMEA Server

Since the serial port on a computer system is a scarce resource, and since I would like to use several tools simultaneously to analyze the NMEA-0183 sentences output by my GPS receiver, I'm developing a simple server application that distributes NMEA data to a number of listeners.

Details

This software is under development in my spare time, and it still lacks some of the capabilities I personally need, but I've achieved good results already with Garmin and Motorola GPS receivers and listener applications ranging from telnet sessions to custom Java visualization and analysis tools.

I intend to have the finished product support the following requirements:

Capabilities

Read NMEA-0183 sentences from the NMEA source connected to the designated serial port and pass them on to all connected listeners.

Support a minimum of ten simultaneous connected listeners, either on the same system as the server or on other systems connected via network.

One capability that is intentionally not a requirement is to support applications that are designed to work directly with a serial port. This application is intended to support custom analysis tools running on a local-area network. It will use standard IP stream sockets for communication.

I'm not the first one to invent this wheel; see the Similar Software by Others section for a few of the alternatives I've discovered.

Performance Issues

In the design I'm currently pursuing, I emphasized the need for the server to avoid being adversely affected by slow or faulty connections. On connections that are too slow or are experiencing severe transmission problems, the server will allow messages to be lost. Other connections will not be made to suffer from difficulties on a problem connection.

Documentation

This is it:

    nmead -h
Since the server is still under development in my spare time, what time I do spend on the project is still spent either on the server itself or on client applications. Once I've reached a good stopping point with the server, I'll provide better user documentation.

This is a continuation of the work by Chuck Taylor (http://home.hiwaay.net/~taylorc/gps/nmea-server/)
