#!/bin/bash
set -e
cd ~/spaxel

CFLAGS="-m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector"
LDFLAGS="-m32 -T linker.ld -ffreestanding -O2 -nostdlib -fno-pie -no-pie"

echo "[1/3] Компиляция..."
gcc -m32 -c boot.s -o boot.o
gcc $CFLAGS -c kernel.c -o kernel.o
gcc $CFLAGS -c System/fs.c -o fs.o
gcc $CFLAGS -c System/terminal.c -o terminal.o
gcc $CFLAGS -c System/desktop.c -o desktop.o
gcc $CFLAGS -c apps/apps.c -o apps.o
gcc $CFLAGS -c apps/exit.c -o exit.o
gcc $CFLAGS -c apps/calc.c -o calc.o
gcc $CFLAGS -c apps/notes.c -o notes.o
gcc $CFLAGS -c apps/info.c -o info.o
gcc $CFLAGS -c apps/files.c -o files.o
gcc $CFLAGS -c apps/editor.c -o editor.o
gcc $CFLAGS -c apps/games/snake.c -o snake.o
gcc $CFLAGS -c apps/games/parity.c -o parity.o
gcc $CFLAGS -c apps/games/numbers.c -o numbers.o

echo "[2/3] Линковка..."
gcc $LDFLAGS -o spaxel.bin \
    boot.o kernel.o fs.o terminal.o desktop.o \
    apps.o exit.o calc.o notes.o info.o files.o editor.o \
    snake.o parity.o numbers.o -lgcc

echo "[3/3] Сборка ISO..."
mkdir -p isodir/boot/grub
cp spaxel.bin isodir/boot/spaxel.bin
cat > isodir/boot/grub/grub.cfg <<'EOF'
menuentry "S-Paxel 1.0" {
    multiboot /boot/spaxel.bin
    boot
}
EOF
rm -f spaxel.iso
grub-mkrescue -o spaxel.iso isodir 2>/dev/null || \
    echo "grub-mkrescue failed (WSL issue?), ISO not created"

echo "Done."