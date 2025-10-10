TARGET = main

# -g:include debuf information
# -I. :find target header files in the current directory
CFLAGS = -Wall -Werror -std=gnu99 -g -I.

#如果是linux则加上-no-pie，不產生pie，方便debug
UNAME_S := $(shell uname -s) 
ifeq ($(UNAME_S),Linux)
CFLAGS += -no-pie
endif


#deps:store the dependencies of the objects
OBJS = main.o verbose.o
deps := $(OBJS:%.o=.%.o.d)


# $@: 代表目標檔案，例如 main.o
# -MMD -MF .$@.d：產生相依性檔案（會追蹤標頭檔的變更）
# $<：第一個相依檔案（例如 main.c）
%.o: %.c
	$(CC) -o $@ $(CFLAGS) -c -MMD -MF .$@.d $<

#$^:所有相依檔案，例如 main.o verbose.o
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean check
clean:
	$(RM) $(TARGET) $(OBJS) $(deps) 

#引入相依性檔案，讓make知道哪些檔案需要重新編譯
-include $(deps)