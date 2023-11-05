@echo off
if not exist "build" (
	echo "Please please build & package the project first"
    exit 1
)

pushd build\litty-llm-bin
START litty-llm.exe
popd