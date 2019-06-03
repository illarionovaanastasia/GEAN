#### I want to ransform GFF file using sam file, what parameter could I tune?
The -l and -w are important. Simply speaking, if you set them with larger values, the result should be better.

#### When I run gean, I got "Illegal instruction" error
Some AVX2 code was used to speed up gean. If you got this error, this suggest you CPU does not support AVX2 technology.
You could delete the two "-o 3 " in the CMakeLists.txt file , recompile the software on a computer which support AVX2. And if you want to run "annowgr", "transgff" or "spltogff", you should set -l as 0 to disable the ZSDP algorithm.