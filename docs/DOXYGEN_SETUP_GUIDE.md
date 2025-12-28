# Doxygen Documentation Setup Guide

## 📚 Overview
TelemetryHub uses **Doxygen** for automatic API documentation generation from C++ source code comments.

---

## 🎯 Quick Start

### Option 1: Automatic (CMake Integration)
```powershell
# Configure with documentation enabled
cmake -B build_docs -DBUILD_DOCS=ON

# Generate documentation
cmake --build build_docs --target docs

# Open documentation
start docs/doxygen/html/index.html
```

### Option 2: Manual (Doxyfile)
```powershell
# Install Doxygen (if not installed)
choco install doxygen.install graphviz

# Generate documentation
doxygen Doxyfile

# Open documentation
start docs/doxygen/html/index.html
```

---

## 📦 Installation

### Windows (Chocolatey - Recommended)
```powershell
# Install Doxygen + Graphviz (for diagrams)
choco install doxygen.install graphviz

# Verify installation
doxygen --version
dot -V
```

### Windows (Manual Download)
1. Download Doxygen: https://www.doxygen.nl/download.html
2. Download Graphviz: https://graphviz.org/download/
3. Install both with default options
4. Add to PATH:
   - `C:\Program Files\doxygen\bin`
   - `C:\Program Files\Graphviz\bin`

### Linux
```bash
# Ubuntu/Debian
sudo apt-get install doxygen graphviz

# Fedora/RHEL
sudo dnf install doxygen graphviz

# Arch
sudo pacman -S doxygen graphviz
```

### macOS
```bash
brew install doxygen graphviz
```

---

## 🔧 Configuration

### CMake Integration
The project is configured in [CMakeLists.txt](../CMakeLists.txt):

```cmake
option(BUILD_DOCS "Build API documentation with Doxygen" OFF)

if(BUILD_DOCS)
    find_package(Doxygen REQUIRED)
    doxygen_add_docs(docs
        common/include
        common/src
        ingestion/gateway/include
        ingestion/gateway/src
        processing/include
        processing/src
        README.md
        docs/architecture.md
        COMMENT "Generating API documentation"
    )
endif()
```

### Doxyfile Configuration
Key settings in [Doxyfile](../Doxyfile):

| Setting | Value | Description |
|---------|-------|-------------|
| `PROJECT_NAME` | TelemetryHub | Project name in docs |
| `PROJECT_NUMBER` | v0.2.0 | Version number |
| `OUTPUT_DIRECTORY` | docs/doxygen | Output location |
| `EXTRACT_ALL` | YES | Document all code (even without comments) |
| `SOURCE_BROWSER` | YES | Include source code |
| `CALL_GRAPH` | YES | Generate call graphs |
| `CALLER_GRAPH` | YES | Generate caller graphs |
| `HAVE_DOT` | YES | Enable Graphviz diagrams |
| `GENERATE_HTML` | YES | HTML output |
| `GENERATE_LATEX` | YES | LaTeX/PDF output |

---

## ✍️ Writing Documentation Comments

### Class Documentation
```cpp
/**
 * @brief Redis client wrapper for telemetry data operations
 * 
 * Provides high-level interface for storing and retrieving telemetry
 * data from Redis. Implements connection pooling and automatic
 * reconnection on failure.
 * 
 * @details
 * This class wraps redis++ library with TelemetryHub-specific
 * functionality including:
 * - Automatic JSON serialization
 * - Queue operations (RPUSH, BLPOP)
 * - TTL management
 * - Error handling with logging
 * 
 * @note Thread-safe when using separate instances per thread
 * @warning Do not share RedisClient across threads without synchronization
 * 
 * @see RedisConfig
 * @see ProtoAdapter
 * 
 * @author TelemetryHub Team
 * @date 2025-12-28
 * @version 0.2.0
 */
class RedisClient {
public:
    /**
     * @brief Construct Redis client with configuration
     * @param config Redis connection configuration
     * @throws std::runtime_error if connection fails
     */
    explicit RedisClient(const RedisConfig& config);
    
    /**
     * @brief Get value by key
     * @param key Redis key to retrieve
     * @return Optional value (nullopt if key doesn't exist)
     * @throws RedisError on connection failure
     */
    std::optional<std::string> get(const std::string& key);
    
private:
    std::unique_ptr<sw::redis::Redis> redis_; ///< Redis connection handle
    RedisConfig config_; ///< Client configuration
};
```

