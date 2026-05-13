param(
    [string]$InputJson = (Join-Path (Split-Path -Parent $PSScriptRoot) 'docs\performance_benchmark_results.json'),
    [string]$OutputPng = (Join-Path (Split-Path -Parent $PSScriptRoot) 'docs\performance_benchmark_chart.png')
)

$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Drawing

$data = Get-Content -LiteralPath $InputJson -Raw | ConvertFrom-Json

$width = 1600
$height = 900
$bitmap = New-Object System.Drawing.Bitmap $width, $height
$graphics = [System.Drawing.Graphics]::FromImage($bitmap)
$graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
$graphics.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::ClearTypeGridFit

$background = [System.Drawing.Color]::FromArgb(8, 16, 30)
$panel = [System.Drawing.Color]::FromArgb(20, 38, 62)
$cyan = [System.Drawing.Color]::FromArgb(72, 214, 255)
$orange = [System.Drawing.Color]::FromArgb(255, 170, 64)
$white = [System.Drawing.Color]::FromArgb(236, 245, 255)
$muted = [System.Drawing.Color]::FromArgb(156, 181, 208)
$grid = [System.Drawing.Color]::FromArgb(45, 83, 118)
$green = [System.Drawing.Color]::FromArgb(48, 232, 152)

$graphics.Clear($background)
$graphics.FillRectangle((New-Object System.Drawing.SolidBrush $panel), 36, 34, 1528, 830)

$titleFont = New-Object System.Drawing.Font('Microsoft YaHei UI', 28, [System.Drawing.FontStyle]::Bold)
$subtitleFont = New-Object System.Drawing.Font('Microsoft YaHei UI', 14, [System.Drawing.FontStyle]::Regular)
$labelFont = New-Object System.Drawing.Font('Microsoft YaHei UI', 13, [System.Drawing.FontStyle]::Bold)
$valueFont = New-Object System.Drawing.Font('Consolas', 12, [System.Drawing.FontStyle]::Regular)
$bigFont = New-Object System.Drawing.Font('Consolas', 18, [System.Drawing.FontStyle]::Bold)

$graphics.DrawString('Brick Collision Before vs After', $titleFont, (New-Object System.Drawing.SolidBrush $white), 72, 58)
$graphics.DrawString('Average frame time in ms, lower is better. Naive full scan vs spatial grid broad phase.', $subtitleFont, (New-Object System.Drawing.SolidBrush $muted), 74, 108)

$results = @($data.results)
$maxFrameMs = 0.0
foreach ($item in $results) {
    $maxFrameMs = [Math]::Max($maxFrameMs, [double]$item.naive.avg_frame_ms)
    $maxFrameMs = [Math]::Max($maxFrameMs, [double]$item.spatial_grid.avg_frame_ms)
}

$chartLeft = 120
$chartTop = 210
$chartWidth = 940
$chartHeight = 520
$groupWidth = 240
$barWidth = 74
$scale = if ($maxFrameMs -gt 0.0) { $chartHeight / $maxFrameMs } else { 1.0 }

for ($i = 0; $i -le 5; ++$i) {
    $y = $chartTop + $chartHeight - ($chartHeight / 5.0 * $i)
    $graphics.DrawLine((New-Object System.Drawing.Pen $grid, 1), $chartLeft, $y, $chartLeft + $chartWidth, $y)
    $tickValue = ('{0:N4}' -f ($maxFrameMs / 5.0 * $i))
    $graphics.DrawString($tickValue, $valueFont, (New-Object System.Drawing.SolidBrush $muted), 38, $y - 10)
}

$graphics.DrawLine((New-Object System.Drawing.Pen $white, 2), $chartLeft, $chartTop + $chartHeight, $chartLeft + $chartWidth, $chartTop + $chartHeight)
$graphics.DrawLine((New-Object System.Drawing.Pen $white, 2), $chartLeft, $chartTop, $chartLeft, $chartTop + $chartHeight)

for ($index = 0; $index -lt $results.Count; ++$index) {
    $item = $results[$index]
    $baseX = $chartLeft + 90 + $index * $groupWidth
    $naiveHeight = [double]$item.naive.avg_frame_ms * $scale
    $gridHeight = [double]$item.spatial_grid.avg_frame_ms * $scale
    $naiveX = $baseX
    $gridX = $baseX + 96

    $graphics.FillRectangle((New-Object System.Drawing.SolidBrush $orange), $naiveX, $chartTop + $chartHeight - $naiveHeight, $barWidth, $naiveHeight)
    $graphics.FillRectangle((New-Object System.Drawing.SolidBrush $cyan), $gridX, $chartTop + $chartHeight - $gridHeight, $barWidth, $gridHeight)

    $graphics.DrawString((('{0:N6}' -f [double]$item.naive.avg_frame_ms)), $valueFont, (New-Object System.Drawing.SolidBrush $orange), $naiveX - 8, $chartTop + $chartHeight - $naiveHeight - 24)
    $graphics.DrawString((('{0:N6}' -f [double]$item.spatial_grid.avg_frame_ms)), $valueFont, (New-Object System.Drawing.SolidBrush $cyan), $gridX - 8, $chartTop + $chartHeight - $gridHeight - 24)
    $graphics.DrawString([string]$item.name, $labelFont, (New-Object System.Drawing.SolidBrush $white), $baseX - 26, $chartTop + $chartHeight + 18)
    $graphics.DrawString((('Speedup {0:N2}x' -f [double]$item.speedup_ratio)), $valueFont, (New-Object System.Drawing.SolidBrush $green), $baseX - 2, $chartTop + $chartHeight + 48)
}

$legendY = 760
$graphics.FillRectangle((New-Object System.Drawing.SolidBrush $orange), 120, $legendY, 24, 24)
$graphics.DrawString('Before: naive full scan', $labelFont, (New-Object System.Drawing.SolidBrush $white), 156, $legendY - 2)
$graphics.FillRectangle((New-Object System.Drawing.SolidBrush $cyan), 420, $legendY, 24, 24)
$graphics.DrawString('After: spatial grid', $labelFont, (New-Object System.Drawing.SolidBrush $white), 456, $legendY - 2)

$best = ($results | Sort-Object { [double]$_.speedup_ratio } -Descending | Select-Object -First 1)
$statusText = if ($data.all_checksums_match) { 'Checksums match' } else { 'Checksums differ' }

$graphics.DrawString('Summary', $titleFont, (New-Object System.Drawing.SolidBrush $white), 1140, 228)
$graphics.DrawString((('Best case: {0}' -f [string]$best.name)), $labelFont, (New-Object System.Drawing.SolidBrush $muted), 1142, 286)
$graphics.DrawString((('Speedup: {0:N2}x' -f [double]$best.speedup_ratio)), $bigFont, (New-Object System.Drawing.SolidBrush $green), 1142, 330)
$graphics.DrawString((('Validation: {0}' -f $statusText)), $labelFont, (New-Object System.Drawing.SolidBrush $white), 1142, 382)
$graphics.DrawString('Use this image together with the JSON benchmark data in the submission folder.', $subtitleFont, (New-Object System.Drawing.SolidBrush $muted), 1142, 430)

$outputDir = Split-Path -Parent $OutputPng
if (-not (Test-Path -LiteralPath $outputDir)) {
    New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
}

$bitmap.Save($OutputPng, [System.Drawing.Imaging.ImageFormat]::Png)
$graphics.Dispose()
$bitmap.Dispose()

Write-Output $OutputPng
