#!/usr/bin/env python3
"""Pure, portable tests of 285 classification; no Wine/filesystem mutation."""
from pathlib import Path
import tempfile
import unittest
from check_native_canary_readiness_285 import summarize, candidates


class ReadinessTests285(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='summit285_test_')
        self.addCleanup(self.tmp.cleanup)
        self.client = Path(self.tmp.name) / 'cliente'
        for rel in ('mob/a.smd', 'mob/a.smb', 'fx/b.smd'):
            file = self.client / rel
            file.parent.mkdir(parents=True, exist_ok=True)
            file.write_bytes(b'fake bytes')
        self.rows = {'mob/a.smd': {'source': 'mob/a.ASE', 'sha256': 'good'},
                     'mob/a.smb': {'source': 'mob/a.ASE', 'sha256': 'good'},
                     'fx/b.smd': {'source': 'fx/b.ASE', 'sha256': 'good'}}

    def test_difference_and_missing_offered(self):
        (self.client / 'mob/a.smb').unlink()
        def inspect(c, rel):
            return ((c / rel, False, []) if rel.endswith('.smd') else (None, False, []))
        def strict(c, rel): return c / rel
        count, groups = summarize(self.rows, self.client, inspect, strict, lambda _: 'old')
        self.assertEqual(count['DIFFERENT'], 2)
        self.assertEqual(count['MISSING'], 1)
        self.assertEqual(len(candidates(groups)), 2)

    def test_same_only_never_suggested(self):
        count, groups = summarize(self.rows, self.client,
                                  lambda c, r: (c / r, False, []),
                                  lambda c, r: c / r, lambda _: 'good')
        self.assertEqual(count['SAME'], 3)
        self.assertEqual(candidates(groups), [])

    def test_ambiguous_part_blocks_entire_source(self):
        def inspect(c, rel):
            return (None, False, ['tmABCD', 'tmabcd']) if rel.endswith('.smb') else (c / rel, False, [])
        count, groups = summarize(self.rows, self.client, inspect,
                                  lambda c, r: c / r, lambda _: 'old')
        self.assertEqual(count['AMBIGUOUS'], 1)
        self.assertNotIn('mob/a.ASE', [row[0] for row in candidates(groups)])

    def test_case_variant_is_not_installable(self):
        def inspect(c, rel): return (c / rel, rel.endswith('a.smd'), [])
        count, groups = summarize(self.rows, self.client, inspect,
                                  lambda c, r: c / r, lambda _: 'old')
        self.assertEqual(count['CASE_VARIANT'], 1)
        self.assertNotIn('mob/a.ASE', [row[0] for row in candidates(groups)])

    def test_strict_path_refusal_blocks_source(self):
        def strict(c, rel):
            if rel.endswith('a.smb'): raise ValueError('collision')
            return c / rel
        count, groups = summarize(self.rows, self.client,
                                  lambda c, r: (c / r, False, []), strict, lambda _: 'old')
        self.assertEqual(count['UNSAFE'], 1)
        self.assertNotIn('mob/a.ASE', [row[0] for row in candidates(groups)])

    def test_historical_resolution_inconsistency_blocks_source(self):
        def inspect(c, rel): return ((c / 'elsewhere', False, []) if rel.endswith('.smb') else (c / rel, False, []))
        count, groups = summarize(self.rows, self.client, inspect,
                                  lambda c, r: c / r, lambda _: 'old')
        self.assertEqual(count['UNSAFE'], 1)
        self.assertEqual([x[0] for x in candidates(groups)], ['fx/b.ASE'])

    def test_candidate_order_prefers_changed_and_smaller(self):
        groups = {'many.ASE': [('a.smd', 'DIFFERENT'), ('a.smb', 'MISSING')],
                  'single.ASE': [('b.smd', 'DIFFERENT')],
                  'missing.ASE': [('c.smd', 'MISSING')]}
        self.assertEqual([x[0] for x in candidates(groups)],
                         ['single.ASE', 'many.ASE', 'missing.ASE'])

    def test_empty_group(self):
        self.assertEqual(candidates({}), [])


if __name__ == '__main__':
    unittest.main(verbosity=2)
