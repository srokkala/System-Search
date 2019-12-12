## Test 01: No Arguments [1 pts]

If no arguments are provided, print usage information and quit. If -h is

```

reference_run ./prep -h

run ./prep

compare_outputs || test_end

Expected Program Output                | Actual Program Output
---------------------------------------V---------------------------------------
Usage: ./prep [-eh] [-d directory] [-t    Usage: ./prep [-eh] [-d directory] [-t

Options:                                  Options:
    * -d directory    specify start di        * -d directory    specify start di
    * -e              print exact name        * -e              print exact name
    * -h              show usage infor        * -h              show usage infor
    * -t threads      set maximum thre        * -t threads      set maximum thre
---------------------------------------^---------------------------------------
 --> OK
```

## Test 02: Invalid Thread Count [1 pts]

Checks to make sure an invalid thread count is handled properly.

```

run ./prep -t -1 wow
Invalid threads count (has to be > 0)

run ./prep -t hello world
Invalid threads count (has to be > 0)

test_end
```

## Test 03: Invalid Directory [1 pts]

The program should not crash when given an invalid directory 

```

run ./prep -t 1000 -e -d /this/does/not/exist xxx yyy zzz
prep.c:476:run_search(): Using directory '/this/does/not/exist'
Failed to open the start directory: No such file or directory

test_end
```

## Test 04: Basic Searches [1 pts]

```

run "${TEST_DIR}/../prep" -d "${TEST_DIR}/test-fs/xyz" sea
prep.c:476:run_search(): Using directory '/home/srokkala/Labs/P4-curr-y/tests/test-fs/xyz'
prep.c:491:run_search(): Absolute path to search directory is '/home/srokkala/Labs/P4-curr-y/tests/test-fs/xyz'
prep.c:500:run_search(): Searching '/home/srokkala/Labs/P4-curr-y/tests/test-fs/xyz' recursively using 2 threads
prep.c:504:run_search(): Search term: 'sea'

compare_outputs || test_end

-- diff of outputs shown below --
---------------------------------
 --> OK

run "${TEST_DIR}/../prep" -d "${TEST_DIR}/test-fs/xyz" sea blerpoblagoperatooogazoa
prep.c:476:run_search(): Using directory '/home/srokkala/Labs/P4-curr-y/tests/test-fs/xyz'
prep.c:491:run_search(): Absolute path to search directory is '/home/srokkala/Labs/P4-curr-y/tests/test-fs/xyz'
prep.c:500:run_search(): Searching '/home/srokkala/Labs/P4-curr-y/tests/test-fs/xyz' recursively using 2 threads
prep.c:504:run_search(): Search term: 'sea'
prep.c:504:run_search(): Search term: 'blerpoblagoperatooogazoa'

compare_outputs

-- diff of outputs shown below --
---------------------------------
 --> OK

test_end
```

## Test 05: Many-Term Search [1 pts]

```

reference_run "${TEST_DIR}/prep.sh" -t 2 -e -d "${TEST_DIR}/test-fs" \
    Cochrone aquamarine Callistonian chandeliers encephalography encyclopedic institutionalized \
        | sort
Searching for Cochrone in /home/srokkala/Labs/P4-curr-y/tests/test-fs
Searching for aquamarine in /home/srokkala/Labs/P4-curr-y/tests/test-fs
Searching for Callistonian in /home/srokkala/Labs/P4-curr-y/tests/test-fs
Searching for chandeliers in /home/srokkala/Labs/P4-curr-y/tests/test-fs
Searching for encephalography in /home/srokkala/Labs/P4-curr-y/tests/test-fs
Searching for encyclopedic in /home/srokkala/Labs/P4-curr-y/tests/test-fs
Searching for institutionalized in /home/srokkala/Labs/P4-curr-y/tests/test-fs

run "${TEST_DIR}/../prep" -t 2 -e -d "${TEST_DIR}/test-fs" \
    Cochrone aquamarine Callistonian chandeliers encephalography encyclopedic institutionalized \
        | sort
prep.c:476:run_search(): Using directory '/home/srokkala/Labs/P4-curr-y/tests/test-fs'
prep.c:491:run_search(): Absolute path to search directory is '/home/srokkala/Labs/P4-curr-y/tests/test-fs'
prep.c:500:run_search(): Searching '/home/srokkala/Labs/P4-curr-y/tests/test-fs' recursively using 2 threads
prep.c:504:run_search(): Search term: 'Cochrone'
prep.c:504:run_search(): Search term: 'aquamarine'
prep.c:504:run_search(): Search term: 'Callistonian'
prep.c:504:run_search(): Search term: 'chandeliers'
prep.c:504:run_search(): Search term: 'encephalography'
prep.c:504:run_search(): Search term: 'encyclopedic'
prep.c:504:run_search(): Search term: 'institutionalized'

compare_outputs

-- diff of outputs shown below --
---------------------------------
 --> OK

test_end
```

