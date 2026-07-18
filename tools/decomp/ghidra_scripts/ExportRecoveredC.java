/*
 * Export a bounded set of Ghidra decompiler results for source-recovery work.
 * The output is evidence in build/, never matching source by itself.
 */
//@category FourSwordAnniversary

import java.io.BufferedWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonArray;
import com.google.gson.JsonObject;

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;

public class ExportRecoveredC extends GhidraScript {
    private static final int DECOMPILE_TIMEOUT_SECONDS = 30;

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length != 4) {
            throw new IllegalArgumentException(
                "usage: ExportRecoveredC.java OUTPUT_C OUTPUT_JSON MIN_ADDRESS MAX_ADDRESS"
            );
        }
        Path cPath = Path.of(args[0]).toAbsolutePath().normalize();
        Path jsonPath = Path.of(args[1]).toAbsolutePath().normalize();
        Address minimum = toAddr(args[2]);
        Address maximum = toAddr(args[3]);
        if (minimum == null || maximum == null || minimum.compareTo(maximum) > 0) {
            throw new IllegalArgumentException("invalid export address range");
        }
        Files.createDirectories(cPath.getParent());
        Files.createDirectories(jsonPath.getParent());

        JsonObject report = new JsonObject();
        report.addProperty("schema_version", 1);
        report.addProperty("program", currentProgram.getName());
        report.addProperty("language", currentProgram.getLanguageID().toString());
        report.addProperty("minimum_address", minimum.toString());
        report.addProperty("maximum_address", maximum.toString());
        report.addProperty("purpose", "analysis-only source recovery; not byte-match evidence");
        JsonArray functions = new JsonArray();
        int selected = 0;
        int completed = 0;

        DecompInterface decompiler = new DecompInterface();
        decompiler.toggleCCode(true);
        decompiler.toggleSyntaxTree(true);
        decompiler.setSimplificationStyle("decompile");
        if (!decompiler.openProgram(currentProgram)) {
            throw new IllegalStateException("cannot open decompiler: " + decompiler.getLastMessage());
        }

        try (BufferedWriter cWriter = Files.newBufferedWriter(cPath, StandardCharsets.UTF_8)) {
            cWriter.write("/*\n");
            cWriter.write(" * Ghidra recovery output. This is not matching source and must be rewritten,\n");
            cWriter.write(" * compiled with the original toolchain, and compared before receiving credit.\n");
            cWriter.write(" */\n\n");
            FunctionIterator iterator = currentProgram.getFunctionManager().getFunctions(true);
            while (iterator.hasNext() && !monitor.isCancelled()) {
                Function function = iterator.next();
                Address entry = function.getEntryPoint();
                if (entry.compareTo(minimum) < 0 || entry.compareTo(maximum) > 0 || function.isExternal()) {
                    continue;
                }
                selected++;
                monitor.setMessage("Decompiling " + function.getName() + " at " + entry);
                DecompileResults result = decompiler.decompileFunction(
                    function, DECOMPILE_TIMEOUT_SECONDS, monitor
                );
                boolean ok = result.decompileCompleted() && result.getDecompiledFunction() != null;
                if (ok) {
                    completed++;
                }

                JsonObject item = new JsonObject();
                item.addProperty("name", function.getName());
                item.addProperty("entry", entry.toString());
                item.addProperty("body_bytes", function.getBody().getNumAddresses());
                item.addProperty("decompiled", ok);
                item.addProperty("message", result.getErrorMessage());
                functions.add(item);

                cWriter.write("/* ================================================================\n");
                cWriter.write(" * " + function.getName() + " @ " + entry + "\n");
                cWriter.write(" * ================================================================ */\n");
                if (ok) {
                    cWriter.write(result.getDecompiledFunction().getC());
                }
                else {
                    cWriter.write("/* decompiler failed: " + result.getErrorMessage() + " */\n");
                }
                cWriter.write("\n\n");
            }
        }
        finally {
            decompiler.dispose();
        }

        report.addProperty("selected_functions", selected);
        report.addProperty("decompiled_functions", completed);
        report.add("functions", functions);
        Gson gson = new GsonBuilder().setPrettyPrinting().create();
        try (BufferedWriter jsonWriter = Files.newBufferedWriter(jsonPath, StandardCharsets.UTF_8)) {
            gson.toJson(report, jsonWriter);
            jsonWriter.write("\n");
        }
        println("Exported " + completed + "/" + selected + " functions to " + cPath);
    }
}
