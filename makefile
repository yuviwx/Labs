.PHONY: all task0 task1 task2 task3 clean

all: task0 task1 task2 task3
	@clear
	@echo "Build completed. Outputs in parent directory."

task0:
	$(MAKE) -C task0

task1:
	$(MAKE) -C task1

task2:
	$(MAKE) -C task2

task3:
	$(MAKE) -C task3

clean:
	$(MAKE) -C task0 clean
	$(MAKE) -C task1 clean
	$(MAKE) -C task2 clean
	$(MAKE) -C task3 clean
	@clear
	@echo "Clean completed."

