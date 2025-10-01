#include "Spinner.h"
#include <QThread>
#include <iostream>
#include <iomanip>
#include <sstream>

// Mã màu ANSI
#define COLOR_YELLOW "\033[33m"
#define COLOR_RESET "\033[0m"

Spinner::Spinner(QObject *parent)
    : QThread(parent), m_running(false), m_paused(false)
{
}

Spinner::~Spinner()
{
    stop();
    wait();
}

void Spinner::run()
{
    const char frames[] = {'|', '/', '-', '\\'};
    int frameIndex = 0;
    m_running = true;
    m_startTime = std::chrono::steady_clock::now();

    std::string lastOutput = "";

    while (m_running)
    {
        if (m_paused)
        {
            // Xóa spinner khi tạm dừng
            for (size_t i = 0; i < lastOutput.length(); ++i)
            {
                std::cout << "\b \b";
            }
            std::cout.flush();
            lastOutput = "";

            QThread::msleep(100);
            continue;
        }

        // Tính thời gian đã trôi qua
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime).count();

        // Tạo chuỗi hiển thị với màu vàng
        std::ostringstream oss;
        oss << COLOR_YELLOW << frames[frameIndex] << " " << elapsed << "s" << COLOR_RESET;
        std::string currentOutput = oss.str();

        // Xóa output cũ (không tính mã màu)
        for (size_t i = 0; i < lastOutput.length(); ++i)
        {
            std::cout << "\b";
        }

        // In output mới
        std::cout << currentOutput;
        std::cout.flush();

        lastOutput = currentOutput;
        frameIndex = (frameIndex + 1) % 4;
        QThread::msleep(100); // 100ms giữa mỗi frame
    }

    // Xóa spinner và timer khi dừng
    for (size_t i = 0; i < lastOutput.length(); ++i)
    {
        std::cout << "\b \b";
    }
    std::cout.flush();
}

void Spinner::stop()
{
    m_running = false;
}

void Spinner::pause()
{
    m_paused = true;
}

void Spinner::resume()
{
    m_paused = false;
}
