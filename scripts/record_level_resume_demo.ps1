param(
    [string]$OutputPath = "$env:USERPROFILE\Desktop\第10课作业_JSON关卡与存档\运行演示.mp4"
)

$ErrorActionPreference = "Stop"

function Get-FfmpegPath {
    $path = python -c "import imageio_ffmpeg; print(imageio_ffmpeg.get_ffmpeg_exe())"
    if (-not $path) {
        throw "Unable to locate ffmpeg from imageio-ffmpeg."
    }
    return $path.Trim()
}

function Encode-Segment {
    param(
        [string]$FfmpegPath,
        [string]$FrameDir,
        [string]$OutputFile
    )

    if (-not (Test-Path (Join-Path $FrameDir "frame_00001.png"))) {
        throw "No captured frames found in $FrameDir"
    }

    & $FfmpegPath -y `
        -framerate 6 `
        -i (Join-Path $FrameDir "frame_%05d.png") `
        -c:v libx264 `
        -preset veryfast `
        -pix_fmt yuv420p `
        $OutputFile | Out-Null
}

$projectRoot = Split-Path -Parent $PSScriptRoot
$releaseDir = Join-Path $projectRoot "build_vs\Release"
$exePath = Join-Path $releaseDir "breakout_week2.exe"
$saveDir = Join-Path $releaseDir "save"
$savePath = Join-Path $saveDir "run_save.json"
$captureRoot = Join-Path $releaseDir "demo_capture"
$segment1Dir = Join-Path $captureRoot "segment1"
$segment2Dir = Join-Path $captureRoot "segment2"
$segment1Mp4 = Join-Path $captureRoot "segment1.mp4"
$segment2Mp4 = Join-Path $captureRoot "segment2.mp4"
$concatFile = Join-Path $captureRoot "concat.txt"
$rootFramePattern = Join-Path $releaseDir "frame_*.png"

if (-not (Test-Path $exePath)) {
    throw "Game executable not found: $exePath"
}

$outputDir = Split-Path -Parent $OutputPath
New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
New-Item -ItemType Directory -Force -Path $saveDir | Out-Null
Remove-Item $captureRoot -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $segment1Dir, $segment2Dir | Out-Null
Remove-Item $savePath -ErrorAction SilentlyContinue
Remove-Item $OutputPath -ErrorAction SilentlyContinue
Remove-Item $rootFramePattern -ErrorAction SilentlyContinue
Get-Process breakout_week2 -ErrorAction SilentlyContinue | Stop-Process -Force

$firstRun = Start-Process -FilePath $exePath -ArgumentList @(
    "--autostart",
    "--demo-exit-level", "2",
    "--demo-hold-seconds", "1.1",
    "--capture-dir", $segment1Dir
) -WorkingDirectory $releaseDir -PassThru
Wait-Process -Id $firstRun.Id -Timeout 20
Move-Item $rootFramePattern $segment1Dir -Force

if (-not (Test-Path $savePath)) {
    throw "First run did not leave a resume save at $savePath"
}

$secondRun = Start-Process -FilePath $exePath -ArgumentList @(
    "--autocontinue",
    "--demo-exit-level", "2",
    "--demo-hold-seconds", "4.2",
    "--capture-dir", $segment2Dir
) -WorkingDirectory $releaseDir -PassThru
Wait-Process -Id $secondRun.Id -Timeout 20
Move-Item $rootFramePattern $segment2Dir -Force

$ffmpeg = Get-FfmpegPath
Encode-Segment -FfmpegPath $ffmpeg -FrameDir $segment1Dir -OutputFile $segment1Mp4
Encode-Segment -FfmpegPath $ffmpeg -FrameDir $segment2Dir -OutputFile $segment2Mp4

$concatContent = @(
    "file '$segment1Mp4'",
    "file '$segment2Mp4'"
)
Set-Content -Path $concatFile -Value $concatContent -Encoding ASCII

& $ffmpeg -y -f concat -safe 0 -i $concatFile -c copy $OutputPath | Out-Null

if (-not (Test-Path $OutputPath)) {
    throw "Recording was not created: $OutputPath"
}

Get-Item $OutputPath | Select-Object FullName, Length, LastWriteTime
