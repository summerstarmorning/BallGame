param(
    [string]$FolderName = "Lesson10_JSON_Level_Save_Submission"
)

$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$desktop = [Environment]::GetFolderPath("Desktop")
$submissionDir = Join-Path $desktop $FolderName
$jsonDir = Join-Path $submissionDir "level_json_examples"
$zipPath = Join-Path $desktop ($FolderName + ".zip")

New-Item -ItemType Directory -Force -Path $submissionDir | Out-Null
New-Item -ItemType Directory -Force -Path $jsonDir | Out-Null

Copy-Item (Join-Path $projectRoot "config\levels\level1.json") $jsonDir -Force
Copy-Item (Join-Path $projectRoot "config\levels\level2.json") $jsonDir -Force
Copy-Item (Join-Path $projectRoot "config\levels\level3.json") $jsonDir -Force

Copy-Item (Join-Path $projectRoot "docs\level_save_system.md") (Join-Path $submissionDir "feature_overview.md") -Force
Copy-Item (Join-Path $projectRoot "docs\level_save_ai_dialogue.md") (Join-Path $submissionDir "ai_dialogue_record.md") -Force
Copy-Item (Join-Path $projectRoot "scripts\record_level_resume_demo.ps1") (Join-Path $submissionDir "record_demo.ps1") -Force

$repoText = @"
GitHub repository
https://github.com/summerstarmorning/BallGame

Recommended branch
main
"@
Set-Content -Path (Join-Path $submissionDir "repo_link.txt") -Value $repoText -Encoding UTF8

$demoText = @"
Demo checklist
1. Reach stage 2 from stage 1
2. Exit the game
3. Reopen the game
4. Continue from save and stay on stage 2

Included features
- JSON-driven level loading
- 3 connected stages
- Continue prompt support
- JSON fallback handling
- Edit mode with E
- Save version migration
"@
Set-Content -Path (Join-Path $submissionDir "demo_notes.txt") -Value $demoText -Encoding UTF8

$videoPath = Join-Path $submissionDir "resume_demo.mp4"
if (-not (Test-Path $videoPath)) {
    & (Join-Path $projectRoot "scripts\record_level_resume_demo.ps1") -OutputPath $videoPath
}

if (Test-Path $zipPath) {
    Remove-Item $zipPath -Force
}
Compress-Archive -Path (Join-Path $submissionDir "*") -DestinationPath $zipPath -Force

Get-Item $submissionDir, $zipPath | Select-Object FullName, Length, LastWriteTime
