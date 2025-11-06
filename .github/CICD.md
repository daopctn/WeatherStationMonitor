# CI/CD Pipeline Documentation

This document describes the automated Continuous Integration and Continuous Deployment (CI/CD) pipeline for WeatherStationMonitor.

## Overview

The CI/CD pipeline provides:
- ✅ Automated builds on every commit
- ✅ Automated testing with Qt Test framework
- ✅ Code coverage tracking with Codecov
- ✅ Code quality checks (Python & C++)
- ✅ Security scanning
- ✅ Automated Debian package builds
- ✅ Automated releases on version tags

## Workflows

### 1. Build and Test (`build-and-test.yml`)

**Triggers:**
- Push to `main`, `develop`, or `claude/**` branches
- Pull requests to `main` or `develop`
- Manual trigger via workflow_dispatch

**Steps:**
1. Install dependencies (Qt5, CMake, Python, lcov)
2. Configure CMake with tests and coverage enabled
3. Build project with `make`
4. Run tests with `ctest`
5. Generate code coverage report with `lcov`
6. Upload coverage to Codecov
7. Upload build artifacts

**Artifacts:**
- `weather-station-monitor-binary` - Compiled executable
- `test-results` - CTest output
- Coverage report uploaded to Codecov

**Status Badge:**
```markdown
[![Build and Test](https://github.com/daopctn/WeatherStationMonitor/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/daopctn/WeatherStationMonitor/actions/workflows/build-and-test.yml)
```

---

### 2. Code Quality (`code-quality.yml`)

**Triggers:**
- Push to `main` or `develop`
- Pull requests to `main` or `develop`
- Manual trigger

**Jobs:**

#### Python Quality
- **flake8** - Linting (max line length: 120)
- **black** - Code formatting check
- **pylint** - Advanced Python linting

#### C++ Quality
- **cppcheck** - Static analysis
- **TODO/FIXME detector** - Finds pending work

#### Security Scan
- **Trivy** - Vulnerability scanner
- **Secret detector** - Checks for hardcoded credentials

**Artifacts:**
- `cppcheck-report` - Full cppcheck analysis
- Trivy SARIF uploaded to GitHub Security

**Status Badge:**
```markdown
[![Code Quality](https://github.com/daopctn/WeatherStationMonitor/actions/workflows/code-quality.yml/badge.svg)](https://github.com/daopctn/WeatherStationMonitor/actions/workflows/code-quality.yml)
```

---

### 3. Release (`release.yml`)

**Triggers:**
- Push tags matching `v*.*.*` (e.g., `v1.0.0`, `v1.2.3`)
- Manual trigger with version input

**Steps:**
1. Install build dependencies
2. Build Debian package using `build-deb.sh`
3. Run `lintian` to check package quality
4. Upload package as artifact (90 days retention)
5. Create GitHub Release with:
   - Debian package attached
   - Auto-generated release notes
   - Installation instructions
6. Test package installation on clean Ubuntu 22.04

**Artifacts:**
- `debian-package-{version}` - Built .deb package

**Creating a Release:**
```bash
# Tag a new version
git tag -a v1.0.1 -m "Release version 1.0.1"
git push origin v1.0.1

# Workflow automatically builds and publishes
```

---

## Code Coverage

### Codecov Integration

**Configuration:** `codecov.yml`

**Coverage Targets:**
- **Project:** 70% overall coverage (Phase 2 target)
- **Patch:** 60% for new code
- **Threshold:** Allow 1% drop

**Ignored Files:**
- Tests (`tests/**`)
- UI files (`ui/**`)
- Resources (`resources/**`)
- Qt generated files (`moc_*`, `ui_*`, `*_automoc.cpp`)

**Status Badge:**
```markdown
[![codecov](https://codecov.io/gh/daopctn/WeatherStationMonitor/branch/main/graph/badge.svg)](https://codecov.io/gh/daopctn/WeatherStationMonitor)
```

**Viewing Coverage:**
1. Visit https://codecov.io/gh/daopctn/WeatherStationMonitor
2. View detailed file-by-file coverage
3. See coverage trends over time
4. Check coverage on pull requests

---

## Branch Protection Rules

### Recommended Settings

**For `main` branch:**

1. **Require pull request reviews before merging**
   - Required approvals: 1
   - Dismiss stale reviews when new commits are pushed

2. **Require status checks to pass before merging**
   - Require branches to be up to date before merging
   - Required checks:
     - `build-and-test / Build and Test on Ubuntu 22.04`
     - `code-quality / Python Code Quality`
     - `code-quality / C++ Code Quality`
     - `codecov/project` (if Codecov GitHub app installed)

3. **Require conversation resolution before merging**
   - All PR comments must be resolved

4. **Do not allow bypassing the above settings**
   - Even administrators must follow the rules

5. **Allow force pushes: ❌ Disabled**

6. **Allow deletions: ❌ Disabled**

### Setting Up Branch Protection

1. Go to repository Settings → Branches
2. Click "Add rule"
3. Branch name pattern: `main`
4. Enable the settings above
5. Click "Create" or "Save changes"

**Screenshot:**
```
Settings → Branches → Branch protection rules → Add rule
```

---

## Secrets Configuration

