cmd_/mnt/ubuntuShare/CSC3150_Assignment_1/source/program2/program2.ko := ld -r -m elf_x86_64 -z max-page-size=0x200000 -T ./scripts/module-common.lds --build-id  -o /mnt/ubuntuShare/CSC3150_Assignment_1/source/program2/program2.ko /mnt/ubuntuShare/CSC3150_Assignment_1/source/program2/program2.o /mnt/ubuntuShare/CSC3150_Assignment_1/source/program2/program2.mod.o ;  true