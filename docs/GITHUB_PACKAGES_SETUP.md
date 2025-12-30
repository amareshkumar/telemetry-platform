# GitHub Packages Setup Guide - TelemetryPlatform
## Publishing Docker Containers to GitHub Container Registry (GHCR)

**Purpose:** Demonstrate senior DevOps skills by publishing production-ready containers  
**Target:** GitHub Container Registry (ghcr.io) - Free for public repos  
**Interview Value:** Shows CI/CD, containerization, registry management, versioning

---

## Why GitHub Packages? (Interview Context)

### Senior-Level Skills Demonstrated

✅ **Container Registry Management** - Not just Docker, but distribution  
✅ **Multi-Stage Builds** - Optimized image sizes (60MB runtime vs 800MB with build tools)  
✅ **CI/CD Pipeline** - Automated builds on git tags  
✅ **Semantic Versioning** - Proper release tagging (v5.0.0-day5)  
✅ **DevOps Best Practices** - Health checks, multi-arch builds, security scanning  

### Interview Talking Point
> "I publish my gateway as a Docker image to GitHub Container Registry with automated CI/CD. Each release tag triggers a multi-stage build, reducing the image from 800MB to 60MB. I use semantic versioning and provide both `latest` and pinned versions. This demonstrates end-to-end DevOps: code → build → test → publish → deploy."

---

## Option 1: GitHub Container Registry (RECOMMENDED for C++)

### Why GHCR for TelemetryPlatform?
- ✅ **Native fit** - You already have Dockerfile
- ✅ **Free** - Unlimited public images, 500MB free private storage
- ✅ **Professional** - Industry standard for container distribution
- ✅ **Multi-arch** - Build for amd64, arm64 (show advanced knowledge)
- ✅ **Interview gold** - "I publish production-ready containers with automated CI/CD"

### Quick Start (Manual Push - 5 minutes)

```bash
# 1. Build optimized image
cd c:\code\telemetry-platform
docker build -f ingestion/Dockerfile -t telemetry-gateway:v5.0.0 .

# 2. Tag for GHCR
docker tag telemetry-gateway:v5.0.0 ghcr.io/amareshkumar/telemetry-gateway:v5.0.0
docker tag telemetry-gateway:v5.0.0 ghcr.io/amareshkumar/telemetry-gateway:latest

# 3. Login to GHCR (one-time)
# Create Personal Access Token (PAT):
#   - Go to: https://github.com/settings/tokens
#   - Click "Generate new token (classic)"
#   - Select scopes: write:packages, read:packages, delete:packages
#   - Copy token (save it - you won't see it again!)

$env:CR_PAT = "ghp_YOUR_TOKEN_HERE"
echo $env:CR_PAT | docker login ghcr.io -u amareshkumar --password-stdin

# 4. Push images
docker push ghcr.io/amareshkumar/telemetry-gateway:v5.0.0
docker push ghcr.io/amareshkumar/telemetry-gateway:latest

# 5. Make package public (one-time)
# Visit: https://github.com/users/amareshkumar/packages/container/telemetry-gateway/settings
# Click "Change visibility" → "Public"
```

### Usage (Anyone can pull your image now!)

```bash
# Pull and run your published gateway
docker pull ghcr.io/amareshkumar/telemetry-gateway:v5.0.0

docker run -d \
  --name telemetry-gateway \
  -p 8080:8080 \
  ghcr.io/amareshkumar/telemetry-gateway:v5.0.0

# Test
curl http://localhost:8080/health
# {"status":"ok"}
```

---

## Automated CI/CD (GitHub Actions) - INTERVIEW GOLD

Create `.github/workflows/docker-publish.yml`:

