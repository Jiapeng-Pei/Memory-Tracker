/*
 * Copyright 2002-2020 Intel Corporation.
 * 
 * This software is provided to you as Sample Source Code as defined in the accompanying
 * End User License Agreement for the Intel(R) Software Development Products ("Agreement")
 * section 1.L.
 * 
 * This software and the related documents are provided as is, with no express or implied
 * warranties, other than those that are expressly stated in the License.
 */

/* This tool shows how to examine only RTNs that are actually executed */

#include <stdio.h>
#include <string>
#include <iostream>
#include <inttypes.h>
#include <fstream>
#include "pin.H"

using namespace::std;

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

int mallocCount = 0;
int freeCount = 0;
std::ofstream TraceFile;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "codyPei.out", "specify trace file name");

/* ===================================================================== */
/* Analysis routines                                                     */
/* ===================================================================== */
 

VOID FuncBefore(const CHAR * name, ADDRINT size)
{
    TraceFile << name << "(" << size << ")" << endl;
}

VOID MallocAfter(ADDRINT ret)
{
    TraceFile << "  returns " << ret << endl;
}


/* ===================================================================== */
/* Instrumentation routines                                              */
/* ===================================================================== */

static VOID Trace(TRACE trace, VOID *v)
{
    INS ins = BBL_InsHead(TRACE_BblHead(trace));
    RTN rtn = INS_Rtn(ins);
    
    if (!RTN_Valid(rtn))
    {
        return;
    }

    if (INS_Address(ins) == RTN_Address(rtn)) 
    {
        /* The first ins of an RTN that will be executed - it is possible at this point to examine all the INSs 
           of the RTN that Pin can statically identify (using whatever standard symbol information is available).
           A tool may wish to parse each such RTN only once, if so it will need to record and identify which RTNs 
           have already been parsed
        */
      
        auto* malloc_ptr = strstr(RTN_Name(rtn).c_str(), "malloc");
        auto* free_ptr = strstr(RTN_Name(rtn).c_str(), "free");
        if (malloc_ptr != nullptr) {
            // Find malloc.
            mallocCount++;
            FuncBefore(RTN_Name(rtn).c_str(), IARG_FUNCARG_ENTRYPOINT_VALUE);
            MallocAfter(IARG_FUNCRET_EXITPOINT_VALUE);
            // RTN_Open(rtn);

            // // Instrument malloc() to print the input argument value and the return value.
            // RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)Arg1Before,
            //                 IARG_ADDRINT, "malloc",
            //                 IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
            //                 IARG_END);
            // RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)MallocAfter,
            //                 IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

            // RTN_Close(rtn);
        }

        if (free_ptr != nullptr) {
            // Find free.
            freeCount++;
            FuncBefore(RTN_Name(rtn).c_str(), IARG_FUNCARG_ENTRYPOINT_VALUE);
            // RTN_Open(rtn);
            // // Instrument free() to print the input argument value.
            // RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)Arg1Before,
            //                 IARG_ADDRINT, "free",
            //                 IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
            //                 IARG_END);
            // RTN_Close(rtn);
        }


        // Get INS inside a rtn. We don't need to use the section.
        bool checkDetails = false;
        if (checkDetails) {
            RTN_Open(rtn);
            for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
            {
                printf ("%p %s\n", reinterpret_cast<void *>(INS_Address(ins)), INS_Disassemble(ins).c_str());
            }
            RTN_Close(rtn);
        }
    }

}

/* ===================================================================== */

VOID Fini (INT32 code, VOID *v)
{
    ASSERTX( mallocCount + freeCount != 0);
    TraceFile.close();
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    TraceFile.open(KnobOutputFile.Value().c_str());
    TraceFile << hex;
    TraceFile.setf(ios::showbase);

    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
