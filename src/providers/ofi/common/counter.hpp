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

#ifndef pMR_PROVIDERS_OFI_COMMON_COUNTER_H
#define pMR_PROVIDERS_OFI_COMMON_COUNTER_H

extern "C" {
#include <rdma/fi_domain.h>
}
#include "domain.hpp"

namespace pMR
{
    namespace OFI
    {
        class Counter
        {
        public:
            Counter(Domain &domain);
            Counter(Counter const &) = delete;
            Counter(Counter &&) = delete;
            Counter &operator=(Counter const &) = delete;
            Counter &operator=(Counter &&) = delete;
            ~Counter();
            fid_cntr *get();
            fid_cntr const *get() const;
            void poll();

        private:
            fid_cntr *mCounter = nullptr;
            fi_context mContext = {};
        };
    }
}
#endif // pMR_PROVIDERS_OFI_COMMON_COUNTER_H