## Test 06: Whitespace Handling [1 pts]

```

reference_run "${TEST_DIR}/prep.sh" -d "${TEST_DIR}/test-fs" \
    blerpoblagoperatooogazoa floccinaucinihilipilification
Searching for blerpoblagoperatooogazoa in /home/srokkala/Labs/P4-curr-y/tests/test-fs
Searching for floccinaucinihilipilification in /home/srokkala/Labs/P4-curr-y/tests/test-fs

run "${TEST_DIR}/../prep" -d "${TEST_DIR}/test-fs" \
    blerpoblagoperatooogazoa floccinaucinihilipilification
prep.c:476:run_search(): Using directory '/home/srokkala/Labs/P4-curr-y/tests/test-fs'
prep.c:491:run_search(): Absolute path to search directory is '/home/srokkala/Labs/P4-curr-y/tests/test-fs'
prep.c:500:run_search(): Searching '/home/srokkala/Labs/P4-curr-y/tests/test-fs' recursively using 2 threads
prep.c:504:run_search(): Search term: 'blerpoblagoperatooogazoa'
prep.c:504:run_search(): Search term: 'floccinaucinihilipilification'

compare_outputs

-- diff of outputs shown below --
---------------------------------
 --> OK

test_end
```

## Test 07: Thread Performance [1 pts]

Tests the performance improvement (>= 1.2 in at least 1 iteration of 3)

```

for i in {1..3}; do
    run "${TEST_DIR}/../prep" -t 1 -d "${TEST_DIR}/test-fs" the
    run1="${program_runtime}"

    run "${TEST_DIR}/../prep" -t 2 -d "${TEST_DIR}/test-fs" the
    run2="${program_runtime}"

    awk '
    {
        improvement = ($1 / $2)
        printf "Speed improvement: %.2f\n", improvement
        printf "(Must be >= 1.2)\n"
        if (improvement < 1.2) {
            exit 1
        } else {
            exit 0
        }
    }' <<< "${run1} ${run2}"
    
    if [[ $? -eq 0 ]]; then
        test_end 0
    fi
done
prep.c:476:run_search(): Using directory '/home/srokkala/Labs/P4-curr-y/tests/test-fs'
prep.c:491:run_search(): Absolute path to search directory is '/home/srokkala/Labs/P4-curr-y/tests/test-fs'
prep.c:500:run_search(): Searching '/home/srokkala/Labs/P4-curr-y/tests/test-fs' recursively using 1 threads
prep.c:504:run_search(): Search term: 'the'
prep.c:476:run_search(): Using directory '/home/srokkala/Labs/P4-curr-y/tests/test-fs'
prep.c:491:run_search(): Absolute path to search directory is '/home/srokkala/Labs/P4-curr-y/tests/test-fs'
prep.c:500:run_search(): Searching '/home/srokkala/Labs/P4-curr-y/tests/test-fs' recursively using 2 threads
prep.c:504:run_search(): Search term: 'the'
Speed improvement: 1.53
(Must be >= 1.2)
```

## Test 08: Exact Match [1 pts]

```

run "${TEST_DIR}/../prep" -d "${TEST_DIR}/test-fs" -e \
    HELLO WHATS COOKIN
prep.c:476:run_search(): Using directory '/home/srokkala/Labs/P4-curr-y/tests/test-fs'
prep.c:491:run_search(): Absolute path to search directory is '/home/srokkala/Labs/P4-curr-y/tests/test-fs'
prep.c:500:run_search(): Searching '/home/srokkala/Labs/P4-curr-y/tests/test-fs' recursively using 2 threads
prep.c:504:run_search(): Search term: 'HELLO'
prep.c:504:run_search(): Search term: 'WHATS'
prep.c:504:run_search(): Search term: 'COOKIN'

compare_outputs || test_end

-- diff of outputs shown below --
---------------------------------
 --> OK

run "${TEST_DIR}/../prep" -d "${TEST_DIR}/test-fs" -e HeLLo
prep.c:476:run_search(): Using directory '/home/srokkala/Labs/P4-curr-y/tests/test-fs'
prep.c:491:run_search(): Absolute path to search directory is '/home/srokkala/Labs/P4-curr-y/tests/test-fs'
prep.c:500:run_search(): Searching '/home/srokkala/Labs/P4-curr-y/tests/test-fs' recursively using 2 threads
prep.c:504:run_search(): Search term: 'HeLLo'

compare_outputs

-- diff of outputs shown below --
---------------------------------
 --> OK

test_end
```

