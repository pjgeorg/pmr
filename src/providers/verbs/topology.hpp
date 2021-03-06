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

#ifndef pMR_PROVIDERS_VERBS_TOPOLOGY_H
#define pMR_PROVIDERS_VERBS_TOPOLOGY_H

#include <cstdint>
#include <vector>
extern "C" {
#include <infiniband/verbs.h>
}
#include "context.hpp"
#include "device.hpp"
#include "deviceattributes.hpp"
#include "gid.hpp"
#include "portattributes.hpp"

namespace pMR
{
    namespace Verbs
    {
        Device getIBAdapter(int const deviceNumber = 1);
        Device getIBAdapter(Devices const &, int const deviceNumber = 1);

        std::uint64_t getNodeGUID(Device const &);
        std::uint64_t getNodeGUID(Context &);

        std::uint8_t getPortCount(Device const &);
        std::uint8_t getPortCount(Context &);

        std::uint16_t getLID(Device const &, std::uint8_t const portNumber);
        std::uint16_t getLID(Context &, std::uint8_t const portNumber);

        std::uint64_t getSubnetPrefix(
            Device const &, std::uint8_t const portNumber);
        std::uint64_t getSubnetPrefix(Context &, std::uint8_t const portNumber);

        std::uint16_t getSwitchLID(
            Device const &, std::uint8_t const portNumber);
        std::uint16_t getSwitchLID(Context &, std::uint8_t const portNumber);
    }
}
#endif // pMR_PROVIDERS_VERBS_TOPOLOGY_H
