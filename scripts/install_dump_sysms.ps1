# Check if WITH_CRASH_REPORTING environment variable is set to True
if ($env:WITH_CRASH_REPORTING -eq "True") {
    # Define URLs and file paths
    $archiveUrl = "https://github.com/mozilla/dump_syms/releases/download/v2.3.1/dump_syms-x86_64-pc-windows-msvc.zip"
    $downloadPath = "$env:TEMP\dump_syms.zip"
    $extractPath = "$env:TEMP\dump_syms"

    # Download the archive
    Invoke-WebRequest -Uri $archiveUrl -OutFile $downloadPath

    # Extract the archive
    Expand-Archive -Path $downloadPath -DestinationPath $extractPath -Force

    # Add the directory containing dump_syms to the PATH environment variable
    $env:Path += ";$extractPath"

    # Check if dump_syms is callable
    if (Test-Path (Join-Path $extractedFolderPath "dump_syms.exe")) {
        Write-Host "dump_syms utility installed successfully and added to PATH."
    } else {
        Write-Host "Failed to install dump_syms utility."
    }
} else {
    Write-Host "Skipping dump_syms installation."
}
