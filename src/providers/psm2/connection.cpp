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

#include "connection.hpp"
#include <array>
#include "../../backends/backend.hpp"
#include "../../misc/singleton.hpp"

pMR::PSM2::Connection::Connection(Target const &target)
    : mEndpoint(&Singleton<GlobalEndpoint>::Instance())
    , mSendTag{reinterpret_cast<std::uintptr_t>(this)}
{
    std::array<std::uint64_t, 2> originAddress = {mSendTag, mEndpoint->getID()};

    decltype(originAddress) targetAddress;

    Backend::exchange(target, originAddress, targetAddress);

    mRecvTag = {std::get<0>(targetAddress)};

    mRemoteEndpoint = {mEndpoint->connect(std::get<1>(targetAddress))};
}

void pMR::PSM2::Connection::postRecv(void *buffer, std::uint32_t sizeByte)
{
    mRecvRequest = {
        mEndpoint->postRecv(mRemoteEndpoint, buffer, {sizeByte}, {mRecvTag})};
}

void pMR::PSM2::Connection::postSend(void const *buffer, std::uint32_t sizeByte)
{
    mSendRequest = {
        mEndpoint->postSend(mRemoteEndpoint, buffer, {sizeByte}, {mSendTag})};
}

void pMR::PSM2::Connection::pollSend()
{
    mEndpoint->poll(mSendRequest);
}

void pMR::PSM2::Connection::pollRecv()
{
    mEndpoint->poll(mRecvRequest);
}