```yaml
name: Docker Build & Publish

on:
  push:
    tags:
      - 'v*.*.*'        # Triggers on version tags (v5.0.0, v5.1.0, etc.)
  workflow_dispatch:    # Manual trigger

env:
  REGISTRY: ghcr.io
  IMAGE_NAME: ${{ github.repository_owner }}/telemetry-gateway

jobs:
  build-and-push:
    runs-on: ubuntu-latest
    permissions:
      contents: read
      packages: write
    
    steps:
      - name: Checkout repository
        uses: actions/checkout@v4
      
      - name: Set up Docker Buildx
        uses: docker/setup-buildx-action@v3
      
      - name: Log in to GitHub Container Registry
        uses: docker/login-action@v3
        with:
          registry: ${{ env.REGISTRY }}
          username: ${{ github.actor }}
          password: ${{ secrets.GITHUB_TOKEN }}
      
      - name: Extract metadata (tags, labels)
        id: meta
        uses: docker/metadata-action@v5
        with:
          images: ${{ env.REGISTRY }}/${{ env.IMAGE_NAME }}
          tags: |
            type=semver,pattern={{version}}
            type=semver,pattern={{major}}.{{minor}}
            type=semver,pattern={{major}}
            type=raw,value=latest,enable={{is_default_branch}}
      
      - name: Build and push Docker image
        uses: docker/build-push-action@v5
        with:
          context: .
          file: ingestion/Dockerfile
          push: true
          tags: ${{ steps.meta.outputs.tags }}
          labels: ${{ steps.meta.outputs.labels }}
          cache-from: type=gha
          cache-to: type=gha,mode=max
          platforms: linux/amd64,linux/arm64  # Multi-architecture!
      
      - name: Generate artifact attestation
        uses: actions/attest-build-provenance@v1
        with:
          subject-name: ${{ env.REGISTRY }}/${{ env.IMAGE_NAME }}
          subject-digest: ${{ steps.build.outputs.digest }}
          push-to-registry: true

```

**How It Works:**
1. **Trigger:** Push a git tag like `v5.0.0-day5`
2. **Build:** Multi-stage build (optimized 60MB image)
3. **Tag:** Automatically creates `v5.0.0`, `v5.0`, `v5`, `latest`
4. **Push:** Publishes to `ghcr.io/amareshkumar/telemetry-gateway`
5. **Multi-arch:** Builds for Intel (amd64) and ARM (arm64) - show advanced knowledge!

**Usage:**
```bash
# After creating the workflow file, just push a tag:
git tag -a v5.0.1 -m "Day 5 complete with automated GHCR publishing"
git push origin v5.0.1

# GitHub Actions will automatically:
# - Build Docker image
# - Tag as v5.0.1, v5.0, v5, latest
# - Push to ghcr.io
# - Build for both amd64 and arm64
```

---

## Option 2: npm Package (For Node.js Test Scripts)

### Why npm for TelemetryPlatform?
- ⚠️ **Less relevant** - Your core is C++ (container is better fit)
- ✅ **Could work for:** Publishing k6 load test scripts as reusable package
- ✅ **Interview angle:** "I published my load testing framework as an npm package"

### Setup (If You Want to Try Both)

```bash
# Create package.json for load test scripts
cd c:\code\telemetry-platform\tests\load_tests

cat > package.json << 'EOF'
{
  "name": "@amareshkumar/telemetry-load-tests",
  "version": "1.0.0",
  "description": "k6 load testing framework for TelemetryPlatform gateway",
  "main": "index.js",
  "type": "module",
  "scripts": {
    "test:simple": "k6 run simple_test.js",
    "test:grafana": "k6 run grafana_test.js",
    "test:high-concurrency": "k6 run high_concurrency_test.js"
  },
  "keywords": [
    "k6",
    "load-testing",
    "telemetry",
    "performance",
    "c++",
    "rest-api"
  ],
  "author": "Amaresh Kumar",
  "license": "MIT",
  "repository": {
    "type": "git",
    "url": "https://github.com/amareshkumar/telemetry-platform.git"
  },
  "dependencies": {
    "k6": "^0.0.0"
  },
  "publishConfig": {
    "@amareshkumar:registry": "https://npm.pkg.github.com"
  }
}
EOF

# Publish
npm login --registry=https://npm.pkg.github.com --scope=@amareshkumar
npm publish
```

**Interview Value:** Medium - Shows polyglot skills, but container is more relevant.

---

## Recommendation: Start with GHCR (Container)

### Why Container Registry is Better for You

| Aspect | GHCR (Container) | npm Package |
|--------|------------------|-------------|
| **Relevance** | ✅ C++ project, production deployment | ⚠️ Test scripts only |
| **Interview Impact** | 🔥🔥🔥 "End-to-end DevOps" | 🔥 "Polyglot tooling" |
| **Complexity** | Medium (Dockerfile exists) | Low (just package.json) |
| **Real-world Use** | ✅ Deploy anywhere with Docker | ⚠️ Developers only |
| **Skills Shown** | DevOps, CI/CD, multi-arch, optimization | Package management |
| **Time to Setup** | 10 minutes (manual), 30 min (CI/CD) | 5 minutes |

