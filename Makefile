MODULE = Toplevel
VRSC = $(shell find $(abspath ./generated) -name "*.v")
# CRSC = $(shell find $(abspath ./sim) -name "*.cpp" -or -name "*.c" -or -name "*.h" )
CRSC = $(shell find $(abspath ./sim) -name "*.cpp")

BUILD_DIR = $(abspath ./build_dir)
EXE   = $(BUILD_DIR)/V$(MODULE)

.PHONY:sim clean verilog

verilog:
	rm -rf ./generated
	mill hiteCPU.run

$(EXE):$(VRSC) $(CRSC) verilog
	rm -rf $(BUILD_DIR)
	verilator --trace -cc --build --x-assign 1 --top-module $(MODULE) -exe $(VRSC) $(CRSC) --Mdir $(BUILD_DIR) -o $(EXE)

sim:$(EXE)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	$(EXE)
	mv ./dump.vcd $(BUILD_DIR)/dump.vcd
	gtkwave $(BUILD_DIR)/dump.vcd

include ~/Workstation/ysyx-workbench/Makefile

clean:
	rm -rf $(BUILD_DIR)
	rm -rf ./generated

bsp:
	mill mill.bsp.BSP/install

echo:
	@echo $(MODULE)
	@echo $(VRSC)
	@echo $(CRSC)
	@echo $(BUILD_DIR)
	@echo $(EXE)
