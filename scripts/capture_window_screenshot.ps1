param(
    [Parameter(Mandatory = $true)]
    [string]$ProcessName,

    [Parameter(Mandatory = $true)]
    [string]$OutputPng
)

$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Drawing
Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;
public static class WindowCaptureNative {
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT {
        public int Left;
        public int Top;
        public int Right;
        public int Bottom;
    }

    [DllImport("user32.dll")]
    public static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);

    [DllImport("user32.dll")]
    public static extern bool SetForegroundWindow(IntPtr hWnd);
}
"@

$process = Get-Process -Name $ProcessName | Select-Object -First 1
if ($process.MainWindowHandle -eq 0) {
    throw "Process '$ProcessName' has no main window handle."
}

[WindowCaptureNative]::SetForegroundWindow($process.MainWindowHandle) | Out-Null
Start-Sleep -Milliseconds 600

$rect = New-Object WindowCaptureNative+RECT
[WindowCaptureNative]::GetWindowRect($process.MainWindowHandle, [ref]$rect) | Out-Null

$width = $rect.Right - $rect.Left
$height = $rect.Bottom - $rect.Top
if ($width -le 0 -or $height -le 0) {
    throw "Window rectangle for '$ProcessName' is invalid: ${width}x${height}"
}

$bitmap = New-Object System.Drawing.Bitmap $width, $height
$graphics = [System.Drawing.Graphics]::FromImage($bitmap)
$graphics.CopyFromScreen($rect.Left, $rect.Top, 0, 0, $bitmap.Size)

$outputDir = Split-Path -Parent $OutputPng
if (-not (Test-Path -LiteralPath $outputDir)) {
    New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
}

$bitmap.Save($OutputPng, [System.Drawing.Imaging.ImageFormat]::Png)
$graphics.Dispose()
$bitmap.Dispose()

Write-Output $OutputPng
