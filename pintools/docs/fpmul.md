   
   # fpmul pintool
  
   
   
   This tool counts the number of floating point multiplication instructions executed by a workload.
   
   It provides the following statistics are outputs:
   - _#Floating Multiply Insns_ : Total number of floating point multiplication instructions executed by a workload.
   - _#Floating Multiply Insns (Mem Refs)_  : Number of SISD floating point multiplication instructions where one of the source operands is a memory address.
   - _#Floating Multiply Insns (Only Regs)_ : Number of SISD floating point multiplication instructions where all the source operands are registers.
   - _#Floating Multiply Insns (Mem Refs) P_: Number of SIMD floating point multiplication instructions where one of the source operands is a memory address.
   - _#Floating Multiply Insns (Only Regs)P_: Number of SIMD floating point multiplication instructions where all the source operands are registers
   
   Further it also logs the operands of multiplications in fpmul.out file. 
   **For some workloads, the generated fpmul.out file can grow in size to multiples GBs! To avoid such scenarios, comment out the lines of code writing to the OutFile in fpmul.cpp.**
   
   **This tool is made for single-threaded workloads only. On usage with multi-threaded workloads it might report incorrect statistics due to races.**
