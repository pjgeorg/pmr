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

#include "exchange.hpp"
#include <algorithm>
#include <sstream>
#include <vector>
extern "C" {
#include <omp.h>
}
#include "../../communicator.hpp"
#include "../../config.hpp"
#include "../../connection.hpp"
#include "../../misc/allocator.hpp"
#include "../../misc/random.hpp"
#include "../../misc/time.hpp"
#include "../../recvwindow.hpp"
#include "../../sendwindow.hpp"
#include "../common/parameter.hpp"
#include "mpi.hpp"
#include "usage.hpp"

void runExchange(int argc, char **argv)
{
    // Default options
    int dim = {1};
    std::vector<int> geom;
    std::vector<int> periodic;
    std::vector<int> active;
    std::vector<int> activeConnections;
    std::uint64_t maxIter = {10000};
    std::uint64_t overallSize = {1024 * 1024 * 1024};
    std::uint32_t minMsgSize = {0};
    std::uint32_t maxMsgSize = {128 * 1024};
    std::uint32_t deltaMsgSize = 100;
    std::uint32_t minDeltaMsgSize = {4};
    std::uint32_t maxDeltaMsgSize = {4 * 1024};
    bool bufferedSend = {false};
    bool bufferedRecv = {false};
    int threads = {1};
    bool verify = {false};

    // Get additional parameter options
    parameterOption(argv, argv + argc, "--dim", dim);
    geom.resize(dim, 0);
    periodic.resize(dim, 1);
    active.resize(dim, 1);
    parameterOption(argv, argv + argc, "--geom", geom);
    parameterOption(argv, argv + argc, "--periodic", periodic);
    parameterOption(argv, argv + argc, "--active", active);

    parameterOption<std::uint64_t>(argv, argv + argc, "--maxIter", maxIter);
    parameterOption<std::uint64_t>(
        argv, argv + argc, "--overallSize", overallSize);
    parameterOption<std::uint32_t>(
        argv, argv + argc, "--minMsgSize", minMsgSize);
    parameterOption<std::uint32_t>(
        argv, argv + argc, "--maxMsgSize", maxMsgSize);

    parameterOption<std::uint32_t>(
        argv, argv + argc, "--deltaMsgSize", deltaMsgSize);
    parameterOption<std::uint32_t>(
        argv, argv + argc, "--minDeltaMsgSize", minDeltaMsgSize);
    parameterOption<std::uint32_t>(
        argv, argv + argc, "--maxDeltaMsgSize", maxDeltaMsgSize);

    if(minDeltaMsgSize > maxDeltaMsgSize)
    {
        printUsage();
    }

    if(parameterExists(argv, argv + argc, "--bufferedSend"))
    {
        bufferedSend = {true};
    }
    if(parameterExists(argv, argv + argc, "--bufferedRecv"))
    {
        bufferedRecv = {true};
    }

    if(parameterExists(argv, argv + argc, "--threaded"))
    {
        if(pMR::cThreadLevel < pMR::ThreadLevel::Multiple)
        {
            throw std::runtime_error(
                "pMR has not been configured for threaded communication.");
        }
        threads = omp_get_max_threads();
    }

    if(parameterExists(argv, argv + argc, "--verify"))
    {
        verify = {true};
    }

    // Prepare benchmark environment
    pMR::Communicator communicator(MPI_COMM_WORLD, geom, periodic);

    // Establish connections
    std::vector<pMR::Connection> connections;

    for(auto n = decltype(communicator.dimensions()){0};
        n != communicator.dimensions(); ++n)
    {
        pMR::Target lNeighbor = communicator.getNeighbor(n, -1);
        pMR::Target rNeighbor = communicator.getNeighbor(n, +1);

        if(communicator.coordinate(n) % 2 == 1)
        {
            if(lNeighbor.isRemote())
            {
                connections.emplace_back(lNeighbor);
                activeConnections.emplace_back(active[n]);
            }
            if(rNeighbor.isRemote())
            {
                connections.emplace_back(rNeighbor);
                activeConnections.emplace_back(active[n]);
            }
        }
        else
        {
            if(rNeighbor.isRemote())
            {
                connections.emplace_back(rNeighbor);
                activeConnections.emplace_back(active[n]);
            }
            if(lNeighbor.isRemote())
            {
                connections.emplace(connections.end() - 1, lNeighbor);
                activeConnections.emplace(
                    activeConnections.end() - 1, active[n]);
            }
        }
    }

    if(connections.size() == 0)
    {
        finalize();
    }

    threads = std::min(threads, static_cast<int>(connections.size()));

    // Print Benchmark Information
    printMaster("Benchmark:    Exchange");
    printMaster("Processes:   ", communicator.size());
    printMaster("Dimensions:  ",
        [&connections, &periodic]() {
            auto dims = connections.size();
            for(auto v : periodic)
            {
                if(v == 0)
                {
                    ++dims;
                }
            }
            return static_cast<int>(dims / 2);
        }(),
        "/", communicator.dimensions());
    printMaster("Topology:    ", communicator.topology());
    printMaster("Periodic:    ", communicator.periodic());
    printMaster("Active:      ", active);
    printMaster("BufferedSend:", bufferedSend);
    printMaster("BufferedRecv:", bufferedRecv);
    printMaster("Threads:     ", threads);
    printMaster("Verify:      ", verify);
    printMaster();

    printMaster(" MPIRank Size[By] Iteration  Time/It[s]");

    // Loop Message Sizes
    auto msgSize = minMsgSize;
    while(msgSize <= maxMsgSize)
    {
        std::vector<std::vector<unsigned char, pMR::Allocator<unsigned char>>>
            vBuffers, sendBuffers, recvBuffers;
        std::vector<pMR::SendWindow<unsigned char>> sendWindows;
        std::vector<pMR::RecvWindow<unsigned char>> recvWindows;

        std::uint64_t activeWindows = 0;

        // Create Buffers and Windows
        for(decltype(connections.size()) c = 0; c != connections.size(); ++c)
        {

            auto const tmpSize = [&activeConnections, &activeWindows, c,
                msgSize] {
                if(activeConnections[c] == 0)
                {
                    return decltype(msgSize){0};
                }
                else
                {
                    ++activeWindows;
                    return msgSize;
                }
            }();

            if(verify)
            {
                vBuffers.emplace_back(tmpSize + pMR::cacheLineSize());
                sendBuffers.emplace_back(tmpSize + pMR::cacheLineSize());
                recvBuffers.emplace_back(tmpSize + pMR::cacheLineSize());

                std::generate_n(vBuffers.back().begin(), tmpSize,
                    pMR::getRandomNumber<std::uint8_t>);
                std::fill_n(vBuffers.back().rbegin(), pMR::cacheLineSize(), 0);
                std::copy(vBuffers.back().begin(), vBuffers.back().end(),
                    sendBuffers.back().begin());
                std::fill(
                    recvBuffers.back().begin(), recvBuffers.back().end(), 0);
            }
            else
            {
                sendBuffers.emplace_back(tmpSize);
                recvBuffers.emplace_back(tmpSize);
            }

            if(bufferedSend)
            {
                sendWindows.emplace_back(connections[c], tmpSize);
            }
            else
            {
                sendWindows.emplace_back(
                    connections[c], sendBuffers.back().data(), tmpSize);
            }

            if(bufferedRecv)
            {
                recvWindows.emplace_back(connections[c], tmpSize);
            }
            else
            {
                recvWindows.emplace_back(
                    connections[c], recvBuffers.back().data(), tmpSize);
            }
        }

        // Only zero sized messages if no active windows
        if(activeWindows == 0)
        {
            msgSize = 0;
            maxMsgSize = 0;
        }

        // Calculate iterations count
        auto iter = maxIter;
        if(msgSize)
        {
            iter = std::min(iter, overallSize / msgSize / activeWindows);
        }

        auto size = sendWindows.size();
        auto s = decltype(size){0};

        double time = -pMR::getTimeInSeconds();
#pragma omp parallel num_threads(threads)
        for(auto i = decltype(iter){0}; i != iter; ++i)
        {
#pragma omp for
            for(s = 0; s < size; ++s)
            {
                recvWindows[s].init();
                sendWindows[s].init();
            }

            if(bufferedSend)
            {
#pragma omp for
                for(s = 0; s < size; ++s)
                {
                    sendWindows[s].insert(sendBuffers[s].cbegin());
                }
            }

#pragma omp for
            for(s = 0; s < size; ++s)
            {
                sendWindows[s].post();
                recvWindows[s].post();
            }

#pragma omp for
            for(s = 0; s < size; ++s)
            {
                sendWindows[s].wait();
                recvWindows[s].wait();
            }

            if(bufferedRecv)
            {
#pragma omp for
                for(s = 0; s < size; ++s)
                {
                    recvWindows[s].extract(recvBuffers[s].begin());
                }
            }

            if(verify)
            {
                if(i % 2 == 1)
                {
#pragma omp for
                    for(s = 0; s < size; ++s)
                    {
                        if(recvBuffers[s] != vBuffers[s])
                        {
                            throw std::runtime_error(
                                "Verification check failed!");
                        }
                    }
                }

#pragma omp for
                for(s = 0; s < size; ++s)
                {
                    std::copy(recvBuffers[s].begin(), recvBuffers[s].end(),
                        sendBuffers[s].begin());
                    std::fill(recvBuffers[s].begin(), recvBuffers[s].end(), 0);
                }
            }
        }
        time += pMR::getTimeInSeconds();

        std::ostringstream oss;
        oss << std::setw(8) << getRank(MPI_COMM_WORLD) << " " << std::setw(8)
            << msgSize << " " << std::setw(8) << iter << " " << std::scientific
            << time / iter << std::endl;
        std::cout << oss.str();

        // Increment msgSize for next loop
        auto tmpSize = (msgSize * (100 + deltaMsgSize)) / 100;
        tmpSize -= tmpSize % minDeltaMsgSize;
        if(tmpSize - msgSize < minDeltaMsgSize)
        {
            tmpSize = msgSize + minDeltaMsgSize;
        }
        if(tmpSize - msgSize > maxDeltaMsgSize)
        {
            tmpSize = msgSize + maxDeltaMsgSize;
        }
        msgSize = tmpSize;
    }

    finalize();
}
