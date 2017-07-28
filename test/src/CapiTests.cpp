/*
 * This file is a part of Stand-alone Future Library (safl).
 */

// Code to test:
#include <safl/capi/ToFuture.h>

// Testing:
#include "Testing.h"

// CommonAPI:
#define COMMONAPI_INTERNAL_COMPILATION
#include <CommonAPI/Proxy.hpp>

class DoItProxyBase
    : public CommonAPI::Proxy
{
public:
    typedef std::function<void(const CommonAPI::CallStatus&, const MyInt&)> DoItAsyncCallback;
    virtual std::future<CommonAPI::CallStatus> doItAsync(
            const int&, const std::string&,
            DoItAsyncCallback = nullptr, const CommonAPI::CallInfo* = nullptr) = 0;
};

class DoItVersionAttribute
        : public CommonAPI::InterfaceVersionAttribute
{
public:
    void getValue(CommonAPI::CallStatus&, CommonAPI::Version&,
                  const CommonAPI::CallInfo*) const override {}
    std::future<CommonAPI::CallStatus> getValueAsync(AttributeAsyncCallback, const CommonAPI::CallInfo*) override
    {
        std::promise<CommonAPI::CallStatus> p;
        return p.get_future();
    }
};

class DoItProxy
    : public DoItProxyBase
{
public:
    std::future<CommonAPI::CallStatus> doItAsync(
            const int &i, const std::string &s,
            DoItAsyncCallback cb = nullptr, const CommonAPI::CallInfo */*info*/ = nullptr) override
    {
        std::promise<CommonAPI::CallStatus> p;
        calledInt = i;
        calledString = s;
        calledCb = cb;
        return p.get_future();
    }
    bool isAvailable() const override { return true; }
    bool isAvailableBlocking() const override { return true; }
    CommonAPI::ProxyStatusEvent &getProxyStatusEvent() override
    {
        static CommonAPI::ProxyStatusEvent event;
        return event;
    }
    CommonAPI::InterfaceVersionAttribute &getInterfaceVersionAttribute() override
    {
        static DoItVersionAttribute attribute;
        return attribute;
    }

public:
    int calledInt;
    std::string calledString;
    DoItAsyncCallback calledCb;
};

class CapiTest:
        public SaflTest
{
protected:
    DoItProxy m_proxy;
};

TEST_F(CapiTest, simpleCall)
{
    int calledWithInt = 0;

    auto f = capi::futurize(m_proxy, &DoItProxy::doItAsync, 5, "44")
            .then([&](const MyInt &value)
    {
        calledWithInt = value.value();
    });

    EXPECT_FALSE(f.isReady());
    EXPECT_EQ(5, m_proxy.calledInt);
    EXPECT_EQ("44", m_proxy.calledString);
    EXPECT_NO_FULFILLED_FUTURES();

    m_proxy.calledCb(CommonAPI::CallStatus::SUCCESS, MyInt{33});
    m_proxy.calledCb = nullptr;
    EXPECT_FUTURE_FULFILLED();
    EXPECT_TRUE(f.isReady());
    EXPECT_EQ(33, calledWithInt);
}

TEST_F(CapiTest, onError)
{
    int calledWithInt = 0;
    CommonAPI::CallStatus calledWithStatus = CommonAPI::CallStatus::SUCCESS;

    auto f = capi::futurize(m_proxy, &DoItProxy::doItAsync, 3, "447")
            .then([&](const MyInt &value)
    {
        calledWithInt = value.value();
    });
    f.onError([&](const capi::CapiError &error)
    {
        calledWithStatus = error.callStatus;
    });

    EXPECT_FALSE(f.isReady());
    EXPECT_EQ(3, m_proxy.calledInt);
    EXPECT_EQ("447", m_proxy.calledString);
    EXPECT_NO_FULFILLED_FUTURES();

    m_proxy.calledCb(CommonAPI::CallStatus::REMOTE_ERROR, MyInt{33});
    m_proxy.calledCb = nullptr;
    EXPECT_FUTURE_FULFILLED();
    EXPECT_TRUE(f.isReady());
    EXPECT_EQ(0, calledWithInt);
    EXPECT_EQ(CommonAPI::CallStatus::REMOTE_ERROR, calledWithStatus);
}
