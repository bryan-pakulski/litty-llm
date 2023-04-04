@echo off

echo Building docker image, this may take some time...
docker build -t litty-llm .

echo Detecting Nvidia GPU...

set status=Does Not Exist

for /f "tokens=*" %%i in ('wmic path Win32_VideoController get description /format:list') do (
   set desc=%%i
   if "!desc!" == "Description=NVIDIA" set status=Exists
)

echo Nvidia GPU: %status%

if "%status%" == "Exists" (
  echo Nvidia GPU found.
  docker compose --file docker\docker-compose-nvidia.yml up -d
) else (
  echo Nvidia GPU not found.
  docker compose --file docker\docker-compose-cpu.yml up -d
)

docker exec litty-llm /home/entrypoint.sh