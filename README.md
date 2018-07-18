# AVR Emulator and Testing client project

## General description
Just sample project of demonstation Qt features and client\server connection.
This is implementation of emulator of remote physical controller.
This project containing two subprojects. Emulator of AVR device and it's client for testing it.  
Project written in C++ language on Qt framework and uses only Qt libraries.  
AVR device can be manipulated by client application for moving and receving it's position.  

## Build guide
For the first of all you need to make shure you have necessary components and software on your computer.

### \*nix and Mac OS
You must have C++ compiler (e.g. GCC), Qt framefork (5.0 version or higher), git, qmake and make tools installed.

1. Create new directory, for example: `$ mkdir /home/user/Documents/avr`
2. Go to your new directoty: `$ cd /home/user/Documents/avr`
3. Clone project from github: `$ git clone https://github.com/fullmetal-a/avr.git`
4. Run qmake: `$ qmake`
5. Run make: `$ make`
6. You are done. Projects has been compiled and linked.

#### Run
* To run AVR Emulatror: `$ ./bin/emulator/AVR_Emulator`
* To run AVR Testing client: `$ ./bin/client/AVR_Testing`


### Windows
The simplest way to compile it is open project in Qt Creator.  
You also must have Git, Qt framework  (5.0 version or higher) and any compiler on your computer (e.g. MSVC or Mingw32).  

1. Open git bash emulator and create a directory, for example: `$ mkdir D:/projects/avr`
2. Go to your new directoty: `$ cd D:/projects/avr`
3. Clone project from github: `$ git clone https://github.com/fullmetal-a/avr.git`
4. Open AVR.pro file in Qt Creator.
5. Select any available compiler you prefer.
6. Setup output pathes in Project settings if you wish.
7. (Optional) Configure project to use static Qt link (instead of dynamic).
8. Select Release configuration in Qt Creator.
9. Right-click main AVR project in project tree of Qt Creator and click Build.

#### Run
If you didn't configured static link applications wouldn't work without Qt DLL libraries. In this case you have to do such actions:

1. Go to compiled folder of AVR project, launch emulator or client and look for what libraries it will ask. 
2. Than go to Qt binaries directory for copmpiler you used. For example: `D:/Qt/<Folder names as your Qt version>/<Folder named as compiler you used>/bin/`
3. Copy all required DLLs from this directory to directory of your compiled executables.

Now just go to directory of your compiled binaries and launch AVR_Emulator.exe and AVR_Testing.exe.

## How to use
This two applications are using the Client-Server principle. AVR Emulator is a server and AVR Testing is a client.  
Default port of AVR Server is 28338. But you are able to change port and host to any value you wish. Just launch AVR emulator with argument `-port <Your port>` or `-host <Your host>` (or even both). For example: `$ ./AVR_Emulator -port 1234 -host 192.168.0.4`  
Also AVR Emulator could lie when client asking for it's position (When initialy saying current position to client it never lies). Default chance to lie is 10%. But you are able to change it if you launch emulator with `-ctl <Chance>` argument. For example: `$ ./AVR_Emulator -ctl 50` (It means launch AVR Emulator with 50% chance to lie about it's position. This value must be between 0 and 100.  
Maximum position of AVR system is 15000 by default. You also can change it by passing launch argument `-maxpos <Value>`. For example: `$ ./AVR_Emulator -maxpos 380000`. This value must be between 1 and 100000.  
The client can connect to the emulator from both the local machine and another computer on the local network (or even the Internet).  
There is only one client is able to be connected to AVR emulator host due to safety reasons.  
Main window of AVR Emulator shows value of current position (if it moves you see in real time how position changes). And also state of connection (Client connected or not) and host information.  
  
### To manipulate launched AVR you have to:

1. launch client application. 
2. In "Connection data" tab enter port and host of AVR Emulator's machine if you need. 
3. In Connection menu click "Connect".
4. When it's connected your client has been initialized and ready to work. You can go now to "AVR Controls" tab.

### You have to know:

1. You are able to move AVR's position for some steps forward or backward. To move forward just enter steps quantity and click Move. For moving backward enter negative value (e.g. -56).
2. All orders sent to AVR System will be processed. They are all will be queued and safely executed one by one.
3. When AVR finished it's moving it will notify client that work has been complete.
4. Client always calculating current AVR position localy to compare obtained position from AVR host with it. If AVR lies about it's position this will be immediately detected.
5. When AVR on zero position it never lies about it.
6. You can disconnect and connect to AVR host any time in Connect menu.
7. You are able to clear output log and save it to file if it's needed.
