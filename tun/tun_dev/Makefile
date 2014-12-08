ifeq ($(OS), LINUX)
MODULES=linux
else
MODULES = osx
endif
BUILD_DIR=$(STITCH_TUN_SRC)/build
LIB_DIR=$(BUILD_DIR)/lib

.PHONY: all $(MODULES)

all: $(MODULES)

$(MODULES):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(LIB_DIR)
	$(MAKE) --directory=$@ BUILD_LIB_DIR=$(LIB_DIR) STITCH_TUN_SRC=$(STITCH_TUN_SRC) OS=$(OS)


