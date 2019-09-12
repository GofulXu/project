ifneq ($(VERSION), )
BUILD_VERSION = .$(VERSION)
endif

#LINK_PATH = /opt/hisi-linux/x86-arm/arm-hisiv200-linux/target/bin/
#CROSS_COMPILE = $(LINK_PATH)arm-hisiv200-linux-

#LINK_PATH = arm-none-linux-gnueabi-
CROSS_COMPILE = $(LINK_PATH)
CC = gcc
LINK = $(CROSS_COMPILE)$(CC)
LINK_AR = $(CROSS_COMPILE)ar

CCSUFIX = cpp
CFLAGS += -DLINUX -g					 \
		 -I$(PROJECT_PATH)/include	

ifeq ($(SUPPORT_BUILD_JNI), y)
CFLAGS += -I$(PROJECT_PATH)/JNI	\
		 -I$(JAVA_HOME)/include	\
		 -I$(JAVA_HOME)/include/linux
CFLAGS += -DANDROID
endif

ifeq ($(SUPPORT_BUILD_STDC), y)
LINKFLAGS += -lstdc++
endif

DYN_LDS_WITH += 

LINKFLAGS += 

SUB_OUT_DIR = $(subst $(PROJECT_PATH),,$(shell pwd))

BUILD_OUT_PATH := $(PROJECT_PATH)/out$(SUB_OUT_DIR)/

OBJ += $(patsubst %.c,$(BUILD_OUT_PATH)%.o,$(patsubst %.$(CCSUFIX),$(BUILD_OUT_PATH)%.o,$(SRC)))
	

ifneq ($(findstring $(BUILD_TARGET_TYPE), EXE exe),)
BUILD_NAME = $(NAME)
LN_NAME = $(BINDIR)/$(NAME)
BIN = $(BINDIR)/$(NAME)$(BUILD_VERSION)
else ifneq ($(findstring $(BUILD_TARGET_TYPE), STATIC static),)
BUILD_NAME = lib$(NAME).a$(BUILD_VERSION)
LN_NAME = $(BINDIR)/lib$(NAME).a
BIN = $(BINDIR)/$(BUILD_NAME)
else ifneq ($(findstring $(BUILD_TARGET_TYPE), DLL dll),)
BUILD_NAME = lib$(NAME).so$(BUILD_VERSION)
LN_NAME = $(BINDIR)/lib$(NAME).so
BIN = $(BINDIR)/$(BUILD_NAME)
endif

ifneq ($(findstring $(BUILD_TARGET_TYPE), EXE exe),)
$(BIN): $(OBJ)
	$(Q_)echo Create bin file $(BIN)
	$(LINK) $(OBJ) -o $(BIN) $(LINKFLAGS) $(DYN_LDS_WITH)  && chmod a+x $(BIN)
ifneq ($(VERSION), )
	$(Q_)rm -f $(LN_NAME)
	$(Q_)ln -s $(BIN) $(LN_NAME)
endif
	$(Q_)echo "\n"
	$(Q_)md5sum $(BIN)
	$(Q_)echo "\n"
	$(Q_)rm -f $(BINDIR)/MD5_$(BUILD_NAME).md5
	$(Q_)md5sum $(BIN) >> $(BINDIR)/MD5_$(BUILD_NAME).md5

else ifneq ($(findstring $(BUILD_TARGET_TYPE), STATIC static),)
$(BIN): $(OBJ)
	$(Q_)echo Create  file $(BIN)
	$(LINK_AR) rcs $(BIN) $(OBJ) && chmod 775 $(BIN)
ifneq ($(VERSION), )
	$(Q_)rm -f $(LN_NAME)
	$(Q_)ln -s $(BIN) $(LN_NAME)
endif
	$(Q_)echo "\n"
	$(Q_)md5sum $(BIN)
	$(Q_)echo "\n"
	$(Q_)rm -f $(BINDIR)/MD5_$(BUILD_NAME).md5
	$(Q_)md5sum $(BIN) >> $(BINDIR)/MD5_$(BUILD_NAME).md5

else ifneq ($(findstring $(BUILD_TARGET_TYPE), DLL dll),)
$(BIN): $(OBJ)
	$(Q_)echo Create  file $(BIN)
	$(LINK) -shared $(OBJ) -o $(BIN) $(LINKFLAGS) $(DYN_LDS_WITH) && chmod 775 $(BIN)
ifneq ($(VERSION), )
	$(Q_)rm -f $(LN_NAME)
	$(Q_)ln -s $(BIN) $(LN_NAME)
endif
	$(Q_)echo "\n"
	$(Q_)md5sum $(BIN)
	$(Q_)echo "\n"
	$(Q_)rm -f $(BINDIR)/MD5_$(BUILD_NAME).md5
	$(Q_)md5sum $(BIN) >> $(BINDIR)/MD5_$(BUILD_NAME).md5
endif

$(BUILD_OUT_PATH)%.o: %.c
	$(Q_)mkdir -p $(dir $@)
	$(Q_)${LINK} $(CFLAGS) -c $< -o $@

play:
	$(Q_)chmod +x $(BIN);$(BIN)

clean:
	$(Q_)rm -f $(OBJ)

cleanall:
	$(Q_)rm -f $(BINDIR)/MD5_$(BUILD_NAME).md5
	$(Q_)rm -f $(LN_NAME)
	$(Q_)rm -f $(BIN)
	$(Q_)rm -f $(OBJ)

copy:
ifneq ($(findstring $(BUILD_TARGET_TYPE), EXE exe),)
	$(Q_)cp -f $(BIN) $(GTFTP)/hisipro/bin
	$(Q_)cp -f $(BINDIR)/MD5_$(BUILD_NAME).md5 $(GTFTP)/hisipro/bin
else ifneq ($(findstring $(BUILD_TARGET_TYPE), STATIC static),)
	$(Q_)cp -f $(BIN) $(GTFTP)/hisipro/lib
	$(Q_)cp -f $(BINDIR)/MD5_$(BUILD_NAME).md5 $(GTFTP)/hisipro/lib
else ifneq ($(findstring $(BUILD_TARGET_TYPE), DLL dll),)
	$(Q_)cp -f $(BIN) $(GTFTP)/hisipro/lib
	$(Q_)cp -f $(BINDIR)/MD5_$(BUILD_NAME).md5 $(GTFTP)/hisipro/lib
else 
	$(Q_)cp -f $(BIN) $(GTFTP)/hisipro 
endif
