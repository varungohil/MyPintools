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
static UINT64 fpmovcount = 0;
static UINT64 intraregcount = 0;
static UINT64 regmemcount = 0;
static UINT64 loadcount = 0;
static UINT64 storecount = 0;
static UINT64 spcount = 0;
static UINT64 dpcount = 0;
static UINT64 dploadcount = 0;
static UINT64 sploadcount = 0;
static UINT64 dpstorecount = 0;
static UINT64 spstorecount = 0;
static UINT64 spintraregcount = 0;
static UINT64 dpintraregcount = 0;

// This function is called before every instruction is executed
VOID loadcounter(bool isSP, UINT32 val)
{
    fpmovcount++;
    regmemcount++;
    loadcount++;
    if(isSP)
    {
        spcount += val;
        sploadcount += val;
    }
    else
    {
        dpcount += val;
        dploadcount += val;
    }
    
}

VOID storecounter(bool isSP, UINT32 val)
{
    fpmovcount++;
    regmemcount++;
    storecount++;
    if(isSP)
    {
        spcount += val;
        spstorecount += val;
    }
    else
    {
        dpcount += val;
        dpstorecount += val;
    }
    
}

VOID intraregcounter(bool isSP, UINT32 val)
{
    fpmovcount++;
    intraregcount++;
    if(isSP)
    {
        spcount += val;
        spintraregcount += val;
    }
    else
    {
        dpcount += val;
        dpintraregcount += val;
    }
    
}


// VOID regmem(INS ins, bool isSP, UINT32 val)
// {
//     regmemcount++;
//     fpmovcount++;
// }

// VOID regreg(INS ins)
// {
//     regregcount++;
//     fpmovcount++;
// }

// VOID dpcounter(INS ins, UINT32 val)
// {
//     dpcount = dpcount + val;
// }

// VOID spcounter(INS ins, UINT32 val)
// {
//     spcount = spcount + val;
// }


// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
    switch (INS_Opcode(ins))
    {
        case XO(MOVAPD):
        case XO(MOVNTPD):
        case XO(MOVUPD):
        case XO(VMASKMOVPD):
        case XO(VMOVAPD):
        case XO(VMOVNTPD):
        case XO(VMOVUPD):
        {
            //2 DP
            uint32_t numOperands = INS_OperandCount(ins);
            assert(numOperands == 2);
            bool isStore = false;
            bool isLoad = false;
            for (uint32_t op = 0; op < numOperands; op++)
            {
                if (INS_OperandWritten(ins, op) && INS_OperandIsMemory(ins, op))
                {
                    isStore = true;
                }
                if (INS_OperandRead(ins, op) && INS_OperandIsMemory(ins, op))
                {
                    isLoad = true;
                }
            }
            UINT32 val = 2;
            if(isLoad)
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)loadcounter, IARG_BOOL, false, IARG_UINT32, val, IARG_END);
            }
            else if(isStore)
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)storecounter, IARG_BOOL, false, IARG_UINT32, val, IARG_END);
            } 
            else
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)intraregcounter, IARG_BOOL, false, IARG_UINT32, val, IARG_END);
            }
            break;
        }
        case XO(MOVAPS):
        case XO(MOVNTPS):
        case XO(MOVUPS):
        case XO(VMASKMOVPS):
        case XO(VMOVAPS):
        case XO(VMOVNTPS):
        case XO(VMOVUPS):
        {
            //4 SP
            uint32_t numOperands = INS_OperandCount(ins);
            assert(numOperands == 2);
            bool isStore = false;
            bool isLoad = false;
            for (uint32_t op = 0; op < numOperands; op++)
            {
                if (INS_OperandWritten(ins, op) && INS_OperandIsMemory(ins, op))
                {
                    isStore = true;
                }
                if (INS_OperandRead(ins, op) && INS_OperandIsMemory(ins, op))
                {
                    isLoad = true;
                }
            }
            UINT32 val = 2;
            if(isLoad)
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)loadcounter, IARG_BOOL, true, IARG_UINT32, val, IARG_END);
            }
            else if(isStore)
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)storecounter, IARG_BOOL, true, IARG_UINT32, val, IARG_END);
            } 
            else
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)intraregcounter, IARG_BOOL, true, IARG_UINT32, val, IARG_END);
            }
            break;
        }
        case XO(MOVHPD):
        case XO(MOVLPD):
        case XO(MOVNTSD):
        case XO(MOVSD):
        case XO(MOVSD_XMM):
        case XO(VMOVDDUP):
        case XO(VBROADCASTSD):
        {
            //1 DP
            uint32_t numOperands = INS_OperandCount(ins);
            assert(numOperands == 2);
            bool isStore = false;
            bool isLoad = false;
            for (uint32_t op = 0; op < numOperands; op++)
            {
                if (INS_OperandWritten(ins, op) && INS_OperandIsMemory(ins, op))
                {
                    isStore = true;
                }
                if (INS_OperandRead(ins, op) && INS_OperandIsMemory(ins, op))
                {
                    isLoad = true;
                }
            }
            UINT32 val = 1;
            if(isLoad)
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)loadcounter, IARG_BOOL, false, IARG_UINT32, val, IARG_END);
            }
            else if(isStore)
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)storecounter, IARG_BOOL, false, IARG_UINT32, val, IARG_END);
            } 
            else
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)intraregcounter, IARG_BOOL, false, IARG_UINT32, val, IARG_END);
            }
            break;
        }
        case XO(MOVHPS):
        case XO(MOVLHPS):
        case XO(MOVLPS):
        case XO(MOVSHDUP):
        case XO(MOVSLDUP):
        case XO(VMOVSHDUP):
        case XO(VMOVSLDUP):
        {
            //2 SP
            uint32_t numOperands = INS_OperandCount(ins);
            assert(numOperands == 2);
            bool isStore = false;
            bool isLoad = false;
            for (uint32_t op = 0; op < numOperands; op++)
            {
                if (INS_OperandWritten(ins, op) && INS_OperandIsMemory(ins, op))
                {
                    isStore = true;
                }
                if (INS_OperandRead(ins, op) && INS_OperandIsMemory(ins, op))
                {
                    isLoad = true;
                }
            }
            UINT32 val = 2;
            if(isLoad)
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)loadcounter, IARG_BOOL, true, IARG_UINT32, val, IARG_END);
            }
            else if(isStore)
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)storecounter, IARG_BOOL, true, IARG_UINT32, val, IARG_END);
            } 
            else
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)intraregcounter, IARG_BOOL, true, IARG_UINT32, val, IARG_END);
            }
            break;
        }
        case XO(MOVNTSS):
        case XO(MOVSS):
        case XO(VBROADCASTSS):
        {
           //1 SP
            uint32_t numOperands = INS_OperandCount(ins);
            assert(numOperands == 2);
            bool isStore = false;
            bool isLoad = false;
            for (uint32_t op = 0; op < numOperands; op++)
            {
                if (INS_OperandWritten(ins, op) && INS_OperandIsMemory(ins, op))
                {
                    isStore = true;
                }
                if (INS_OperandRead(ins, op) && INS_OperandIsMemory(ins, op))
                {
                    isLoad = true;
                }
            }
            UINT32 val = 2;
            if(isLoad)
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)loadcounter, IARG_BOOL, true, IARG_UINT32, val, IARG_END);
            }
            else if(isStore)
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)storecounter, IARG_BOOL, true, IARG_UINT32, val, IARG_END);
            } 
            else
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)intraregcounter, IARG_BOOL, true, IARG_UINT32, val, IARG_END);
            }
            break;
        }
        case XO(FSTP):
        case XO(FST):
        {
            uint32_t numOperands = INS_OperandCount(ins);
            assert(numOperands == 1);
            UINT32 val = 1;
            if (INS_OperandIsMemory(ins, 0))
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)storecounter, IARG_BOOL, true, IARG_UINT32, val, IARG_END);
            }
            else
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)intraregcounter, IARG_BOOL, true, IARG_UINT32, val, IARG_END);
            }
            break;
        }
        // case XO(FSD):
        // {
        //     uint32_t numOperands = INS_OperandCount(ins);
        //     assert(numOperands == 1);
        //     UINT32 val = 1;
        //     if (INS_OperandIsMemory(ins, 0))
        //     {
        //         INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)loadcounter, IARG_BOOL, false, IARG_UINT32, val, IARG_END);
        //     }
        //     else
        //     {
        //         INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)intraregcounter, IARG_BOOL, false, IARG_UINT32, val, IARG_END);
        //     }
        //     break;            
        // }
        case XO(FCMOVB):
        case XO(FCMOVBE):
        case XO(FCMOVE):
        case XO(FCMOVNB):
        case XO(FCMOVNBE):
        case XO(FCMOVNE):
        case XO(FCMOVNU):
        case XO(FCMOVU):
        {
            //1 DP
            UINT32 val = 1;
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)intraregcounter, IARG_BOOL, false, IARG_UINT32, val, IARG_END);  
            break; 
        }
        case XO(VMOVHLPS):
        case XO(VMOVLHPS):
        {
            //3 operand -> intrareg -> 4 SP
            uint32_t numOperands = INS_OperandCount(ins);
            assert(numOperands == 3);
            UINT32 val = 2;
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)intraregcounter, IARG_BOOL, true, IARG_UINT32, val, IARG_END);
            break;
        }
        case XO(VMOVHPD):
        case XO(VMOVLPD):
        {
            //3 operand
            //Load : 2 DP
            //Store : 1 DP
            uint32_t numOperands = INS_OperandCount(ins);
            UINT32 val;
            if(numOperands == 3)
            {
                //isLoad = true;
                val = 2;
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)loadcounter, IARG_BOOL, false, IARG_UINT32, val, IARG_END);
            }
            else
            {
                //isStore = true;
                val = 1;
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)storecounter, IARG_BOOL, false, IARG_UINT32, val, IARG_END);
            }
            break;
        }
        case XO(VMOVHPS):
        case XO(VMOVLPS):
        {
            //3 operand
            //Load : 4 SP
            //Store: 2 SP
            uint32_t numOperands = INS_OperandCount(ins);
            UINT32 val;
            if(numOperands == 3)
            {
                //isLoad = true;
                val = 4;
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)loadcounter, IARG_BOOL, true, IARG_UINT32, val, IARG_END);
            }
            else
            {
                //isStore = true;
                val = 2;
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)storecounter, IARG_BOOL, true, IARG_UINT32, val, IARG_END);
            }
            break;
        }
        case XO(VMOVSD):
        {
            //3 operands -> intra reg -> 2 DP
            //2 ops -> 1 DP
            uint32_t numOperands = INS_OperandCount(ins);
            UINT32 val;
            if(numOperands == 3)
            {
                //is intra reg
                val = 2;
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)intraregcounter, IARG_BOOL, false, IARG_UINT32, val, IARG_END);
            }
            else if(numOperands == 2)
            {
                bool isStore = false;
                bool isLoad = false;
                for (uint32_t op = 0; op < numOperands; op++)
                {
                    if (INS_OperandWritten(ins, op) && INS_OperandIsMemory(ins, op))
                    {
                        isStore = true;
                    }
                    if (INS_OperandRead(ins, op) && INS_OperandIsMemory(ins, op))
                    {
                        isLoad = true;
                    }
                }
                val = 1;
                if(isLoad)
                {
                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)loadcounter, IARG_BOOL, false, IARG_UINT32, val, IARG_END);
                }
                else if(isStore)
                {
                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)storecounter, IARG_BOOL, false, IARG_UINT32, val, IARG_END);
                }  
            }
            break;
        }
        case XO(VMOVSS):
        {
            //3 operands -> intra reg -> 2 SP
            //2 ops -> 1 SP
            uint32_t numOperands = INS_OperandCount(ins);
            UINT32 val;
            if(numOperands == 3)
            {
                //is intra reg
                val = 2;
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)intraregcounter, IARG_BOOL, true, IARG_UINT32, val, IARG_END);
            }
            else if(numOperands == 2)
            {
                bool isStore = false;
                bool isLoad = false;
                for (uint32_t op = 0; op < numOperands; op++)
                {
                    if (INS_OperandWritten(ins, op) && INS_OperandIsMemory(ins, op))
                    {
                        isStore = true;
                    }
                    if (INS_OperandRead(ins, op) && INS_OperandIsMemory(ins, op))
                    {
                        isLoad = true;
                    }
                }
                val = 1;
                if(isLoad)
                {
                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)loadcounter, IARG_BOOL, true, IARG_UINT32, val, IARG_END);
                }
                else if(isStore)
                {
                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)storecounter, IARG_BOOL, true, IARG_UINT32, val, IARG_END);
                }  
            }
            break;
        }     
        default:
            break;
    }
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "fpmov.out", "specify output file name");

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)  
{
    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    OutFile << "FPMOV Count      : " << fpmovcount << endl;
    OutFile << "Intra. Reg Count : " << intraregcount << endl;
    OutFile << "RegMem Count     : " << regmemcount << endl;
    OutFile << "Load Count       : " << loadcount << endl;
    OutFile << "Store Count      : " << storecount << endl;
    OutFile << "SP Count         : " << spcount << endl;
    OutFile << "DP Count         : " << dpcount << endl;
    OutFile << "SP Load Count    : " << sploadcount << endl;
    OutFile << "DP Load Count    : " << dploadcount << endl;
    OutFile << "SP Store Count   : " << spstorecount << endl;
    OutFile << "DP Store Count   : " << dpstorecount << endl;
    OutFile << "SP IntraReg Count: " << spintraregcount << endl;
    OutFile << "DP IntraReg Count: " << dpintraregcount << endl;
    OutFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool counts the number of floating point MOV instructions and reports their distribution." << endl;
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

