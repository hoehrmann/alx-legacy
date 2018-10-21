## Build with Emscripten

```bash
mkdir -p build 
cd build
source "$EMSDK/emsdk_env.sh"
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_TOOLCHAIN_FILE="$EMSCRIPTEN/cmake/Modules/Platform/Emscripten.cmake" \
  ../
make

# standalone result is src/alx.js

( echo '#!/usr/bin/env node' ; 
  echo '// https://github.com/kripken/emscripten/raw/incoming/LICENSE'; 
  curl https://raw.githubusercontent.com/kripken/emscripten/incoming/LICENSE \
    | sed -re 's/^/\/\/ /'; 
  cat src/alx.js ) > alx.js
chmod +x alx.js  

```
