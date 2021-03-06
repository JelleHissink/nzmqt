// Copyright 2011 Johann Duscher. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
//    1. Redistributions of source code must retain the above copyright notice, this list of
//       conditions and the following disclaimer.
//
//    2. Redistributions in binary form must reproduce the above copyright notice, this list
//       of conditions and the following disclaimer in the documentation and/or other materials
//       provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY JOHANN DUSCHER ''AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation are those of the
// authors and should not be interpreted as representing official policies, either expressed
// or implied, of Johann Duscher.

#ifndef PUSHPULLWORKER_H
#define PUSHPULLWORKER_H

#include <QObject>
#include <QRunnable>
#include <QDebug>
#include <QList>
#include <QByteArray>
#include <QTimer>
#include <QWaitCondition>
#include <QMutex>

#include "nzmqt/nzmqt.hpp"


class PushPullWorker : public QObject, public QRunnable
{
    Q_OBJECT

    typedef QObject super;

public:
    explicit PushPullWorker(const QString& ventilatorAddress, const QString& sinkAddress, QObject *parent)
        : super(parent), ventilatorAddress_(ventilatorAddress), sinkAddress_(sinkAddress)
    {
        nzmqt::ZMQContext* context = new nzmqt::ZMQContext(4, this);

        ventilator_ = context->createSocket(ZMQ_PULL);
        connect(ventilator_, SIGNAL(readyRead()), SLOT(workAvailable()));

        sink_ = context->createSocket(ZMQ_PUSH);
    }

    void run()
    {
        sink_->connectTo(sinkAddress_);
        ventilator_->connectTo(ventilatorAddress_);
    }

protected slots:
    void workAvailable()
    {
        QList<QByteArray> msg = ventilator_->receiveMessage();
        quint32 work = QString(msg[0]).toUInt();

        // Do the work ;-)
        qDebug() << "snore" << work << "msec";
        sleep(work);

        // Send results to sink.
        sink_->sendMessage("");
    }

private:
    QString ventilatorAddress_;
    QString sinkAddress_;

    nzmqt::ZMQSocket* ventilator_;
    nzmqt::ZMQSocket* sink_;

    static inline void sleep(unsigned msec)
    {
        QMutex mutex;
        QWaitCondition wc;
        wc.wait(&mutex, msec);
    }
};

#endif // PUSHPULLWORKER_H