## Test 09: Searches the local (non-test) file system for matches. [2 pts]

```

reference_run "${TEST_DIR}/prep.sh" -d /usr/share hippopotamus | sort
Searching for hippopotamus in /usr/share
grep: /usr/share/factory/etc/crypttab: Permission denied
grep: /usr/share/factory/etc/gshadow: Permission denied
grep: /usr/share/factory/etc/shadow: Permission denied
grep: /usr/share/polkit-1/rules.d: Permission denied

run "${TEST_DIR}/../prep" -d /usr/share hippopotamus | sort
prep.c:476:run_search(): Using directory '/usr/share'
prep.c:491:run_search(): Absolute path to search directory is '/usr/share'
prep.c:500:run_search(): Searching '/usr/share' recursively using 2 threads
prep.c:504:run_search(): Search term: 'hippopotamus'
Failed to open file '/usr/share/factory/etc/crypttab'Permission denied
Failed to open file '/usr/share/factory/etc/gshadow'Permission denied
Failed to open file '/usr/share/factory/etc/shadow'Permission denied
Failed to open the directory '/usr/share/polkit-1/rules.d'Permission denied

compare_outputs || test_end

-- diff of outputs shown below --
---------------------------------
 --> OK

reference_run "${TEST_DIR}/prep.sh" -d /usr/share/nano color | sort
Searching for color in /usr/share/nano

run "${TEST_DIR}/../prep" -d /usr/share/nano color | sort
prep.c:476:run_search(): Using directory '/usr/share/nano'
prep.c:491:run_search(): Absolute path to search directory is '/usr/share/nano'
prep.c:500:run_search(): Searching '/usr/share/nano' recursively using 2 threads
prep.c:504:run_search(): Search term: 'color'

compare_outputs

-- diff of outputs shown below --
---------------------------------
 --> OK

test_end
```

## Test 10: Ensures the number of threads is limited properly [2 pts]

```

for i in {1..5}; do
    threads=$(( i + 10 ))
    "${TEST_DIR}/../prep" -d / -t "${threads}" \
        this is a very large test of the project a \
        long time ago in a galaxy far far away &> /dev/null &
    job=${!}

    sleep 1
    detected_threads=$(grep Threads /proc/${job}/status | awk '{print $2}')
    echo "Number of active threads detected: ${detected_threads}"
    (( detected_threads-- )) # To account for the main thread
    kill -9 "${job}"

    if [[ ${threads} -eq ${detected_threads} ]]; then
        test_end 0
    fi
done
Number of active threads detected: 12
```

## Test 11: Static Analysis [1 pts]

Checks for programming and stylistic errors with cppcheck and gcc/clang

```

if ! ( which cppcheck &> /dev/null ); then
    # "cppcheck is not installed. Please install (as root) with:"
    # "pacman -Sy cppcheck"
    test_end 1
fi

cppcheck --enable=warning,performance \
    --error-exitcode=1 \
    "${TEST_DIR}/../prep.c" || test_end 1
Checking /home/srokkala/Labs/P4-curr-y/prep.c ...

cc -Wall -Werror -pthread "${TEST_DIR}"/../{*.c,*.h} || test_end 1

test_end
```

## Test 12: Documentation Check [1 pts]

Ensures documentation is provided for each function and data structure

```

if ! ( which doxygen &> /dev/null ); then
    # "Doxygen is not installed. Please install (as root) with:"
    # "pacman -Sy doxygen"
    test_end 1
fi

# All .c and .h files will be considered; if you'd like to exclude temporary or
# backup files then add a different extension (such as .bak).
for file in $(find . -type f \( -iname "*.c" -o -iname "*.h" \) -not -path "./tests/*"); do
    if ! ( grep '@file' "${file}" &> /dev/null ); then
        echo "@file documentation preamble not found in ${file}"
        file_failed=true
    fi
done

if [[ ${file_failed} == true ]]; then
    # A file didn't have the @file preamble
    test_end 1
fi

doxygen "${TEST_DIR}/Doxyfile" 2>&1 \
    | grep -v '(variable)' \
    | grep -v '(macro definition)' \
    | grep 'is not documented' \
        && test_end 1

test_end 0
```

## Test 13: Memory Leak Check [1 pts]

