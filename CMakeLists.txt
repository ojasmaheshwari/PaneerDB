
cmake_minimum_required(VERSION 3.12)

project(PaneerDB)

set(PROJECT_SOURCES
	src/main.cc
	src/Repl/Repl.cc
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

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
