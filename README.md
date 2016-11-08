# HobbyOS
A small, hobby RTOS in C.  

Supports real-time scheduling (obviously), lists, queues, software timers, and events. Right now, it's only ported to the PIC32MX family. Features automated unit testing on x86 hosts.

Building:  
1. Download and unzip https://cpputest.github.io/  
2. Edit Makefile and change CPPUTEST_LOC to the location of CppUTest.  
3. Run make.  
4. If everything went correctly, at the end you should see:  

  >./build/HobbyOS.exe..................................................  
  .  
  OK (51 tests, 51 ran, 515 checks, 0 ignored, 0 filtered out, 47 ms)  
