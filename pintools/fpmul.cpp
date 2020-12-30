/*
 * Copyright 2002-2019 Intel Corporation.
 * 
 * This software is provided to you as Sample Source Code as defined in the accompanying
 * End User License Agreement for the Intel(R) Software Development Products ("Agreement")
 * section 1.L.
 * 
 * This software and the related documents are provided as is, with no express or implied
 * warranties, other than those that are expressly stated in the License.
 */


// Intel ISA Manual Ref. Link
// https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-instruction-set-reference-manual-325383.pdf
//Latest Ref: https://software.intel.com/en-us/download/intel-64-and-ia-32-architectures-sdm-combined-volumes-1-2a-2b-2c-2d-3a-3b-3c-3d-and-4


#include <iostream>
#include <fstream>
#include "pin.H"
using std::cerr;
using std::ofstream;
using std::ios;
using std::string;
using std::endl;
#define XO(opcode) (XED_ICLASS_##opcode)

ofstream OutFile;

// The running count of instructions is kept here
// make it static to help the compiler optimize docount
static UINT64 fpmulinsncount = 0;
static UINT64 mulregmemcount = 0;
static UINT64 mulregregcount = 0;
static UINT64 mulregmemcountp = 0;
static UINT64 mulregregcountp = 0;

// This function is called before every instruction is executed
VOID regmemcounter( PIN_REGISTER* reg, ADDRINT addr, uint32_t size, bool isSP)
{
    fpmulinsncount++;
    mulregmemcount++;
    
    if (isSP)
    {
        FLT32 val;
        PIN_SafeCopy(&val, (void *)addr, size); 
        OutFile << "SP " << reg->flt[0] << " " << val << endl;
    }
    else
    {
        FLT64 val;
        PIN_SafeCopy(&val, (void *)addr, size);
        OutFile << "DP " << reg->dbl[0] << " " << val << endl;
    } 
}

VOID regmemcounterp( PIN_REGISTER* reg, ADDRINT addr, uint32_t size, bool isSP)
{
    fpmulinsncount++;
    mulregmemcount++;
    mulregmemcountp++;
    if (isSP)
    {
        FLT32 val;
        for(unsigned int i=0; i < MAX_FLOATS_PER_PIN_REG; i++)
        {
            PIN_SafeCopy(&val, (void *)(addr + i), 4);
            OutFile << "SP " << reg->flt[i] << " " << val << endl;
        }
    }
    else
    {
        FLT64 val;
        for(unsigned int i=0; i < MAX_DOUBLES_PER_PIN_REG; i++)
        {
            PIN_SafeCopy(&val, (void *)(addr + i), 8);
            OutFile << "DP " << reg->dbl[i] << " " << val << endl;
        }
    }
}


VOID regregcounter(PIN_REGISTER* reg1, PIN_REGISTER* reg2, bool isSP)
{
    fpmulinsncount++;
    mulregregcount++;
    if(isSP)
    {
        OutFile << "SP " << reg1->flt[0] << " " << reg2->flt[0] << endl;
    }
    else
    {
        OutFile << "DP " << reg1->dbl[0] << " " << reg2->dbl[0] << endl;
    }
}

VOID regregcounterp(PIN_REGISTER* reg1, PIN_REGISTER* reg2, bool isSP)
{
    fpmulinsncount++;
    mulregregcount++;
    mulregregcountp++;
    if(isSP)
    {
        for(unsigned int i=0; i < MAX_FLOATS_PER_PIN_REG; i++)
        {
            OutFile << "SP " << reg1->flt[i] << " " << reg2->flt[i] << endl;
        }
    }
    else
    {
        for(unsigned int i=0; i < MAX_DOUBLES_PER_PIN_REG; i++)
        {
            OutFile << "DP " << reg1->dbl[i] << " " << reg2->dbl[i] << endl;
        }
    }
}





// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
    switch (INS_Opcode(ins))
    {
        case XO(MULSS):
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 2);
             REG reg1 = INS_OperandReg(ins, 0);
             if(INS_OperandIsMemory(ins, 1))
             {
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter, IARG_REG_REFERENCE, reg1, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, true, IARG_END);
             }
             else
             {
                 REG reg2 = INS_OperandReg(ins, 1);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_BOOL, true, IARG_END);
             } 
             break;         
        }
        case XO(MULSD):
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 2);
             REG reg1 = INS_OperandReg(ins, 0);
             if(INS_OperandIsMemory(ins, 1))
             {
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter, IARG_REG_REFERENCE, reg1, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
             }
             else
             {
                 REG reg2 = INS_OperandReg(ins, 1);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_BOOL, false, IARG_END);
             }
             break;          
        }
        case XO(VMULSS):
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 3);
             assert(INS_OperandWritten(ins, 0));
             //REG reg1 = INS_OperandReg(ins, 0);
             REG reg2 = INS_OperandReg(ins, 1);
             if(INS_OperandIsMemory(ins, 2))
             {
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, true, IARG_END);
             }
             else
             {
                 REG reg3 = INS_OperandReg(ins, 2);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, true, IARG_END);
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
             } 
             break;   
        }   
        case XO(VMULSD):
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 3);
             assert(INS_OperandWritten(ins, 0));
             //REG reg1 = INS_OperandReg(ins, 0);
             REG reg2 = INS_OperandReg(ins, 1);
             if(INS_OperandIsMemory(ins, 2))
             {
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
             }
             else
             {
                 REG reg3 = INS_OperandReg(ins, 2);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
             } 
             break;   
        }
        case XO(VFMADD132SD):
        case XO(VFMSUB132SD):
        case XO(VFNMADD132SD):
        case XO(VFNMSUB132SD):
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 3);
             assert(INS_OperandWritten(ins, 0));
             REG reg1 = INS_OperandReg(ins, 0);
             //REG reg2 = INS_OperandReg(ins, 1);
             if(INS_OperandIsMemory(ins, 2))
             {
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter, IARG_REG_REFERENCE, reg1, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
             }
             else
             {
                 REG reg3 = INS_OperandReg(ins, 2);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
             } 
             break;   
        }
        case XO(VFMADD213SD):
        case XO(VFMSUB213SD):
        case XO(VFNMADD213SD):
        case XO(VFNMSUB213SD):
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 3);
             assert(INS_OperandWritten(ins, 0));
             REG reg1 = INS_OperandReg(ins, 0);
             REG reg2 = INS_OperandReg(ins, 1);
             INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_BOOL, false, IARG_END);
             break;   
        }
        case XO(VFMADD231SD):
        case XO(VFMSUB231SD):
        case XO(VFNMADD231SD):
        case XO(VFNMSUB231SD):
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 3);
             assert(INS_OperandWritten(ins, 0));
             //REG reg1 = INS_OperandReg(ins, 0);
             REG reg2 = INS_OperandReg(ins, 1);
             if(INS_OperandIsMemory(ins, 2))
             {
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
             }
             else
             {
                 REG reg3 = INS_OperandReg(ins, 2);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
             } 
             break;   
        }        
        case XO(VFMADDSD):
        case XO(VFNMADDSD):
        case XO(VFMSUBSD):
        case XO(VFNMSUBSD):
        {
            //REF: https://en.wikipedia.org/wiki/FMA_instruction_set
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 4);
             assert(INS_OperandWritten(ins, 0));
             //REG reg1 = INS_OperandReg(ins, 0);
             REG reg2 = INS_OperandReg(ins, 1);
             if(INS_OperandIsMemory(ins, 2))
             {
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
             }
             else
             {
                 REG reg3 = INS_OperandReg(ins, 2);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
             } 
             break;   
        } 
        case XO(VFMADDSS):
        case XO(VFNMADDSS):
        case XO(VFMSUBSS):
        case XO(VFNMSUBSS):
        {
            //REF: https://en.wikipedia.org/wiki/FMA_instruction_set
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 4);
             assert(INS_OperandWritten(ins, 0));
             //REG reg1 = INS_OperandReg(ins, 0);
             REG reg2 = INS_OperandReg(ins, 1);
             if(INS_OperandIsMemory(ins, 2))
             {
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, true, IARG_END);
             }
             else
             {
                 REG reg3 = INS_OperandReg(ins, 2);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, true, IARG_END);
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
             } 
             break;   
        }        
        case XO(VFMADD132SS):
        case XO(VFMSUB132SS):
        case XO(VFNMADD132SS):
        case XO(VFNMSUB132SS):
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 3);
             assert(INS_OperandWritten(ins, 0));
             REG reg1 = INS_OperandReg(ins, 0);
             //REG reg2 = INS_OperandReg(ins, 1);
             if(INS_OperandIsMemory(ins, 2))
             {
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter, IARG_REG_REFERENCE, reg1, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, true, IARG_END);
             }
             else
             {
                 REG reg3 = INS_OperandReg(ins, 2);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg3, IARG_BOOL, true, IARG_END);
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
             } 
             break;   
        }
        case XO(VFMADD213SS): 
        case XO(VFMSUB213SS): 
        case XO(VFNMADD213SS):  
        case XO(VFNMSUB213SS):
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 3);
             assert(INS_OperandWritten(ins, 0));
             REG reg1 = INS_OperandReg(ins, 0);
             REG reg2 = INS_OperandReg(ins, 1);
             INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_BOOL, true, IARG_END);
             break;   
        }
        case XO(VFMADD231SS):
        case XO(VFMSUB231SS):
        case XO(VFNMADD231SS):
        case XO(VFNMSUB231SS):
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 3);
             assert(INS_OperandWritten(ins, 0));
             //REG reg1 = INS_OperandReg(ins, 0);
             REG reg2 = INS_OperandReg(ins, 1);
             if(INS_OperandIsMemory(ins, 2))
             {
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, true, IARG_END);
             }
             else
             {
                 REG reg3 = INS_OperandReg(ins, 2);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, true, IARG_END);
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
             } 
             break;   
        }
        case XO(MULPS):
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 2);
             REG reg1 = INS_OperandReg(ins, 0);
             if(INS_OperandIsMemory(ins, 1))
             {
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounterp, IARG_REG_REFERENCE, reg1, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, true, IARG_END);
             }
             else
             {
                 REG reg2 = INS_OperandReg(ins, 1);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounterp, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_BOOL, true, IARG_END);
             }
             break;              
        }
        case XO(MULPD):
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 2);
             REG reg1 = INS_OperandReg(ins, 0);
             if(INS_OperandIsMemory(ins, 1))
             {
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounterp, IARG_REG_REFERENCE, reg1, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
             }
             else
             {
                 REG reg2 = INS_OperandReg(ins, 1);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounterp, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_BOOL, false, IARG_END);
             }
             break;              
        }
        case XO(VMULPS):
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 3);
             assert(INS_OperandWritten(ins, 0));
             REG reg2 = INS_OperandReg(ins, 1);
             if(INS_OperandIsMemory(ins, 2))
             {
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounterp, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, true, IARG_END);
             }
             else
             {
                 REG reg3 = INS_OperandReg(ins, 2);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounterp, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, true, IARG_END);
             }
             break;              
        } 
        case XO(VMULPD): 
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 3);
             assert(INS_OperandWritten(ins, 0));
             REG reg2 = INS_OperandReg(ins, 1);
             if(INS_OperandIsMemory(ins, 2))
             {
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounterp, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
             }
             else
             {
                 REG reg3 = INS_OperandReg(ins, 2);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounterp, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
             }
             break;              
        }                 
        case XO(VFMADD132PD):
        case XO(VFMADDSUB132PD):
        case XO(VFMSUBADD132PD):
        case XO(VFMSUB132PD):
        case XO(VFNMADD132PD):
        case XO(VFNMSUB132PD):
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 3);
             assert(INS_OperandWritten(ins, 0));
             REG reg1 = INS_OperandReg(ins, 0);
             //REG reg2 = INS_OperandReg(ins, 1);
             if(INS_OperandIsMemory(ins, 2))
             {
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounterp, IARG_REG_REFERENCE, reg1, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
             }
             else
             {
                 REG reg3 = INS_OperandReg(ins, 2);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounterp, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
             } 
             break;   
        }
        case XO(VFMADD231PD):
        case XO(VFMADDSUB231PD):
        case XO(VFMSUB231PD):
        case XO(VFMSUBADD231PD):
        case XO(VFNMADD231PD):
        case XO(VFNMSUB231PD):
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 3);
             assert(INS_OperandWritten(ins, 0));
             REG reg2 = INS_OperandReg(ins, 1);
             //REG reg2 = INS_OperandReg(ins, 1);
             if(INS_OperandIsMemory(ins, 2))
             {
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounterp, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
             }
             else
             {
                 REG reg3 = INS_OperandReg(ins, 2);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounterp, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
             } 
             break;   
        }
        case XO(VFMADD213PD):
        case XO(VFMADDSUB213PD):
        case XO(VFMSUB213PD):	
        case XO(VFMSUBADD213PD):	
        case XO(VFNMADD213PD):
        case XO(VFNMSUB213PD):
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 3);
             assert(INS_OperandWritten(ins, 0));
             REG reg1 = INS_OperandReg(ins, 0);
             REG reg2 = INS_OperandReg(ins, 1);
             INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounterp, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_BOOL, false, IARG_END);
             break;   
        }        
        case XO(VFMADD132PS):
        case XO(VFMADDSUB132PS):
        case XO(VFMSUB132PS):
        case XO(VFMSUBADD132PS):
        case XO(VFNMADD132PS):
        case XO(VFNMSUB132PS):
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 3);
             assert(INS_OperandWritten(ins, 0));
             REG reg1 = INS_OperandReg(ins, 0);
             //REG reg2 = INS_OperandReg(ins, 1);
             if(INS_OperandIsMemory(ins, 2))
             {
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounterp, IARG_REG_REFERENCE, reg1, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, true, IARG_END);
             }
             else
             {
                 REG reg3 = INS_OperandReg(ins, 2);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounterp, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg3, IARG_BOOL, true, IARG_END);
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
             } 
             break;   
        }
        case XO(VFMADD231PS):
        case XO(VFMADDSUB231PS):
        case XO(VFMSUB231PS):
        case XO(VFMSUBADD231PS):
        case XO(VFNMADD231PS):
        case XO(VFNMSUB231PS):
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 3);
             assert(INS_OperandWritten(ins, 0));
             REG reg2 = INS_OperandReg(ins, 1);
             //REG reg2 = INS_OperandReg(ins, 1);
             if(INS_OperandIsMemory(ins, 2))
             {
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounterp, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, true, IARG_END);
             }
             else
             {
                 REG reg3 = INS_OperandReg(ins, 2);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounterp, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, true, IARG_END);
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
             } 
             break;   
        }
        case XO(VFMADD213PS):	
        case XO(VFMADDSUB213PS):
        case XO(VFMSUB213PS):
        case XO(VFMSUBADD213PS):
        case XO(VFNMADD213PS):
        case XO(VFNMSUB213PS):
        {
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 3);
             assert(INS_OperandWritten(ins, 0));
             REG reg1 = INS_OperandReg(ins, 0);
             REG reg2 = INS_OperandReg(ins, 1);
             INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounterp, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_BOOL, false, IARG_END);
             break;   
        } 
        case XO(VFMADDPD):
        case XO(VFMSUBPD):
        case XO(VFMADDSUBPD):
        case XO(VFMSUBADDPD):
        case XO(VFNMADDPD):
        case XO(VFNMSUBPD):
        {
            //REF: https://www.amd.com/system/files/TechDocs/43479.pdf
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 4);
             assert(INS_OperandWritten(ins, 0));
             //REG reg1 = INS_OperandReg(ins, 0);
             REG reg2 = INS_OperandReg(ins, 1);
             if(INS_OperandIsMemory(ins, 2))
             {
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounterp, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
             }
             else
             {
                 REG reg3 = INS_OperandReg(ins, 2);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounterp, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
             } 
             break;   
        }
        case XO(VFMADDPS):
        case XO(VFMADDSUBPS):
        case XO(VFMSUBADDPS):
        case XO(VFNMADDPS):
        case XO(VFMSUBPS):
        case XO(VFNMSUBPS):
        {
            //REF: https://www.amd.com/system/files/TechDocs/43479.pdf
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 4);
             assert(INS_OperandWritten(ins, 0));
             //REG reg1 = INS_OperandReg(ins, 0);
             REG reg2 = INS_OperandReg(ins, 1);
             if(INS_OperandIsMemory(ins, 2))
             {
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounterp, IARG_REG_REFERENCE, reg2, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, true, IARG_END);
             }
             else
             {
                 REG reg3 = INS_OperandReg(ins, 2);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounterp, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, true, IARG_END);
                 //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounter2, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_REG_REFERENCE, reg3, IARG_BOOL, false, IARG_END);
             } 
             break;   
        }
        case XO(DPPS):
        {
             //Selective muls not taken into account
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 3);
             REG reg1 = INS_OperandReg(ins, 0);
             if(INS_OperandIsMemory(ins, 1))
             {
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounterp, IARG_REG_REFERENCE, reg1, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, true, IARG_END);
             }
             else
             {
                 REG reg2 = INS_OperandReg(ins, 1);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounterp, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_BOOL, true, IARG_END);
             }
             break;              
        } 
        case XO(DPPD):
        {
             //Selective muls not taken into account
             uint32_t numOperands = INS_OperandCount(ins);
             assert(numOperands == 2);
             REG reg1 = INS_OperandReg(ins, 0);
             if(INS_OperandIsMemory(ins, 1))
             {
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regmemcounterp, IARG_REG_REFERENCE, reg1, IARG_MEMORYREAD_EA , IARG_MEMORYREAD_SIZE, IARG_BOOL, false, IARG_END);
             }
             else
             {
                 REG reg2 = INS_OperandReg(ins, 1);
                 INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)regregcounterp, IARG_REG_REFERENCE, reg1, IARG_REG_REFERENCE, reg2, IARG_BOOL, false, IARG_END);
             }
             break;              
        }    
        default:
        {
            break;
        }
    }
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "fpmul.out", "specify output file name");

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)  
{
    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    OutFile << "#Floating Multiply Insns             : " << fpmulinsncount << endl;
    OutFile << "#Floating Multiply Insns (Mem Refs)  : " << mulregmemcount << endl;
    OutFile << "#Floating Multiply Insns (Only Regs) : " << mulregregcount << endl;
    OutFile << "#Floating Multiply Insns (Mem Refs) P: " << mulregmemcountp << endl;
    OutFile << "#Floating Multiply Insns (Only Regs)P: " << mulregregcountp << endl;
    OutFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool counts the number of floating point multiplication instructions and reports their distribution." << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    OutFile.open(KnobOutputFile.Value().c_str());

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
