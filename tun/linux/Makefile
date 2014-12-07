CFLAGS=-Wall -g
CC = gcc
INCLUDES = -I./
BUILD_DIR = build/
STATIC_LINKER = ar

#deifne the C source files
SRCS = tun.c 

# define the C object files 
# 
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.c=.o)

LIB = libtun.a

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#
#
.PHONY: depend clean
#
all:    $(OBJS) $(LIB)
	@echo Compiled $(OBJS) .... 

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# # the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# # (see the gnu make manual section about automatic variables)
.c.o:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $(BUILD_DIR)/$@

$(LIB):
	$(STATIC_LINKER) -cvq $(BUILD_DIR)/$(LIB) $(BUILD_DIR)/*.o
	mv $(BUILD_DIR)/$(LIB) $(BUILD_LIB_DIR)

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

#                         # DO NOT DELETE THIS LINE -- make depend needs it


