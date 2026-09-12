function Edit-Src {
  param([string]$File, [string]$Old, [string]$New, [string]$Tag = "edit")
  $p = Join-Path "C:\VirtualDrive\SLH_ECOSYSTEM\esp-firmware" $File
  if (-not (Test-Path $p)) { Write-Host "  [X] $Tag - file missing: $File" -ForegroundColor Red; return $false }
  $t = Get-Content $p -Raw
  $n = ([regex]::Matches($t, [regex]::Escape($Old))).Count
  if ($n -eq 0) { Write-Host "  [X] $Tag - pattern NOT FOUND in $File" -ForegroundColor Red; return $false }
  if ($n -gt 1) { Write-Host "  [!] $Tag - pattern found $n times, aborting" -ForegroundColor Yellow; return $false }
  $t = $t.Replace($Old, $New)
  Set-Content -Path $p -Value $t -Encoding ASCII -NoNewline
  Write-Host "  [OK] $Tag" -ForegroundColor Green
  return $true
}
