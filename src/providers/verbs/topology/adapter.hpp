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

#ifndef pMR_PROVIDERS_VERBS_TOPOLOGY_ADAPTER_H
#define pMR_PROVIDERS_VERBS_TOPOLOGY_ADAPTER_H

extern "C" {
#include <infiniband/verbs.h>
}
#include "../device.hpp"

namespace pMR
{
    namespace Verbs
    {
        int getAdapter(Devices const &, ibv_node_type const nodeType,
            ibv_transport_type const transportType, int deviceNumber = 1);

        Device getIBAdapter(int const deviceNumber = 1);
        Device getIBAdapter(Devices const &, int const deviceNumber = 1);
    }
}
#endif // pMR_PROVIDERS_VERBS_TOPOLOGY_ADAPTER_H
