include ../Makefile

MODULE = Toplevel
NAME ?= default

# ----- source code -----
V_RSC = $(shell find $(abspath ./verilog) -name "*.v")
H_RSC = $(shell find $(abspath ./sim) -name "*.h")
C_RSC = $(shell find $(abspath ./sim) -name "*.cpp")
SCALA_RSC = $(shell find $(abspath ./hiteCPU/src) -name "*.scala")
V_RESULT = $(abspath ./verilog/$(MODULE).v) # a representative for generating verilog code

# ----- build target -----
BUILD_DIR = $(abspath ./build_dir)
VCD_DIR = $(abspath ./vcd_dir)
EXE = $(BUILD_DIR)/V$(MODULE)
VCD = $(VCD_DIR)/$(NAME).vcd

# ----- verilator exec flag -----
INCLUDE = /home/zqybegin/Workstation/ysyx-workbench/npc/sim/include
EMU_CXXFLAGS = -I$(INCLUDE)

VERILATOR_FLAGS =                   \
 	--build --exe                   \
	--trace                         \
	--cc -O3 --top-module $(MODULE) \
 	-CFLAGS "$(EMU_CXXFLAGS)"       \
	--Mdir $(BUILD_DIR) -o $(EXE)

# ----- simulate execute flag -----
DIFF_SO = $(abspath ../nemu/build/riscv32-nemu-interpreter-so)
SIM_FLAGS += --vcd=$(VCD)
SIM_FLAGS += --diff=$(DIFF_SO)
SIM_FLAGS += $(if $(IMG), --img=$(IMG).bin --elf=$(IMG).elf,)
SIM_FLAGS += $(if $(LOG), --log=$(LOG),)

# ----- COMPILE RULES -----
$(V_RESULT):$(SCALA_RSC)
	mill hiteCPU.run

$(EXE):$(C_RSC) $(V_RESULT) $(H_RSC)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	verilator $(VERILATOR_FLAGS) $(V_RSC) $(C_RSC)

$(VCD):$(EXE)
	-$(EXE) $(SIM_FLAGS)

# ----- MAKEFILE TAGERTS -----
.PHONY:verilog sim wave clean bsp echo

verilog:$(V_RESULT)

sim:$(EXE)
	$(EXE) $(SIM_FLAGS)

wave:$(VCD)
	gtkwave $(VCD)

clean:
	rm -rf ./build_dir/*
	rm -rf ./vcd_dir/*
	rm -rf ./verilog/*

bsp:
	mill mill.bsp.BSP/install
