param(
    [Parameter(Mandatory = $true)]
    [string]$InstallRoot,

    [Parameter(Mandatory = $true)]
    [string]$CertumKeyId,

    [Parameter(Mandatory = $false)]
    [string]$TimestampUrl = "http://timestamp.certum.pl",

    [Parameter(Mandatory = $false)]
    [string[]]$Extensions = @("*.exe", "*.dll", "*.sys", "*.ocx", "*.msi", "*.msix", "*.cat")
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# ── Helpers ──────────────────────────────────────────────────────────────────

function Test-IsSigned {
    param([string]$FilePath)
    try {
        $sig = Get-AuthenticodeSignature -FilePath $FilePath -ErrorAction Stop
        return $sig.Status -eq [System.Management.Automation.SignatureStatus]::Valid
    }
    catch {
        return $false
    }
}

function Invoke-SignFile {
    param([string]$FilePath)

    Write-Host "  Signing: $FilePath"

    signtool.exe sign `
        /tr  $TimestampUrl `
        /td  sha256 `
        /fd  sha256 `
        /sha1 $CertumKeyId `
        $FilePath

    if ($LASTEXITCODE -ne 0) {
        throw "signtool.exe failed for '$FilePath' (exit code $LASTEXITCODE)"
    }
}

# ── Validation ────────────────────────────────────────────────────────────────

if (-not (Test-Path $InstallRoot)) {
    Write-Error "Install root not found: $InstallRoot"
    exit 1
}

if (-not (Get-Command signtool.exe -ErrorAction SilentlyContinue)) {
    Write-Error "signtool.exe not found in PATH. Add the 'Add SignTool to PATH' step before this one."
    exit 1
}

# ── Discovery ─────────────────────────────────────────────────────────────────

Write-Host ""
Write-Host "==================================================="
Write-Host " Scanning: $InstallRoot"
Write-Host " Looking for: $($Extensions -join ', ')"
Write-Host "==================================================="

$allFiles = @()
foreach ($ext in $Extensions) {
    $allFiles += Get-ChildItem -Path $InstallRoot -Filter $ext -Recurse -File -ErrorAction SilentlyContinue
}

# Deduplicate (a file could match two patterns in theory)
$allFiles = $allFiles | Sort-Object -Property FullName -Unique

Write-Host "Found $($allFiles.Count) candidate file(s)."
Write-Host ""

# ── Sign loop ─────────────────────────────────────────────────────────────────

$signed   = 0
$skipped  = 0
$failed   = 0
$failList = @()

foreach ($file in $allFiles) {
    if (Test-IsSigned $file.FullName) {
        Write-Host "  [SKIP] Already signed: $($file.FullName)"
        $skipped++
        continue
    }

    try {
        Invoke-SignFile $file.FullName
        $signed++
    }
    catch {
        Write-Warning "  [FAIL] $($file.FullName): $_"
        $failList += $file.FullName
        $failed++
    }
}

# ── Summary ───────────────────────────────────────────────────────────────────

Write-Host ""
Write-Host "==================================================="
Write-Host " Signing complete"
Write-Host "   Signed : $signed"
Write-Host "   Skipped: $skipped  (already signed)"
Write-Host "   Failed : $failed"
Write-Host "==================================================="

if ($failList.Count -gt 0) {
    Write-Host ""
    Write-Host "Failed files:"
    $failList | ForEach-Object { Write-Host "  - $_" }
    exit 1
}

exit 0
