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

#include "memoryregion.hpp"
#include <stdexcept>

#ifdef VERBS_ODP
#include "odp.hpp"
#endif // VERBS_ODP

pMR::Verbs::MemoryRegion::MemoryRegion(Context &context,
    ProtectionDomain &protectionDomain, void *buffer, std::uint32_t size,
    int access)
{
#ifdef VERBS_ODP
    access = updateMemoryRegionAccessODP(context, access);
#endif // VERBS_ODP

    if(size > context.getMaxMemoryRegionSize() ||
        size > context.getMaxMessageSize())
    {
        throw std::length_error("pMR: Message size overflow.");
    }

    registerMemoryRegion(protectionDomain, buffer, size, access);
}

pMR::Verbs::MemoryRegion::~MemoryRegion()
{
    if(getLength() > 0)
    {
        ibv_dereg_mr(mMemoryRegion);
    }
    else
    {
        delete mMemoryRegion;
    }
}

ibv_mr *pMR::Verbs::MemoryRegion::get()
{
    return mMemoryRegion;
}

ibv_mr const *pMR::Verbs::MemoryRegion::get() const
{
    return mMemoryRegion;
}

std::uint64_t pMR::Verbs::MemoryRegion::getAddress() const
{
    return {reinterpret_cast<std::uintptr_t>(mMemoryRegion->addr)};
}

std::uint32_t pMR::Verbs::MemoryRegion::getLKey() const
{
    return {mMemoryRegion->lkey};
}

std::uint32_t pMR::Verbs::MemoryRegion::getRKey() const
{
    return {mMemoryRegion->rkey};
}

std::uint32_t pMR::Verbs::MemoryRegion::getLength() const
{
    return {static_cast<std::uint32_t>(mMemoryRegion->length)};
}

void pMR::Verbs::MemoryRegion::registerMemoryRegion(
    ProtectionDomain &protectionDomain, void *buffer, std::uint32_t const size,
    int const access)
{
    if(size > 0)
    {
        mMemoryRegion =
            ibv_reg_mr(protectionDomain.get(), buffer, size, access);
        if(!mMemoryRegion)
        {
            throw std::runtime_error("pMR: Could not register Memory Region.");
        }
    }
    else
    {
        mMemoryRegion = new ibv_mr;
        mMemoryRegion->addr = buffer;
        mMemoryRegion->lkey = 0;
        mMemoryRegion->rkey = 0;
        mMemoryRegion->length = 0;
    }
}
