$env:BUILD_FOLDER = "$env:APPVEYOR_BUILD_FOLDER\build";
$env:INSTALL_ROOT = "$env:BUILD_FOLDER\install-root\valentina";

$file_name = "valentina-$env:PLATFORM-$env:COMPILER-$env:ARCH-$env:QT_VERSION-$env:APPVEYOR_REPO_BRANCH-$env:APPVEYOR_REPO_COMMIT.exe";

if($env:DEPLOY -eq "true") {
    Write-Host "[CI] Preparing installer." -ForegroundColor Green;
    $installerPath = "$env:INSTALL_ROOT\ValentinaInstaller.exe";
    if (-not (Test-Path $installerPath)) {
        Write-Error -Message "[CI] Installer file not found at $installerPath" -Category InvalidResult;
        exit 1;
    }
    Write-Host "[CI] Done." -ForegroundColor Green;

    Write-Host "[CI] Starting cleaning." -ForegroundColor Green;
    & $env:PYTHON\python.exe "$env:APPVEYOR_BUILD_FOLDER\scripts\deploy.py" clean $env:ACCESS_TOKEN;
    if ($LastExitCode -ne 0) {
        Write-Error -Message "[CI] Error cleaning stale artifacts." -Category InvalidResult;
        exit 1;
    } else {
        Write-Host "[CI] Cleaning done." -ForegroundColor Green;
    }

    Write-Host "[CI] Uploading." -ForegroundColor Green;
    & $env:PYTHON\python.exe "$env:APPVEYOR_BUILD_FOLDER\scripts\deploy.py" upload $env:ACCESS_TOKEN "$env:INSTALL_ROOT\ValentinaInstaller.exe" "/0.7.x/Windows/$file_name";
    if ($LastExitCode -ne 0) {
        Write-Error -Message "[CI] Error uploading an artifact." -Category InvalidResult;
        exit 1;
    } else {
        Write-Host "[CI] Uploading has been finished." -ForegroundColor Green;
    }
    Exit 0;
}
else {
    Write-Host "[CI] No deployment needed." -ForegroundColor Green;
    Exit 0;
}
