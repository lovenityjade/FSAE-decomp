/*
 * Export a bounded, human-readable instruction listing from the persistent
 * analysis database.  This is analysis evidence only; it is never matching
 * source or linker input.
 */
//@category FourSwordAnniversary

import java.io.BufferedWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressSet;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;

public class DumpInstructions extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length != 3) {
            throw new IllegalArgumentException(
                "usage: DumpInstructions.java OUTPUT MIN_ADDRESS MAX_ADDRESS"
            );
        }
        Path output = Path.of(args[0]).toAbsolutePath().normalize();
        Address minimum = toAddr(args[1]);
        Address maximum = toAddr(args[2]);
        if (minimum == null || maximum == null || minimum.compareTo(maximum) > 0) {
            throw new IllegalArgumentException("invalid instruction range");
        }
        Files.createDirectories(output.getParent());

        InstructionIterator instructions = currentProgram.getListing()
            .getInstructions(new AddressSet(minimum, maximum), true);
        try (BufferedWriter writer = Files.newBufferedWriter(output, StandardCharsets.UTF_8)) {
            while (instructions.hasNext() && !monitor.isCancelled()) {
                Instruction instruction = instructions.next();
                writer.write(instruction.getAddress().toString());
                writer.write("  ");
                byte[] bytes = instruction.getBytes();
                for (byte value : bytes) {
                    writer.write(String.format("%02x", value & 0xff));
                }
                writer.write("  ");
                writer.write(instruction.toString());
                writer.newLine();
            }
        }
        println("Instruction listing written to " + output);
    }
}
