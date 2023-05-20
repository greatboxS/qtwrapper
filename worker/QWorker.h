#ifndef __QWORKER_H__
#define __QWORKER_H__

#include <QThread>
#include <QMutexLocker>
#include <functional>

namespace qtwrapper
{
    using QWorkerHandler = std::function<void *(void *)>;

    class IWorker
    {
        friend class QWorker;
        friend class QQueueWorker;

    protected:
        virtual int OnWorkerInitialize() = 0;
        virtual int OnWorkerFinalize() = 0;
        virtual int OnWorkerTerminate() = 0;
        virtual int OnWorkerRun(void *param) = 0;

    public:
        virtual ~IWorker() {}
        virtual int OnRequestWorkerStart() = 0;
        virtual int OnRequestWorkerStop() = 0;
    };

    class INotify
    {
    public:
        virtual int OnNotify(void *) = 0;
    };

    typedef enum {
        WORKER_INIT,
        WORKER_FINAL,
        WORKER_RUN,
        WORKER_STOP,
        WORKER_PRE_EXIT,
        WORKER_EXIT_DONE,
    } eWorkerState;

    class QWorker : private QThread
    {
        Q_OBJECT
    private:
        IWorker *m_poIWorker;
        QMutex m_stMtx;
        QString m_strName;
        int32_t m_s32WorkerState;
        void *m_pParam;
        QWorkerHandler m_workFunc;
        bool m_finalized;

        void run() override;

    public:
        explicit QWorker(const char *cWorkName, IWorker *iWorker, void *param = NULL, int priority = 0, int core = -1);
        explicit QWorker(const char *cWorkName, QWorkerHandler fnc, void *param = NULL);
        ~QWorker();

        int IsRunning();
        int IsTerminated();

        virtual int TerminateWorker(unsigned long wait);
        virtual int StartWorker(void *param = NULL);
        virtual int StopWorker();
    };
};     // namespace qtwrapper
#endif // __QWORKER_H__
