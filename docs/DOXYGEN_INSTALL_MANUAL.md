# Doxygen Installation Guide (Manual)

## ⚠️ Issue: Chocolatey Requires Admin Rights

The automated installation failed due to insufficient permissions:
```
Access to the path 'C:\ProgramData\chocolatey\lib-bad' is denied
```

## 🔧 Solution Options

### Option 1: Run PowerShell as Administrator (Recommended)
```powershell
# 1. Close current terminal
# 2. Right-click PowerShell → "Run as Administrator"
# 3. Navigate to project directory
cd C:\code\telemetry-platform

# 4. Install Doxygen + Graphviz
choco install doxygen.install graphviz -y

# 5. Verify installation
doxygen --version
dot -V

# 6. Generate documentation
cmake -B build_docs -DBUILD_DOCS=ON
cmake --build build_docs --target docs
start docs/doxygen/html/index.html
```

### Option 2: Manual Download (No Admin Required)
```powershell
# 1. Download Doxygen (Portable)
# URL: https://www.doxygen.nl/download.html
# File: doxygen-1.15.0-portable.zip (~20MB)

# 2. Download Graphviz (Portable)
# URL: https://graphviz.org/download/
# File: windows_10_cmake_Release_graphviz-install-12.2.1-win64.exe

# 3. Extract to user directory
mkdir C:\Users\$env:USERNAME\tools
Expand-Archive -Path doxygen-1.15.0-portable.zip -DestinationPath C:\Users\$env:USERNAME\tools\doxygen

# 4. Add to PATH (session-only)
$env:PATH += ";C:\Users\$env:USERNAME\tools\doxygen"
$env:PATH += ";C:\Program Files\Graphviz\bin"

# 5. Verify
doxygen --version
dot -V

# 6. Generate documentation
doxygen Doxyfile
start docs\doxygen\html\index.html
```

### Option 3: Use Pre-generated Documentation (Skip Install)
If you're only reviewing documentation:
```powershell
# Ask team member to generate and share docs/doxygen/ directory
# Or view online documentation (if hosted)
# Or use GitHub Actions to auto-generate (see CI/CD section)
```

### Option 4: Scoop Package Manager (Alternative)
```powershell
# Install Scoop (no admin required)
irm get.scoop.sh | iex

# Install Doxygen
scoop install doxygen graphviz

# Verify
doxygen --version
dot -V
```

## 📝 Manual Installation Steps (Detailed)

### Step 1: Download Doxygen
1. Visit: https://www.doxygen.nl/download.html
2. Download: `doxygen-1.15.0.windows.x64.bin.zip` (Portable Version)
3. Extract to: `C:\Users\YourName\doxygen\`

### Step 2: Download Graphviz
1. Visit: https://graphviz.org/download/
2. Download: `stable_windows_10_cmake_Release_x64_graphviz-install-12.2.1-win64.exe`
3. Install to: `C:\Program Files\Graphviz\` (or custom location)

### Step 3: Configure PATH
```powershell
# Temporary (session-only)
$env:PATH += ";C:\Users\$env:USERNAME\doxygen"
$env:PATH += ";C:\Program Files\Graphviz\bin"

# Permanent (requires restart)
[System.Environment]::SetEnvironmentVariable(
    "Path",
    $env:Path + ";C:\Users\$env:USERNAME\doxygen;C:\Program Files\Graphviz\bin",
    [System.EnvironmentVariableTarget]::User
)
```

### Step 4: Verify Installation
```powershell
# Check Doxygen
doxygen --version
# Expected: 1.15.0

# Check Graphviz
dot -V
# Expected: dot - graphviz version 12.2.1

# Test diagram generation
echo "digraph G { A -> B; }" | dot -Tpng -o test.png
# Should create test.png without errors
```

### Step 5: Generate Documentation
```powershell
# Option A: Using CMake
cmake -B build_docs -DBUILD_DOCS=ON
cmake --build build_docs --target docs

# Option B: Using Doxyfile directly
doxygen Doxyfile

# Open documentation
start docs\doxygen\html\index.html
```

## 🐛 Troubleshooting

### Error: "doxygen: command not found"
**Solution**:
```powershell
# Check if installed
Get-Command doxygen

# If not found, add to PATH
$env:PATH += ";C:\path\to\doxygen"

# Verify
doxygen --version
```

### Error: "Graphviz not found"
**Solution**:
```powershell
# Doxygen will work but without diagrams
# To enable diagrams, install Graphviz

# Check if installed
Get-Command dot

# If not found, add to PATH
$env:PATH += ";C:\Program Files\Graphviz\bin"
```

### Error: "Permission denied" (Chocolatey)
**Solution**:
- Run PowerShell as Administrator
- Or use manual download (Option 2 above)
- Or use Scoop (Option 4 above)

### Error: "Unable to obtain lock file access"
**Solution**:
```powershell
# Remove stale lock file
Remove-Item -Force "C:\ProgramData\chocolatey\lib\5b9a57160ae7d851ff69258c26283fbc2355522e"

# Retry installation
choco install doxygen.install graphviz -y
```

## ✅ Verification Checklist

After installation:
- [ ] `doxygen --version` shows version 1.15.0+
- [ ] `dot -V` shows Graphviz version
- [ ] `doxygen Doxyfile` runs without errors
- [ ] `docs/doxygen/html/index.html` is generated
- [ ] Call graphs appear in documentation (requires Graphviz)
- [ ] Class diagrams render correctly

## 📚 Next Steps

1. **Document Code**: Add Doxygen comments to key classes
   - `common/include/telemetry_common/proto_adapter.h`
   - `common/include/telemetry_common/redis_client.h`
   - `processing/include/bounded_queue.h`

2. **Regenerate Docs**: After adding comments
   ```powershell
   doxygen Doxyfile
   ```

3. **Review Output**: Check HTML documentation
   ```powershell
   start docs\doxygen\html\index.html
   ```

4. **Iterate**: Add more examples, cross-references, diagrams

---

**Installation Status**: ⏳ Pending (requires admin rights or manual download)  
**Workaround**: Continue with code documentation, generate docs later  
**Target**: Complete Doxygen setup before end of Day 3
