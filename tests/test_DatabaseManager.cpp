/**
 * @file test_DatabaseManager.cpp
 * @brief Unit tests for DatabaseManager class
 *
 * Tests database connection, query execution, error handling, and thread safety.
 * Requires MySQL test database to be set up (see fixtures/test_schema.sql)
 */

#include <QtTest/QtTest>
#include <DatabaseManager.h>
#include <QSqlQuery>
#include <QSqlError>
#include <QSignalSpy>

class TestDatabaseManager : public QObject
{
    Q_OBJECT

private:
    DatabaseManager *dbManager;

    // Test database credentials
    const QString TEST_HOST = "localhost";
    const QString TEST_DB = "weather_station_test_db";
    const QString TEST_USER = "test_user";
    const QString TEST_PASS = "test_password";
    const int TEST_PORT = 3306;

private slots:
    /**
     * @brief Runs once before all tests - setup test environment
     */
    void initTestCase()
    {
        qDebug() << "Starting DatabaseManager test suite...";
        // Note: Assumes test database already exists (run test_schema.sql first)
    }

    /**
     * @brief Runs once after all tests - cleanup test environment
     */
    void cleanupTestCase()
    {
        qDebug() << "DatabaseManager test suite complete.";
    }

    /**
     * @brief Runs before each individual test
     */
    void init()
    {
        dbManager = new DatabaseManager(this);
    }

    /**
     * @brief Runs after each individual test
     */
    void cleanup()
    {
        if (dbManager) {
            dbManager->disconnectFromDatabase();
            delete dbManager;
            dbManager = nullptr;
        }
    }

    //
    // CONNECTION TESTS
    //

    /**
     * @brief Test successful database connection
     */
    void testConnectionSuccess()
    {
        // Setup signal spy to verify connection signal
        QSignalSpy spyConnection(dbManager, &DatabaseManager::connectionStatusChanged);

        // Attempt connection
        bool connected = dbManager->connectToDatabase(
            TEST_HOST,
            TEST_DB,
            TEST_USER,
            TEST_PASS,
            TEST_PORT,
            "test_connection_success"
        );

        // Verify connection succeeded
        QVERIFY2(connected, "Database connection should succeed");
        QVERIFY2(dbManager->isConnected(), "isConnected() should return true");

        // Verify signal was emitted
        QCOMPARE(spyConnection.count(), 1);

        // Verify signal parameter is true
        QList<QVariant> arguments = spyConnection.takeFirst();
        QVERIFY(arguments.at(0).toBool() == true);

        // Verify no error message
        QVERIFY2(dbManager->getLastError().isEmpty(),
                 "No error should be present on successful connection");
    }

    /**
     * @brief Test connection failure with invalid credentials
     */
    void testConnectionFailure_InvalidCredentials()
    {
        QSignalSpy spyError(dbManager, &DatabaseManager::errorOccurred);

        bool connected = dbManager->connectToDatabase(
            TEST_HOST,
            TEST_DB,
            "invalid_user",
            "wrong_password",
            TEST_PORT,
            "test_connection_fail"
        );

        // Verify connection failed
        QVERIFY2(!connected, "Connection with invalid credentials should fail");
        QVERIFY2(!dbManager->isConnected(), "isConnected() should return false");

        // Verify error was recorded
        QVERIFY2(!dbManager->getLastError().isEmpty(),
                 "Error message should be set on connection failure");
    }

    /**
     * @brief Test connection failure with invalid hostname
     */
    void testConnectionFailure_InvalidHost()
    {
        bool connected = dbManager->connectToDatabase(
            "invalid.hostname.local",
            TEST_DB,
            TEST_USER,
            TEST_PASS,
            TEST_PORT,
            "test_connection_invalid_host"
        );

        QVERIFY2(!connected, "Connection to invalid host should fail");
        QVERIFY2(!dbManager->getLastError().isEmpty(), "Error message should be set");
    }

