This tool counts the number of floating point MOV instructions executed by a workload.

It provides the following statistics as output:
- _FPMOV Count_      : Total number of floating point MOV instructions executed by the workload
- _Intra. Reg Count_ : Number of FP MOV instructions where both the source and destination operands are registers.
- _RegMem Count_     : Number of FP MOV instructions where either the source is register and destination is memory or source is memory and destination is register.
- _Load Count_       : Number of FP MOV instructions where the source operand is memory and destination operand is register.
- _Store Count_      : Number of FP MOV instructions where the source operand is register and destination operand is memory.
- _SP Count_         : Number of single-precision (SP) floating point MOV instructions executed by the workload.
- _DP Count_         : Number of double-precision (DP) floating point MOV instructions executed by the workload.
- _SP Load Count_    : Number of single-precision (SP) floating point MOV instructions executed by the workload, where source operand is memory and destination operand is register.
- _DP Load Count_    : Number of double-precision (DP) floating point MOV instructions executed by the workload, where source operand is memory and destination operand is register.
- _SP Store Count_   : Number of single-precision (SP) floating point MOV instructions executed by the workload, where source operand is register and destination operand is memory.
- _DP Store Count_   : Number of double-precision (DP) floating point MOV instructions executed by the workload, where source operand is register and destination operand is memory.
- _SP IntraReg Count_: Number of single-precision (SP) floating point MOV instructions executed by the workload, where both source and destination operands are registers.
- _DP IntraReg Count_: Number of double-precision (DP) floating point MOV instructions executed by the workload, where both source and destination operands are registers.

**This tool is made for single-threaded workloads only. On usage with multi-threaded workloads it might report incorrect statistics due to races.**
