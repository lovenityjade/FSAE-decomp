from __future__ import annotations

from pathlib import Path
import subprocess
import tempfile
import unittest

from tools.release import public_audit


class PublicAuditTests(unittest.TestCase):
    def make_repository(self) -> tuple[tempfile.TemporaryDirectory[str], Path]:
        temporary = tempfile.TemporaryDirectory()
        root = Path(temporary.name)
        subprocess.run(["git", "init", "-q", str(root)], check=True)
        return temporary, root

    def test_safe_public_tree_passes(self) -> None:
        temporary, root = self.make_repository()
        self.addCleanup(temporary.cleanup)
        (root / "README.md").write_text("public source\n", encoding="utf-8")
        self.assertEqual(
            public_audit.audit_paths(["README.md"], root, 1024),
            [],
        )

    def test_forbidden_binary_is_rejected(self) -> None:
        temporary, root = self.make_repository()
        self.addCleanup(temporary.cleanup)
        (root / "game.nds").write_bytes(b"private")
        findings = public_audit.audit_paths(["game.nds"], root, 1024)
        self.assertIn("forbidden binary/archive type", {item.reason for item in findings})

    def test_license_file_is_rejected(self) -> None:
        temporary, root = self.make_repository()
        self.addCleanup(temporary.cleanup)
        (root / "license.dat").write_text("proprietary license\n", encoding="utf-8")
        findings = public_audit.audit_paths(["license.dat"], root, 1024)
        self.assertIn("forbidden binary/archive type", {item.reason for item in findings})

    def test_ignored_private_input_is_not_in_worktree_inventory(self) -> None:
        temporary, root = self.make_repository()
        self.addCleanup(temporary.cleanup)
        (root / ".gitignore").write_text("*.nds\n", encoding="utf-8")
        (root / "private.nds").write_bytes(b"private")
        self.assertNotIn("private.nds", public_audit.git_paths(root, "worktree"))

    def test_token_is_rejected(self) -> None:
        temporary, root = self.make_repository()
        self.addCleanup(temporary.cleanup)
        path = root / "bad.txt"
        credential = "access_" + "token = '" + "ghp_" + "abcdefghijklmnopqrstuvwxyz'\n"
        path.write_text(credential, encoding="utf-8")
        findings = public_audit.audit_paths(["bad.txt"], root, 1024)
        self.assertTrue(any("GitHub token" in item.reason for item in findings))

    def test_escaping_symlink_is_rejected(self) -> None:
        temporary, root = self.make_repository()
        self.addCleanup(temporary.cleanup)
        (root / "escape").symlink_to(Path(temporary.name).parent)
        findings = public_audit.audit_paths(["escape"], root, 1024)
        self.assertIn("symlink escapes the repository", {item.reason for item in findings})


if __name__ == "__main__":
    unittest.main()
