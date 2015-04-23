C++ = $(CPLUS)
# Copyright (c) 1999 Apple Computer, Inc.  All rights reserved.
#  

NAME = libUtilitiesLib
C++ = $(CPLUS)
CC = $(CCOMP)
LINK = $(LINKER)
CCFLAGS += $(COMPILER_FLAGS) -g -Wall $(INCLUDE_FLAG) ../PlatformHeader.h

LIBS = -lUtilitiesLib

# OPTIMIZATION
#CCFLAGS += -O3

# EACH DIRECTORY WITH HEADERS MUST BE APPENDED IN THIS MANNER TO THE CCFLAGS

CCFLAGS += -I.
CCFLAGS += -I..
CCFLAGS += -I../Server.tproj

C++FLAGS = $(CCFLAGS)

CFILES = 

CPPFILES =	atomic.cpp \
        	MyAssert.cpp \
        	Thread.cpp \
        	Cond.cpp \
        	Heap.cpp \
        	Queue.cpp \
        	Mutex.cpp \
        	MutexRW.cpp \
        	Memory.cpp \
        	Task.cpp \
        	OS.cpp \
        	EventContext.cpp \
        	Socket.cpp \
        	SocketUtils.cpp \
        	UDPSocket.cpp \
        	UDPDemuxer.cpp \
        	UDPSocketPool.cpp \
        	TCPSocket.cpp \
        	TCPListenerSocket.cpp \
        	Ref.cpp \
        	StrPtrLen.cpp \
        	FileSource.cpp \
        	BufferPool.cpp \
        	TimeoutTask.cpp \
        	IdleTask.cpp

all: libUtilitiesLib.a

libUtilitiesLib.a: $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)
	ar -r libUtilitiesLib.a $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)
	ranlib libUtilitiesLib.a
    
install: libUtilitiesLib.a

clean:
	rm -f libUtilitiesLib.a $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)

.SUFFIXES: .cpp .c .o

.cpp.o:
	$(C++) -c -o $*.o $(DEFINES) $(C++FLAGS) $*.cpp

.c.o:
	$(CC) -c -o $*.o $(DEFINES) $(CCFLAGS) $*.c

