#ifndef PYTHONBRIDGE_H
#define PYTHONBRIDGE_H
#include <DatabaseManager.h>

class PythonBridge
{
public:
    PythonBridge();
    ~PythonBridge();

    void calculateAverageData(const QString &host,
                              const QString &database,
                              const QString &username,
                              const QString &password,
                              const QString &tbl,
                              double &avgTemp,
                              double &avgHumidity);

    // Get statistics for all locations in JSON format
    // Returns JSON string with all location statistics
    QString getStatisticsJson(const QString &host,
                              const QString &database,
                              const QString &username,
                              const QString &password);

    // Get period comparison (24h vs 7 days) for a specific location
    // Returns JSON string with period comparison data
    QString getPeriodComparisonJson(const QString &host,
                                     const QString &database,
                                     const QString &username,
                                     const QString &password,
                                     const QString &table);

    // Get cross-location comparison for all 5 locations
    // Returns JSON string with comparative statistics across all locations
    QString getLocationComparisonJson(const QString &host,
                                       const QString &database,
                                       const QString &username,
                                       const QString &password,
                                       int hours = 24);

private:
    bool m_initialized;
    bool initialize();
    void finalize();
};

#endif // PYTHONBRIDGE_H