### Function Documentation
```cpp
/**
 * @brief Convert TelemetryData to Protobuf binary format
 * 
 * Serializes a TelemetryData struct into efficient Protobuf binary
 * representation. This is ~10x faster than JSON serialization and
 * produces 30% smaller output.
 * 
 * @param data Telemetry data to serialize
 * @return Binary serialized data
 * 
 * @pre data.device_id must not be empty
 * @post Return value contains valid Protobuf binary
 * 
 * @throws std::invalid_argument if data.device_id is empty
 * @throws std::runtime_error if serialization fails
 * 
 * @code
 * TelemetryData data;
 * data.device_id = "sensor-001";
 * data.temperature = 25.5;
 * auto binary = toProtobuf(data);
 * // binary.size() ~= 30 bytes
 * @endcode
 * 
 * @see fromProtobuf()
 * @see TelemetryData
 */
std::string toProtobuf(const TelemetryData& data);
```

### Common Tags Reference

| Tag | Purpose | Example |
|-----|---------|---------|
| `@brief` | Short description (1 line) | `@brief Redis client wrapper` |
| `@details` | Long description | `@details This class provides...` |
| `@param` | Parameter description | `@param key Redis key to retrieve` |
| `@return` | Return value description | `@return Optional value` |
| `@throws` | Exception documentation | `@throws std::runtime_error on error` |
| `@pre` | Precondition | `@pre key must not be empty` |
| `@post` | Postcondition | `@post Connection is established` |
| `@note` | Additional note | `@note Thread-safe` |
| `@warning` | Warning for users | `@warning Not reentrant` |
| `@see` | Cross-reference | `@see RedisConfig` |
| `@code` | Code example | `@code auto x = func(); @endcode` |
| `@todo` | Future work | `@todo Add connection pooling` |
| `@bug` | Known bug | `@bug Memory leak on disconnect` |
| `@deprecated` | Deprecated API | `@deprecated Use newFunc() instead` |
| `@since` | Version added | `@since v0.2.0` |
| `@author` | Author name | `@author John Doe` |
| `@date` | Date | `@date 2025-12-28` |
| `@version` | Version | `@version 1.0.0` |

---

## 📊 Generated Output

### HTML Documentation
Located at: `docs/doxygen/html/index.html`

**Features**:
- ✅ Full class hierarchy with inheritance diagrams
- ✅ Call graphs (who calls this function?)
- ✅ Caller graphs (who does this function call?)
- ✅ Collaboration diagrams (class relationships)
- ✅ Include dependency graphs
- ✅ File structure tree
- ✅ Full-text search
- ✅ Source code browser with syntax highlighting

### PDF Documentation
Located at: `docs/doxygen/latex/refman.pdf`

**To generate PDF**:
```powershell
# After running doxygen
cd docs/doxygen/latex
pdflatex refman.tex
# Or use make (if installed)
make
```

---

## 🎨 Customization

### Modern Theme (Optional)
Install [doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css):

```powershell
# Clone theme
git clone https://github.com/jothepro/doxygen-awesome-css.git docs/doxygen-awesome

# Update Doxyfile
# HTML_EXTRA_STYLESHEET = docs/doxygen-awesome/doxygen-awesome.css

# Regenerate
doxygen Doxyfile
```

### Custom Header/Footer
```doxyfile
# In Doxyfile
HTML_HEADER = docs/doxygen-header.html
HTML_FOOTER = docs/doxygen-footer.html
HTML_STYLESHEET = docs/custom-style.css
```

---

## 🚀 CI/CD Integration

### GitHub Actions
```yaml
# .github/workflows/docs.yml
name: Generate Documentation

on:
  push:
    branches: [main, master]
    paths:
      - '**.h'
      - '**.hpp'
      - '**.cpp'
      - 'Doxyfile'
      - 'README.md'

jobs:
  docs:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      
      - name: Install Doxygen
        run: sudo apt-get install -y doxygen graphviz
      
      - name: Generate Documentation
        run: doxygen Doxyfile
      
      - name: Deploy to GitHub Pages
        uses: peaceiris/actions-gh-pages@v3
        with:
          github_token: ${{ secrets.GITHUB_TOKEN }}
          publish_dir: ./docs/doxygen/html
          publish_branch: gh-pages
```

### Read the Docs (Optional Future)
For integration with Sphinx + Breathe:
```yaml
# .readthedocs.yml
version: 2
sphinx:
  configuration: docs/conf.py
```

---

## 📖 Best Practices