The following secrets need to be configured in GitHub repository settings:

### Required Secrets

| Secret | Purpose | How to Get |
|--------|---------|------------|
| `CODECOV_TOKEN` | Upload coverage to Codecov | Sign up at codecov.io, add repo, copy token |

### Setting Secrets

1. Go to repository Settings → Secrets and variables → Actions
2. Click "New repository secret"
3. Add name and value
4. Click "Add secret"

**Optional Secrets:**
- `GITHUB_TOKEN` - Automatically provided by GitHub Actions

---

## Workflow Status

### Viewing Workflow Runs

1. Go to repository "Actions" tab
2. Click on a workflow name to see all runs
3. Click on a specific run to see details
4. Download artifacts if needed

### Badges in README

All status badges are displayed at the top of README.md:
- Build Status
- Code Quality Status
- Code Coverage Percentage

### Notifications

By default, GitHub sends emails on workflow failures. Configure in:
- Your GitHub account → Settings → Notifications

---

## Local Testing

Before pushing, test locally:

### Build and Test
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ..
make -j$(nproc)
ctest --output-on-failure
```

### Code Coverage
```bash
cmake -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ..
make && ctest
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
firefox coverage_html/index.html
```

### Python Linting
```bash
flake8 python/ --max-line-length=120
black --check python/
```

### C++ Analysis
```bash
cppcheck --enable=all src/ include/
```

---

## Troubleshooting

### Build Fails in CI But Works Locally

**Possible causes:**
1. Missing dependencies in workflow
2. Different Qt version
3. Different compiler version
4. Environment variables not set

**Solution:**
- Check workflow logs for specific error
- Update `build-and-test.yml` dependencies if needed
- Ensure local environment matches Ubuntu 22.04

### Tests Pass Locally But Fail in CI

**Possible causes:**
1. Test database not configured
2. Timing issues in tests
3. Network access restricted

**Solution:**
- Add database setup to workflow
- Add `continue-on-error: true` temporarily to debug
- Check test logs in CI artifacts

### Code Coverage Not Uploading

**Possible causes:**
1. `CODECOV_TOKEN` not set
2. Coverage file not generated
3. Codecov service down

**Solution:**
```yaml
# Check workflow step output
- name: Upload coverage to Codecov
  env:
    CODECOV_TOKEN: ${{ secrets.CODECOV_TOKEN }}
```

### Release Not Creating

**Possible causes:**
1. Tag format incorrect (must be `v*.*.*`)
2. Build fails before release step
3. Insufficient permissions

**Solution:**
- Use semantic versioning: `v1.0.0`
- Check build logs
- Ensure `GITHUB_TOKEN` has write permissions

---

## Maintenance

### Updating Workflows

1. Edit workflow files in `.github/workflows/`
2. Test changes on a feature branch first
3. Review workflow run before merging to main

### Updating Dependencies

Update versions in workflows periodically:
- `actions/checkout@v4` → Check for v5
- `actions/upload-artifact@v4` → Check for updates
- `codecov/codecov-action@v4` → Check for updates

### Adding New Tests

1. Write test in `tests/test_NewFeature.cpp`
2. Add to `tests/CMakeLists.txt`
3. Push and verify CI runs tests
4. Check coverage increased

---

## Performance

### Workflow Execution Times

| Workflow | Average Duration | Max Duration |
|----------|------------------|--------------|
| Build and Test | ~5-7 minutes | 10 minutes |
| Code Quality | ~2-3 minutes | 5 minutes |
| Release | ~8-10 minutes | 15 minutes |

### Optimization Tips

1. **Cache Dependencies**
   ```yaml
   - uses: actions/cache@v3
     with:
       path: ~/.cache/pip
       key: ${{ runner.os }}-pip-${{ hashFiles('**/requirements.txt') }}
   ```

2. **Parallel Jobs**
   - Code quality checks run in parallel
   - Can add more parallel build matrix (different Qt versions)

3. **Conditional Steps**
   ```yaml
   - name: Upload coverage
     if: github.event_name == 'push'  # Only on push, not PR
   ```

---

## Security

### Best Practices

1. ✅ Never commit secrets to repository
2. ✅ Use GitHub Secrets for sensitive data
3. ✅ Regularly update action versions
4. ✅ Review security alerts in Security tab
5. ✅ Enable Dependabot for dependency updates

### Security Scanning

Trivy scanner runs automatically and uploads results to GitHub Security tab.

**View Security Alerts:**
1. Go to repository Security tab
2. Click "Code scanning alerts"
3. Review and fix vulnerabilities

---

## Resources

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Qt Test Framework](https://doc.qt.io/qt-5/qtest-overview.html)
- [Codecov Documentation](https://docs.codecov.com/)
- [cppcheck Manual](http://cppcheck.sourceforge.net/manual.pdf)
- [Debian Packaging Guide](https://www.debian.org/doc/manuals/maint-guide/)

---

## Support

For CI/CD issues:
1. Check workflow logs in Actions tab
2. Review this documentation
3. Check GitHub Actions status page
4. Open an issue with workflow run URL

---

**Last Updated:** 2025-11-06
**Pipeline Version:** Phase 3
**Status:** ✅ Fully Operational
