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

#include "queuepair.hpp"
#include <stdexcept>
#include "connectionaddress.hpp"
#include "verbs.hpp"

pMR::Verbs::QueuePair::QueuePair(
    ProtectionDomain &protectionDomain, CompletionQueue &completionQueue)
    : QueuePair(protectionDomain, completionQueue, completionQueue)
{
}

pMR::Verbs::QueuePair::QueuePair(ProtectionDomain &protectionDomain,
    CompletionQueue &sendCompletionQueue, CompletionQueue &recvCompletionQueue)
{
    ibv_qp_init_attr initialAttributes = {};
    initialAttributes.send_cq = sendCompletionQueue.get();
    initialAttributes.recv_cq = recvCompletionQueue.get();
    initialAttributes.cap.max_send_wr = {cMaxSend};
    initialAttributes.cap.max_recv_wr = {cMaxRecv};
    initialAttributes.cap.max_send_sge = {cMaxSendSG};
    initialAttributes.cap.max_recv_sge = {cMaxRecvSG};
    initialAttributes.cap.max_inline_data = {cMaxInlineDataSize};
    initialAttributes.sq_sig_all = 1;
    initialAttributes.qp_type = IBV_QPT_RC;

    mQueuePair = ibv_create_qp(protectionDomain.get(), &initialAttributes);

    if(!mQueuePair)
    {
        throw std::runtime_error("pMR: Unable to create Queue Pair.");
    }
}

pMR::Verbs::QueuePair::~QueuePair()
{
    ibv_destroy_qp(mQueuePair);
}

ibv_qp *pMR::Verbs::QueuePair::get()
{
    return mQueuePair;
}

ibv_qp const *pMR::Verbs::QueuePair::get() const
{
    return mQueuePair;
}

std::uint32_t pMR::Verbs::QueuePair::getQPN() const
{
    return {mQueuePair->qp_num};
}

void pMR::Verbs::QueuePair::setStateINIT(std::uint8_t const portNumber)
{
    ibv_qp_attr attr = {};
    attr.qp_state = IBV_QPS_INIT;
    attr.qp_access_flags = IBV_ACCESS_REMOTE_WRITE;
    attr.pkey_index = {cPKeyIndex};
    attr.port_num = portNumber;

    if(ibv_modify_qp(this->get(), &attr,
           IBV_QP_STATE | IBV_QP_ACCESS_FLAGS | IBV_QP_PKEY_INDEX |
               IBV_QP_PORT))
    {
        throw std::runtime_error("pMR: Unable to modify QueuePair to INIT.");
    }
}

void pMR::Verbs::QueuePair::setStateRTR(
    std::uint8_t const portNumber, ConnectionAddress const &targetAddress)
{
    ibv_qp_attr attr = {};
    attr.qp_state = IBV_QPS_RTR;
    attr.path_mtu = {cMTU};
    attr.rq_psn = {cPSN};
    attr.dest_qp_num = {targetAddress.getQPN()};
    attr.max_dest_rd_atomic = {cDestRDAtomic};
    attr.min_rnr_timer = {cRNRTimer};
    attr.ah_attr.dlid = {targetAddress.getLID()};
    attr.ah_attr.sl = {cServiceLevel};
    attr.ah_attr.src_path_bits = {cSrcPath};
    attr.ah_attr.static_rate = {cStaticRate};
    attr.ah_attr.is_global = {cGlobal};
    attr.ah_attr.port_num = {portNumber};
    attr.ah_attr.grh.dgid = targetAddress.getGID();
    attr.ah_attr.grh.sgid_index = {cSGIDIndex};
    attr.ah_attr.grh.hop_limit = {cHopLimit};

    if(ibv_modify_qp(this->get(), &attr,
           IBV_QP_STATE | IBV_QP_PATH_MTU | IBV_QP_RQ_PSN | IBV_QP_DEST_QPN |
               IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER | IBV_QP_AV))
    {
        throw std::runtime_error("pMR: Unable to modify QueuePair to RTR.");
    }
}

void pMR::Verbs::QueuePair::setStateRTS()
{
    ibv_qp_attr attr = {};
    attr.qp_state = IBV_QPS_RTS;
    attr.sq_psn = {cPSN};
    attr.max_rd_atomic = {cRDAtomic};
    attr.timeout = {cTimeout};
    attr.retry_cnt = {cRetryCounter};
    attr.rnr_retry = {cRNRRetry};

    if(ibv_modify_qp(this->get(), &attr,
           IBV_QP_STATE | IBV_QP_SQ_PSN | IBV_QP_MAX_QP_RD_ATOMIC |
               IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT | IBV_QP_RNR_RETRY))
    {
        throw std::runtime_error("pMR: Unable to modify QueuePair to RTS.");
    }
}
