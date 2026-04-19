#---------------------------------------------------------------------------------------------------------------------
# TARGET is the name of the output.
TARGET      := battle_chatbox

# BUILD is the directory where object files & intermediate files will be placed.
BUILD       := build

# LIBBUTANO is the main directory of butano library (https://github.com/GValiente/butano).
LIBBUTANO   := ../../extern/butano/butano

# PYTHON is the path to the python interpreter.
PYTHON      := python

# SOURCES is a list of directories containing source code.
SOURCES     := src ../../extern/butano/common/src

# INCLUDES is a list of directories containing extra header files.
INCLUDES    := include ../../extern/butano/common/include

# DATA is a list of directories containing binary data files with *.bin extension.
DATA        :=

# GRAPHICS is a list of files and directories containing files to be processed by grit.
GRAPHICS    := ../../extern/butano/common/graphics

# AUDIO is a list of files and directories containing files to be processed by the audio backend.
AUDIO       := ../../extern/butano/common/audio

# AUDIOBACKEND specifies the backend used for audio playback. Supported backends: maxmod, aas, null.
AUDIOBACKEND := maxmod

# AUDIOTOOL is the path to the tool used process the audio files.
AUDIOTOOL   :=

# DMGAUDIO is a list of files and directories containing files to be processed by the DMG audio backend.
DMGAUDIO    := ../../extern/butano/common/dmg_audio

# DMGAUDIOBACKEND specifies the backend used for DMG audio playback. Supported backends: default, null.
DMGAUDIOBACKEND := default

# ROMTITLE specifies the title of the ROM.
ROMTITLE    := BATTLE CHATBOX

# ROMCODE specifies the ROM code.
ROMCODE     := SVTS

# USERFLAGS specifies additional flags for the compiler.
USERFLAGS   :=

# USERCXXFLAGS specifies additional flags for the C++ compiler.
USERCXXFLAGS :=

# USERASFLAGS specifies additional flags for the assembler.
USERASFLAGS :=

# USERLDFLAGS specifies additional flags for the linker.
USERLDFLAGS :=

# USERLIBDIRS specifies additional directories containing libraries.
USERLIBDIRS :=

# USERLIBS specifies additional libraries.
USERLIBS    :=

# DEFAULTLIBS specifies default libraries to link.
DEFAULTLIBS :=

#---------------------------------------------------------------------------------------------------------------------
# End of configuration
#---------------------------------------------------------------------------------------------------------------------

ifndef LIBBUTANOABS
	export LIBBUTANOABS := $(realpath $(LIBBUTANO))
endif

include $(LIBBUTANOABS)/butano.mak