### My Recommendation (Senior Interview Focus)

**Do GHCR first (30 minutes):**
1. Manual push (prove it works): 10 min
2. CI/CD workflow (automation): 20 min
3. Result: Production-ready container with automated releases

**Optionally add npm (5 minutes):**
- Publish load test scripts as bonus
- Shows full-stack thinking
- But don't spend more than 5 minutes on this

---

## Step-by-Step: GHCR Setup (Complete Guide)

### Step 1: Optimize Dockerfile (Already Done!)

Your existing `ingestion/Dockerfile` is already excellent:
- ✅ Multi-stage build (builder + runtime)
- ✅ Ubuntu 22.04 base (good balance)
- ✅ Health check
- ✅ Proper EXPOSE

**Enhancement (Optional - Add Labels):**
```dockerfile
# Add at the top of runtime stage
FROM ubuntu:22.04 AS runtime
LABEL org.opencontainers.image.source=https://github.com/amareshkumar/telemetry-platform
LABEL org.opencontainers.image.description="TelemetryHub Gateway - High-Performance C++ Telemetry Ingestion"
LABEL org.opencontainers.image.licenses=MIT
LABEL org.opencontainers.image.version="5.0.0"
```

### Step 2: Create GitHub Personal Access Token

1. Go to: https://github.com/settings/tokens/new
2. Note: "GHCR Push Access for TelemetryPlatform"
3. Expiration: 90 days (renewable)
4. Select scopes:
   - ✅ `write:packages` (publish images)
   - ✅ `read:packages` (pull images)
   - ✅ `delete:packages` (cleanup old versions)
