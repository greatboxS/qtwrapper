#include "QWorker.h"
#include "../mutexsafe/mutexsafe.h"
#include <exception>
#include <QDebug>

QWorker::QWorker(const char *cWorkName, IWorker *iWorker, void *param, int priority, int core) :
    m_strName(QString(cWorkName)),
    m_poIWorker(iWorker),
    m_s32WorkerState(WORKER_STOP),
    m_pParam(param),
    m_workFunc(NULL),
    m_finalized(false),
    QThread() {
    QMutexLocker locker(&m_stMtx);
}

QWorker::QWorker(const char *cWorkName, QWorkerHandler fnc, void *param) :
    m_strName(QString(cWorkName)),
    m_poIWorker(NULL),
    m_s32WorkerState(WORKER_STOP),
    m_pParam(param),
    m_workFunc(fnc),
    m_finalized(false),
    QThread() {
    QMutexLocker locker(&m_stMtx);
}

QWorker::~QWorker() {
}

inline int QWorker::IsRunning() {
    return (MtxSafeRead(&m_stMtx, m_s32WorkerState) == WORKER_RUN);
}

int QWorker::IsTerminated() {
    return static_cast<int>(m_finalized == true);
}

int QWorker::TerminateWorker(unsigned long wait) {
    if (QThread::isRunning()) {
        StopWorker();
        MtxSafeWrite(&m_stMtx, m_finalized, true);
        QThread::terminate();
        QThread::wait(wait);
    }
}

void QWorker::run() try {

    while (m_finalized == false) {
        int32_t state = MtxSafeRead(&m_stMtx, m_s32WorkerState);
        switch (state) {

        case WORKER_INIT:
            MtxSafeWrite(&m_stMtx, m_s32WorkerState, static_cast<int32_t>(WORKER_RUN));
            if (m_poIWorker)
                if (m_poIWorker->OnWorkerInitialize() < 0) {
                }
            break;

        case WORKER_FINAL:
            MtxSafeWrite(&m_stMtx, m_s32WorkerState, static_cast<int32_t>(WORKER_STOP));
            if (m_poIWorker)
                if (m_poIWorker->OnWorkerFinalize() < 0) {
                }
            break;

        case WORKER_RUN:
            if (m_poIWorker)
                m_poIWorker->OnWorkerRun(m_pParam);
            if (m_workFunc)
                m_workFunc(m_pParam);
            break;

        case WORKER_PRE_EXIT:
            MtxSafeWrite(&m_stMtx, m_s32WorkerState, static_cast<int32_t>(WORKER_EXIT_DONE));
            break;

        case WORKER_EXIT_DONE:
        case WORKER_STOP:
        default:
            QThread::msleep(10);
            break;
        }
    }
} catch (std::exception &ex) {
}

int QWorker::StartWorker(void *param) {
    try {
        if (IsTerminated()) return 0;

        if (QThread::isRunning() == false) {
            QThread::start();
        }

        if (param != NULL && param != reinterpret_cast<void *>(-1)) m_pParam = param;

        if (m_poIWorker)
            m_poIWorker->OnRequestWorkerStart();
        MtxSafeWrite(&m_stMtx, m_s32WorkerState, static_cast<int32_t>(WORKER_INIT));
    } catch (std::exception &exp) {}
    return 0;
}

int QWorker::StopWorker() {
    try {
        if (IsTerminated()) return 0;

        if (m_poIWorker)
            m_poIWorker->OnRequestWorkerStop();
        MtxSafeWrite(&m_stMtx, m_s32WorkerState, static_cast<int32_t>(WORKER_FINAL));
        while (MtxSafeRead(&m_stMtx, m_s32WorkerState) != static_cast<int32_t>(WORKER_STOP))
            ;
    } catch (std::exception &exp) {}
    return 0;
}
