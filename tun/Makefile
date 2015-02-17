include $(STITCH_TUN_SRC)/platform.mk
INCLUDES = -I./ \
 	   -I./tun_dev\
	   -I./log

#deifne the C source files
SRCS = tunclient.c

# define the C object files 
# 
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.c=.o)
MODULES := tun_dev pkt_process
BUILD_DIR = $(shell pwd)/build
STITCH_TUN_SRC = $(shell pwd)
LIB_DIR = $(BUILD_DIR)/lib
LIBRARIES = -ltun \
	    -lpktprocess\
	    -lpthread\
	    $(OPENSSL_SUPPORT_LIB)



LIBRARY_PATH = -L$(BUILD_DIR)/lib

# define the executable file 
MAIN = stitch_tun
#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#
.PHONY: all depend clean $(MODULES)
#
all:    $(MAIN)
	@echo Compiled $(MAIN) .... 
#
$(MAIN): $(MODULES)
	sudo rm -rf $(MAIN)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRCS) -o $(MAIN) $(LIBRARY_PATH) $(LIBRARIES) $(LFLAGS)

$(MODULES):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(LIB_DIR)
	$(MAKE) --directory=$@ BUILD_LIB_DIR=$(LIB_DIR) STITCH_TUN_SRC=$(STITCH_TUN_SRC) OS=$(OS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# # the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# # (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~
	sudo $(RM) $(MAIN)
	$(RM) -rf $(BUILD_DIR)

install:
	sudo chown root $(MAIN)
	sudo chmod +s $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

#                         # DO NOT DELETE THIS LINE -- make depend needs it


