#!/usr/bin/env bash
set -euo pipefail

################################################################################
# CONFIGURAÇÃO
################################################################################

CXX=i686-w64-mingw32-g++
JOBS=${JOBS:-$(nproc)}

ROOT=$(pwd)

BUILD_DIR=build_mingw_server_i686
CPP_LIST=cpp_files_server_i686.txt
OBJ_LIST=object_files_server_mingw.txt
LOG=build_server_i686_compile_errors.log

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

: > "$LOG"

################################################################################
# FLAGS
################################################################################

CXXFLAGS=(
-m32
-std=gnu++14
-fpermissive
-w

-DWIN32
-D_WINDOWS
-D_M_IX86
-Di386
-D__i386__

-I.
)

################################################################################
# DESCOBRE TODOS OS DIRETÓRIOS COM HEADERS
################################################################################

echo "Detectando includes..."

while read d
do
    CXXFLAGS+=("-I$d")
done < <(

find SrcServer SrcGame Shared Shared2 modules Dependency \
-type f \
\( \
-name '*.h' -o \
-name '*.hpp' -o \
-name '*.H' \
\) \
-print0 |

xargs -0 -n1 dirname |

sort -u

)

################################################################################
# DESCOBRE TODOS OS DIRETÓRIOS COM CPP
################################################################################

while read d
do
    CXXFLAGS+=("-I$d")
done < <(

find SrcServer  Shared  modules Dependency \
-type f \
\( \
-name '*.cpp' -o \
-name '*.CPP' -o \
-name '*.c' \
\) \
-print0 |

xargs -0 -n1 dirname |

sort -u

)

################################################################################
# DIRECTX
################################################################################

if [ -d "Dependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Include" ]
then
    CXXFLAGS+=(
"-IDependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Include"
)
fi

################################################################################
# REMOVE DUPLICADOS
################################################################################

mapfile -t CXXFLAGS < <(

printf "%s\n" "${CXXFLAGS[@]}" |

awk '!seen[$0]++'

)

################################################################################
# LISTA DE CPP
################################################################################

find SrcServer/src modules \
-type f \
\( \
-iname '*.cpp' -o \
-iname '*.CPP' -o \
-iname '*.c' \
\) |

grep -v '/lua/lua.c' |

grep -v '/luac/' |

grep -v 'ltests.c' |

sort -u > "$CPP_LIST"

TOTAL=$(wc -l < "$CPP_LIST")

echo
echo "Total CPP: $TOTAL"
echo

################################################################################
# COMPILAÇÃO
################################################################################

# Exporta CXXFLAGS de forma segura para os processos do parallel
export CXXFLAGS_SERIALIZED
CXXFLAGS_SERIALIZED=$(declare -p CXXFLAGS)

export BUILD_DIR
export LOG
export CXX
export CXXFLAGS_SERIALIZED



# Exporta CXXFLAGS de forma segura para os processos do parallel
CXXFLAGS_SERIALIZED=$(declare -p CXXFLAGS)

export BUILD_DIR
export LOG
export CXX
export CXXFLAGS_SERIALIZED
compile_one()
{
    eval "$CXXFLAGS_SERIALIZED"

    src="$1"

    obj="$BUILD_DIR/${src//\//_}"
    obj="${obj%.*}.o"

    "$CXX" \
        "${CXXFLAGS[@]}" \
        -c "$src" \
        -o "$obj" \
        >"$obj.err" 2>&1

    if [ $? -eq 0 ]; then
        rm -f "$obj.err"
        echo "OK $src"
    else
        echo "FAIL $src"

        {
            echo "===== FAIL: $src ====="
            cat "$obj.err"
        } >> "$LOG"

        rm -f "$obj"
    fi
}

export -f compile_one


################################################################################
# PARALLEL
################################################################################

if command -v parallel >/dev/null
then

parallel \
-j"$JOBS" \
--eta \
compile_one \
:::: "$CPP_LIST"

else

while read cpp
do
    compile_one "$cpp"
done < "$CPP_LIST"

fi

################################################################################
# RESULTADO
################################################################################

find "$BUILD_DIR" -name '*.o' |

sort > "$OBJ_LIST"

OK=$(wc -l < "$OBJ_LIST")

FAIL=$((TOTAL-OK))

echo
echo "===================================="
echo "TOTAL : $TOTAL"
echo "OK    : $OK"
echo "FAIL  : $FAIL"
echo "===================================="

if [ "$FAIL" -gt 0 ]
then

echo
echo "Últimos erros:"
echo

grep -Ei "fatal|error:" "$LOG" |

tail -200

fi
