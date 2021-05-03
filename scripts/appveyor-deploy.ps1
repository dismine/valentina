$env:ROOT_FOLDER = "C:\projects\valentina";
$env:BUILD_FOLDER = "C:\projects\valentina\build";

if($env:DEPLOY -eq "true") {
    Write-Host "[CI] Starting packing." -ForegroundColor Green;
    & $env:PYTHON\python.exe $env:ROOT_FOLDER\scripts\deploy.py pack .\package\valentina\ "$env:BUILD_FOLDER\package\valentina-win-$(PLATFORM)-$(QT_VERSION)-$(APPVEYOR_REPO_BRANCH)-$(APPVEYOR_REPO_COMMIT).zip";
    if ($LastExitCode -ne 0) {
        Write-Error -Message "[CI] Error creating an archive." -Category InvalidResult;
        exit 1;
    } else {
        Write-Host "[CI] Done." -ForegroundColor Green;
    }

    Write-Host "[CI] Uploading." -ForegroundColor Green;
    & $env:PYTHON\python.exe $env:ROOT_FOLDER\scripts\deploy.py upload $env:ACCESS_TOKEN "$env:BUILD_FOLDER\package\valentina-win-$(PLATFORM)-$(QT_VERSION)-$(APPVEYOR_REPO_BRANCH)-$(APPVEYOR_REPO_COMMIT).zip" "/0.7.x/Windows/valentina-win-$(PLATFORM)-$(QT_VERSION)-$(APPVEYOR_REPO_BRANCH)-$(APPVEYOR_REPO_COMMIT).zip";
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
