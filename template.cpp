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

/*! @file
 * This file exemplifies XED usage on IA-32 and Intel(R) 64 architectures.
 */

#include "pin.H"
extern "C" {
#include "xed-interface.h"
}
#include <iostream>
#include <iomanip>
#include <fstream>

//#include <cassert>
//#include "../Utils/regvalue_utils.h"
using std::string;
using std::stringstream;

using std::ofstream;

using std::hex;
using std::cerr;
using std::endl;

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */
std::ofstream* out = 0;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */


/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool prints IA-32 and Intel(R) 64 instructions"
         << endl;
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

/* ===================================================================== */


string Val2Str(const void* value, unsigned int size)
{
    stringstream sstr;
    sstr << hex;
    const unsigned char* cval = (const unsigned char*)value;
    // Traverse cval from end to beginning since the MSB is in the last block of cval.
    while (size)
    {
        --size;
        sstr << (unsigned int)cval[size];
    }
    return string("0x")+sstr.str();
}


VOID use_xed(ADDRINT pc, const CONTEXT * ctxt) {
#if defined(TARGET_IA32E)
    static const xed_state_t dstate = {XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b};
#else
    static const xed_state_t dstate = { XED_MACHINE_MODE_LEGACY_32, XED_ADDRESS_WIDTH_32b};
#endif
    xed_decoded_inst_t xedd;
    xed_decoded_inst_zero_set_mode(&xedd,&dstate);

    //Pass in the proper length: 15 is the max. But if you do not want to
    //cross pages, you can pass less than 15 bytes, of course, the
    //instruction might not decode if not enough bytes are provided.
    const unsigned int max_inst_len = 15;

    xed_error_enum_t xed_code = xed_decode(&xedd, reinterpret_cast<UINT8*>(pc), max_inst_len);
    BOOL xed_ok = (xed_code == XED_ERROR_NONE);
    if (xed_ok) {
        *out << hex << std::setw(8) << pc << " ";
        char buf[2048];

        // set the runtime adddress for disassembly
        xed_uint64_t runtime_address = static_cast<xed_uint64_t>(pc);

        xed_format_context(XED_SYNTAX_INTEL, &xedd,
                           buf, 2048, runtime_address, 0, 0);
        *out << buf << endl;
    }
    static UINT stRegSize = REG_Size(REG_ST_BASE);
    for (int reg = (int)REG_GR_BASE; reg <= (int)REG_GR_LAST; ++reg)
    {
        // For the integer registers, it is safe to use ADDRINT. But make sure to pass a pointer to it.
        ADDRINT val;
        PIN_GetContextRegval(ctxt, (REG)reg, reinterpret_cast<UINT8*>(&val));
        *out << REG_StringShort((REG)reg) << ": 0x" << hex << val << endl;
    }
    for (int reg = (int)REG_ST_BASE; reg <= (int)REG_ST_LAST; ++reg)
    {
        // For the x87 FPU stack registers, using PIN_REGISTER ensures a large enough buffer.
        PIN_REGISTER val;
        PIN_GetContextRegval(ctxt, (REG)reg, reinterpret_cast<UINT8*>(&val));
        *out << REG_StringShort((REG)reg) << ": " << Val2Str(&val, stRegSize) << endl;
    }

    stRegSize = REG_Size(REG_XMM_BASE);
    for (int reg = (int)REG_XMM_BASE; reg <= (int)REG_XMM_LAST; ++reg)
    {
        //XMM Registers
        PIN_REGISTER val;
        PIN_GetContextRegval(ctxt, (REG)reg,  reinterpret_cast<UINT8*>(&val));
        *out << REG_StringShort((REG)reg) << ": " << Val2Str(&val, stRegSize) << endl;
    }

}

/* ===================================================================== */

VOID Instruction(INS ins, VOID *v)
{
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)use_xed,
                   IARG_INST_PTR, IARG_CONST_CONTEXT,
                   IARG_END);
}

/* ===================================================================== */


/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    out = new std::ofstream("xed-print.out");
    if( PIN_Init(argc,argv) )
        return Usage();
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_StartProgram();    // Never returns
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
