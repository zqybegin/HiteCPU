include ../Makefile

MODULE = Toplevel
NAME ?= default

BUILD_DIR = $(abspath ./build_dir)
VCD_DIR = $(abspath ./vcd_dir)

# source code
V_RSC = $(shell find $(abspath ./verilog) -name "*.v")
C_RSC = $(shell find $(abspath ./sim) -name "*.cpp")
SCALA_RSC = $(shell find $(abspath ./hiteCPU/src) -name "*.scala")

# a representative for generating verilog code
V_RESULT = $(abspath ./verilog/$(MODULE).v)

# a ref model used for difftest
DIFF_SO = $(abspath ../nemu/build/riscv32-nemu-interpreter-so)

# build target
EXE = $(BUILD_DIR)/V$(MODULE)
VCD = $(VCD_DIR)/$(NAME).vcd

# ----- COMPILE RULES -----
$(V_RESULT):$(SCALA_RSC)
	mill hiteCPU.run

$(EXE):$(C_RSC) $(V_RESULT)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	verilator --trace -cc --build --top-module $(MODULE) -exe $(V_RSC) $(C_RSC) --Mdir $(BUILD_DIR) -o $(EXE)

$(VCD):$(EXE)
	-$(EXE) $(VCD) $(IMG) $(DIFF_SO)

# ----- MAKEFILE TAGERTS -----
.PHONY:verilog sim wave clean bsp echo

verilog:$(V_RESULT)

sim:$(EXE)
	$(EXE) $(VCD) $(IMG) $(DIFF_SO)

wave:$(VCD)
	gtkwave $(VCD)

clean:
	rm -rf ./build_dir/*
	rm -rf ./vcd_dir/*
	rm -rf ./verilog/*

bsp:
	mill mill.bsp.BSP/install

echo:
	@echo $(NAME)
	@echo $(IMG)
	@echo $(MODULE)
	@echo $(V_RSC)
	@echo $(C_RSC)
	@echo $(BUILD_DIR)
	@echo $(EXE)
