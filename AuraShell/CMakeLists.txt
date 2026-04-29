cmake_minimum_required(VERSION 3.20)
project(AuraShell C)

add_executable(AuraShell
  src/main.c
  src/shell.c
  src/parser.c
  src/commands.c
  src/process.c
  src/history.c
  src/theme.c
  src/alias.c
  src/voice.c
)

target_include_directories(AuraShell PRIVATE include)
target_compile_definitions(AuraShell PRIVATE UNICODE _UNICODE)
target_link_libraries(AuraShell PRIVATE shell32 shlwapi user32 ole32 sapi)


