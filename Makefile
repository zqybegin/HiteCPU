include ../Makefile

MODULE = Toplevel

V_RSC = $(shell find $(abspath ./generated) -name "*.v")
C_RSC = $(shell find $(abspath ./sim) -name "*.cpp")
SCALA_RSC = $(shell find $(abspath ./hiteCPU/src) -name "*.scala")

V_RESULT = $(abspath ./generated/$(MODULE).v)

BUILD_DIR = $(abspath ./build_dir)
EXE = $(BUILD_DIR)/V$(MODULE)
VCD = $(BUILD_DIR)/dump.vcd

$(V_RESULT):$(SCALA_RSC)
	mill hiteCPU.run

$(EXE):$(V_RSC) $(C_RSC) $(V_RESULT)
	verilator --trace -cc --build --x-assign 1 --top-module $(MODULE) -exe $(V_RSC) $(C_RSC) --Mdir $(BUILD_DIR) -o $(EXE)

$(VCD):$(EXE)
	$(EXE)
	mv ./dump.vcd $(VCD)

.PHONY:verilog sim clean bsp echo

verilog:$(V_RESULT)

sim:$(VCD)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	gtkwave $(BUILD_DIR)/dump.vcd

clean:
	rm -rf $(BUILD_DIR)
	rm -rf ./generated

bsp:
	mill mill.bsp.BSP/install

echo:
	@echo $(MODULE)
	@echo $(V_RSC)
	@echo $(C_RSC)
	@echo $(BUILD_DIR)
	@echo $(EXE)
