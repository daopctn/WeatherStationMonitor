# WeatherStationMonitor - Test Suite

This directory contains the automated test suite for the WeatherStationMonitor project, built using Qt Test framework.

## Test Coverage

### Current Tests

1. **test_DatabaseManager** - Comprehensive database manager tests
   - Connection success/failure scenarios
   - Query execution (INSERT, SELECT, UPDATE, DELETE)
   - SQL injection protection
   - Error handling
   - Multi-connection (thread safety)
   - Reconnection after disconnect

2. **test_WeatherData** - Data structure tests
   - Default construction
   - Field assignment and validation
   - Copy construction and assignment
   - Realistic and extreme value ranges
   - QVector compatibility
   - Timestamp ordering (duplicate prevention)
   - Day/night detection logic

### Planned Tests

- **test_PythonBridge** - Python/C++ integration tests
- **test_WeatherWorker** - API worker thread tests
- **test_ThreadManager** - Multi-threading coordinator tests
- **test_integration** - End-to-end integration tests

## Prerequisites

### 1. Build Dependencies

```bash
sudo apt-get install -y \
    cmake \
    qtbase5-dev \
    libqt5charts5-dev \
    libqt5sql5-mysql \
    python3-dev \
    lcov  # For code coverage
```

### 2. Test Database Setup

Create a test MySQL database and user:

```bash
# Login to MySQL
sudo mysql -u root -p

# Create test database and user
CREATE DATABASE weather_station_test_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE USER 'test_user'@'localhost' IDENTIFIED BY 'test_password';
GRANT ALL PRIVILEGES ON weather_station_test_db.* TO 'test_user'@'localhost';
FLUSH PRIVILEGES;
EXIT;

# Load test schema
mysql -u test_user -p weather_station_test_db < tests/fixtures/test_schema.sql
```

**Security Note:** The test database uses simple credentials for convenience. Never use these in production!

## Running Tests

### Quick Start

```bash
# From project root
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ..
make
ctest --output-on-failure
```

### Individual Test Execution

```bash
# Run specific test
./tests/test_DatabaseManager
./tests/test_WeatherData

# Run with verbose output
./tests/test_DatabaseManager -v2
```

### CTest Commands

```bash
# Run all tests
ctest

# Verbose output (shows test details)
ctest -V

# Run tests matching pattern
ctest -R Database

# Run tests in parallel
ctest -j4
```

## Code Coverage

### Generate Coverage Report

```bash
# Build with coverage enabled
mkdir build_coverage && cd build_coverage
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON ..
make

# Run tests
ctest

# Generate coverage report
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage_filtered.info
lcov --list coverage_filtered.info

# Generate HTML report
genhtml coverage_filtered.info --output-directory coverage_html

# View in browser
firefox coverage_html/index.html
```

### Coverage Targets

- **Phase 2 Target:** 70% code coverage
- **Long-term Goal:** 80%+ coverage

## Test Structure

### Directory Layout

```
tests/
├── CMakeLists.txt              # Test build configuration
├── README.md                   # This file
├── fixtures/                   # Test data and configurations
│   ├── test_config.json        # Test application config
│   ├── test_schema.sql         # Test database schema
│   └── mock_api_response.json  # Mock API responses
├── mocks/                      # Mock objects (future)
│   └── MockNetworkReply.h      # Mock Qt network responses
├── test_DatabaseManager.cpp    # Database layer tests
├── test_WeatherData.cpp        # Data structure tests
├── test_PythonBridge.cpp       # Python integration tests (TODO)
├── test_WeatherWorker.cpp      # API worker tests (TODO)
└── test_integration.cpp        # End-to-end tests (TODO)
```

### Test Naming Conventions

- **Test Files:** `test_<ClassName>.cpp`
- **Test Class:** `Test<ClassName>`
- **Test Methods:** `test<Functionality>`
- **Example:** `void testConnectionSuccess()`

### Qt Test Macros

```cpp
QVERIFY(condition)                    // Assert true
QVERIFY2(condition, "message")        // Assert with message
QCOMPARE(actual, expected)            // Assert equality
QCOMPARE_EQ(a, b)                     // Assert ==
QCOMPARE_NE(a, b)                     // Assert !=
QCOMPARE_LT(a, b)                     // Assert <
QVERIFY_EXCEPTION_THROWN(expr, Exc)  // Assert exception
```

