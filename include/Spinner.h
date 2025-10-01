#ifndef SPINNER_H
#define SPINNER_H

#include <QThread>
#include <atomic>
#include <iostream>
#include <chrono>

class Spinner : public QThread
{
    Q_OBJECT

public:
    explicit Spinner(QObject *parent = nullptr);
    ~Spinner();

    void stop();
    void pause();
    void resume();

protected:
    void run() override;

private:
    std::atomic<bool> m_running;
    std::atomic<bool> m_paused;
    std::chrono::steady_clock::time_point m_startTime;
};

#endif // SPINNER_H
