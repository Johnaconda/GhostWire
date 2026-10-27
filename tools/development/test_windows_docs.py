#!/usr/bin/env python3
"""No-execution regression for safer PowerShell invocation documentation."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]


class WindowsDocumentationTests(unittest.TestCase):
    def test_active_build_guide_no_unconditional_bypass(self):
        s = (ROOT/'BUILD.md').read_text(encoding='utf-8')
        self.assertNotIn('-ExecutionPolicy Bypass -File', s)
        self.assertIn(r'.\tools\windows\dev-windows.ps1', s)

    def test_checkpoint_markdown_invocation_examples(self):
        docs = list((ROOT/'docs/checkpoints').glob('*.md'))
        self.assertTrue(docs)
        for p in docs:
            with self.subTest(path=str(p)):
                content = p.read_text(encoding='utf-8')
                self.assertNotIn('-ExecutionPolicy Bypass -File', content)

    def test_no_defender_exclusions_or_encoded_script_in_guides(self):
        for p in [ROOT/'BUILD.md', *(ROOT/'docs/checkpoints').glob('*.md')]:
            s = p.read_text(encoding='utf-8')
            with self.subTest(path=str(p)):
                self.assertNotIn('Add-MpPreference -Exclusion', s)
                self.assertNotIn('Set-MpPreference -Disable', s)
                self.assertNotIn(' -EncodedCommand ', s)


if __name__ == '__main__':
    unittest.main()
