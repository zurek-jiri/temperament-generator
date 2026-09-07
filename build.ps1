$ErrorActionPreference = 'Stop'
python "$PSScriptRoot\Tools\build.py" @args
exit $LASTEXITCODE
