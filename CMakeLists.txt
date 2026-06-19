
cmake_minimum_required(VERSION 3.12)

project(PaneerDB)

set(PROJECT_SOURCES
    src/main.cc
    src/Repl/Repl.cc
    src/token.cc
    src/QueryParser.cc
    src/parsers/StatementParser.cc
    src/parsers/SelectStatementParser.cc
    src/statements/SelectStatement.cc
    src/Expression.cc
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src)

# ClockSweep test

set(ClockSweep_SOURCES
    src/clocksweep.cc
)

add_executable(ClockSweep ${ClockSweep_SOURCES})

# RedBlackTree test

set(RedBlackTree_SOURCES
    src/redblacktree.cc
    src/ds/RedBlackTree.cc
)

add_executable(RedBlackTree ${RedBlackTree_SOURCES})