## Writing New Tests

### Template

```cpp
#include <QtTest/QtTest>
#include <YourClass.h>

class TestYourClass : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() { /* Setup before all tests */ }
    void cleanupTestCase() { /* Cleanup after all tests */ }
    void init() { /* Setup before each test */ }
    void cleanup() { /* Cleanup after each test */ }

    void testYourFeature() {
        // Arrange
        YourClass obj;

        // Act
        bool result = obj.doSomething();

        // Assert
        QVERIFY(result);
    }
};

QTEST_MAIN(TestYourClass)
#include "test_YourClass.moc"
```

### Best Practices

1. **Isolation:** Each test should be independent
2. **Cleanup:** Use `cleanup()` to reset state
3. **Naming:** Clear, descriptive test names
4. **AAA Pattern:** Arrange, Act, Assert
5. **One Assertion Focus:** Each test should verify one behavior
6. **Use Fixtures:** Share common test data via fixtures
7. **Mock External Dependencies:** Use mocks for network, filesystem
8. **Fast Tests:** Keep individual tests under 1 second

## Continuous Integration

### GitHub Actions Integration

Tests run automatically on:
- Every push to `main`, `develop`, `claude/*` branches
- Every pull request to `main`, `develop`

See `.github/workflows/build-and-test.yml` for configuration.

### CI Pipeline

```
┌─────────────────┐
│  Code Push      │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Build Project  │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Run Tests      │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│Generate Coverage│
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Upload to       │
│ Codecov         │
└─────────────────┘
```

## Troubleshooting

### Common Issues

#### 1. "Cannot connect to database"

**Solution:**
```bash
# Verify MySQL is running
sudo systemctl status mysql

# Check test user exists
mysql -u test_user -p -e "SHOW DATABASES;"

# Recreate test database
mysql -u root -p < tests/fixtures/test_schema.sql
```

#### 2. "Qt Test not found"

**Solution:**
```bash
sudo apt-get install qtbase5-dev
# Ensure Qt5::Test is in CMakeLists.txt
```

#### 3. "Tests not built"

**Solution:**
```bash
# Ensure BUILD_TESTS is ON
cmake -DBUILD_TESTS=ON ..
make
ls tests/  # Should see test executables
```

#### 4. "Coverage report empty"

**Solution:**
```bash
# Ensure coverage flags are set
cmake -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ..
make clean && make
ctest
# Then generate coverage
```

## Performance

### Test Execution Times

| Test Suite | Test Count | Avg Duration | Status |
|------------|------------|--------------|--------|
| test_DatabaseManager | 15 tests | ~2-3s | ✅ |
| test_WeatherData | 9 tests | ~0.1s | ✅ |
| test_PythonBridge | TBD | TBD | 🚧 |
| test_WeatherWorker | TBD | TBD | 🚧 |
| **Total** | **24 tests** | **~3s** | **🚧** |

**Target:** All tests should complete in < 10 seconds

## Contributing

### Adding New Tests

1. Create test file: `tests/test_NewClass.cpp`
2. Add to `tests/CMakeLists.txt`:
   ```cmake
   add_weather_test(test_NewClass test_NewClass.cpp)
   ```
3. Write tests following best practices
4. Run locally: `ctest -R NewClass -V`
5. Ensure coverage: `lcov --list coverage.info`
6. Commit with message: `test: Add NewClass unit tests`

### Test Review Checklist

- [ ] All tests pass locally
- [ ] Code coverage maintained/improved
- [ ] No test interdependencies
- [ ] Proper cleanup in `cleanup()` slots
- [ ] Meaningful test names
- [ ] Commented complex test logic
- [ ] Updated this README if needed

## References

- [Qt Test Documentation](https://doc.qt.io/qt-5/qtest-overview.html)
- [Qt Test Tutorial](https://doc.qt.io/qt-5/qtest-tutorial.html)
- [lcov Manual](http://ltp.sourceforge.net/coverage/lcov.php)
- [CTest Documentation](https://cmake.org/cmake/help/latest/manual/ctest.1.html)

## Support

For test-related questions:
1. Check this README
2. Review existing test files for examples
3. Consult Qt Test documentation
4. Open an issue on GitHub

---

**Last Updated:** 2025-11-06
**Test Coverage:** ~70% (Phase 2 target)
**Status:** 🚧 Active Development - Phase 2
