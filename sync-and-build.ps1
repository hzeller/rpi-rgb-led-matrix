<#
This script was written by ChatGPT based on user requirements, so BEWARE!!
This script is for synchronizing files between a windows system and a raspberry pi
using WSL (Windows Subsystem for Linux) as an intermediary. It uses rsync if available (Tested)
Otherwise, it falls back to a tar-over-ssh method.
After syncing, it triggers a remote build

Edit params below to suit your system

Usage:
  .\sync-and-build.ps1            # sync + build
  .\sync-and-build.ps1 -SkipBuild # sync only
#>

param(
  [string]$LocalPath = 'D:\Data\Code\GitHub\Mine\Forked\rpi-rgb-led-matrix',
  [string]$RemoteUser = 'root',
  [string]$RemoteHost = 'dietpi',
  [string]$RemotePath = '/root/dev/rpi-rgb-led-matrix',
  [switch]$SkipBuild = $false
)

function Convert-ToWslPath {
  param([string]$winPath)
  if ($winPath -match '^[a-zA-Z]:\\') {
    $drive = $winPath.Substring(0,1).ToLower()
    $rest  = $winPath.Substring(2) -replace '\\','/'
    return "/mnt/$drive/$rest"
  }
  return $winPath
}

Write-Host "LocalPath: $LocalPath"
Write-Host ("Remote: {0}@{1}:{2}" -f $RemoteUser, $RemoteHost, $RemotePath)

# Ensure WSL is available
if (-not (Get-Command wsl -ErrorAction SilentlyContinue)) {
  Write-Error "WSL not found. Install WSL or use another sync method (WinSCP, SCP)."
  exit 1
}

$wslPath = Convert-ToWslPath $LocalPath

# Check for rsync inside WSL
$hasRsync = (& wsl bash -lc "if command -v rsync >/dev/null 2>&1; then echo yes; else echo no; fi").Trim()

if (${hasRsync} -eq 'yes') {
  Write-Host "rsync found in WSL - using rsync."
  # use ${} for interpolation to avoid parser ambiguity
  $rsyncCmd = "rsync -avz --delete --exclude='.git' --exclude='build' --exclude='.vs' '${wslPath}/' ${RemoteUser}@${RemoteHost}:'${RemotePath}/'"
  Write-Host "Running: ${rsyncCmd}"
  & wsl bash -lc "${rsyncCmd}"
  $rc = $LASTEXITCODE
  if (${rc} -ne 0) {
    Write-Error "rsync failed with exit code ${rc}"
    exit ${rc}
  }
} else {
  Write-Host "rsync not found in WSL - using tar-over-ssh fallback."
  $tarCmd = "cd '${wslPath}' && tar -czf - --exclude='.git' --exclude='build' . | ssh ${RemoteUser}@${RemoteHost} 'mkdir -p ${RemotePath} && tar -xzf - -C ${RemotePath}'"
  Write-Host "Running tar pipeline..."
  & wsl bash -lc $tarCmd
  $rc = $LASTEXITCODE
  if ($rc -ne 0) {
    Write-Error "tar-over-ssh transfer failed with exit code $rc"
    exit $rc
  }
}

if ($SkipBuild) {
  Write-Host "Skipping remote build (SkipBuild set)."
  exit 0
}

# Trigger remote build: prefer Makefile in repo root, else use CMake if present
$remoteScript = "if [ -f '${RemotePath}/Makefile' ]; then \
  cd '${RemotePath}' && make -j`$(nproc); \
elif [ -f '${RemotePath}/CMakeLists.txt' ]; then \
  mkdir -p '${RemotePath}/build' && cd '${RemotePath}/build' && cmake .. && make -j`$(nproc); \
else \
  echo 'No Makefile or CMakeLists.txt found in ${RemotePath}' 1>&2; exit 1; \
fi"

Write-Host ("Triggering remote build on {0}..." -f ${RemoteHost})

# Run the detector/script on the Pi (pass as a single argument)
& ssh ("${RemoteUser}@${RemoteHost}") ("${remoteScript}")
$exit = $LASTEXITCODE
if ($exit -ne 0) {
  Write-Error "Remote build failed with exit code $exit"
  exit $exit
}

Write-Host "Sync and build completed successfully."
