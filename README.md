
## **SPRINT 0: Environment Setup (Day 1-3)**

### **Day 1: Development Tools Installation**

**Morning (2-3 hours):**
1. Download and install Visual Studio 2019/2022 Community
2. Download Qt Online Installer from qt.io
3. During Qt installation, select:
   - Qt 5.15.2 (LTS version)
   - MinGW 8.1.0 64-bit compiler
   - Qt Creator (optional)
   - Sources (optional)

**Afternoon (2-3 hours):**
1. Install Python 3.9 or 3.10 (not latest - for compatibility)
2. Install MySQL Community Server 8.0
3. Install MySQL Workbench
4. Install VS Code
5. Install CMake (3.16 or higher)
6. Install Git

**Evening (1 hour):**
1. Verify all installations:
   - Open Qt Designer
   - Run python --version
   - Connect to MySQL
   - Open VS Code
s
### **Day 2: VS Code Configuration**

**Morning (2 hours):**
1. Open VS Code
2. Install extensions:
   - C/C++ (Microsoft)
   - CMake Tools
   - Python
   - Qt Configure
   - MySQL (Weijan Chen)
   - GitLens

**Afternoon (3 hours):**
1. Create main project folder: `WeatherStationMonitor`
2. Open folder in VS Code
3. Initialize Git repository (Terminal: `git init`)
4. Create `.gitignore` file
5. Add to .gitignore:
   - build/
   - *.user
   - __pycache__/
   - .vscode/settings.json (local settings)
   - config/api_keys.json

**Evening (1 hour):**
1. Configure VS Code settings for Qt
2. Test: Create dummy .ui file and open with Designer
3. Set up Python interpreter in VS Code
4. Create initial README.md

### **Day 3: Project Structure & Database**

**Morning (2 hours):**
1. Create folder structure:
   ```
   WeatherStationMonitor/
   ├── src/
   ├── include/
   ├── python/
   ├── ui/
   ├── resources/
   ├── database/
   └── docs/
   ```

2. Create CMakeLists.txt (empty for now)
3. Create src/main.cpp (empty)
4. Commit: "Initial project structure"

**Afternoon (3 hours):**
1. Open MySQL Workbench
2. Create new database: `weather_monitor_db`
3. Design tables (on paper first):
   - stations table
   - weather_data table
   - settings table
4. Create SQL script file in database/schema.sql
5. Execute script to create tables
6. Test connection from MySQL Workbench

**Evening (1 hour):**
1. Create database/sample_data.sql
2. Insert 5 test stations (Hanoi, Saigon, etc.)
3. Document database structure in docs/database.md
4. Commit: "Database schema created"

---

## **SPRINT 1: Walking Skeleton (Day 4-10)**

### **Day 4: Basic Qt Application**

**Morning (3 hours):**  ✅
1. Open Qt Designer 
2. Create new Main Window form
3. Add basic widgets:
   - One QLabel (for temperature)
   - One QPushButton (for refresh)
4. Save as ui/mainwindow.ui
5. Close Designer

**Afternoon (3 hours):** ✅
1. Write CMakeLists.txt configuration
2. Add Qt5 find_package
3. Add source files
4. Configure VS Code CMake
5. Build empty application
6. See window appear (even if empty)

**Evening:** ✅
1. Commit: "Basic Qt window created"
2. Create Sprint 1 plan in docs/sprints/sprint1.md

### **Day 5: API Connection**

**Morning (2 hours):**✅
1. Sign up for OpenWeatherMap account ✅
2. Get free API key ✅
3. Test API in browser ✅
4. Create resources/config/ folder ✅
5. Save API key in config file (git ignored) ✅

**Afternoon (3 hours):** ✅
1. Research Qt Network module ✅
2. Create weatherfetcher class files ✅
3. Implement basic HTTP GET request ✅
4. Parse JSON response ✅
5. Display raw temperature in QLabel ✅

**Evening:**
1. Test with real API✅
2. Handle errors (no internet, wrong API key) ✅
3. Commit: "API connection working" ✅

### **Day 6: Python Integration**

**Morning (3 hours):**✅
1. Create python/requirements.txt✅
2. Add: numpy, pandas (simple libraries)✅
3. Create virtual environment in python/venv/✅
4. Install requirements✅
5. Create python/processor.py (empty)✅

**Afternoon (3 hours):**✅
1. Research Python C API ✅
2. Add Python to CMakeLists.txt✅
3. Create PythonBridge class✅
4. Test: Call Python print from C++✅
5. Pass data to Python and get result back✅

**Evening:**
1. Display Python result in UI ✅
2. Commit: "Python integration complete"✅

### **Day 7: MySQL Connection**