    /**
     * @brief Test connection to non-existent database
     */
    void testConnectionFailure_InvalidDatabase()
    {
        bool connected = dbManager->connectToDatabase(
            TEST_HOST,
            "nonexistent_database_12345",
            TEST_USER,
            TEST_PASS,
            TEST_PORT,
            "test_connection_invalid_db"
        );

        QVERIFY2(!connected, "Connection to non-existent database should fail");
    }

    /**
     * @brief Test disconnection
     */
    void testDisconnect()
    {
        // First connect
        dbManager->connectToDatabase(TEST_HOST, TEST_DB, TEST_USER, TEST_PASS,
                                    TEST_PORT, "test_disconnect");
        QVERIFY(dbManager->isConnected());

        // Setup signal spy
        QSignalSpy spyConnection(dbManager, &DatabaseManager::connectionStatusChanged);

        // Disconnect
        dbManager->disconnectFromDatabase();

        // Verify disconnected
        QVERIFY2(!dbManager->isConnected(), "Should be disconnected");

        // Verify signal emitted with false
        QCOMPARE(spyConnection.count(), 1);
        QList<QVariant> arguments = spyConnection.takeFirst();
        QVERIFY(arguments.at(0).toBool() == false);
    }

    //
    // QUERY EXECUTION TESTS
    //

    /**
     * @brief Test successful INSERT query
     */
    void testInsertQuery()
    {
        // Connect to database
        dbManager->connectToDatabase(TEST_HOST, TEST_DB, TEST_USER, TEST_PASS,
                                    TEST_PORT, "test_insert");
        QVERIFY(dbManager->isConnected());

        // Prepare test data
        qint64 testTimestamp = QDateTime::currentSecsSinceEpoch();

        // Execute INSERT
        QSqlQuery query = dbManager->prepareQuery(
            "INSERT INTO london (temperature, humidity, pressure, windSpeed, "
            "weather_id, description, timestamp, sunrise, sunset) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)"
        );
        query.addBindValue(285.15); // temperature
        query.addBindValue(72.5);   // humidity
        query.addBindValue(1014);   // pressure
        query.addBindValue(6.2);    // windSpeed
        query.addBindValue(801);    // weather_id
        query.addBindValue("Partly cloudy"); // description
        query.addBindValue(testTimestamp);   // timestamp
        query.addBindValue(testTimestamp - 3600);  // sunrise
        query.addBindValue(testTimestamp + 3600);  // sunset

        QVERIFY2(query.exec(), "INSERT query should execute successfully");

        // Verify insertion by querying back
        QSqlQuery verifyQuery = dbManager->prepareQuery(
            "SELECT temperature, humidity FROM london WHERE timestamp = ?"
        );
        verifyQuery.addBindValue(testTimestamp);
        QVERIFY(verifyQuery.exec());
        QVERIFY(verifyQuery.next());

        QCOMPARE(verifyQuery.value(0).toDouble(), 285.15);
        QCOMPARE(verifyQuery.value(1).toDouble(), 72.5);

        // Cleanup
        QSqlQuery cleanup = dbManager->prepareQuery(
            "DELETE FROM london WHERE timestamp = ?"
        );
        cleanup.addBindValue(testTimestamp);
        cleanup.exec();
    }

    /**
     * @brief Test SELECT query
     */
    void testSelectQuery()
    {
        dbManager->connectToDatabase(TEST_HOST, TEST_DB, TEST_USER, TEST_PASS,
                                    TEST_PORT, "test_select");

        // Query existing test data (inserted by test_schema.sql)
        QSqlQuery query = dbManager->prepareQuery(
            "SELECT temperature, humidity, description FROM london LIMIT 1"
        );

        QVERIFY2(query.exec(), "SELECT query should execute");
        QVERIFY2(query.next(), "Should have at least one row of test data");

        // Verify we can read values
        double temp = query.value(0).toDouble();
        double humidity = query.value(1).toDouble();
        QString desc = query.value(2).toString();

        QVERIFY(temp > 0);          // Should have valid temperature
        QVERIFY(humidity >= 0 && humidity <= 100);  // Valid humidity range
        QVERIFY(!desc.isEmpty());   // Should have description
    }