```

valgrind --leak-check=full \
    "${TEST_DIR}/../prep" -d "${TEST_DIR}/test-fs" -t 50 \
        this is only a test \
    | grep 'are definitely lost'
==2531894== Memcheck, a memory error detector
==2531894== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==2531894== Using Valgrind-3.14.0 and LibVEX; rerun with -h for copyright info
==2531894== Command: /home/srokkala/Labs/P4-curr-y/tests/../prep -d /home/srokkala/Labs/P4-curr-y/tests/test-fs -t 50 this is only a test
==2531894== 
prep.c:476:run_search(): Using directory '/home/srokkala/Labs/P4-curr-y/tests/test-fs'
prep.c:491:run_search(): Absolute path to search directory is '/home/srokkala/Labs/P4-curr-y/tests/test-fs'
prep.c:500:run_search(): Searching '/home/srokkala/Labs/P4-curr-y/tests/test-fs' recursively using 50 threads
prep.c:504:run_search(): Search term: 'this'
prep.c:504:run_search(): Search term: 'is'
prep.c:504:run_search(): Search term: 'only'
prep.c:504:run_search(): Search term: 'a'
prep.c:504:run_search(): Search term: 'test'
==2531894== 
==2531894== HEAP SUMMARY:
==2531894==     in use at exit: 4,792 bytes in 3 blocks
==2531894==   total heap usage: 5,749,015 allocs, 5,749,012 frees, 6,028,803,243 bytes allocated
==2531894== 
==2531894== 272 bytes in 1 blocks are possibly lost in loss record 1 of 3
==2531894==    at 0x483AB65: calloc (vg_replace_malloc.c:752)
==2531894==    by 0x4012AC1: allocate_dtv (in /usr/lib/ld-2.29.so)
==2531894==    by 0x4013431: _dl_allocate_tls (in /usr/lib/ld-2.29.so)
==2531894==    by 0x48621AD: pthread_create@@GLIBC_2.2.5 (in /usr/lib/libpthread-2.29.so)
==2531894==    by 0x10AC38: search_in_file (prep.c:402)
==2531894==    by 0x10AE44: search_in_directory (prep.c:450)
==2531894==    by 0x10ADBA: search_in_directory (prep.c:448)
==2531894==    by 0x10ADBA: search_in_directory (prep.c:448)
==2531894==    by 0x10ADBA: search_in_directory (prep.c:448)
==2531894==    by 0x10ADBA: search_in_directory (prep.c:448)
==2531894==    by 0x10B2A6: run_search (prep.c:505)
==2531894==    by 0x10B55E: main (prep.c:582)
==2531894== 
==2531894== 272 bytes in 1 blocks are possibly lost in loss record 2 of 3
==2531894==    at 0x483AB65: calloc (vg_replace_malloc.c:752)
==2531894==    by 0x4012AC1: allocate_dtv (in /usr/lib/ld-2.29.so)
==2531894==    by 0x4013431: _dl_allocate_tls (in /usr/lib/ld-2.29.so)
==2531894==    by 0x48621AD: pthread_create@@GLIBC_2.2.5 (in /usr/lib/libpthread-2.29.so)
==2531894==    by 0x10AC38: search_in_file (prep.c:402)
==2531894==    by 0x10AE44: search_in_directory (prep.c:450)
==2531894==    by 0x10ADBA: search_in_directory (prep.c:448)
==2531894==    by 0x10ADBA: search_in_directory (prep.c:448)
==2531894==    by 0x10ADBA: search_in_directory (prep.c:448)
==2531894==    by 0x10ADBA: search_in_directory (prep.c:448)
==2531894==    by 0x10ADBA: search_in_directory (prep.c:448)
==2531894==    by 0x10ADBA: search_in_directory (prep.c:448)
==2531894== 
==2531894== LEAK SUMMARY:
==2531894==    definitely lost: 0 bytes in 0 blocks
==2531894==    indirectly lost: 0 bytes in 0 blocks
==2531894==      possibly lost: 544 bytes in 2 blocks
==2531894==    still reachable: 4,248 bytes in 1 blocks
==2531894==         suppressed: 0 bytes in 0 blocks
==2531894== Reachable blocks (those to which a pointer was found) are not shown.
==2531894== To see them, rerun with: --leak-check=full --show-leak-kinds=all
==2531894== 
==2531894== For counts of detected and suppressed errors, rerun with: -v
==2531894== ERROR SUMMARY: 2 errors from 2 contexts (suppressed: 0 from 0)
[[ $? -eq 1 ]]

test_end
```

