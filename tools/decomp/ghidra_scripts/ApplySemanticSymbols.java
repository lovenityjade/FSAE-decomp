/*
 * Apply guarded, public semantic names to the ARM9 analysis program.
 * These names improve navigation only and are never matching evidence.
 */
//@category FourSwordAnniversary

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.SourceType;
import ghidra.program.model.symbol.Symbol;

public class ApplySemanticSymbols extends GhidraScript {
    private static final String PURPOSE =
        "analysis-only semantic function names; never matching evidence";
    private static final Set<String> ROOT_FIELDS = Set.of(
        "$schema", "schema_version", "image", "program", "program_sha256",
        "target_image_sha256", "purpose", "symbols"
    );
    private static final Set<String> REQUIRED_SYMBOL_FIELDS = Set.of(
        "address", "name", "expected_names", "confidence", "rationale"
    );
    private static final Set<String> SYMBOL_FIELDS = Set.of(
        "address", "name", "expected_names", "confidence", "rationale",
        "create_if_missing"
    );
    private static final Set<String> CONFIDENCE = Set.of(
        "verified", "descriptive", "provisional"
    );

    private static final class Rename {
        Address address;
        Function function;
        String addressText;
        String oldName;
        String newName;
        String confidence;
        String rationale;
        boolean needsCreate;
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 2 || args.length > 3 ||
            (args.length == 3 && !"--dry-run".equals(args[2]))) {
            throw new IllegalArgumentException(
                "usage: ApplySemanticSymbols.java CONFIG_JSON REPORT_JSON [--dry-run]"
            );
        }
        Path configPath = Path.of(args[0]).toAbsolutePath().normalize();
        Path reportPath = Path.of(args[1]).toAbsolutePath().normalize();
        boolean dryRun = args.length == 3;
        JsonObject config = readObject(configPath);
        validateRoot(config);

        String expectedProgram = string(config, "program");
        String expectedHash = string(config, "program_sha256");
        String actualHash = currentProgram.getExecutableSHA256();
        if (!expectedProgram.equals(currentProgram.getName())) {
            throw new IllegalStateException(
                "program guard failed: expected " + expectedProgram +
                ", opened " + currentProgram.getName()
            );
        }
        if (actualHash == null || !expectedHash.equals(actualHash.toLowerCase())) {
            throw new IllegalStateException(
                "program SHA-256 guard failed: expected " + expectedHash +
                ", opened " + actualHash
            );
        }

        List<Rename> renames = validateAndResolve(config.getAsJsonArray("symbols"));
        int created = 0;
        int renamed = 0;
        int unchanged = 0;
        if (!dryRun) {
            int transaction = currentProgram.startTransaction("Apply ARM9 semantic symbols");
            boolean commit = false;
            try {
                for (Rename rename : renames) {
                    if (rename.needsCreate) {
                        if (currentProgram.getListing().getInstructionAt(rename.address) == null &&
                            !disassemble(rename.address)) {
                            throw new IllegalStateException(
                                "cannot disassemble guarded function at " + rename.addressText
                            );
                        }
                        rename.function = createFunction(rename.address, rename.newName);
                        if (rename.function == null) {
                            throw new IllegalStateException(
                                "cannot create guarded function at " + rename.addressText
                            );
                        }
                        created++;
                    }
                    else if (rename.oldName.equals(rename.newName)) {
                        unchanged++;
                    }
                    else {
                        rename.function.setName(rename.newName, SourceType.USER_DEFINED);
                        renamed++;
                    }
                }
                commit = true;
            }
            finally {
                currentProgram.endTransaction(transaction, commit);
            }
        }
        else {
            for (Rename rename : renames) {
                if (rename.needsCreate) {
                    created++;
                }
                else if (rename.oldName.equals(rename.newName)) {
                    unchanged++;
                }
                else {
                    renamed++;
                }
            }
        }

