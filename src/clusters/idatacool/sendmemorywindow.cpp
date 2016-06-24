#include "sendmemorywindow.hpp"
#include "connection.hpp"
#include "../../providers/null/memorywindow.hpp"
#include "../../providers/self/memorywindow.hpp"
#include "../../providers/loop/memorywindow.hpp"
#include "../../providers/cma/memorywindow.hpp"
#include "../../providers/verbs/memorywindow.hpp"

pMR::SendMemoryWindow::SendMemoryWindow(Connection const &connection,
        void *buffer, std::uint32_t const sizeByte)
    :   mBuffer(buffer),
        mSizeByte(sizeByte),
        mProvider(connection.mProvider)
{
    switch(mProvider)
    {
        case Provider::null:
        {
            mNull = std::unique_ptr<null::SendMemoryWindow,
                    null::SendMemoryWindowDeleter>(new null::SendMemoryWindow(
                                connection.mNull, buffer, sizeByte));
            break;
        }
        case Provider::self:
        {
            mSelf = std::unique_ptr<self::SendMemoryWindow,
                    self::SendMemoryWindowDeleter>(new self::SendMemoryWindow(
                                connection.mSelf, buffer, sizeByte));
            break;
        }
        case Provider::loop:
        {
            mLoop = std::unique_ptr<loop::SendMemoryWindow,
                    loop::SendMemoryWindowDeleter>(new loop::SendMemoryWindow(
                                connection.mLoop, buffer, sizeByte));
            break;
        }
        case Provider::cma:
        {
            mCMA = std::unique_ptr<cma::SendMemoryWindow,
                    cma::SendMemoryWindowDeleter>(new cma::SendMemoryWindow(
                                connection.mCMA, buffer, sizeByte));
            break;
        }
        case Provider::verbs:
        {
            mVerbs = std::unique_ptr<verbs::SendMemoryWindow,
                    verbs::SendMemoryWindowDeleter>(new verbs::SendMemoryWindow(
                                connection.mVerbs, buffer, sizeByte));
            break;
        }
    }
}

pMR::SendMemoryWindow::~SendMemoryWindow() { }

void pMR::SendMemoryWindow::init()
{
    switch(mProvider)
    {
        case Provider::null:
        {
            mNull->init();
            break;
        }
        case Provider::self:
        {
            mSelf->init();
            break;
        }
        case Provider::loop:
        {
            mLoop->init();
            break;
        }
        case Provider::cma:
        {
            mCMA->init();
            break;
        }
        case Provider::verbs:
        {
            mVerbs->init();
            break;
        }
    }
}

void pMR::SendMemoryWindow::post()
{
    post(mSizeByte);
}

void pMR::SendMemoryWindow::post(std::uint32_t const sizeByte)
{
    if(sizeByte > mSizeByte)
    {
        throw std::length_error("pMR: Send message exceed SendWindow.");
    }

    switch(mProvider)
    {
        case Provider::null:
        {
            mNull->post(sizeByte);
            break;
        }
        case Provider::self:
        {
            mSelf->post(sizeByte);
            break;
        }
        case Provider::loop:
        {
            mLoop->post(sizeByte);
            break;
        }
        case Provider::cma:
        {
            mCMA->post(sizeByte);
            break;
        }
        case Provider::verbs:
        {
            mVerbs->post(sizeByte);
            break;
        }
    }
}

void pMR::SendMemoryWindow::wait()
{
    switch(mProvider)
    {
        case Provider::null:
        {
            mNull->wait();
            break;
        }
        case Provider::self:
        {
            mSelf->wait();
            break;
        }
        case Provider::loop:
        {
            mLoop->wait();
            break;
        }
        case Provider::cma:
        {
            mCMA->wait();
            break;
        }
        case Provider::verbs:
        {
            mVerbs->wait();
            break;
        }
    }
}

void* pMR::SendMemoryWindow::data()
{
    return mBuffer;
}

void const* pMR::SendMemoryWindow::data() const
{
    return mBuffer;
}

std::uint32_t pMR::SendMemoryWindow::size() const
{
    return mSizeByte;
}
