/*
 * Copyright 2023 CAS—Atlantic (University of New Brunswick, CASA)
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __YYOSYS_H__
#define __YYOSYS_H__

#include <string>
#include <stdlib.h>
#include <sys/types.h> // pid_t

#include "odin_globals.h" // global_args

/* to set local environment variable */
#ifdef WIN32
#    define set_env _setenv
#else
#    define set_env setenv
#endif

/**
 * @brief A class to provide the general object of Yosys synthezier
 */
class YYosys {
  public:
    /**
     * @brief Construct the object
     * required by compiler
     */
    YYosys();
    /**
     * @brief Destruct the object
     * to avoid memory leakage
     */
    ~YYosys();

    /**
     * ---------------------------------------------------------------------------------------------
     * (function: perform_elaboration)
     * 
     * @brief Perform Yosys elaboration and set the Odin-II input BLIF
     * file with the Yosys generated output BLIF file
     * -------------------------------------------------------------------------------------------*/
    void perform_elaboration();

  protected:
    std::string log_file;          // a log file including Yosys output logs
    std::string coarse_grain_blif; // Yosys coarse-grain output BLIF

  private:
    pid_t yosys_pid;                           // the fork pid created to run Yosys
    std::string odin_techlib;                  // the path od Odin-II techlib
    std::ostream* yosys_log_stream;            // Yosys log output stream
    std::string vtr_primitives_file;           // the path of VTR primitives Verilog file
    std::vector<std::string> verilog_circuits; // Odin-II input Verilog files

    /**
     * ---------------------------------------------------------------------------------------------
     * (function: load_target_dsp_blocks)
     * 
     * @brief this routine generates a Verilog file, including the 
     * declaration of all DSP blocks available in the targer architecture.
     * Then, the Verilog fle is read by Yosys to make the modules reachable.
     * -------------------------------------------------------------------------------------------*/
    void load_target_dsp_blocks();
    /**
     * ---------------------------------------------------------------------------------------------
     * (function: set_default_variables)
     * 
     * @brief Initialize Yosys
     * -------------------------------------------------------------------------------------------*/
    void init_yosys();
    /**
     * ---------------------------------------------------------------------------------------------
     * (function: set_default_variables)
     * 
     * @brief set Yosys+Odin default variables
     * -------------------------------------------------------------------------------------------*/
    void set_default_variables();
    /**
     * ---------------------------------------------------------------------------------------------
     * (function: execute)
     * 
     * @brief Executing Yosys using its API
     * -------------------------------------------------------------------------------------------*/
    void execute();
    /**
     * ---------------------------------------------------------------------------------------------
     * (function: elaborate)
     * 
     * @brief Perform Yosys elaboration and set the Odin-II input BLIF
     * file with the Yosys generated output BLIF file
     * -------------------------------------------------------------------------------------------*/
    void elaborate();
    /**
     * ---------------------------------------------------------------------------------------------
     * (function: output_blif)
     * 
     * @brief Ask Yosys to generate an output BLIF file in 
     * the path specified in this->coarse_grain_blif
     * -------------------------------------------------------------------------------------------*/
    void output_blif();
    /**
     * ---------------------------------------------------------------------------------------------
     * (function: re_initialize_odin_globals)
     * 
     * @brief Modify Odin-II global variables to activate the techmap flow
     * -------------------------------------------------------------------------------------------*/
    void re_initialize_odin_globals();
};

#endif //__YYOSYS_H__