### 1. Comment Every Public API
```cpp
// ❌ Bad: No documentation
class RedisClient {
public:
    bool connect();
};

// ✅ Good: Clear documentation
/**
 * @brief Redis client for telemetry data
 */
class RedisClient {
public:
    /**
     * @brief Establish connection to Redis server
     * @return true if connection successful, false otherwise
     */
    bool connect();
};
```

### 2. Include Usage Examples
```cpp
/**
 * @brief Queue telemetry data for processing
 * 
 * @param data Telemetry data to enqueue
 * @return true if enqueued successfully
 * 
 * @code
 * TelemetryData data;
 * data.device_id = "sensor-001";
 * data.temperature = 25.5;
 * 
 * RedisClient client(config);
 * if (client.connect()) {
 *     client.enqueue(data);
 * }
 * @endcode
 */
bool enqueue(const TelemetryData& data);
```

### 3. Document Complex Algorithms
```cpp
/**
 * @brief Priority-based task scheduling
 * 
 * @details
 * Uses a binary heap to maintain tasks in priority order.
 * Time complexity:
 * - enqueue: O(log n)
 * - dequeue: O(log n)
 * - peek: O(1)
 * 
 * Space complexity: O(n) where n is number of tasks
 * 
 * @algorithm
 * 1. Insert task at end of heap
 * 2. Bubble up while priority > parent priority
 * 3. Maintain heap invariant at all times
 */
void enqueueTask(const Task& task);
```

### 4. Use Grouping
```cpp
/**
 * @defgroup redis_operations Redis Operations
 * @brief High-level Redis operations for telemetry
 * @{
 */

void set(const std::string& key, const std::string& value);
std::string get(const std::string& key);
void del(const std::string& key);

/** @} */ // end of redis_operations
```

---

## 🐛 Troubleshooting

### Issue: "Doxygen not found"
**Solution**:
```powershell
# Check if installed
doxygen --version

# If not found, install
choco install doxygen.install

# Restart terminal
```

### Issue: "Dot not found" (No diagrams)
**Solution**:
```powershell
# Install Graphviz
choco install graphviz

# Verify
dot -V

# Update PATH if needed
$env:PATH += ";C:\Program Files\Graphviz\bin"
```

### Issue: "LaTeX not found" (No PDF)
**Solution**:
```powershell
# Install MiKTeX (LaTeX for Windows)
choco install miktex

# Or disable PDF generation
# In Doxyfile: GENERATE_LATEX = NO
```

### Issue: Slow generation (>5 minutes)
**Solution**:
```doxyfile
# In Doxyfile, optimize settings:
EXTRACT_ALL = NO          # Only document commented code
SOURCE_BROWSER = NO       # Disable source browser
DOT_GRAPH_MAX_NODES = 30  # Reduce graph complexity
MAX_DOT_GRAPH_DEPTH = 2   # Limit graph depth
```

---

## 📚 Additional Resources

### Official Documentation
- Doxygen Manual: https://www.doxygen.nl/manual/
- Doxygen Tags: https://www.doxygen.nl/manual/commands.html
- CMake Doxygen: https://cmake.org/cmake/help/latest/module/FindDoxygen.html

### Examples & Tutorials
- Google Style Guide: https://google.github.io/styleguide/cppguide.html#Comments
- LLVM Doxygen: https://llvm.org/doxygen/
- Qt Documentation: https://doc.qt.io/qt-6/

### Alternative Tools
- Sphinx + Breathe: Modern alternative (see [DOCUMENTATION_TOOLS_COMPARISON.md](DOCUMENTATION_TOOLS_COMPARISON.md))
- Mkdocs-Material: Beautiful static site generator
- Standardese: Modern C++ documentation tool

---

## ✅ Verification Checklist

After generating documentation:

- [ ] HTML index page loads correctly (`docs/doxygen/html/index.html`)
- [ ] Class hierarchy diagram shows inheritance relationships
- [ ] Call graphs are generated (requires Graphviz)
- [ ] Source browser shows syntax-highlighted code
- [ ] Search function works
- [ ] All public classes are documented
- [ ] All public methods have `@brief` descriptions
- [ ] Code examples compile and run
- [ ] Cross-references (`@see`) link correctly
- [ ] PDF generates without errors (if GENERATE_LATEX=YES)

---

**Last Updated**: Day 3 Morning (2025-12-28)  
**Status**: ✅ Doxygen configured and ready to use  
**Next Steps**: Document ProtoAdapter, RedisClient, BoundedQueue with examples
