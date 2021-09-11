include(RunCMake)

function(run_TargetMessages case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/TargetMessages-${case}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  set(RunCMake_TEST_OPTIONS "${ARGN}")
  run_cmake(TargetMessages-${case})
  run_cmake_command(TargetMessages-${case}-build ${CMAKE_COMMAND} --build .)
endfunction()

run_TargetMessages(ON)
run_TargetMessages(OFF)

run_TargetMessages(VAR-ON -DCMAKE_TARGET_MESSAGES=ON)
run_TargetMessages(VAR-OFF -DCMAKE_TARGET_MESSAGES=OFF)

function(run_VerboseBuild)
  run_cmake(VerboseBuild)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/VerboseBuild-build)
  if(RunCMake_GENERATOR STREQUAL "Watcom WMake")
    # wmake does not actually show the verbose output.
    set(RunCMake-stdout-file VerboseBuild-build-watcom-stdout.txt)
  endif()
  run_cmake_command(VerboseBuild-build ${CMAKE_COMMAND} --build . -v --clean-first)
  unset(RunCMake-stdout-file)
  set(_backup_lang "$ENV{LANG}")
  set(_backup_lc_Messages "$ENV{LC_MESSAGES}")
  if(MAKE_IS_GNU)
    set(RunCMake-stdout-file VerboseBuild-nowork-gnu-stdout.txt)
    set(ENV{LANG} "C")
    set(ENV{LC_MESSAGES} "C")
  endif()
  run_cmake_command(VerboseBuild-nowork ${CMAKE_COMMAND} --build . --verbose)
  set(ENV{LANG} "${_backup_lang}")
  set(ENV{LC_MESSAGES} "${_backup_lc_messages}")
endfunction()
run_VerboseBuild()

run_cmake(IncludeRegexSubdir)

function(run_MakefileConflict)
  run_cmake(MakefileConflict)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/MakefileConflict-build)
  run_cmake_command(MakefileConflict-build ${CMAKE_COMMAND} --build . --target Custom)
endfunction()
run_MakefileConflict()

function(run_CMP0113 val)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CMP0113-${val}-build)
  run_cmake(CMP0113-${val})
  set(RunCMake_TEST_NO_CLEAN 1)
  set(_backup_lang "$ENV{LANG}")
  set(_backup_lc_Messages "$ENV{LC_MESSAGES}")
  if(MAKE_IS_GNU)
    set(RunCMake-stderr-file CMP0113-${val}-build-gnu-stderr.txt)
    set(ENV{LANG} "C")
    set(ENV{LC_MESSAGES} "C")
  endif()
  run_cmake_command(CMP0113-${val}-build ${CMAKE_COMMAND} --build .)
  set(ENV{LANG} "${_backup_lang}")
  set(ENV{LC_MESSAGES} "${_backup_lc_messages}")
endfunction()

if(NOT RunCMake_GENERATOR STREQUAL "Watcom WMake")
  run_CMP0113(WARN)
  run_CMP0113(OLD)
  run_CMP0113(NEW)
endif()