5. Click "Generate token"
6. **SAVE THE TOKEN** (you won't see it again!)

### Step 3: Login to GHCR

```powershell
# PowerShell (Windows)
$env:CR_PAT = "ghp_YOUR_TOKEN_HERE"
$env:CR_PAT | docker login ghcr.io -u amareshkumar --password-stdin
```

Expected output:
```
Login Succeeded
```

### Step 4: Build, Tag, Push

```powershell
# Build image
cd c:\code\telemetry-platform
docker build -f ingestion/Dockerfile -t telemetry-gateway:5.0.0 .

# Tag for GHCR (multiple tags)
docker tag telemetry-gateway:5.0.0 ghcr.io/amareshkumar/telemetry-gateway:5.0.0
docker tag telemetry-gateway:5.0.0 ghcr.io/amareshkumar/telemetry-gateway:5.0
docker tag telemetry-gateway:5.0.0 ghcr.io/amareshkumar/telemetry-gateway:5
docker tag telemetry-gateway:5.0.0 ghcr.io/amareshkumar/telemetry-gateway:latest

# Push all tags
docker push ghcr.io/amareshkumar/telemetry-gateway:5.0.0
docker push ghcr.io/amareshkumar/telemetry-gateway:5.0
docker push ghcr.io/amareshkumar/telemetry-gateway:5
docker push ghcr.io/amareshkumar/telemetry-gateway:latest
```

### Step 5: Make Package Public

1. Visit: https://github.com/users/amareshkumar/packages/container/telemetry-gateway/settings
2. Scroll to "Danger Zone"
3. Click "Change visibility"
4. Select "Public"
5. Type package name to confirm
6. Click "I understand, change package visibility"

### Step 6: Verify & Test

```powershell
# Anyone can now pull your image (no login needed for public)
docker pull ghcr.io/amareshkumar/telemetry-gateway:latest

# Run it
docker run -d --name test-gateway -p 8080:8080 ghcr.io/amareshkumar/telemetry-gateway:latest

# Test
curl http://localhost:8080/health
# {"status":"ok"}

# Cleanup
docker stop test-gateway
docker rm test-gateway
```

### Step 7: Add to README.md

```markdown
## 🐳 Docker Quick Start

### Pull from GitHub Container Registry

```bash
# Pull latest version
docker pull ghcr.io/amareshkumar/telemetry-gateway:latest

# Or specific version
docker pull ghcr.io/amareshkumar/telemetry-gateway:5.0.0

# Run gateway
docker run -d \
  --name telemetry-gateway \
  -p 8080:8080 \
  ghcr.io/amareshkumar/telemetry-gateway:latest

# Health check
curl http://localhost:8080/health
```

**Available Tags:**
- `latest` - Latest stable release
- `5.0.0` - Specific version (Day 5 - threading validated)
- `5.0` - Latest 5.0.x release
- `5` - Latest 5.x.x release

**Image Details:**
- Base: Ubuntu 22.04 LTS
- Size: ~60MB (multi-stage optimized)
- Health Check: Built-in
- Platforms: linux/amd64, linux/arm64
```

---

## Step-by-Step: CI/CD Automation (GitHub Actions)

### Step 1: Create Workflow File

```powershell
cd c:\code\telemetry-platform
mkdir .github\workflows -Force
```

Create `.github\workflows\docker-publish.yml` (see full YAML above).

### Step 2: Test Workflow

```powershell
# Commit workflow
git add .github/workflows/docker-publish.yml
git commit -m "ci: Add automated Docker publishing to GHCR"
git push origin master

# Create and push a test tag
git tag -a v5.0.0-ghcr -m "Test GHCR automation"
git push origin v5.0.0-ghcr
```

### Step 3: Monitor Build

1. Go to: https://github.com/amareshkumar/telemetry-platform/actions
2. Click on "Docker Build & Publish" workflow
3. Watch the build progress (takes ~5-10 minutes for multi-arch)
4. Check for ✅ green checkmark

### Step 4: Verify Published Image

```powershell
# Pull the auto-built image
docker pull ghcr.io/amareshkumar/telemetry-gateway:v5.0.0-ghcr

# Verify it works
docker run -d --name auto-test -p 8080:8080 ghcr.io/amareshkumar/telemetry-gateway:v5.0.0-ghcr
curl http://localhost:8080/health
docker stop auto-test && docker rm auto-test
```

---

## Interview Talking Points (Memorize These)

### Quick Facts (30 seconds)
- ✅ **Published to:** GitHub Container Registry (ghcr.io)
- ✅ **Multi-arch:** Builds for amd64 and arm64 (Intel + ARM CPUs)
- ✅ **Optimized:** 60MB runtime image (vs 800MB with build tools)
- ✅ **Automated:** CI/CD triggers on git tags
- ✅ **Versioned:** Semantic versioning (v5.0.0, v5.0, v5, latest)
- ✅ **Public:** Anyone can `docker pull` and run

### STAR Story: Container Distribution

**Situation:** Need to distribute gateway for testing and deployment  
**Task:** Set up professional container registry with CI/CD  
**Action:**
- Optimized Dockerfile with multi-stage build (60MB)
- Published to GitHub Container Registry (free, professional)
- Created CI/CD workflow with GitHub Actions
- Automated builds on git tags with semantic versioning
- Multi-architecture support (amd64 + arm64)

**Result:** 
- Production-ready distribution (pull → run → test in 30 seconds)
- Automated releases (git tag → build → publish, zero manual steps)
- Professional DevOps portfolio piece
- Shows understanding: containerization, CI/CD, registry management, multi-arch

### Q&A Prep

**Q: "How do you distribute your application?"**
> "I publish production-ready Docker images to GitHub Container Registry with automated CI/CD. When I push a git tag like v5.0.0, GitHub Actions automatically builds a multi-stage optimized image (60MB), tags it semantically (v5.0.0, v5.0, v5, latest), and publishes to ghcr.io. Anyone can pull and run with a single command. I also build for multiple architectures (amd64, arm64) to support different hardware."

**Q: "Why GitHub Packages instead of Docker Hub?"**
> "GHCR integrates seamlessly with my GitHub workflow - same authentication, automatic linking to source repo, free for public projects. Docker Hub is great, but GHCR shows I understand modern DevOps ecosystems. Plus, GitHub's artifact attestation provides supply chain security (SLSA compliance) which is important for enterprise deployments."

**Q: "How do you optimize Docker images?"**
> "Multi-stage builds. My Dockerfile has a builder stage (800MB with compilers, CMake, etc.) and a runtime stage (60MB with just the binary and dependencies). This 13x reduction matters for CI/CD speed and deployment costs. I also use Alpine base images where possible and leverage Docker layer caching in GitHub Actions."

---

## Recruiter/Interview Value Assessment

### What Recruiters See

When they check your GitHub:
1. ✅ **Green checkmarks** - CI/CD workflows passing
2. ✅ **Packages tab** - Published container visible
3. ✅ **Professional README** - "Docker Quick Start" section
4. ✅ **Version tags** - Proper release management
5. ✅ **Activity** - Recent commits, automated builds

### Skills This Demonstrates

**DevOps (High Value):**
- ✅ CI/CD pipelines (GitHub Actions)
- ✅ Container registry management (GHCR)
- ✅ Multi-stage Docker builds (optimization)
- ✅ Semantic versioning (release management)
- ✅ Multi-architecture builds (advanced Docker knowledge)

**Modern Practices (Senior-Level):**
- ✅ Automation over manual processes
- ✅ Reproducible builds (anyone can run your container)
- ✅ Supply chain security (artifact attestation)
- ✅ Cost optimization (60MB vs 800MB)
- ✅ Professional distribution (not just "here's my code")

### "One Man Army" Proof

Recruiters looking for full-stack/one-man-army candidates see:
- ✅ **Backend:** C++ high-performance server
- ✅ **DevOps:** Docker, CI/CD, registry management
- ✅ **Testing:** k6 load tests, GoogleTest, profiling
- ✅ **Documentation:** 8,000+ lines guides
- ✅ **Monitoring:** Grafana dashboards, metrics
- ✅ **Distribution:** Published container, public package

**This is the complete package!**

---

## Timeline: How Long Will This Take?

### Manual GHCR Setup (Today - 15 minutes)
- [ ] Create GitHub PAT: 2 min
- [ ] Docker build: 5 min (first build, then cached)
- [ ] Tag + push: 3 min
- [ ] Make public: 2 min
- [ ] Test pull: 2 min
- [ ] Update README: 1 min

### CI/CD Automation (Optional - 30 minutes)
- [ ] Create workflow file: 10 min (copy/paste template)
- [ ] Commit + push: 2 min
- [ ] Create test tag: 1 min
- [ ] Monitor first build: 10 min (multi-arch takes time)
- [ ] Verify: 5 min
- [ ] Document: 2 min

### Total Time Investment
- **Minimum:** 15 minutes (manual GHCR)
- **Complete:** 45 minutes (manual + CI/CD)
- **Interview ROI:** 🔥🔥🔥🔥🔥 (worth 2+ hours of interview prep value)

---

## Next Steps (After Day 5)

1. **Today (15 min):** Manual GHCR push
   - Build, tag, push gateway image
   - Make package public
   - Add Docker Quick Start to README
   - Commit + push LICENSE file

2. **Tomorrow (30 min):** CI/CD automation
   - Create docker-publish.yml workflow
   - Test with git tag
   - Verify automated build

3. **Day 6 (ongoing):** Leverage in development
   - Use published image for integration tests
   - Update image with each release
   - Show continuous delivery in action

---

## Summary: Why This Matters

### For Interviews
- ✅ "I publish production-ready containers with automated CI/CD"
- ✅ Shows end-to-end thinking (code → build → publish → deploy)
- ✅ Demonstrates DevOps skills (not just coding)

### For Recruiters
- ✅ GitHub Packages tab shows professionalism
- ✅ Proves you can ship, not just code
- ✅ "One man army" - backend + DevOps + distribution

### For Your Career
- ✅ Reusable skill (every C++ project needs distribution)
- ✅ Portfolio differentiator (most candidates don't do this)
- ✅ Modern practice (containers are industry standard)

---

**Ready to start? Let's begin with manual GHCR push (15 minutes)!**

```powershell
# Step 1: Build
cd c:\code\telemetry-platform
docker build -f ingestion/Dockerfile -t telemetry-gateway:5.0.0 .

# Step 2: Tag
docker tag telemetry-gateway:5.0.0 ghcr.io/amareshkumar/telemetry-gateway:5.0.0
docker tag telemetry-gateway:5.0.0 ghcr.io/amareshkumar/telemetry-gateway:latest

# Step 3: Login (you'll need PAT from GitHub)
# Visit: https://github.com/settings/tokens/new
$env:CR_PAT = "ghp_YOUR_TOKEN_HERE"
$env:CR_PAT | docker login ghcr.io -u amareshkumar --password-stdin

# Step 4: Push
docker push ghcr.io/amareshkumar/telemetry-gateway:5.0.0
docker push ghcr.io/amareshkumar/telemetry-gateway:latest

# Step 5: Make public (web UI)
# https://github.com/users/amareshkumar/packages/container/telemetry-gateway/settings
```

**Interview soundbite ready:**  
*"I publish my C++ gateway as a Docker image to GitHub Container Registry with automated CI/CD, demonstrating end-to-end DevOps skills from code to production deployment."*