    /**
     * @brief Test UPDATE query
     */
    void testUpdateQuery()
    {
        dbManager->connectToDatabase(TEST_HOST, TEST_DB, TEST_USER, TEST_PASS,
                                    TEST_PORT, "test_update");

        // Insert test data first
        qint64 testTimestamp = QDateTime::currentSecsSinceEpoch();
        QSqlQuery insert = dbManager->prepareQuery(
            "INSERT INTO paris (temperature, humidity, pressure, windSpeed, "
            "weather_id, description, timestamp, sunrise, sunset) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)"
        );
        insert.addBindValue(280.0);
        insert.addBindValue(60.0);
        insert.addBindValue(1013);
        insert.addBindValue(4.0);
        insert.addBindValue(800);
        insert.addBindValue("Initial description");
        insert.addBindValue(testTimestamp);
        insert.addBindValue(testTimestamp - 3600);
        insert.addBindValue(testTimestamp + 3600);
        QVERIFY(insert.exec());

        // Update the record
        QSqlQuery update = dbManager->prepareQuery(
            "UPDATE paris SET description = ? WHERE timestamp = ?"
        );
        update.addBindValue("Updated description");
        update.addBindValue(testTimestamp);
        QVERIFY(update.exec());

        // Verify update
        QSqlQuery verify = dbManager->prepareQuery(
            "SELECT description FROM paris WHERE timestamp = ?"
        );
        verify.addBindValue(testTimestamp);
        QVERIFY(verify.exec());
        QVERIFY(verify.next());
        QCOMPARE(verify.value(0).toString(), QString("Updated description"));

        // Cleanup
        QSqlQuery cleanup = dbManager->prepareQuery(
            "DELETE FROM paris WHERE timestamp = ?"
        );
        cleanup.addBindValue(testTimestamp);
        cleanup.exec();
    }

    /**
     * @brief Test DELETE query
     */
    void testDeleteQuery()
    {
        dbManager->connectToDatabase(TEST_HOST, TEST_DB, TEST_USER, TEST_PASS,
                                    TEST_PORT, "test_delete");

        // Insert test data
        qint64 testTimestamp = QDateTime::currentSecsSinceEpoch();
        QSqlQuery insert = dbManager->prepareQuery(
            "INSERT INTO rome (temperature, humidity, pressure, windSpeed, "
            "weather_id, description, timestamp, sunrise, sunset) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)"
        );
        insert.addBindValue(283.0);
        insert.addBindValue(55.0);
        insert.addBindValue(1016);
        insert.addBindValue(3.5);
        insert.addBindValue(800);
        insert.addBindValue("To be deleted");
        insert.addBindValue(testTimestamp);
        insert.addBindValue(testTimestamp - 3600);
        insert.addBindValue(testTimestamp + 3600);
        QVERIFY(insert.exec());

        // Delete the record
        QSqlQuery deleteQuery = dbManager->prepareQuery(
            "DELETE FROM rome WHERE timestamp = ?"
        );
        deleteQuery.addBindValue(testTimestamp);
        QVERIFY(deleteQuery.exec());

        // Verify deletion
        QSqlQuery verify = dbManager->prepareQuery(
            "SELECT COUNT(*) FROM rome WHERE timestamp = ?"
        );
        verify.addBindValue(testTimestamp);
        QVERIFY(verify.exec());
        QVERIFY(verify.next());
        QCOMPARE(verify.value(0).toInt(), 0); // Should be 0 rows
    }

