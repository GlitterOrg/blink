/*
 * Copyright (C) 2010 Google, Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ResourceLoadTiming_h
#define ResourceLoadTiming_h

#include "wtf/PassRefPtr.h"
#include "wtf/RefCounted.h"
#include "wtf/RefPtr.h"

namespace blink {

class ResourceLoadTiming : public RefCounted<ResourceLoadTiming> {
public:
    static PassRefPtr<ResourceLoadTiming> create();
    PassRefPtr<ResourceLoadTiming> deepCopy();
    bool operator==(const ResourceLoadTiming&) const;
    bool operator!=(const ResourceLoadTiming&) const;

    void setDnsStart(double);
    void setRequestTime(double requestTime) { m_requestTime = requestTime; }
    void setProxyStart(double proxyStart) { m_proxyStart = proxyStart; }
    void setProxyEnd(double proxyEnd) { m_proxyEnd = proxyEnd; }
    void setDnsEnd(double);
    void setConnectStart(double);
    void setConnectEnd(double);
    void setServiceWorkerFetchStart(double serviceWorkerFetchStart) { m_serviceWorkerFetchStart = serviceWorkerFetchStart; }
    void setServiceWorkerFetchReady(double serviceWorkerFetchReady) { m_serviceWorkerFetchReady = serviceWorkerFetchReady; }
    void setServiceWorkerFetchEnd(double serviceWorkerFetchEnd) { m_serviceWorkerFetchEnd = serviceWorkerFetchEnd; }
    void setSendStart(double);
    void setSendEnd(double sendEnd) { m_sendEnd = sendEnd; }
    void setReceiveHeadersEnd(double);
    void setSslStart(double);
    void setSslEnd(double sslEnd) { m_sslEnd = sslEnd; }

    double dnsStart() const { return m_dnsStart; }
    double requestTime() const { return m_requestTime; }
    double proxyStart() const { return m_proxyStart; }
    double proxyEnd() const { return m_proxyEnd; }
    double dnsEnd() const { return m_dnsEnd; }
    double connectStart() const { return m_connectStart; }
    double connectEnd() const { return m_connectEnd; }
    double serviceWorkerFetchStart() const { return m_serviceWorkerFetchStart; }
    double serviceWorkerFetchReady() const { return m_serviceWorkerFetchReady; }
    double serviceWorkerFetchEnd() const { return m_serviceWorkerFetchEnd; }
    double sendStart() const { return m_sendStart; }
    double sendEnd() const { return m_sendEnd; }
    double receiveHeadersEnd() const { return m_receiveHeadersEnd; }
    double sslStart() const { return m_sslStart; }
    double sslEnd() const { return m_sslEnd; }

    double calculateMillisecondDelta(double time) const { return time ? (time - m_requestTime) * 1000 : -1; }

private:
    ResourceLoadTiming();

    // We want to present a unified timeline to Javascript. Using walltime is problematic, because the clock may skew while resources
    // load. To prevent that skew, we record a single reference walltime when root document navigation begins. All other times are
    // recorded using monotonicallyIncreasingTime(). When a time needs to be presented to Javascript, we build a pseudo-walltime
    // using the following equation (m_requestTime as example):
    //   pseudo time = document wall reference + (m_requestTime - document monotonic reference).

    // All monotonicallyIncreasingTime() in seconds
    double m_requestTime;
    double m_proxyStart;
    double m_proxyEnd;
    double m_dnsStart;
    double m_dnsEnd;
    double m_connectStart;
    double m_connectEnd;
    double m_serviceWorkerFetchStart;
    double m_serviceWorkerFetchReady;
    double m_serviceWorkerFetchEnd;
    double m_sendStart;
    double m_sendEnd;
    double m_receiveHeadersEnd;
    double m_sslStart;
    double m_sslEnd;
};

}

#endif
