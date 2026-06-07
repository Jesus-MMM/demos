INCDIRS = ./include/
CODEDIRS = ./ ./lib/

CC = gcc
DEPFLAGS = -MP -MD
NOFLAGS = -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding 
CFLAGS = -m32 -Wall -Wextra -Werror -g $(foreach D, $(INCDIRS), -I$(D)) $(DEPFLAGS) $(NOFLAGS)

LDFLAGS = -T link.ld -melf_i386
AS = nasm
ASFLAGS = -f elf32

BUILDDIR = build

CFILES = $(foreach D, $(CODEDIRS), $(wildcard $(D)/*.c))
SFILES = loader.s

OBJECTS = $(addprefix $(BUILDDIR)/, $(CFILES:.c=.o) $(SFILES:.s=.o))
DEPFILES = $(OBJECTS:.o=.d)

$(shell mkdir -p $(BUILDDIR) $(foreach D, $(CODEDIRS), $(BUILDDIR)/$(D)))
$(shell mkdir -p $(BUILDDIR))

-include $(DEPFILES)

.PHONY: all clean runqemu

all: $(BUILDDIR)/kernel.elf
	mkdir -p iso/boot/grub
	cp grub.cfg iso/boot/grub/grub.cfg
	cp $(BUILDDIR)/kernel.elf iso/boot/kernel.elf
	grub-mkrescue -o DemOS.iso iso -d /usr/lib/grub/i386-pc

$(BUILDDIR)/kernel.elf: $(OBJECTS)
	ld $(LDFLAGS) $(OBJECTS) -o $@

$(BUILDDIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILDDIR)/loader.o: loader.s
	mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

runqemu: all
	qemu-system-i386 -cdrom DemOS.iso -serial stdio

cleanrunqemu: clean all
	qemu-system-i386 -cdrom DemOS.iso -serial stdio


clean:
	rm -rf $(BUILDDIR) DemOS.iso iso/