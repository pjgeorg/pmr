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

//! @file target.hpp
//! @brief Public interface for Target.
//!
//! @author Peter Georg
#ifndef pMR_BACKENDS_MPI_TARGET_H
#define pMR_BACKENDS_MPI_TARGET_H

#include <memory>
extern "C"
{
#include <mpi.h>
}

namespace pMR
{
    //! @brief Backend-agnostic Target.
    class Target
    {
        public:
            //! @brief Convert MPI Target to backend-agnostic Target.
            //! @warning Only one of the paramteter null and self can be true.
            //! @param communicator MPI Communicator.
            //! @param targetRank Target MPI rank.
            //! @param uniqueSendID Unique ID to distiguish MPI messages
            //!     (Send Tag).
            //! @param uniqueRecvID Unique ID to distinguish MPI messages
            //!     (Recv Tag).
            //! @param null Target is no communication (MPI_PROC_NULL).
            //! @param self Target is self communication.
            //! @return Return backend-agnostic Target.
            Target(MPI_Comm const communicator, int const targetRank,
                    unsigned const uniqueSendID, unsigned const uniqueRecvID,
                    bool const null, bool const self);
            //! @brief Convert MPI Target to backend-agnostic Target.
            //! @param communicator MPI Communicator.
            //! @param targetRank Target MPI rank.
            //! @param uniqueSendID Unique ID to distiguish MPI messages
            //!     (Send Tag).
            //! @param uniqueRecvID Unique ID to distinguish MPI messages
            //!     (Recv Tag).
            //! @return Return backend-agnostic Target.
            Target(MPI_Comm const communicator, int const targetRank,
                    unsigned const uniqueSendID, unsigned const uniqueRecvID);
            ~Target() = default;
            //! @brief Checks whether the target is null (MPI_PROC_NULL).
            //! @return true if null, false otherwise.
            bool isNull() const;
            //! @brief Checks whether the target is the same process.
            //! @details Connections to the same process may or may not require
            //!     a second connection attempt by the same process. Using only
            //!     the backend-agnostic communicator the bahavior is naturally.
            //!     A connection to a Target retrieved calling getNeighbor with
            //!     a displacement not equal to zero, but still being the same
            //!     process, assume there is a second connection attempt. For
            //!     all other cases, getNeighbor with displacement of zero or
            //!     getTarget of own ID, there is no other connection attempt
            //!     required.
            //!     In terms of MPI: The second is a SendRecv call with both
            //!     same source and destination rank, and send and receive tag.
            //! @return true if self, false otherwise.
            bool isSelf() const;
            int getTargetRank() const;
            int getUniqueSendID() const;
            int getUniqueRecvID() const;
            MPI_Comm getMPICommunicator() const;
        private:
            MPI_Comm const mCommunicator;
            int const mTarget;
            unsigned const mUniqueSendID;
            unsigned const mUniqueRecvID;
            bool mNull = false;
            bool mSelf = false;
            int queryRank() const;
            void queryTarget();
    };
}
#endif // pMR_BACKENDS_MPI_TARGET_H