**Morning (2 hours):**✅
1. Add Qt SQL module to CMakeLists.txt✅
2. Create DatabaseManager class✅
3. Implement connection to MySQL✅
4. Test connection✅
5. Handle connection errors✅

**Afternoon (2 hours):**
1. Implement insert weather data ✅
2. Implement select recent data✅
3. Create simple query functions✅
4. Test with MySQL Workbench✅

**Evening:**
1. Integration test: Full flow working✅
2. Commit: "Database connection established"✅

### **Day 8-9: Integration & Polish**

**Full days:**
1. Connect all components✅
2. Fetch → Store → Process → Display✅
3. Add refresh timer (60 seconds)✅
4. Error handling✅
5. Loading indicators✅
6. Code cleanup✅
7. Documentation update✅

### **Day 10: Sprint 1 Review**

1. Test complete flow✅
2. Make demo video/screenshots✅
3. List completed features✅
4. List issues/bugs✅
5. Plan Sprint 2✅
6. Tag release: v0.1.0✅

---

## **SPRINT 2: Multiple Stations (Day 11-17)**

### **Day 11: UI Redesign**

**Morning:**
1. Open Qt Designer
2. Replace single label with QTableWidget
3. Design table columns (City, Temp, Humidity)
4. Add status bar
5. Save and rebuild

**Afternoon:**
1. Populate table with dummy data
2. Style table headers
3. Add row colors
4. Test UI responsiveness

### **Day 12: Multi-threading**

**Morning:**
1. Design thread architecture (on paper)
2. Create ThreadManager class
3. Implement QThread for fetching
4. One thread per station

**Afternoon:**
1. Test concurrent fetching
2. Handle thread synchronization
3. Update UI from threads safely
4. Test with 5 stations

### **Day 13: Batch Processing**

1. Modify Python to handle arrays
2. Calculate statistics for all stations
3. Return min/max/average
4. Display in status bar
5. Add more stations (10+)

### **Day 14-15: Data Management**

1. Implement data caching
2. Batch insert to database
3. Query optimizations
4. History tracking
5. Memory management

### **Day 16-17: Sprint 2 Completion**

1. Bug fixes
2. Performance testing
3. Documentation
4. Sprint review
5. Tag release: v0.2.0

---

## **SPRINT 3: Data Visualization (Day 18-24)**

### **Day 18: Statistics Panel**

1. Design statistics UI in Designer
2. Add QGroupBox for stats
3. Create labels for metrics
4. Layout management

### **Day 19: Python Analytics**

1. Implement statistical functions
2. Add trend detection
3. Calculate percentiles
4. Identify outliers

### **Day 20: Real-time Updates**

1. Implement observer pattern
2. Auto-refresh UI
3. Smooth animations
4. Color coding by temperature

### **Day 21-22: Charts**

1. Research Qt Charts or QCustomPlot
2. Add chart widget
3. Display temperature timeline
4. Update charts from Python data

### **Day 23-24: Polish**

1. UI improvements
2. Bug fixes
3. Performance optimization
4. Sprint review
5. Tag release: v0.3.0

---

## **SPRINT 4: Map & Polish (Day 25-31)**

### **Day 25-26: Map Integration**

1. Research map widgets
2. Implement map view
3. Place station markers
4. Click interactions

### **Day 27-28: UI Theme**

1. Create QSS stylesheet
2. Modern dark/light theme
3. Icons and resources
4. Splash screen

### **Day 29-30: Settings & Preferences**

1. Settings dialog
2. Save user preferences
3. Station management
4. API key configuration

### **Day 31: Final Release**

1. Complete testing
2. Create installer
3. Documentation
4. Tag release: v1.0.0

---

## **Daily Routine (Throughout Project):**

**Morning Standup (5 min):**
- What did I complete yesterday?
- What will I do today?
- Any blockers?

**Evening Commit (10 min):**
- Git add, commit with meaningful message
- Update documentation if needed
- Note tomorrow's tasks

**Weekly Review:**
- Progress assessment
- Backlog update
- Next week planning

---

## **Key Milestones:**

✅ **Day 3:** Environment ready
✅ **Day 10:** Working prototype (Sprint 1)
✅ **Day 17:** Multi-station monitoring (Sprint 2)
✅ **Day 24:** Analytics working (Sprint 3)
✅ **Day 31:** Polished release (Sprint 4)

---

## **Important Notes:**

1. **Don't skip setup days** - they save time later
2. **Commit daily** - even small progress
3. **Test after each feature** - don't accumulate bugs
4. **Document as you go** - not at the end
5. **If blocked over 2 hours** - move to next task
6. **Keep scope small** - better to finish less than fail more

Ready to start Day 1, sir?