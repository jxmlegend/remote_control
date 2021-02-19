VERSION = 1.1

# arm -> linux arm rk3188 
# x86 -> window x86 mingw32 dll
# x64 -> linux x64

TARGET_ARCH = x64
CROSS_COMPILE = #arm-
DEBUG = -g #-O2

TOP_DIR := $(shell pwd)
OBJ_DIR := $(TOP_DIR)/obj
outdir := $(TOP_DIR)/bin
QT_DIR := $(TOP_DIR)/qt_pro

SDL_DIR := ./SDL/$(TARGET_ARCH)
FFMPEG_DIR := ./ffmpeg/$(TARGET_ARCH)

AS = $(CROSS_COMPILE)as
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar
STRIP = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
QMAKE= qmake
CP = cp

CONFIG_COMPILER = gnu

outdir = ./bin

exeobj = ScreenCast

dllobj = ScreenCast.dll

qtobj = qt_window

libobj = libScreenCast.dll.a

mainobj = main.o socket.o frame.o packet.o version.o audio.o video.o avi_enc.o log.o crc.o rtspd.o rtp.o rtcp.o \
		mmap_cache.o alarm.o signals.o list.o dll.o pipe.o ffmpeg.o sdl.o client.o h264.o server.o tools.o

cppobj = 

all: $(exeobj)

DISPLAY = SDL

CFLAGS := -I. -I./include/ -I$(SDL_DIR)/include -I$(FFMPEG_DIR)/include \
		 -L$(SDL_DIR)/lib  -lSDL2 -lSDL2_ttf \
		 -L$(FFMPEG_DIR)/lib -L./lib -lavcodec -lavformat -lswscale -lavutil -lavdevice -lavfilter -lpostproc \
		#alsa_lib/include/ -L alsa_lib/lib/ -lasound


ifeq ($(TARGET_ARCH), arm)
CFLAGS += -lX11 -lpthread  -lXtst  -lXext -lXinerama -lXrandr -lXfixes -lXdamage
			#-I./vpu/include -L./vpu/lib -lvpu -lvpu_avcdec -lion_vpu -lrk_codec -lvpu_avcenc  -lGLESv2  -lUMP \
			-lpthread #-lvpu -lvpu_avcdec -lvpu_avcenc 
else ifeq ($(TARGET_ARCH), x86)
CFLAGS += -lmingw32 -lm -lws2_32 -lpthreadGC2 -lgdi32 
else ifeq ($(TARGET_ARCH), x64)
CFLAGS += -lpthread -lX11 -lXext -lXrandr
endif

ifeq ($(DISPLAY), QT)
	CFLAGS += -lqt_window
	DEFINES += -D QT
else ifeq ($(DISPLAY), SDL)
	DEFINES += -D SDL
endif

CXXFLAGS = $(CFLAGS)

VPATH = .:./include:./lib:./src:


$(mainobj):%.o:%.c
	$(CC) -Wall $(DEBUG) $(CFLAGS) $(DEFINES) -c $< -o $@ 
    
$(cppobj):%.o:%.cpp
	$(CXX) -Wall $(DEBUG) $(CXXFLAGS) $(DEFINES) -c $< -o $@

$(exeobj):$(mainobj) $(cppobj) 
	$(CXX) $(DEBUG)  -o $(outdir)/$(exeobj) $(mainobj) $(cppobj)  $(CXXFLAGS) 
	rm -f *.o
	@echo "Version $(VERSION)"
	@echo "Build  $(TARGET_ARCH) program $(exeobj) OK"
ifeq ($(TARGET_ARCH), x86)
	$(CP) ./lib/*.dll $(outdir)
	$(CP) ./$(FFMPEG_DIR)/bin/*.dll $(outdir)
	$(CP) ./$(SDL_DIR)/bin/*.dll $(outdir)
endif

$(dllobj):$(mainobj) $(cppobj)
	$(CXX) $(DEBUG) -shared -o $(outdir)/$(dllobj) $(mainobj) $(cppobj) $(CXXFLAGS) $(DLL) -Wl,--kill-at,--out-implib,$(outdir)/$(libobj) $(DEFINES)
	rm -f *.o
	@echo "Version $(VERSION)"
ifeq ($(TARGET_ARCH), x86)
	$(CP) ./lib/*.dll $(outdir)
	$(CP) ./$(FFMPEG_DIR)/bin/*.dll $(outdir)
	$(CP) ./$(SDL_DIR)/bin/*.dll $(outdir)
endif

dll:$(dllobj)
	@echo "build  only dll $(dllobj)"

qt:
	cd $(QT_DIR) && $(QMAKE) 
	$(MAKE) -C $(QT_DIR) -j4 
	#cd $(QT_DIR) && /c/Qt/4.8.6/mingw32/bin/mingw32-make.exe -f Makefile.Debug

clean:
	rm -f *.o $(outdir)/$(exeobj) $(outdir)/*.dll $(outdir)/*.a
	#cd $(QT_DIR) && $(MAKE) clean

