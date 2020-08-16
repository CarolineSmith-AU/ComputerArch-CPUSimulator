Caroline Smith - cls0097

COMMANDS TO COMPILE AND RUN FILES:
Visual Studio Code: 
>> Use Environment C++(GDB/LLDB). Select configuration g++.exe.
>> Make sure the launch.json file that's generated has "${fileDirname}\\${fileBasenameNoExtension}.exe" as the key for the "program" 
   property and make sure "C:\\MinGW\\bin\\gbd.exe" is the key for the "miDebuggerPath" property.

//--------------------------------------------------------------------------------------------------------------------------

HOW TO USE THESE SIMULATIONS
1. Compile and the .cpp files using directions above.
2. Make sure stackCodeEncoded.txt and accCodeEncoded.txt contain no text.
3. Run the .cpp files using the commands above.
4. Observe output in the DEBUG CONSOLE window.

//--------------------------------------------------------------------------------------------------------------------------

HOW THE SYSTEM WORKS
>> The stackSim.cpp file uses the stack-based implementation to compute the quadratic. It reads from the stackCode.s file 
   for the souorce code and writes the binary encoded instructions to stackCodeEncoded.txt.

>> The accSim.cpp file uses the accumulator-based implementation to compute the quadratic. It reads from the accCode.s file 
   for the souorce code and writes the binary encoded instructions to accCodeEncoded.txt.

>>I used string arrays as my data structures for the stack, data and text memory segments. Each array is connected to a data segment 
  by typedef constants that are set to the base address I assigned to each segment.

>> The way that binary encoded data is stored is in little endian order, so the code I wrote is lengthy to accomodate
   for the level of detail I wanted my memory segments to implement. See the Helper Functions section in my code
   to see the functions I wrote to assist in handling binary encoded data.
   
>> The comments in the code explain exactly what everything does in detail.

>> The output that is printed in the debug console contains information about the various instructions that are being executed 
   as well as the calculations of the program bit and byte size for both implementations.