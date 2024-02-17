//
//  Created by Recep ASLANTAS
//  Copyright (c) 2013 Recep ASLANTAS. All rights reserved.
//

#ifndef THREAD_DISPATCHER_H
#define THREAD_DISPATCHER_H

#include <QCoreApplication>
#include <QMetaObject>
#include <QObject>
#include <QThread>
#include <cassert>
#include <functional>
#include <utility>

using voidBlock = std::function<void()>;

class WorkerClass : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit WorkerClass(QThread *thread)
    {
        moveToThread(thread);
        connect(QThread::currentThread(), &QThread::finished, this, &WorkerClass::deleteLater);
    }
public slots:
    void DoWork(const voidBlock &block)
    {
        block();
        deleteLater();
    }
};

Q_DECL_UNUSED static inline void q_dispatch_async(QThread *thread, const voidBlock &block);
static inline void q_dispatch_async(QThread *thread, const voidBlock &block)
{
    qRegisterMetaType<voidBlock>("voidBlock");

    auto *worker = new WorkerClass(thread);
    QMetaObject::invokeMethod(worker, "DoWork", Qt::QueuedConnection, Q_ARG(voidBlock, block));
}

Q_DECL_UNUSED static inline void q_dispatch_async_main(const voidBlock &block);
static inline void q_dispatch_async_main(const voidBlock &block)
{
    QThread *mainThread = QCoreApplication::instance()->thread();
    q_dispatch_async(mainThread, block);
}

using msgHandlerBlock = std::function<void(QtMsgType, const QMessageLogContext &, const QString &)>;

class MsgHandlerWorkerClass : public QObject
{
    Q_OBJECT // NOLINT

public:
    MsgHandlerWorkerClass(QThread *thread, QtMsgType type, const QMessageLogContext &context, QString msg)
      : m_type(type),
        m_msg(std::move(msg)),
        m_line(context.line),
        m_file(context.file),
        m_function(context.function),
        m_category(context.category)
    {
#ifndef V_NO_ASSERT
        assert(context.version == 2);
#endif
        moveToThread(thread);
        connect(QThread::currentThread(), &QThread::finished, this, &WorkerClass::deleteLater);
    }
public slots:
    void DoWork(const msgHandlerBlock &block)
    {
        block(
            m_type,
            QMessageLogContext(qUtf8Printable(m_file), m_line, qUtf8Printable(m_function), qUtf8Printable(m_category)),
            m_msg);
        deleteLater();
    }

private:
    QtMsgType m_type;
    QString m_msg;

    // We cannot make copy of QMessageLogContext. So, we must save its data instead and recreate it later.
    int m_line;
    QString m_file;
    QString m_function;
    QString m_category;
};

Q_DECL_UNUSED static inline void q_dispatch_async(QThread *thread, const msgHandlerBlock &block, QtMsgType type,
                                                  const QMessageLogContext &context, const QString &msg);
static inline void q_dispatch_async(QThread *thread, const msgHandlerBlock &block, QtMsgType type,
                                    const QMessageLogContext &context, const QString &msg)
{
    qRegisterMetaType<msgHandlerBlock>("msgHandlerBlock");

    auto *worker = new MsgHandlerWorkerClass(thread, type, context, msg);
    QMetaObject::invokeMethod(worker, "DoWork", Qt::QueuedConnection, Q_ARG(msgHandlerBlock, block));
}

Q_DECL_UNUSED static inline void q_dispatch_async_main(const msgHandlerBlock &block, QtMsgType type,
                                                       const QMessageLogContext &context, const QString &msg);
static inline void q_dispatch_async_main(const msgHandlerBlock &block, QtMsgType type,
                                         const QMessageLogContext &context, const QString &msg)
{
    QThread *mainThread = QCoreApplication::instance()->thread();
    q_dispatch_async(mainThread, block, type, context, msg);
}

#undef V_UNUSED

#endif // THREAD_DISPATCHER_H
