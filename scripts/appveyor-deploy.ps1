$INSTALL_ROOT = "$env:VALENTINA_BUILD_FOLDER\release\install-root\valentina";

$type = "$env:TARGET_PLATFORM-$env:COMPILER-$env:ARCH-$env:QT_VERSION-$env:APPVEYOR_REPO_BRANCH-$env:APPVEYOR_REPO_COMMIT";
$file_name = "valentina-${type}.exe";
$portable_file_name = "valentina-portable-${type}.7z";

if($env:DEPLOY -eq "true") {
    Write-Host "[CI] Checking installer." -ForegroundColor Green;
    $installerPath = "$INSTALL_ROOT\ValentinaInstaller.exe";
    if (-not (Test-Path $installerPath)) {
        Write-Error -Message "[CI] Installer file not found at $installerPath" -Category InvalidResult;
        exit 1;
    }

    Write-Host "[CI] Starting cleaning." -ForegroundColor Green;
    & $env:PYTHON\python.exe "$env:APPVEYOR_BUILD_FOLDER\scripts\deploy.py" clean $env:ACCESS_TOKEN;
    if ($LastExitCode -ne 0) {
        Write-Error -Message "[CI] Error cleaning stale artifacts." -Category InvalidResult;
        exit 1;
    } else {
        Write-Host "[CI] Cleaning done." -ForegroundColor Green;
    }

    Write-Host "[CI] Uploading installer." -ForegroundColor Green;
    & $env:PYTHON\python.exe "$env:APPVEYOR_BUILD_FOLDER\scripts\deploy.py" upload $env:ACCESS_TOKEN "$INSTALL_ROOT\ValentinaInstaller.exe" "/1.1.x/Windows/$file_name";
    if ($LastExitCode -ne 0) {
        Write-Error -Message "[CI] Error uploading an artifact." -Category InvalidResult;
        exit 1;
    } else {
        Write-Host "[CI] Uploading has been finished." -ForegroundColor Green;
    }

    Write-Host "[CI] Starting packing." -ForegroundColor Green;
    Remove-Item -Path "$INSTALL_ROOT\ValentinaInstaller.exe";
    & $env:PYTHON\python.exe "$env:APPVEYOR_BUILD_FOLDER\scripts\deploy.py" pack "$INSTALL_ROOT" "$env:VALENTINA_BUILD_FOLDER\release\install-root\$portable_file_name";
    if ($LastExitCode -ne 0) {
        Write-Error -Message "[CI] Error creating an archive." -Category InvalidResult;
        exit 1;
    } else {
        Write-Host "[CI] Done." -ForegroundColor Green;
    }

    Write-Host "[CI] Uploading portable bundle." -ForegroundColor Green;
    & $env:PYTHON\python.exe "$env:APPVEYOR_BUILD_FOLDER\scripts\deploy.py" upload $env:ACCESS_TOKEN "$env:VALENTINA_BUILD_FOLDER\release\install-root\$portable_file_name" "/1.1.x/Windows/$portable_file_name";
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
