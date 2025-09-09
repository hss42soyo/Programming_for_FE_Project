g++ -O2 -g -pg Test_Program.cpp Original_Linear_Operation.cpp -o test_prog

./test_prog.exe

gprof ./test_prog.exe gmon.out > gprof_report.txt