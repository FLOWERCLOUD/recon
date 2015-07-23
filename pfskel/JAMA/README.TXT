The file jama_eig.h was modified to remove the variable declarations in the for loops 
  that do not compile under Microsoft's Visual C++ 6.0.

Example: for(int i=0; ...) 
         .....
         for(int i=0; ....)

If a variable is declared a second time at the same level this generates a compile error.

So, I removed only those declarations that caused a compile error (maybe a few more that 
caught my eye).
