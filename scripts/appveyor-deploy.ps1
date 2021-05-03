$env:ROOT_FOLDER = "C:\projects\valentina";
$env:BUILD_FOLDER = "$env:ROOT_FOLDER\build";

$file_name = "valentina-win-$env:PLATFORM-$env:QT_VERSION-$env:APPVEYOR_REPO_BRANCH-$env:APPVEYOR_REPO_COMMIT.tar.xz";

if($env:DEPLOY -eq "true") {
    Write-Host "[CI] Starting packing." -ForegroundColor Green;
    & $env:PYTHON\python.exe "$env:ROOT_FOLDER\scripts\deploy.py" pack "$env:BUILD_FOLDER\package\valentina" "$env:BUILD_FOLDER\package\$file_name";
    if ($LastExitCode -ne 0) {
        Write-Error -Message "[CI] Error creating an archive." -Category InvalidResult;
        exit 1;
    } else {
        Write-Host "[CI] Done." -ForegroundColor Green;
    }

    Write-Host "[CI] Uploading." -ForegroundColor Green;
    & $env:PYTHON\python.exe "$env:ROOT_FOLDER\scripts\deploy.py" upload $env:ACCESS_TOKEN "$env:BUILD_FOLDER\package\$file_name" "/0.7.x/Windows/$file_name";
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
