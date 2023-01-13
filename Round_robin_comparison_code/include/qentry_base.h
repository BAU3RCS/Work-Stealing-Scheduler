#ifndef QENTRY_BASE_H
#define QENTRY_BASE_H

class QEBase {
public:
    virtual ~QEBase() {}
    virtual void run_and_complete() = 0;
};

#endif