    /**
     * @brief Test SQL injection protection (prepared statements)
     */
    void testSQLInjectionProtection()
    {
        dbManager->connectToDatabase(TEST_HOST, TEST_DB, TEST_USER, TEST_PASS,
                                    TEST_PORT, "test_sql_injection");

        // Attempt SQL injection via parameter
        QString maliciousInput = "'; DROP TABLE london; --";

        QSqlQuery query = dbManager->prepareQuery(
            "SELECT * FROM london WHERE description = ?"
        );
        query.addBindValue(maliciousInput);

        // Should execute safely without dropping table
        query.exec(); // May fail to find match, but shouldn't execute injection

        // Verify table still exists
        QSqlQuery verifyTable = dbManager->prepareQuery(
            "SELECT COUNT(*) FROM london"
        );
        QVERIFY2(verifyTable.exec(), "Table should still exist (injection prevented)");
    }

    //
    // ERROR HANDLING TESTS
    //

    /**
     * @brief Test error handling for invalid SQL
     */
    void testInvalidSQLError()
    {
        dbManager->connectToDatabase(TEST_HOST, TEST_DB, TEST_USER, TEST_PASS,
                                    TEST_PORT, "test_invalid_sql");

        QSignalSpy spyError(dbManager, &DatabaseManager::errorOccurred);

        // Execute invalid SQL
        QSqlQuery query = dbManager->prepareQuery("INVALID SQL SYNTAX HERE");
        bool result = query.exec();

        QVERIFY2(!result, "Invalid SQL should fail");
        QVERIFY2(!query.lastError().text().isEmpty(), "Should have error message");
    }

    /**
     * @brief Test error message retrieval
     */
    void testGetLastError()
    {
        // Connect with invalid credentials
        dbManager->connectToDatabase(TEST_HOST, TEST_DB, "bad_user", "bad_pass",
                                    TEST_PORT, "test_error");

        QString error = dbManager->getLastError();
        QVERIFY2(!error.isEmpty(), "Should have error message");
        QVERIFY2(error.contains("Access denied") || error.contains("authentication"),
                 "Error should mention authentication failure");
    }

    //
    // THREAD SAFETY TESTS
    //

    /**
     * @brief Test multiple simultaneous connections with different names
     */
    void testMultipleConnections()
    {
        DatabaseManager *dbManager2 = new DatabaseManager(this);

        // Connect with different connection names (simulates multi-threading)
        bool conn1 = dbManager->connectToDatabase(TEST_HOST, TEST_DB, TEST_USER,
                                                 TEST_PASS, TEST_PORT, "connection_1");
        bool conn2 = dbManager2->connectToDatabase(TEST_HOST, TEST_DB, TEST_USER,
                                                  TEST_PASS, TEST_PORT, "connection_2");

        QVERIFY2(conn1, "First connection should succeed");
        QVERIFY2(conn2, "Second connection should succeed");
        QVERIFY2(dbManager->isConnected(), "First connection should be active");
        QVERIFY2(dbManager2->isConnected(), "Second connection should be active");

        // Both connections should work independently
        QSqlQuery query1 = dbManager->prepareQuery("SELECT 1");
        QSqlQuery query2 = dbManager2->prepareQuery("SELECT 2");

        QVERIFY(query1.exec());
        QVERIFY(query2.exec());

        // Cleanup
        dbManager2->disconnectFromDatabase();
        delete dbManager2;
    }

    /**
     * @brief Test reconnection after disconnect
     */
    void testReconnection()
    {
        // Initial connection
        bool conn1 = dbManager->connectToDatabase(TEST_HOST, TEST_DB, TEST_USER,
                                                 TEST_PASS, TEST_PORT, "test_reconnect");
        QVERIFY(conn1);

        // Disconnect
        dbManager->disconnectFromDatabase();
        QVERIFY(!dbManager->isConnected());

        // Reconnect
        bool conn2 = dbManager->connectToDatabase(TEST_HOST, TEST_DB, TEST_USER,
                                                 TEST_PASS, TEST_PORT, "test_reconnect_2");
        QVERIFY2(conn2, "Should be able to reconnect after disconnect");
        QVERIFY(dbManager->isConnected());
    }
};

// Qt Test main macro
QTEST_MAIN(TestDatabaseManager)
#include "test_DatabaseManager.moc"
