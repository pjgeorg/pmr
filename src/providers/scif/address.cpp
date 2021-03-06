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

#include "address.hpp"
#include "nodeid.hpp"

pMR::SCIF::Address::Address(std::uint16_t port) : Address({getNodeID()}, {port})
{
}

pMR::SCIF::Address::Address(std::uint16_t node, std::uint16_t port)
{
    mPortID.node = {node};
    mPortID.port = {port};
}

scif_portID *pMR::SCIF::Address::get()
{
    return &mPortID;
}

scif_portID const *pMR::SCIF::Address::get() const
{
    return &mPortID;
}

std::uint16_t pMR::SCIF::Address::getNode() const
{
    return {mPortID.node};
}

std::uint16_t pMR::SCIF::Address::getPort() const
{
    return {mPortID.port};
}

void pMR::SCIF::Address::setNode(std::uint16_t const node)
{
    mPortID.node = {node};
}

void pMR::SCIF::Address::setPort(std::uint16_t const port)
{
    mPortID.port = {port};
}

bool pMR::SCIF::Address::operator==(Address const &other) const
{
    if((mPortID.node == other.mPortID.node) &&
        (mPortID.port == other.mPortID.port))
    {
        return {true};
    }
    else
    {
        return {false};
    }
}

bool pMR::SCIF::Address::operator!=(Address const &other) const
{
    return {!operator==(other)};
}
