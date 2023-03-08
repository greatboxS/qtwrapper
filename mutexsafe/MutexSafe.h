#ifndef __MUTEXSAFE_H__
#define __MUTEXSAFE_H__

#include <QMutexLocker>
#include <string.h>

template <typename T>
void MtxSafeWrite(QMutex *mtx, T &obj, T value) {
    QMutexLocker locker(mtx);
    obj = value;
}

template <typename T>
void MtxSafeWrite(QMutex *mtx, T *obj, T *value, size_t len) {
    QMutexLocker locker(mtx);
    memcpy(obj, value, len);
}

template <typename T>
T MtxSafeRead(QMutex *mtx, T &obj) {
    QMutexLocker locker(mtx);
    return obj;
}

void MtxSafeRead(QMutex *mtx, void *src, void *des, size_t size) {
    QMutexLocker locker(mtx);
    memcpy(src, des, size);
}

#endif // __SAFEMUTEX_H__