        JsonObject report = buildReport(
            configPath, renames, dryRun, created, renamed, unchanged, actualHash.toLowerCase()
        );
        Path parent = reportPath.getParent();
        if (parent != null) {
            Files.createDirectories(parent);
        }
        Gson gson = new GsonBuilder().setPrettyPrinting().create();
        try (BufferedWriter writer = Files.newBufferedWriter(
            reportPath, StandardCharsets.UTF_8)) {
            gson.toJson(report, writer);
            writer.write("\n");
        }
        println(
            (dryRun ? "Validated " : "Applied ") + renames.size() +
            " ARM9 semantic name(s): " + created + " created, " +
            renamed + " rename(s), " +
            unchanged + " unchanged"
        );
    }

    private JsonObject readObject(Path path) throws Exception {
        try (BufferedReader reader = Files.newBufferedReader(path, StandardCharsets.UTF_8)) {
            JsonElement value = JsonParser.parseReader(reader);
            if (!value.isJsonObject()) {
                throw new IllegalArgumentException("semantic-symbol config must be an object");
            }
            return value.getAsJsonObject();
        }
    }

    private void validateRoot(JsonObject config) {
        if (!ROOT_FIELDS.containsAll(config.keySet()) ||
            !config.keySet().containsAll(Set.of(
                "schema_version", "image", "program", "program_sha256",
                "target_image_sha256", "purpose", "symbols"
            ))) {
            throw new IllegalArgumentException("semantic-symbol config has invalid root fields");
        }
        if (config.get("schema_version").getAsInt() != 1) {
            throw new IllegalArgumentException("unsupported semantic-symbol schema version");
        }
        if (!"arm9".equals(string(config, "image")) ||
            !PURPOSE.equals(string(config, "purpose"))) {
            throw new IllegalArgumentException("config is not ARM9 analysis-only semantic data");
        }
        if (!config.has("symbols") || !config.get("symbols").isJsonArray()) {
            throw new IllegalArgumentException("symbols must be an array");
        }
    }

    private List<Rename> validateAndResolve(JsonArray symbols) {
        List<Rename> result = new ArrayList<>();
        Set<Address> addresses = new HashSet<>();
        Set<String> names = new HashSet<>();
        for (int index = 0; index < symbols.size(); index++) {
            JsonElement element = symbols.get(index);
            if (!element.isJsonObject()) {
                throw new IllegalArgumentException("symbols[" + index + "] must be an object");
            }
            JsonObject symbol = element.getAsJsonObject();
            if (!SYMBOL_FIELDS.containsAll(symbol.keySet()) ||
                !symbol.keySet().containsAll(REQUIRED_SYMBOL_FIELDS)) {
                throw new IllegalArgumentException("symbols[" + index + "] has invalid fields");
            }
            boolean createIfMissing = false;
            if (symbol.has("create_if_missing")) {
                JsonElement create = symbol.get("create_if_missing");
                if (!create.isJsonPrimitive() ||
                    !create.getAsJsonPrimitive().isBoolean() || !create.getAsBoolean()) {
                    throw new IllegalArgumentException(
                        "symbols[" + index + "].create_if_missing may only be true"
                    );
                }
                createIfMissing = true;
            }
            String addressText = string(symbol, "address");
            String desired = string(symbol, "name");
            String confidence = string(symbol, "confidence");
            String rationale = string(symbol, "rationale");
            if (!addressText.matches("0x[0-9a-fA-F]{8}") ||
                !desired.matches("[A-Za-z_][A-Za-z0-9_]*") ||
                !CONFIDENCE.contains(confidence)) {
                throw new IllegalArgumentException("symbols[" + index + "] has invalid values");
            }
            if (!symbol.get("expected_names").isJsonArray()) {
                throw new IllegalArgumentException(
                    "symbols[" + index + "].expected_names must be an array"
                );
            }
            Set<String> expected = new HashSet<>();
            for (JsonElement name : symbol.getAsJsonArray("expected_names")) {
                if (!name.isJsonPrimitive() || !name.getAsJsonPrimitive().isString() ||
                    !name.getAsString().matches("[A-Za-z_][A-Za-z0-9_]*") ||
                    !expected.add(name.getAsString())) {
                    throw new IllegalArgumentException(
                        "symbols[" + index + "].expected_names is invalid"
                    );
                }
            }
            if (expected.isEmpty() || !expected.contains(desired)) {
                throw new IllegalArgumentException(
                    "symbols[" + index + "].expected_names lacks desired idempotent name"
                );
            }

            Address address = toAddr(addressText);
            Function function = address == null ? null :
                currentProgram.getFunctionManager().getFunctionAt(address);
            boolean needsCreate = function == null;
            String currentName;
            if (needsCreate) {
                if (!createIfMissing) {
                    throw new IllegalStateException("no function starts at " + addressText);
                }
                if (currentProgram.getFunctionManager().getFunctionContaining(address) != null) {
                    throw new IllegalStateException(
                        "guarded function entry overlaps an existing function at " + addressText
                    );
                }
                if (!currentProgram.getMemory().contains(address) ||
                    currentProgram.getMemory().getBlock(address) == null ||
                    !currentProgram.getMemory().getBlock(address).isExecute()) {
                    throw new IllegalStateException(
                        "guarded function entry is not executable memory at " + addressText
                    );
                }
                Symbol primary = currentProgram.getSymbolTable().getPrimarySymbol(address);
                currentName = primary == null ? "NO_SYMBOL" : primary.getName();
            }
            else {
                currentName = function.getName();
            }
            if (!expected.contains(currentName)) {
                throw new IllegalStateException(
                    "name guard failed at " + addressText + ": expected one of " +
                    expected + ", found " + currentName
                );
            }
            if (!addresses.add(address) || !names.add(desired)) {
                throw new IllegalArgumentException("duplicate semantic address or name");
            }

            Rename rename = new Rename();
            rename.address = address;
            rename.function = function;
            rename.addressText = String.format("0x%08x", address.getOffset());
            rename.oldName = currentName;
            rename.newName = desired;
            rename.confidence = confidence;
            rename.rationale = rationale;
            rename.needsCreate = needsCreate;
            result.add(rename);
        }
        return result;
    }

    private JsonObject buildReport(
        Path configPath,
        List<Rename> renames,
        boolean dryRun,
        int created,
        int renamed,
        int unchanged,
        String programHash
    ) throws Exception {
        JsonObject report = new JsonObject();
        report.addProperty("schema_version", 1);
        report.addProperty("status", dryRun ? "dry-run" : "applied");
        report.addProperty("purpose", PURPOSE);
        report.addProperty("program", currentProgram.getName());
        report.addProperty("program_sha256", programHash);
        report.addProperty("config", configPath.toString());
        report.addProperty("config_sha256", sha256(configPath));
        report.addProperty("requested", renames.size());
        report.addProperty("created", created);
        report.addProperty("renamed", renamed);
        report.addProperty("unchanged", unchanged);
        JsonArray entries = new JsonArray();
        for (Rename rename : renames) {
            JsonObject entry = new JsonObject();
            entry.addProperty("address", rename.addressText);
            entry.addProperty("old_name", rename.oldName);
            entry.addProperty("new_name", rename.newName);
            String action = rename.needsCreate ? "create" :
                (rename.oldName.equals(rename.newName) ? "unchanged" : "rename");
            entry.addProperty("action", action);
            entry.addProperty("confidence", rename.confidence);
            entry.addProperty("rationale", rename.rationale);
            entries.add(entry);
        }
        report.add("entries", entries);
        return report;
    }

    private String string(JsonObject value, String field) {
        JsonElement element = value.get(field);
        if (element == null || !element.isJsonPrimitive() ||
            !element.getAsJsonPrimitive().isString() || element.getAsString().isBlank()) {
            throw new IllegalArgumentException(field + " must be a non-empty string");
        }
        return element.getAsString();
    }

    private String sha256(Path path) throws Exception {
        MessageDigest digest = MessageDigest.getInstance("SHA-256");
        try (var stream = Files.newInputStream(path)) {
            byte[] buffer = new byte[1024 * 1024];
            int count;
            while ((count = stream.read(buffer)) >= 0) {
                digest.update(buffer, 0, count);
            }
        }
        StringBuilder result = new StringBuilder();
        for (byte value : digest.digest()) {
            result.append(String.format("%02x", value & 0xff));
        }
        return result.toString();
    }
}
