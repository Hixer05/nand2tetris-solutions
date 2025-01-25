make
echo ""
echo "Test files:"
echo ""
./VMtranslator test/BasicLoop.vm
./VMtranslator test/BasicTest.vm
./VMtranslator test/FibonacciElement.vm
./VMtranslator test/SimpleAdd.vm
./VMtranslator test/SimpleFunction.vm
./VMtranslator test/StackTest.vm
./VMtranslator test/StaticTest.vm

~/Scaricati/nand2tetris/nand2tetris/tools/CPUEmulator.sh test/BasicLoop.tst
~/Scaricati/nand2tetris/nand2tetris/tools/CPUEmulator.sh test/BasicTest.tst
~/Scaricati/nand2tetris/nand2tetris/tools/CPUEmulator.sh test/FibonacciElement.tst
~/Scaricati/nand2tetris/nand2tetris/tools/CPUEmulator.sh test/SimpleAdd.tst
~/Scaricati/nand2tetris/nand2tetris/tools/CPUEmulator.sh test/SimpleFunction.tst
~/Scaricati/nand2tetris/nand2tetris/tools/CPUEmulator.sh test/StackTest.tst
~/Scaricati/nand2tetris/nand2tetris/tools/CPUEmulator.sh test/StaticTest.tst

echo ""
echo "OS:"
echo ""
./VMtranslator os
echo ""
echo ""
wc -l os.asm
wc -l os.asm >> optimization_stats
