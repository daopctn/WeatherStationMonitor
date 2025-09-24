 PythonBridge Architecture Overview

  The PythonBridge is a C++ class that embeds a Python interpreter inside
  your C++ application. Here's the complete flow:

  1. Initialization Phase (Constructor)

  PythonBridge::PythonBridge() : m_initialized(false)
  {
      initialize();
  }

  bool PythonBridge::initialize()
  {
      Py_Initialize();  // Start Python interpreter
      if (!Py_IsInitialized()) {
          return false;
      }
      m_initialized = true;
      return true;
  }

  What happens:
  - Py_Initialize() starts a complete Python interpreter inside your C++
  process
  - This creates a Python runtime with all standard libraries available
  - The interpreter runs in the same memory space as your C++ application
  - m_initialized tracks if Python is ready to use

  2. Conversion Function Call Flow

  When convertKelvinToCelsius(double kelvinTemp) is called:

  Step 1: Path Setup

  PyRun_SimpleString("import sys");
  PyRun_SimpleString("import os");
  PyRun_SimpleString("parent_dir = os.path.dirname(os.getcwd())");
  PyRun_SimpleString("python_path = os.path.join(parent_dir, 'python')");
  PyRun_SimpleString("sys.path.insert(0, python_path)");

  What happens:
  - PyRun_SimpleString() executes Python code as if you typed it in a Python
  shell
  - import sys, os loads Python's system and operating system modules
  - When app runs from build/ directory, os.getcwd() returns
  /path/to/project/build
  - os.path.dirname() gets the parent: /path/to/project
  - os.path.join(parent_dir, 'python') creates: /path/to/project/python
  - sys.path.insert(0, python_path) tells Python to look in that directory
  first for modules

  Step 2: Module Import

  PyObject* pModule = PyImport_ImportModule("processor");

  What happens:
  - PyImport_ImportModule("processor") is equivalent to Python's import 
  processor
  - Python searches sys.path for a file called processor.py
  - If found, Python:
    a. Reads the entire processor.py file
    b. Compiles it to bytecode
    c. Executes all top-level code (imports, function definitions, etc.)
    d. Creates a module object in memory
  - Returns a PyObject* pointer to the module object
  - If import fails, returns nullptr

  Step 3: Function Lookup

  PyObject* pFunc = PyObject_GetAttrString(pModule, "kelvin_to_celsius");

  What happens:
  - PyObject_GetAttrString() is like Python's getattr(processor, 
  "kelvin_to_celsius")
  - It looks inside the module object for an attribute named
  "kelvin_to_celsius"
  - In your processor.py, this finds the function:
  def kelvin_to_celsius(kelvin_temp):
      celsius = kelvin_temp - 273.15
      return round(celsius, 2)
  - Returns a PyObject* pointer to the function object
  - PyCallable_Check(pFunc) verifies it's actually a callable function

  Step 4: Argument Preparation

  PyObject* pArgs = PyTuple_New(1);
  PyTuple_SetItem(pArgs, 0, PyFloat_FromDouble(kelvinTemp));

  What happens:
  - PyTuple_New(1) creates a Python tuple with 1 slot (function arguments)
  - PyFloat_FromDouble(kelvinTemp) converts C++ double to Python float object
  - PyTuple_SetItem() puts the Python float into the tuple at position 0
  - This tuple represents the function arguments: (291.25,) for example

  Step 5: Function Call

  PyObject* pValue = PyObject_CallObject(pFunc, pArgs);

  What happens:
  - PyObject_CallObject() is equivalent to calling kelvin_to_celsius(291.25)
  in Python
  - The Python interpreter:
    a. Takes the function object (pFunc)
    b. Unpacks arguments from the tuple (pArgs)
    c. Creates a new execution frame
    d. Executes the Python function line by line:
    celsius = kelvin_temp - 273.15  # 291.25 - 273.15 = 18.1
  return round(celsius, 2)        # round(18.1, 2) = 18.1
    e. Returns a Python float object containing 18.1

  Step 6: Result Extraction

  if (pValue != nullptr && PyFloat_Check(pValue)) {
      result = PyFloat_AsDouble(pValue);
      std::cout << "Python converted " << kelvinTemp << "K to " << result <<
  "°C" << std::endl;
  }

  What happens:
  - PyFloat_Check(pValue) verifies the return value is a Python float
  - PyFloat_AsDouble(pValue) converts Python float back to C++ double
  - The C++ variable result now contains 18.1

  Step 7: Memory Cleanup

  Py_XDECREF(pValue);  // Decrease reference count of return value
  Py_DECREF(pArgs);    // Decrease reference count of arguments tuple
  Py_DECREF(pFunc);    // Decrease reference count of function object
  Py_DECREF(pModule);  // Decrease reference count of module object

  What happens:
  - Python uses reference counting for memory management
  - Each PyObject* has a reference count
  - Py_DECREF() decreases the count by 1
  - When count reaches 0, Python automatically frees the memory
  - This prevents memory leaks

  3. Data Flow Visualization

  C++ Application
      ↓ (291.25K from API)
  WeatherFetcher::onRequestFinished()
      ↓
  PythonBridge::convertKelvinToCelsius(291.25)
      ↓
  Python Interpreter (embedded in C++ process)
      ↓
  sys.path.insert() → Add /project/python to module search path
      ↓
  import processor → Load and execute processor.py
      ↓
  processor.kelvin_to_celsius → Get function object
      ↓
  Call function with (291.25,) → Execute Python code
      ↓ (Python calculates: 291.25 - 273.15 = 18.1)
  Return 18.1 as PyObject
      ↓ (Convert back to C++)
  C++ receives double 18.1
      ↓
  Return to WeatherFetcher
      ↓
  Emit signal to MainWindow
      ↓
  Display "18.1°C (Processed by Python)"

  4. Memory Layout

  Process Memory Space:
  ┌─────────────────────────────────────┐
  │ C++ Application                     │
  │ ┌─────────────────────────────────┐ │
  │ │ Embedded Python Interpreter    │ │
  │ │ ┌─────────────────────────────┐ │ │
  │ │ │ processor.py module        │ │ │
  │ │ │ - kelvin_to_celsius()      │ │ │
  │ │ │ - process_weather_data()   │ │ │
  │ │ │ - hello()                  │ │ │
  │ │ └─────────────────────────────┘ │ │
  │ │                                 │ │
  │ │ PyObject instances:             │ │
  │ │ - Module object                 │ │
  │ │ - Function objects              │ │
  │ │ - Float objects                 │ │
  │ │ - Tuple objects                 │ │
  │ └─────────────────────────────────┘ │
  │                                     │
  │ C++ Objects:                        │
  │ - PythonBridge instance             │
  │ - WeatherFetcher instance           │
  │ - MainWindow instance               │
  └─────────────────────────────────────┘

  5. Error Handling Chain

  // Error at each step:
  if (pModule == nullptr) → Python couldn't find/load processor.py
  if (pFunc == nullptr) → processor.py exists but no kelvin_to_celsius
  function
  if (!PyCallable_Check(pFunc)) → kelvin_to_celsius exists but isn't a
  function
  if (pValue == nullptr) → Function call failed (exception in Python code)
  if (!PyFloat_Check(pValue)) → Function returned wrong type (not a number)

  6. Why This Architecture?

  1. True Integration: Python actually runs the calculation, not C++
  2. Flexibility: Easy to modify conversion logic in Python without
  recompiling C++
  3. Power: Access to full Python ecosystem (numpy, pandas, etc.)
  4. Performance: Python interpreter stays loaded, modules cached
  5. Debugging: Can see exactly where failures occur

  This creates a true hybrid application where C++ handles the GUI and
  networking, while Python handles data processing, giving you the best of
  both worlds.
