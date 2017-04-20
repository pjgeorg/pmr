//  Copyright 2016 Peter Georg
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <cstdlib>
#include "../misc/print.hpp"
#include "allreduce.hpp"
#include "exchange.hpp"
#include "info.hpp"
#include "mpi.hpp"
#include "parameter.hpp"
#include "usage.hpp"

int main(int argc, char **argv)
{
    try
    {
        init(argc, argv);

        if(parameterExists(argv, argv + argc, "--usage") or
            parameterExists(argv, argv + argc, "--help"))
        {
            printUsage();
        }

        if(parameterExists(argv, argv + argc, "--benchmark"))
        {
            std::string benchmark;
            parameterOption<std::string>(
                argv, argv + argc, "--benchmark", benchmark);
            if(benchmark == "exchange")
            {
                runExchange(argc, argv);
            }
            if(benchmark == "allreduce")
            {
                runAllReduce(argc, argv);
            }
            printUsage();
        }

        printInfo(argc, argv);
        finalize();
    }
    catch(const std::exception &e)
    {
        pMR::print(e.what());
        abort(1);
    }
}
